/*
    This file is part of KitchenSync.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KSYNC_REMOTEKONNECTOR_H
#define KSYNC_REMOTEKONNECTOR_H

#include <libkcal/calendarlocal.h>
#include <kabc/addressbook.h>
#include <kbookmarkmanager.h>
#include <kio/job.h>

#include <konnector.h>

#include <qiconset.h>
#include <qptrlist.h>

namespace KABC {
class ResourceFile;
}

namespace KSync {

class RemoteKonnectorConfig;

class RemoteKonnector : public KSync::Konnector
{
    Q_OBJECT
  public:
    RemoteKonnector( const KConfig *config );
    ~RemoteKonnector();

    void writeConfig( KConfig * );

    SynceeList syncees() { return mSyncees; }

    bool readSyncees();
    bool writeSyncees();

    bool connectDevice();
    bool disconnectDevice();

    /** the state and some informations */
    KSync::KonnectorInfo info() const;

    void setCalendarUrl( const QString &f ) { mCalendarUrl = f; }
    QString calendarUrl() const { return mCalendarUrl; }

    void setAddressBookUrl( const QString &f ) { mAddressBookUrl = f; }
    QString addressBookUrl() const { return mAddressBookUrl; }

    void setBookmarkUrl( const QString &f ) { mBookmarkUrl = f; }
    QString bookmarkUrl() const { return mBookmarkUrl; }

  protected:
    void finishRead();
    void finishWrite();

  protected slots:
    void slotCalendarData( KIO::Job *, const QByteArray &d );
    void slotCalendarReadResult( KIO::Job *job );
    void slotAddressBookData( KIO::Job *, const QByteArray &d );
    void slotAddressBookReadResult( KIO::Job *job );
    void slotCalendarDataReq( KIO::Job *, QByteArray &d );
    void slotCalendarWriteResult( KIO::Job *job );
    void slotAddressBookDataReq( KIO::Job *, QByteArray &d );
    void slotAddressBookWriteResult( KIO::Job *job );

  private:
    RemoteKonnectorConfig *mConfigWidget;
    QString mCalendarUrl;
    QString mAddressBookUrl;
    QString mBookmarkUrl;

    QString mMd5sumCal;
    QString mMd5sumBkm;
    QString mMd5sumAbk;

    KCal::CalendarLocal mCalendar;
    KABC::AddressBook mAddressBook;

    KSync::AddressBookSyncee *mAddressBookSyncee;
    KSync::CalendarSyncee *mCalendarSyncee;

    class LocalBookmarkManager : public KBookmarkManager
    {
      public:
        LocalBookmarkManager() : KBookmarkManager() {}
    };
    LocalBookmarkManager mBookmarkManager;

    SynceeList mSyncees;

    int mSynceeReadCount;
    int mSynceeWriteCount;

    QString mCalendarData;
    QString mAddressBookData;
};

}

#endif
