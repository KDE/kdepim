#ifndef _KPILOT_PILOTUSER_H
#define _KPILOT_PILOTUSER_H
/* pilotUser.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** Wrapper for the PilotUser struct from pilot-link, which describes
** the user-data set in the Pilot.
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <pi-dlp.h>

#include "config.h"

class KPilotUser
{
public:
	KPilotUser() { ::memset(&fUser,0,sizeof(struct PilotUser)); }
	KPilotUser(const PilotUser* user) { fUser = *user; }

	PilotUser *pilotUser() { return &fUser; }

	/**
	* Ensures the names are properly terminated.  Needed incase we
	* are syncing a new and bogus pilot.
	*/
	void boundsCheck()
	{
	}

	const char* getUserName() const     { return fUser.username; }
	void setUserName(const char* name)
	{
		strlcpy(fUser.username, name,sizeof(fUser.username));
		boundsCheck();
	}

	const int getPasswordLength() const { return fUser.passwordLength; }
	const char* getPassword() const     { return fUser.password; }
	void setPassword(char* password)
	{
		memset(&fUser.password, 0, sizeof(fUser.password));
		strlcpy(fUser.password, password,sizeof(fUser.password));
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

#endif
