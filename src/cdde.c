/*
Compact Disc Detect & Execute

Copyright(C) 2002-04 Eric Lathrop <eric@ericlathrop.com>
Copyright(C) 2008, Stanislav Maslovski <stanislav.maslovski@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

For more details see the file COPYING.

Changes:
	2008/08/22, Stanislav Maslovski:
	    Use strdup() instead of substr().
	2008/08/24:
	    Add support for big-endian machines.
*/


// regular headers
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h> 
#include <limits.h>
#include <linux/cdrom.h>
#include <fstab.h>
#include <signal.h>
#include <errno.h>
#include <syslog.h>

// my headers
#include "config.h"
#include "cdde.h"
#include "string.h"
#include "configfile.h"
#include "util.h"

///////////////////////
// globals
///////////////////////
char * datatags[] = { "audio", "data", "dvd", "mixed", "blank", "vcd", "svcd" };
extern int errno; // for error checking
char * filename = NULL; // the config file filename
int keeprunning = 1; // keep checking drives
int numchildren = 0; // the number of children we have executed
int delay; // the polling delay
list * drives = NULL;

///////////////////////////
// command line options and their defaults
///////////////////////////
int verbose = 0; // -v
char * configfile = NULL; // -c
int batchmode = 0; // -b
int dontrunfirst = 1; // -r


// signal handler to find out when to re-read config file
void sighup(int signum)
{
	struct stat fileinfo;

	syslog(LOG_INFO, "Caught SIGHUP, re-reading config file.");
	
	// free old stuff
	while (list_size(drives) > 0)
	{
		removedrive(0);
	}
	
	// make sure the config file is there
	if (stat(filename, &fileinfo) == -1)
	{
		switch (errno)
		{
			case ENOENT:
				syslog(LOG_ERR, "Error: Config file %s does not exist!", filename);
				if (saveconfig(filename))
				{
					syslog(LOG_ERR, "Error: Couldn't create template file.");
				} else {
					syslog(LOG_INFO, "I created a template configuration file located at %s", filename);
					syslog(LOG_INFO, "I suggest you edit it to make sure the values are useful before you run cdde again.");
				}
				break;
			case EACCES:
				syslog(LOG_ERR, "Error: Can't read from config file %s", filename);
				break;
		}
		
		exit (1);
	}
	
	// read the config file
	if (loadconfig(filename))
	{
		syslog(LOG_ERR, "Error: Can't read config file %s", filename);
		exit(1);
	}

	// make sure there's a drive to check
	if (list_size(drives) == 0)
	{
		syslog(LOG_WARNING, "Warning: No drives to poll!");
	}

	// re-install the signal handler
	signal(SIGHUP, sighup);
}


// executes all commands for a related disc type
void executeall(drive * d, int cmdtype)
{
	int i;
	char * command;
	
	// get fstab info on where the device's mount point is
	struct fstab * cdfs = getfsspec(d->filename);
	char * mountpoint;
	if (cdfs == NULL)
	{
		syslog(LOG_ERR, "Error: couldn't get %s entry for %s", _PATH_FSTAB, d->filename);
		return;
	} else {
		mountpoint = strdup(cdfs->fs_file);
	}
	
	for (i=list_size(d->commands[cmdtype])-1; i>=0; i--)
	{
		command = replace(list_nth(d->commands[cmdtype], i), "%dev%", d->filename);
		command = replace_destruct(&command, "%mnt%", mountpoint);
		execute(command);
		free(command);
	}
}


// read a both-endian field of the specified width
// and advance the file pointer respectively
void read_bothendian(int fd, void * var, int width)
{
#ifdef WORDS_BIGENDIAN
	lseek(fd, width, SEEK_CUR);
#endif
	read(fd, var, width);
#ifndef WORDS_BIGENDIAN
	lseek(fd, width, SEEK_CUR);
#endif
}


