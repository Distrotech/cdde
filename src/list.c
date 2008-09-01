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

#include "list.h"
#include <stdlib.h>

/*
   push data after the given list item
   or at the list top if item == NULL.
   returns: the newly allocated item.
*/
list * list_push(list * l, list * item, void * data)
{
	list * temp = malloc(sizeof(list));

	if (temp == NULL)
		return NULL;

	temp->data = data;
	temp->prev = item;

	if (item != NULL)
	{
		if (item->next != NULL)
			item->next->prev = temp;
		temp->next = item->next;
		item->next = temp;
		return temp;
	}

	if (l != NULL)
		l->prev = temp;

	temp->next = l;
	return temp;
}


/*
   pop the given item off the list, store a pointer to
   its data in data and free() the item.
   returns: the new list top.
*/ 
list * list_pop(list * l, list * item, void ** data)
{
	*data = item->data;

	if (item->next != NULL)
		item->next->prev = item->prev;

	if (item->prev != NULL)
		item->prev->next = item->next;
	else
		l = item->next;

	free(item);
	return l;
}
