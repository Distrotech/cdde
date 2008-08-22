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
	    Removed indexof(), trim() and trim_destruct() functions.
*/


#include <stdlib.h>
#include <string.h>

#include "string.h"

// create a smaller string
// from part of a larger string
// (note: user must free() the returned string when done)
//
char * substr(const char * str, unsigned int start, unsigned int length)
{
	char * result = "";
	int i;
	int stop = start+length;

	if ((start >= 0) && (start <= stop) && (stop <= strlen(str)))
	{
		result = malloc((stop-start+1)*sizeof(char));
		if (result == NULL)
		{
			return NULL;
		}
		for (i=0; i<(stop-start); i++)
		{
			result[i] = str[start+i];
		}
		result[i] = '\0';
	}
	return result;
}

// replace all  occourances of oldstr with newstr in str
// (note: user must free() the returned string when done)
//
char * replace(const char * str, const char * oldstr, const char * newstr)
{
	char * pos;
	char * start;
	char * end;
	char * retval = substr(str, 0, strlen(str));
	char * temp;

	while ((pos = (char *)strstr(retval, oldstr)) != NULL)
	{
		start = substr(retval, 0, pos-retval);
		end = substr(pos, strlen(oldstr), strlen(pos)-strlen(oldstr));

		temp = malloc(sizeof(char) * (strlen(start) + strlen(newstr) + strlen(end) + 1));
		if (temp == NULL)
		{
			free(start);
			free(end);
			return NULL;
		}
		temp[0] = '\0';
		temp = (char *)strcat(temp, start);
		temp = (char *)strcat(temp, newstr);
		temp = (char *)strcat(temp, end);
		
		free(retval);
		retval = temp;
		free(start);
		free(end);
	}
	return retval;
}

// replace all occourances of oldstr with newstr in str
// (note: user must free() the returned string when done)
//
// this is different from replace() in that it destroys
// the original string, and replaces it with the replace()ed version
//
char * replace_destruct(char ** str, const char * oldstr, const char * newstr)
{
	char * newstring = replace(*str, oldstr, newstr);
	free(*str);
	*str = newstring;
	return newstring;
}