// determine if the device has a directory in its top level
//
// device is the filename of the special device file
// directory is the name of the directory to look for in the discs top directory
//
// returns nonzero if the file exists
// otherwise returns 0
int disc_has_dir(const char * device, const char * directory)
{
	int ret = 0; // return value
	int fd = 0; // file descriptor for drive
	unsigned short bs; // the discs block size
	unsigned int ts; // the path table size
	unsigned int tl; // the path table location (in blocks)
	int i;
	unsigned char len_di = 0; // length of the directory name in current path table entry
	unsigned short parent = 0; // the number of the parent directory's path table entry
	char * dirname = NULL; // filename for the current path table entry
	int pos = 0; // our position into the path table
	int curr_record = 1; // the path table record we're on
	
	// open the drive
	fd = open(device, O_RDONLY | O_NONBLOCK);
	if (fd == -1)
	{
		syslog(LOG_ERR, "Error: Failed to open device %s", device);
		return 0;
	}

	// read the block size
	// (stored as both-endian word)
	lseek(fd, 0x8080, SEEK_CUR);
	read_bothendian(fd, &bs, 2);

	// read in size of path table
	// (stored as both-endian double word)
	read_bothendian(fd, &ts, 4);
	
#ifdef WORDS_BIGENDIAN
	// skip little-endian table data
	lseek(fd, 8, SEEK_CUR);
#endif
	// read in which block path table is in
	// (stored as both-endian double word)
	read_bothendian(fd, &tl, 4);
	
	// seek to the path table (either little-endian or big-endian one)
	lseek(fd, ((int)(bs) * tl), SEEK_SET);
	
	// loop through the path table entries
	while (pos < ts)
	{
		// the table data are in correct endianness
		// get the length of the filename of the current entry
		read(fd, &len_di, 1);

		// get the record number of this entry's parent
		// i'm pretty sure that the 1st entry is always the top directory
		lseek(fd, 5, SEEK_CUR);		
		read(fd, &parent, 2);
		
		// allocate and zero a string for the filename
		dirname = calloc(len_di+1, 1);
		
		// read the name
		read(fd, dirname, len_di);
		
		// if we found a folder that has the root as a parent, and the directory name matches
		// then return success
		if ((parent == 1) && (strcasecmp(dirname, directory) == 0))
		{
			ret = 1;
			free(dirname);
			break;
		}
		
		free(dirname);
	
		// all path table entries are padded to be even, so if this is an odd-length table, seek a byte to fix it
		if (len_di%2 == 1)
		{
			lseek(fd, 1, SEEK_CUR);
			pos++;
		}
		
		// update our position
		pos += 8+len_di;
		curr_record++;
	}
		
	close(fd);
	return ret;
}


// see if a drive has a disc in it
// if so, determine type of disc
// and execute proper command
//
// returns 0 on ok and nonzero on error
int checkdrive(drive * d)
{
	int fd;
	int status;
	struct cdrom_tochdr th;
	
	// open the device
	fd = open(d->filename, O_RDONLY | O_NONBLOCK);
	if (fd < 0)
	{
		syslog(LOG_ERR, "Error: Couldn't open %s: ", d->filename);
		return -1;
	}

	// read the drive status info
	status = ioctl(fd, CDROM_DRIVE_STATUS, CDSL_CURRENT);
	
	switch (status)
	{
		// if there's a ok disc in there
		case CDS_DISC_OK:
			if (d->dontexecute)
			{
				// release the device
				close(fd);
				break;
			}

			d->dontexecute = 1;

			// see if we can read the disc's table of contents (TOC).
			status = ioctl(fd, CDROMREADTOCHDR, &th);
			if (status != 0)
			{
				// release the device
				close(fd);

				if (verbose) syslog(LOG_INFO, "Can't read disc TOC. The disc is either a blank, or has a broken TOC.");
				executeall(d, DATA_BLANK);
				break;
			}

			// read disc status info
			status = ioctl(fd, CDROM_DISC_STATUS, CDSL_CURRENT);

			// release the device
			close(fd);

			switch (status)
			{
				case CDS_AUDIO:
					// found a audio cd
					if (verbose) syslog(LOG_INFO, "Detected an audio cd!\n");
					executeall(d, DATA_AUDIO);
					break;
				case CDS_DATA_1:
				case CDS_DATA_2:
					// found a data cd
					if (disc_has_dir(d->filename, "video_ts") != 0)
					{
						if (verbose) syslog(LOG_INFO, "Detected a DVD");
						executeall(d, DATA_DVD);
					} else if (disc_has_dir(d->filename, "vcd") != 0) {
						if (verbose) syslog(LOG_INFO, "Detected a VCD");
						executeall(d, DATA_VCD);
					} else if (disc_has_dir(d->filename, "svcd") != 0) {
						if (verbose) syslog(LOG_INFO, "Detected a SVCD");
						executeall(d, DATA_SVCD);
					} else {
						if (verbose) syslog(LOG_INFO, "Detected a data CD");
						executeall(d, DATA_DATA);
					}
					break;
				case CDS_MIXED:
					// found a mixed cd
					if (verbose) syslog(LOG_INFO, "Detected a mixed audio/data cd!");
					executeall(d, DATA_MIXED);
					break;
				default:
					if (verbose) syslog(LOG_INFO, "Could not determine disc type: Doing nothing!\n");
					break;
			}
			break;
		case CDS_NO_INFO:
			// drive doesnt support querying, so this program will never work on that drive.
			syslog(LOG_WARNING, "Warning: %s does not support status queries.\n", filename);
		default:
			// release the device
			d->dontexecute = 0;
			close(fd);
	}
	return 0;
}


