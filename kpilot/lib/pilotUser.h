#ifndef _KPILOT_PILOTUSER_H
#define _KPILOT_PILOTUSER_H
/* pilotUser.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** See the .cc file for an explanation of what this file is for.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef _PILOT_DLP_H_
#include <pi-dlp.h>
#endif

class KPilotUser
{
public:
	KPilotUser() { }
	KPilotUser(const PilotUser* user) { fUser = *user; }

	PilotUser *pilotUser() { return &fUser; }

	/**
	* Ensures the names are properly terminated.  Needed incase we
	* are syncing a new and bogus pilot.
	*/
	void boundsCheck() 
	{ 
		fUser.username[sizeof(fUser.username)-1] = 0; 
		fUser.password[sizeof(fUser.password)-1] = 0; 
	}

	const char* getUserName() const     { return fUser.username; }
	void setUserName(const char* name)  
	{ 
		strncpy(fUser.username, name,sizeof(fUser.username)-1); 
		boundsCheck();
	}
    
	const int getPasswordLength() const { return fUser.passwordLength; }
	const char* getPassword() const     { return fUser.password; }
	void setPassword(char* password) 
	{ 
		strncpy(fUser.password, password,sizeof(fUser.password)-1); 
		boundsCheck();
		fUser.passwordLength = strlen(fUser.password); 
	}

	unsigned long getUserID() const     { return fUser.userID; }
	unsigned long getViewerID() const   { return fUser.viewerID; }

	unsigned long getLastSyncPC() const { return fUser.lastSyncPC; }
	void setLastSyncPC(unsigned long pc) { fUser.lastSyncPC = pc; }

	time_t getLastSuccessfulSyncDate() { return fUser.successfulSyncDate; }
	void setLastSuccessfulSyncDate(time_t when) 
		{ fUser.successfulSyncDate = when; }

	time_t getLastSyncDate()           { return fUser.lastSyncDate; }
	void setLastSyncDate(time_t when) { fUser.lastSyncDate = when; }

private:
	struct PilotUser fUser;
};



// $Log$
// Revision 1.1  2001/10/08 21:56:02  adridg
// Start of making a separate KPilot lib
//
// Revision 1.9  2001/09/29 16:26:18  adridg
// The big layout change
//
// Revision 1.8  2001/09/24 22:24:32  adridg
// Squashed buffer overflows
//
// Revision 1.7  2001/09/05 22:15:34  adridg
// Operator & is just *too* weird
//
// Revision 1.6  2001/04/16 13:48:35  adridg
// --enable-final cleanup and #warning reduction
//
// Revision 1.5  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.4  2001/02/06 08:05:20  adridg
// Fixed copyright notices, added CVS log, added surrounding #ifdefs. No code changes.
//
#endif
