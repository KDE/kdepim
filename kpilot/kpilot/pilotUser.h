/* pilotUser.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** See the .cc file for an explanation of what this file is for.
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
#ifndef _KPILOT_PILOTUSER_H
#define _KPILOT_PILOTUSER_H

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
    
    PilotUser* operator&() { return &fUser; }
      
    /**
     * Ensures the names are properly terminated.  Needed incase we
     * are syncing a new and bogus pilot.
     */
    void boundsCheck() { fUser.username[127] = 0; fUser.password[127] = 0; }

    const char* getUserName() const     { return fUser.username; }
    void setUserName(const char* name)  { strcpy(fUser.username, name); }
    
    const int getPasswordLength() const { return fUser.passwordLength; }
    const char* getPassword() const     { return fUser.password; }
    void setPassword(char* password) { strcpy(fUser.password, password); fUser.passwordLength = strlen(password); }

    unsigned long getUserID() const     { return fUser.userID; }
    unsigned long getViewerID() const   { return fUser.viewerID; }

    unsigned long getLastSyncPC() const { return fUser.lastSyncPC; }
    void setLastSyncPC(unsigned long pc) { fUser.lastSyncPC = pc; }
    
    time_t getLastSuccessfulSyncDate() { return fUser.successfulSyncDate; }
    void setLastSuccessfulSyncDate(time_t when) { fUser.successfulSyncDate = when; }

    time_t getLastSyncDate()           { return fUser.lastSyncDate; }
    void setLastSyncDate(time_t when) { fUser.lastSyncDate = when; }

    private:
    struct PilotUser fUser;
    };

#else
#warning "File doubly included"
#endif


// $Log$
// Revision 1.4  2001/02/06 08:05:20  adridg
// Fixed copyright notices, added CVS log, added surrounding #ifdefs. No code changes.
//
