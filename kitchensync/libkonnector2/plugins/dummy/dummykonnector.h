/*
    This file is part of KitchenSync.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KSYNC_DUMMYKONNECTOR_H
#define KSYNC_DUMMYKONNECTOR_H

#include <qiconset.h>
#include <qptrlist.h>

#include <libkcal/calendarlocal.h>

#include <konnector.h>

namespace KSync {
class CalendarSyncee;

/**
 * This plugin gets loaded by the KonnectorManager
 * this is the key to the KonnectorWorld
 * we need to implement the interface to fully support it...
 */
class DummyKonnector : public KSync::Konnector
{
    Q_OBJECT
  public:
    DummyKonnector( const KConfig *config );
    ~DummyKonnector();

    SynceeList syncees();

    bool readSyncees();
    bool writeSyncees();

    bool connectDevice();
    bool disconnectDevice();

    /** the state and some informations */
    KSync::KonnectorInfo info()const;

  private:
    KCal::CalendarLocal mCalendar;

    SynceeList mSyncees;
    CalendarSyncee *mCalSyncee;
};

}


#endif
