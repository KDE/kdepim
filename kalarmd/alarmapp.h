/*
    This file is part of the KDE alarm daemon.
    Copyright (c) 1997-1999 Preston Brown
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2001 David Jarvie <software@astrojar.org.uk>

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
#ifndef _ALARMAPP_H
#define _ALARMAPP_H
// $Id$

#include <kuniqueapp.h>

class AlarmDaemon;

class AlarmApp : public KUniqueApplication
{
    Q_OBJECT
  public:
    AlarmApp();
    virtual ~AlarmApp();

    int  newInstance();
    bool startedAtLogin() const   { return mStartedAtLogin; }

  private:
    AlarmDaemon* mAd;
    bool         mStartedAtLogin;
};

#endif
