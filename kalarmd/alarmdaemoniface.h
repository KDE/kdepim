/*
    This file is part of the KDE alarm daemon.
    Copyright (c) 1997-1999 Preston Brown
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2001 David Jarvie <software@astrojar.org.uk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef ALARMDAEMONIFACE_H
#define ALARMDAEMONIFACE_H
// $Id$

#include <dcopobject.h>

class AlarmDaemonIface : virtual public DCOPObject
{
    K_DCOP
  k_dcop:
    virtual void addCal(const QString& appname, const QString& urlString) = 0;
    virtual void addMsgCal(const QString& appname, const QString& urlString) = 0;
    virtual void reloadCal(const QString& appname, const QString& urlString) = 0;
    virtual void reloadMsgCal(const QString& appname, const QString& urlString) = 0;
    virtual void removeCal(const QString& urlString) = 0;
    virtual void resetMsgCal(const QString& appname, const QString& urlString) = 0;
    virtual void registerApp(const QString& appName, const QString& appTitle,
                             const QString& dcopObject, bool commandLineNotify,
                             bool displayCalendarName) = 0;
    virtual void quit() = 0;
};

#endif