// remove a drive from the list
void removedrive(unsigned int drivenum)
{
	xmlChar * data;
	drive * d;
	int i;
	
	drives = list_removenth(drives, (void **)&d, drivenum);
	
	for (i=0; i<NUM_DATA_TYPES; i++)
	{
		while (list_size(d->commands[i]) > 0)
		{
			d->commands[i] = list_pop(d->commands[i], (void **)&data);
			xmlFree(data);
		}
	}
	free(d);
}


// the program's starting point
int main(int argc, char ** argv)
{
	struct stat fileinfo;
	int i=0;

	parseargs(argc, argv);
	
	if (filename == NULL)
	{
		// create string representing where the config file is
		filename = (char *) strdup(getenv("HOME"));
		filename = realloc(filename, ((strlen(filename)+1) + strlen("/.cdde.xml")) * sizeof(char));
		filename = (char *) strcat(filename, "/.cdde.xml");
	}

	// make sure the config file is there
	if (stat(filename, &fileinfo) == -1)
	{
		switch (errno)
		{
			case ENOENT:
				fprintf(stderr, "Error: Config file %s does not exist!\n", filename);
				if (saveconfig(filename))
				{
					fprintf(stderr, "Error: Couldn't create template file.\n");
				} else {
					printf("I created a template configuration file located at %s\n", filename);
					printf("I suggest you edit it to make sure the values are useful before you run cdde again.\n");
				}
				break;
			case EACCES:
				fprintf(stderr, "Error: Can't read from config file %s!\n", filename);
				break;
		}
		return 0;
	}

	if (!batchmode) daemonize(); // don't call printf after this point (we don't have a terminal anymore)
	syslog(LOG_INFO, "CDDE v%s started", VERSION);
	
	// install signal handlera
	signal(SIGHUP, sighup);

	// read the config file
	if (loadconfig(filename) != 0)
	{
		syslog(LOG_ERR, "Error: Unable to load config file %s", filename);
		exit(1);
	}

	// make sure there's a drive to check
	if (list_size(drives) == 0)
	{
		syslog(LOG_WARNING, "Warning: No drives to poll");
	}
	
	// pause for "delay" microseconds and check the drive forever
	while (keeprunning)
	{
		for (i=0; i<list_size(drives); i++)
		{
			if (checkdrive((drive *)list_nth(drives, i)) != 0)
			{
				removedrive(i);
				i--;
			}
		}
		if (batchmode) break;
		usleep(delay);
	}

	// cleanup time!
	while (list_size(drives) > 0)
	{
		removedrive(0);
	}
	free(filename);
	
	return 0;
}
