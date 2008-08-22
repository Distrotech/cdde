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


#include <signal.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h> 
#include <stdio.h>

#include "config.h"
#include "util.h"
#include "cdde.h"


// signal handler to find out when out child has exited
void sigchld(int signum)
{
	int status;
	wait(&status);

	// if there are still children waiting 
	// re-install the signal handler
	numchildren--;
	if (numchildren > 0)
	{
		// re-install the signal handler
		signal(SIGCHLD, sigchld);
	}
}


// fork and exec a command
// wait for it to exit before returning
void execute(const char * command)
{
	char * fixed_str;
	char ** args;
	int num_args;
	int curr_arg;
	int temp_len;
	int found_start;
	int pos;
	
	// exit if nothing to execute;
	if (command == "") return;

	numchildren++;
	signal(SIGCHLD, sigchld);

	if (fork() == 0)
	{
		// im the child
		// i get to execute the command

		// trim whitespace from the command
		fixed_str = strdup(command);
		fixed_str = (char *) trim_destruct(&fixed_str);

		// find the number of arguments
		num_args = 0;
		found_start = 0;
		for (pos=0; pos<strlen(fixed_str); pos++)
		{
			if ((!found_start) && (fixed_str[pos] != ' '))
			{
				found_start = 1;
				num_args++;
			}

			if ((found_start) && (fixed_str[pos] == ' '))
			{
				found_start = 0;
			}
		}

		// create the argument array
		args = malloc((num_args + 1) * sizeof(char *));
		found_start = 0;
		curr_arg = 0;
		temp_len = strlen(fixed_str);
		for (pos=0; pos<temp_len; pos++)
		{
			if ((!found_start) && (fixed_str[pos] != ' '))
			{
				found_start = 1;
				args[curr_arg] = &fixed_str[pos];
				curr_arg++;
			}

			if ((found_start) && (fixed_str[pos] == ' '))
			{
				found_start = 0;
				fixed_str[pos] = '\0';
			}
		}
		args[curr_arg] = NULL;

		// call execvp
		execvp(args[0], args);
		
		// should never get here
		syslog(LOG_ERR, "Error: Command execution failed");
		exit(2);
	}

	// i'm the parent
	// i already have a signal handler to tell me when a child dies
	// so I can just get on with my business
}


// parse command line arguments
void parseargs(int argc, char ** argv)
{
	int c;
	int longindex = 0;
	
	struct option long_options[] =
	{
		{ "help", 0, NULL, '?' },
		{ "version", 0, NULL, 'V' },
		{ "verbose", 0, NULL, 'v' },
		{ "config", 1, NULL, 'c' },
		{ "run", 0, NULL, 'r' },
		{ "batch", 0, NULL, 'b' },
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, "c:Vvhrb", long_options, &longindex)) != -1)
	{
		switch (c)
		{
			case 'c':
				filename = (char *) substr(optarg, 0, strlen(optarg));
				break;
			case 'V':
				printf("%s\n", VERSION);
				exit(0);
				break;
			case 'v':
				verbose = 1;
				break;
			case 'r':
				dontrunfirst = 0;
				break;
			case 'b':
				dontrunfirst = 0;
				batchmode = 1;
				break;
			case 'h':
			case '?':
			default:
				printf("Usage: %s [-c filename] [-rbvVh]\n", PACKAGE);
				printf("Run commands when discs are inserted into CD/DVD drives.\n\n");

				printf(" -r\t--run\t\tRun commands on disc already in drive when is started.\n");
				printf(" -b\t--batch\t\tBatch mode: check all drives once then exit.\n");
				printf("\t\t\t(implies -r)\n");
				printf(" -c\t--config\tUse an alternate config file. (Default is ~/.cdde.xml)\n");
				printf(" -V\t--version\tPrint out version information.\n");
				printf(" -v\t--verbose\tPrint out lots of information.\n");
				printf(" -h\t--help\t\tPrint this help message.\n");
				exit(0);
		}
	}
}

// Muwahahaha! Turn into a daemon!
// code derived from "Advanced Programming in the Unix Environment"
// by W. Richard Stevens
void daemonize()
{
	pid_t childpid = fork();
	if (childpid < 0)
	{
		syslog(LOG_ERR, "Error: fork() failed");
		exit(1);
	} else if (childpid > 0) {
		_exit(0);
	}

	if (setsid() == -1)
	{
		syslog(LOG_ERR, "Error: setsid() failed");
		exit(1);
	}

	if (chdir("/") < 0)
	{
		syslog(LOG_ERR, "Error: chdir() failed");
		exit(1);
	}

	umask(0);

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
}

