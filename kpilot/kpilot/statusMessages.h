/* statusMessages.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This file defines the status messages that can (or should) be
** passed through the link between condutis and the daemon.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/



#ifndef _KPILOT_STATUSMESSAGES_H
#define _KPILOT_STATUSMESSAGES_H

#include <unistd.h>

#define KPILOTLINK_PORT  10159

class CStatusMessages
{
public:
	typedef enum LinkMessages {
		CONDUIT_READY=1,
		SYNC_STARTING,
		SYNC_COMPLETED,
		SYNCING_DATABASE,
		RECORD_MODIFIED,
		RECORD_DELETED,
		FILE_INSTALL_REQUEST, // Used between kpilotDaemon and kpilot
		NEW_RECORD_ID,
		WRITE_RECORD,
		NEXT_MODIFIED_REC,
		NEXT_REC_IN_CAT,
		READ_REC_BY_INDEX,
		READ_REC_BY_ID,
		NO_SUCH_RECORD,
		REC_DATA,
		LOG_MESSAGE,
		READ_APP_INFO,
		LAST_MESSAGE
	} ;

	static int write(int fd,LinkMessages m)
	{
		int i = m;
		return ::write(fd,&i,sizeof(int));
	}
};


#else
#warning "File doubly included"
#endif


// $Log$
// Revision 1.7  2001/03/02 16:59:35  adridg
// Added new protocol message READ_APP_INFO for conduit->daemon communication
//
// Revision 1.6  2001/02/06 08:05:20  adridg
// Fixed copyright notices, added CVS log, added surrounding #ifdefs. No code changes.
//
