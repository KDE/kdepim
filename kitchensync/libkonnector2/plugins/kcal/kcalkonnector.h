/*
    This file is part of KitchenSync.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

#ifndef KSYNC_KCALKONNECTOR_H
#define KSYNC_KCALKONNECTOR_H

#include <libkcal/calendarresources.h>
#include <libkcal/resourcecalendar.h>

#include <konnector.h>

namespace KSync {

class KCalKonnectorConfig;

class KCalKonnector : public KSync::Konnector
{
  Q_OBJECT

  public:
    KCalKonnector( const KConfig *config );
    ~KCalKonnector();

    void writeConfig( KConfig * );

    SynceeList syncees() { return mSyncees; }

    bool readSyncees();
    bool writeSyncees();

    bool connectDevice();
    bool disconnectDevice();

    KSync::KonnectorInfo info() const;

    void setCurrentResource( const QString &identifier ) { mResourceIdentifier = identifier; }
    QString currentResource() const { return mResourceIdentifier; }

  protected slots:
    void loadingFinished();
    void savingFinished();

  private:
    KCal::ResourceCalendar* createResource( const QString &identifier );

    KCalKonnectorConfig *mConfigWidget;
    QString mResourceIdentifier;
    QString mMd5sum;

    KCal::CalendarResources *mCalendar;
    KCal::ResourceCalendar *mResource;

    KSync::CalendarSyncee *mCalendarSyncee;

    SynceeList mSyncees;
};

}

#endif
