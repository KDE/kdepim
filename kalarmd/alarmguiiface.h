/*
    This file is part of the KDE alarm daemon GUI.
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
#ifndef DAEMONGUIIFACE_H
#define DAEMONGUIIFACE_H
// $Id$

#include <dcopobject.h>

class AlarmGuiIface : virtual public DCOPObject
{
    K_DCOP
  k_dcop:
    virtual ASYNC alarmDaemonUpdate(const QString& change,
                                    const QString& calendarURL,
                                    const QCString& appName) = 0;
    virtual ASYNC handleEvent(const QString& calendarURL,
                              const QString& eventID) = 0;
};

#endif
