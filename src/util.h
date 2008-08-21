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


#ifndef __util_h__
#define __util_h__

// signal handler to find out when out child has exited
void sigchld(int signum);

// fork and exec a command
// wait for it to exit before returning
void execute(const char * command);

// parse command line arguments
void parseargs(int argc, char ** argv);

// Muwahahaha! Turn into a daemon!
void daemonize();

#endif
