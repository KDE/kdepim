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

#ifndef KSYNC_KABCKONNECTOR_H
#define KSYNC_KABCKONNECTOR_H

#include <kabc/addressbook.h>

#include <konnector.h>

namespace KABC {
class ResourceFile;
}

namespace KSync {

class KABCKonnectorConfig;

class KABCKonnector : public KSync::Konnector
{ 
  Q_OBJECT

  public:
    KABCKonnector( const KConfig *config );
    ~KABCKonnector();

    void writeConfig( KConfig * );

    SynceeList syncees() { return mSyncees; }

    bool readSyncees();
    bool writeSyncees();

    bool connectDevice();
    bool disconnectDevice();

    KSync::KonnectorInfo info() const;
    virtual QStringList supportedFilterTypes() const;

    void setCurrentResource( const QString &identifier ) { mResourceIdentifier = identifier; }
    QString currentResource() const { return mResourceIdentifier; }

  protected slots:
    void loadingFinished();

  private:
    KABC::Resource* createResource( const QString& );

    KABCKonnectorConfig *mConfigWidget;
    QString mResourceIdentifier;
    QString mMd5sum;

    KABC::AddressBook mAddressBook;
    KABC::Resource *mResource;

    KSync::AddressBookSyncee *mAddressBookSyncee;

    SynceeList mSyncees;
};

}

#endif
