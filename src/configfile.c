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
	drive  * d = malloc(sizeof(drive));
	xmlChar * data;
	
	if (d == NULL)
	{
		syslog(LOG_ERR, "Error: Couldn't allocate memory for drive");
		exit(1);
	}
	for (i=0; i<NUM_DATA_TYPES; i++)
	{
		d->commands[i] = NULL;
	}
	d->dontexecute = dontrunfirst;
	
	d->filename = xmlGetProp(cur, "path");
	
	// search it's children for commands, and add them
	cur = cur->xmlChildrenNode;
	while (cur != NULL)
	{
		for (i=0; i<NUM_DATA_TYPES; i++)
		{
			if (!xmlStrcmp(cur->name, (const xmlChar *) datatags[i]))
			{
				data = xmlGetProp(cur, "command");
				d->commands[i] = list_push(d->commands[i], data);
				if (d->commands[i] == NULL)
				{
					syslog(LOG_ERR, "Error: Couldn't allocate memory");
					exit(1);
				}
				
				break;
			}
		}
		cur = cur->next;
	}
	drives = list_push(drives, d);
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
	int retval = 0;
	
	// create the root
	root = xmlNewNode(NULL, (xmlChar *) "cdde");
	xmlSetProp(root, (xmlChar *) "delay", (xmlChar *) DEFAULT_SLEEP_S);

	// add a drive
	// FIXME: try to guess cdrom device paths by looking at fstab
	subroot = xmlNewNode(NULL, (xmlChar *) "drive");
	xmlSetProp(subroot, (xmlChar *) "path", (xmlChar *) "/dev/cdrom");
	xmlAddChild(root, subroot);

	// add commands
	cur = xmlNewNode(NULL, (xmlChar *) "audio");
	xmlSetProp(cur, (xmlChar *) "command", (xmlChar *) "echo A audio cd was inserted.");
	xmlAddChild(subroot, cur);

	// add commands
	cur = xmlNewNode(NULL, (xmlChar *) "data");
	xmlSetProp(cur, (xmlChar *) "command", (xmlChar *) "echo A data cd was inserted.");
	xmlAddChild(subroot, cur);

	// add commands
	cur = xmlNewNode(NULL, (xmlChar *) "dvd");
	xmlSetProp(cur, (xmlChar *) "command", (xmlChar *) "echo A dvd was inserted.");
	xmlAddChild(subroot, cur);

	// add commands
	cur = xmlNewNode(NULL, (xmlChar *) "vcd");
	xmlSetProp(cur, (xmlChar *) "command", (xmlChar *) "echo A vcd was inserted.");
	xmlAddChild(subroot, cur);

	// add commands
	cur = xmlNewNode(NULL, (xmlChar *) "svcd");
	xmlSetProp(cur, (xmlChar *) "command", (xmlChar *) "echo A svcd was inserted.");
	xmlAddChild(subroot, cur);

	// add commands
	cur = xmlNewNode(NULL, (xmlChar *) "blank");
	xmlSetProp(cur, (xmlChar *) "command", (xmlChar *) "echo A blank cdr was inserted.");
	xmlAddChild(subroot, cur);

	// add commands
	cur = xmlNewNode(NULL, (xmlChar *) "mixed");
	xmlSetProp(cur, (xmlChar *) "command", (xmlChar *) "echo A mixed (audio/data) cd was inserted.");
	xmlAddChild(subroot, cur);

	// create the document
	doc = xmlNewDoc("1.0");
	xmlDocSetRootElement(doc, root);

	// write out the xml document
	if (xmlSaveFormatFile (filename, doc, 1) == -1) retval = -1;

	// free memory
	xmlFreeDoc(doc);
	
	return retval;
}
