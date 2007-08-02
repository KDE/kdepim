/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef KABC_RESOURCEGROUPWISE_H
#define KABC_RESOURCEGROUPWISE_H

#include "soap/groupwiseserver.h"

#include <kabcresourcecached.h>
#include <kdepimmacros.h>

#include <libkdepim/progressmanager.h>

#include <kio/job.h>

class KConfig;

class GroupwiseServer;

namespace KABC {

class GroupwisePrefs;

class KDE_EXPORT ResourceGroupwise : public ResourceCached
{
  friend class ResourceGroupwiseConfig;

  Q_OBJECT

  public:
    ResourceGroupwise( const KConfig * );
    ResourceGroupwise( const KURL &url,
                       const QString &user, const QString &password,
                       const QStringList &readAddressBooks,
                       const QString &writeAddressBook );
    ~ResourceGroupwise();

    void readConfig( const KConfig * );
    void writeConfig( KConfig * );

    void readAddressBooks();
    void writeAddressBooks();

    void retrieveAddressBooks();

    GroupwisePrefs *prefs() const { return mPrefs; }

    GroupWise::AddressBook::List addressBooks() const { return mAddressBooks; }

    bool doOpen();
    void doClose();

    Ticket *requestSaveTicket();
    void releaseSaveTicket( Ticket* );

    bool load();
    bool asyncLoad();
    bool save( Ticket * );
    bool asyncSave( Ticket * );
    enum SABState { Error, Stale, InSync, RefreshNeeded };

    /**
     * Clears the cached data, in memory and on disk
     */
    void clearCache();
  protected:
    enum ResourceState { Start, FetchingSAB, SABUptodate, FetchingUAB, Uptodate };
    enum BookType { System, User };
    enum AccessMode { Fetch, Update };
    void init();
    void initGroupwise();

    /* STATE CHANGING METHODS */
    /**
    * Begin asynchronously fetching the system address book , replacing the cached copy
    */
    void fetchAddressBooks( const BookType booktype );
    /**
    *  Asynchronously update the system address book
    */
    void updateSystemAddressBook();
    /**
    * Wrap up the load sequence
    */
    void loadCompleted();

    /** HELPER METHODS **/
    /**
    * Check to see if a local download of the SAB already exists
    */
    SABState systemAddressBookState();
    /**
    * Check if the resource is configured to download the SAB
    */
    bool shouldFetchSystemAddressBook();
    /**
    * Check if the resource is configured to download personal address
    * books
    */
    bool shouldFetchUserAddressBooks();

    /**
    * Create a URL for a single addressbook access.
    * To fetch an address book completely, use mode = Fetch
    * To just update an addressbook, use mode = Update and give the ast sequence number already held
    * If Update is given without a sequence number, the mode falls back to Fetch
    */
    KURL createAccessUrl( BookType bookType, AccessMode mode, unsigned long lastSequenceNumber = 0, unsigned long lastPORebuildTime = 0 );

    /**
    * Persist the last known delta info.  Call after the SAB is up to date.
    */
    void storeDeltaInfo();

    /**
    * Check if the application which has loaded this resource is whitelisted
    * to load the System Address Book (time-consuming)
    */
    bool appIsWhiteListedForSAB();

  private slots:
    /** STATE CHANGING SLOTS **/
    void fetchSABResult( KIO::Job * );
    void fetchUABResult( KIO::Job * );
    void updateSABResult( KIO::Job * );
    /** DATA PROCESSING SLOTS **/
    void slotReadJobData( KIO::Job *, const QByteArray & );
    void slotUpdateJobData( KIO::Job *, const QByteArray & );
    /** HELPER SLOT **/
    void slotJobPercent( KIO::Job *job, unsigned long percent );

    void cancelLoad();
  private:
    GroupwisePrefs *mPrefs;
    GroupWise::AddressBook::List mAddressBooks;

    GroupwiseServer *mServer;

    KIO::TransferJob *mJob;
    KPIM::ProgressItem *mProgress;
    KPIM::ProgressItem *mSABProgress;
    KPIM::ProgressItem *mUABProgress;
    QString mJobData;
    ResourceState mState;
    unsigned long mServerFirstSequence, mServerLastSequence, mServerLastPORebuildTime;

    bool mLimitedMode;
};

}

#endif
