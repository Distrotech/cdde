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


#include "list.h"
#include <stdlib.h>

// push data onto the top of the list
//
// returns: the new list
//
list * list_push(list * l, void * data)
{
	list * temp = malloc(sizeof(list));
	if (temp == NULL)
	{
		return NULL;
	}
	temp->data = data;
	temp->next = l;
	return temp;
}

// pop the top item off the list,
// store a pointer to its data in data
// and free() the item
//
// returns: the new list
//
list * list_pop(list * l, void ** data)
{
	list * temp = NULL;
	if (l == NULL)
	{
		*data = NULL;
		return NULL;
	}
	temp = l->next;
	*data = l->data;
	l->data = NULL;
	l->next = NULL;
	free(l);
	return temp;
}

// returns the nth item of the list
// store a pointer to its data in data
// and free the item
//
// returns: the new list
//
list * list_removenth(list * l, void ** data, int n)
{
	int i=0;
	list * temp;
	list * holder;
	if (l == NULL)
	{
		*data = NULL;
		return NULL;
	}
	if (n < 0)
	{
		*data = NULL;
		return l;
	}
	
	if (n == 0)
	{
		temp = l->next;
		*data = l->data;
		l->next = NULL;
		l->data = NULL;
		free(l);
		return temp;
	}
	temp = l;
	for (i=0; i<n-1; i++)
	{
		temp = temp->next;
		if (temp == NULL)
		{
			*data = NULL;
			return l;
		}
	}
	holder = temp->next;
	temp->next = temp->next->next;
	*data = holder->data;
	holder->next = NULL;
	holder->data = NULL;
	free(holder);
	return l;
}

// get data from nth list element
void * list_nth(list * l, int n)
{
	int i;
	if (n < 0) return NULL;
	for (i=0; i<n; i++)
	{
		if (l == NULL) return NULL;
		l = l->next;
	}
	return l->data;
}

// get size of list
int list_size(list * l)
{
	int i=0;
	while (l != NULL)
	{
		l = l->next;
		i++;
	}
	return i;
}
