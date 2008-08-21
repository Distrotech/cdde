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


#include <stdlib.h>
#include <string.h>

#include "string.h"

// find the first occurance of a character
// in a string (from start position)
//
int indexof(const char * str, char c, int start)
{
	int i;

	if (start < 0) start = 0;

	for (i=start; i<strlen(str); i++)
	{
		if (str[i] == c) return i;
	}
	return -1;
}

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

// creates a copy of a string but without
// leading or trailing whitespace;
// (note: user must free() the returned string when done)
//
char * trim(const char * str)
{
	int start = 0;
	int stop = strlen(str);

	while ((
			(str[start] == ' ') ||
			(str[start] == '\t') ||
			(str[start] == '\n')
		) && (start < strlen(str))) start++;
	while ((
			(str[stop-1] == ' ') ||
			(str[stop-1] == '\t') ||
			(str[stop-1] == '\n')
		) && (stop > -1)) stop--;

	return substr(str, start, stop-start);
}

// creates a copy of a string but without
// leading or trailing whitespace;
// (note: user must free() the returned string when done)
//
// this is different from trim() in that it destroys
// the original string, and replaces it with the trim()ed version
//
char * trim_destruct(char ** str)
{
	char * newstring = trim(*str);
	free(*str);
	*str = newstring;
	return newstring;
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

