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
	2008/08/31, Stanislav Maslovski:
	    Make list bidirectional, major rewrite.
*/


#ifndef __list_h__
#define __list_h__

typedef struct _list
{
	struct _list * next;
	struct _list * prev;
	void * data;
} list;

/* list abstract interface */
#define list_get(item) ((item)->data)
#define list_next(item) ((item)->next)
#define list_prev(item) ((item)->prev)

list * list_push(list * l, list * item, void * data);
list * list_pop(list * l, list * item, void ** data);

#endif
