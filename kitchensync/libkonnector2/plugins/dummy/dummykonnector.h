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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
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

    /** return our capabilities() */
    KSync::Kapabilities capabilities();

    /**
     * the user configured this konnector
     * apply his preferecnes
     */
    void setCapabilities( const KSync::Kapabilities& );

    SynceeList syncees();

    bool readSyncees();
    bool writeSyncees();

    bool connectDevice();
    bool disconnectDevice();

    /** the state and some informations */
    KSync::KonnectorInfo info()const;

    /** download a resource/url/foobar */
    void download( const QString& );

    /** configuration widgets */
    KSync::ConfigWidget* configWidget( const KSync::Kapabilities&, QWidget* parent, const char* name );
    KSync::ConfigWidget* configWidget( QWidget* parent, const char* name );

  private:
    KCal::CalendarLocal mCalendar;

    SynceeList mSyncees;
    CalendarSyncee *mCalSyncee;
};

}


#endif
