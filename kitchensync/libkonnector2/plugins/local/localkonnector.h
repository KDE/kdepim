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
#ifndef KSYNC_LOCALKONNECTOR_H
#define KSYNC_LOCALKONNECTOR_H

#include "libkcal/calendarlocal.h"

#include <konnectorplugin.h>

#include <qiconset.h>
#include <qptrlist.h>

namespace KSync {

class LocalKonnectorConfig;

class LocalKonnector : public KSync::Konnector
{ 
    Q_OBJECT
  public:
    /**
     * @param parent the Parent Object
     * @param name the name
     * @param strlist a QStringList which is not used but neccessary for KGenericFactory
     */
    LocalKonnector( QObject*, const char*, const QStringList = QStringList() );
    ~LocalKonnector();

    /** return our capabilities() */
    KSync::Kapabilities capabilities();

    /**
     * the user configured this konnector
     * apply his preferecnes
     */
    void setCapabilities( const KSync::Kapabilities& );

    bool readSyncees();
    bool writeSyncees();

    bool startBackup(const QString& path );
    bool startRestore( const QString& path );

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
    LocalKonnectorConfig *mConfigWidget;
    QString mCalendarFile;
    QString mAddressBookFile;

    KCal::CalendarLocal mCalendar;
};

}


#endif
