/*
    This file is part of KitchenSync.

    Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>

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

#include <libkcal/calendarlocal.h>
#include <kabc/addressbook.h>
#include <kbookmarkmanager.h>

#include <konnector.h>

#include <qiconset.h>
#include <qptrlist.h>

namespace KABC {
class ResourceFile;
}

namespace KSync {

class LocalKonnectorConfig;

class LocalKonnector : public KSync::Konnector
{ 
    Q_OBJECT
  public:
    LocalKonnector( const KConfig *config );
    ~LocalKonnector();

    void writeConfig( KConfig * );

    /** return our capabilities() */
    KSync::Kapabilities capabilities();

    /**
     * the user configured this konnector
     * apply his preferecnes
     */
    void setCapabilities( const KSync::Kapabilities& );

    SynceeList syncees() { return mSyncees; }

    bool readSyncees();
    bool writeSyncees();

    bool connectDevice();
    bool disconnectDevice();

    /** the state and some informations */
    KSync::KonnectorInfo info() const;

    /** download a resource/url/foobar */
    void download( const QString& );

    void setCalendarFile( const QString &f ) { mCalendarFile = f; }
    QString calendarFile() const { return mCalendarFile; }
    
    void setAddressBookFile( const QString &f ) { mAddressBookFile = f; }
    QString addressBookFile() const { return mAddressBookFile; }

    void setUseStdAddressBook( bool value ) { mUseStdAddressBook = value; }
    bool useStdAddressBook() const { return mUseStdAddressBook; }

    void setBookmarkFile( const QString &f ) { mBookmarkFile = f; }
    QString bookmarkFile() const { return mBookmarkFile; }

  protected slots:
    void loadingAddressBookFinished();
    void checkLoaded();

  private:
    LocalKonnectorConfig *mConfigWidget;
    QString mCalendarFile;
    QString mAddressBookFile;
    QString mBookmarkFile;

    KCal::CalendarLocal mCalendar;
    KABC::AddressBook mAddressBook;
    bool mUseStdAddressBook;
    bool mAddressBookLoaded;
    bool mCalendarLoaded;
    KABC::ResourceFile *mAddressBookResourceFile;
    
    KSync::AddressBookSyncee *mAddressBookSyncee;
    KSync::CalendarSyncee *mCalendarSyncee;
    
    class LocalBookmarkManager : public KBookmarkManager
    {
      public:
        LocalBookmarkManager() : KBookmarkManager() {}
    };
    LocalBookmarkManager mBookmarkManager;
    
    SynceeList mSyncees;
};

}

#endif
