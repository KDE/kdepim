/* *******************************************************
   KPilot - Hot-Sync Software for Unix.
   Copyright 1998 by Dan Pilone
   This code is released under the GNU PUBLIC LICENSE.
   Please see the file 'COPYING' included in the KPilot
   distribution.
   *******************************************************
 */

#ifndef __PILOT_USER_H
#define __PILOT_USER_H

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pi-dlp.h>

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

#endif
