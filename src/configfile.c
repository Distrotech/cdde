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
	2008/08/25, Stanislav Maslovski:
	    Fixed segfaults when properties "path" or "command" are not defined.
	2008/08/31:
	    Adapted for simplified lists.
	2008/09/01:
	    Simplify saveconfig().
*/


#include <syslog.h>

#include "configfile.h"
#include "cdde.h"

#include <errno.h>
extern int errno;

// read a configuration file
// returns nonzero on error
int loadconfig(char * filename)
{
	xmlDocPtr doc = NULL;
	xmlNodePtr cur = NULL;
	char * temp = NULL;
	
	// parse the document
	doc = xmlParseFile(filename);
	if (doc == NULL)
	{
		syslog(LOG_ERR, "Error: Could not parse document");
		xmlFreeDoc(doc);
		return 1;
	}

	// get the root node
	cur = xmlDocGetRootElement(doc);
	if (cur == NULL)
	{
		syslog(LOG_ERR, "Error: Could not parse document: unable to find root element");
		xmlFreeDoc(doc);
		return 1;
	}

	// make sure the root is a "cdde"
	if (xmlStrcmp(cur->name, (const xmlChar *) "cdde"))
	{
		syslog(LOG_ERR, "Error: Could not parse document: root element is not \"cdde\"");
		xmlFreeDoc(doc);
		return 1;
	}

	// find the delay
	temp = xmlGetProp(cur, (const xmlChar *) "delay");
	if (temp == NULL)
	{
		syslog(LOG_WARNING, "Warning: Did not find polling delay in config file, using %d", DEFAULT_SLEEP_N);
		delay = DEFAULT_SLEEP_N;
	} else {
		errno = 0;
		delay = strtol(temp, NULL, 10);
		if (errno != 0)
		{
			syslog(LOG_WARNING, "Warning: Delay had under/overflow, using %d", DEFAULT_SLEEP_N);
			delay = DEFAULT_SLEEP_N;
		}
		if (delay < 0)
		{
			syslog(LOG_WARNING, "Warning: Delay can't be negative, using %d", DEFAULT_SLEEP_N);
			delay = DEFAULT_SLEEP_N;
		}
		
		xmlFree(temp);
	}
	if (verbose) syslog(LOG_INFO, "Using polling delay of %d microseconds", delay);
	
	// find all of the children
	cur = cur->xmlChildrenNode;
	while (cur != NULL)
	{
		if (!xmlStrcmp(cur->name, (const xmlChar *) "drive"))
		{
			parsedrive(cur);
		}
		cur = cur->next;
	}
	xmlFreeDoc(doc);
	
	return 0;
}

// parse the <drive> nodes
// and their children
void parsedrive(xmlNodePtr cur)
{
	int i;
	drive * d;
	xmlChar * data;
	list * tail[NUM_DATA_TYPES];
	
	data = xmlGetProp(cur, "path");
	if (!data)
	{
		syslog(LOG_WARNING,
			"Warning: no \"path\" property defined for drive, skipping.");
		return;
	}

	d = malloc(sizeof(drive));
	if (!d)
	{
		syslog(LOG_ERR, "Error: Couldn't allocate memory for drive.");
		exit(1);
	}

	d->filename = data;
	for (i=0; i<NUM_DATA_TYPES; i++)
		tail[i] = d->commands[i] = NULL;

	d->dontexecute = dontrunfirst;
	d->mediachange = 1;
	
	// search it's children for commands, and add them
	cur = cur->xmlChildrenNode;
	while (cur != NULL)
	{
		for (i=0; i<NUM_DATA_TYPES; i++)
		{
			if (!xmlStrcmp(cur->name, (const xmlChar *) datatags[i]))
			{
				if (data = xmlGetProp(cur, "command"))
				{
					tail[i] = list_push(d->commands[i], tail[i], data);
					if (tail[i] == NULL)
					{
						syslog(LOG_ERR, "Error: Couldn't allocate memory");
						exit(1);
					}
					if (d->commands[i] == NULL)
						d->commands[i] = tail[i];
				}
				break;
			}
		}
		cur = cur->next;
	}
	drives = list_push(drives, NULL, d);
	if (drives == NULL)
	{
		syslog(LOG_ERR, "Error: Couldn't allocate memory");
		exit(1);
	}
}

// write a template configuation file
// returns zero on success
int saveconfig(char * filename)
{
	xmlDocPtr doc;
	xmlNodePtr cur;
	xmlNodePtr root;
	xmlNodePtr subroot;
	int i, retval = 0;
	
	// create the root
	root = xmlNewNode(NULL, (xmlChar *) "cdde");
	xmlSetProp(root, (xmlChar *) "delay", (xmlChar *) DEFAULT_SLEEP_S);

	// add a drive
	// FIXME: try to guess cdrom device paths by looking at fstab
	subroot = xmlNewNode(NULL, (xmlChar *) "drive");
	xmlSetProp(subroot, (xmlChar *) "path", (xmlChar *) "/dev/cdrom");
	xmlAddChild(root, subroot);

	// add commands
	for (i = 0; i < NUM_DATA_TYPES; ++i)
	{
		char buf[32];
		cur = xmlNewNode(NULL, (xmlChar *) datatags[i]);
		sprintf(buf, "xmessage %s disc was inserted.", datatags[i]);
		xmlSetProp(cur, (xmlChar *) "command", (xmlChar *) buf);
		xmlAddChild(subroot, cur);
	}

	// create the document
	doc = xmlNewDoc("1.0");
	xmlDocSetRootElement(doc, root);

	// write out the xml document
	if (xmlSaveFormatFile (filename, doc, 1) == -1) retval = -1;

	// free memory
	xmlFreeDoc(doc);
	
	return retval;
}
