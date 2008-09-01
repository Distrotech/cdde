/*
Compact Disc Detect & Execute

Copyright(C) 2002-04 Eric Lathrop <eric@ericlathrop.com>

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

For more details see the file COPYING
*/

#ifndef __cdde_h__
#define __cdde_h__

#define NUM_DATA_TYPES 8
#define DATA_AUDIO 0
#define DATA_DATA 1
#define DATA_DVD 2
#define DATA_MIXED 3
#define DATA_BLANK 4
#define DATA_VCD 5
#define DATA_SVCD 6
#define DATA_UNKNOWN 7

#define DEFAULT_SLEEP_N 5000000
#define DEFAULT_SLEEP_S "5000000"


///////////////////////
// globals
///////////////////////
extern char * datatags[];
extern char * filename; // the config file filename
extern int numchildren; // the number of children we have executed
extern int delay; // the polling delay
#include "list.h"
extern list * drives;


///////////////////////////
// command line options
///////////////////////////
extern int verbose; // -v
extern int batchmode; // -b
extern int dontrunfirst; // -r


///////////////////////
// structs
///////////////////////
typedef struct _drive
{
	char * filename;
	list * commands[NUM_DATA_TYPES];
	int cmdtype;
	int dontexecute;
	int mediachange;
} drive;


///////////////////////
// function prototypes
///////////////////////

// signal handler to find out when to re-read config file
void sighup(int signum);

// executes all commands for a related disc type
void executeall(drive * d);

// determine if the device has a directory in its top level
//
// device is the filename of the special device file
// directory is the name of the directory to look for in the discs top directory
//
// returns nonzero if the file exists
// otherwise returns 0
int disc_has_dir(const char * device, const char * directory);

// see if a drive has a disc in it
// if so, determine type of disc
// and execute proper command
int checkdrive(drive * d);

// remove a drive from the list
list * removedrive(list * entry);

#endif
