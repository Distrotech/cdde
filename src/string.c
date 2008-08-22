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
	    Got rid of substr(), replace() rewritten.
*/

#include <stdlib.h>
#include <string.h>

#include "string.h"

// replace all  occourances of oldstr with newstr in str
// (note: user must free() the returned string when done)
//
char * replace(const char * str, const char * oldstr, const char * newstr)
{
	int len, oldlen, newlen, cnt;
	char *buf, *dst;
	const char *left, *right;

	len = strlen(str);
	oldlen = strlen(oldstr);
	if (len == 0 || oldlen == 0)
		return strdup(str);

	newlen = strlen(newstr);
	buf = malloc(len + (len*newlen)/oldlen + 1);

	dst = buf; left = str;
	while (right = strstr(left, oldstr))
	{
		cnt = right - left;
		memcpy(dst, left, cnt);
		dst += cnt;
		memcpy(dst, newstr, newlen);
	    	dst += newlen;
		left = right + oldlen;
	}

	strcpy(dst, left);
	buf = realloc(buf, strlen(buf) + 1);
	return buf;
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

