/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>
    Copyright (C) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef KABC_RESOURCEGROUPWARE_H
#define KABC_RESOURCEGROUPWARE_H

#include <kurl.h>
#include <kabcresourcecached.h>
#include <kdepimmacros.h>

class KConfig;

namespace KPIM {
class GroupwareJob;
class GroupwareDownloadJob;
class GroupwareUploadJob;
class FolderLister;
class GroupwarePrefsBase;
}

namespace KABC {

class AddressBookAdaptor;

class KDE_EXPORT ResourceGroupwareBase : public ResourceCached
{
  Q_OBJECT

  public:
    ResourceGroupwareBase( const KConfig * );
    ~ResourceGroupwareBase();

    void readConfig( const KConfig * );
    void writeConfig( KConfig * );

    KPIM::GroupwarePrefsBase *prefs() const { return mPrefs; }
    void setPrefs( KPIM::GroupwarePrefsBase *prefs );

    KPIM::FolderLister *folderLister() const { return mFolderLister; }
    void setFolderLister( KPIM::FolderLister *folderLister );

    AddressBookAdaptor *adaptor() const { return mAdaptor; }
    void setAdaptor( AddressBookAdaptor *adaptor );

    bool doOpen();
    void doClose();

    Ticket *requestSaveTicket();
    void releaseSaveTicket( Ticket* );

    bool load();
    bool asyncLoad();
    bool save( Ticket * );
    bool asyncSave( Ticket * );

  protected:
    void init();

    KPIM::GroupwarePrefsBase *createPrefs();
    virtual KPIM::GroupwareDownloadJob *createDownloadJob(
                                                  AddressBookAdaptor *adaptor );
    virtual KPIM::GroupwareUploadJob *createUploadJob(
                                                  AddressBookAdaptor *adaptor );

  private slots:
    void slotDownloadJobResult( KPIM::GroupwareJob * );
    void slotUploadJobResult( KPIM::GroupwareJob * );

  private:
    KPIM::GroupwarePrefsBase *mPrefs;
    KPIM::FolderLister *mFolderLister;
    AddressBookAdaptor *mAdaptor;

    KPIM::GroupwareDownloadJob *mDownloadJob;
    KPIM::GroupwareUploadJob *mUploadJob;
};

}

#endif
