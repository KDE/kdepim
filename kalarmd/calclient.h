/*
    Calendar and client access for KDE Alarm Daemon.

    This file is part of the KDE alarm daemon.
    Copyright (c) 2001 David Jarvie <software@astrojar.org.uk>
    Based on the original, (c) 1998, 1999 Preston Brown

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef _CALCLIENT_H
#define _CALCLIENT_H

#include <qstrlist.h>

#include <libkcal/calendarlocal.h>

#include "adcalendarbase.h"

using namespace KCal;

class ADCalendar;     // this class must be derived from ADCalendarBase


// Alarm Daemon client which receives calendar events
struct ClientInfo
{
    enum NotificationType       // how to notify client about events if client isn't running
    {
      NO_START_NOTIFY     = 0,  // don't start client if it isn't running
      DCOP_NOTIFY         = 1,  // start client and use DCOP to notify about events
      COMMAND_LINE_NOTIFY = 2   // start client and use command line arguments to notify about events
    };
    ClientInfo() { }
    ClientInfo(const QString &appName, const QString& title,
               const QString& dcopObj, int notifyType, bool disp,
               bool wait=false);

    void             setNotificationType(int type);

    QString          appName;
    QString          title;             // application title for display purposes
    QString          dcopObject;        // object to receive DCOP messages (if applicable)
    NotificationType notificationType;  // whether and how to notify events if client app isn't running
    bool             displayCalName;    // true to display calendar name in tooltip
    // Data which is not used by all alarm daemon applications
    bool     waitForRegistration; // don't notify any events until client has registered
    int      menuIndex;           // context menu index to this client's entry
};

//typedef QMap<QString, ClientInfo> ClientMap;   // maps client names against client data

typedef QValueList<ClientInfo> ClientList;

#endif
