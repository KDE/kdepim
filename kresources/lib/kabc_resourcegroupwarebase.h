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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
}

namespace KABC {

class AddressBookAdaptor;
class GroupwarePrefsBase;

class KDE_EXPORT ResourceGroupwareBase : public ResourceCached
{
  Q_OBJECT

  public:
    ResourceGroupwareBase( const KConfig * );
    ~ResourceGroupwareBase();

    void readConfig( const KConfig * );
    void writeConfig( KConfig * );

    GroupwarePrefsBase *prefs() const { return mPrefs; }
    void setPrefs( GroupwarePrefsBase *prefs );

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

    GroupwarePrefsBase *createPrefs();
    virtual KPIM::GroupwareDownloadJob *createDownloadJob(
                                                  AddressBookAdaptor *adaptor );
    virtual KPIM::GroupwareUploadJob *createUploadJob(
                                                  AddressBookAdaptor *adaptor );

  private slots:
    void slotDownloadJobResult( KPIM::GroupwareJob * );
    void slotUploadJobResult( KPIM::GroupwareJob * );

  private:
    GroupwarePrefsBase *mPrefs;
    KPIM::FolderLister *mFolderLister;
    AddressBookAdaptor *mAdaptor;

    KPIM::GroupwareDownloadJob *mDownloadJob;
    KPIM::GroupwareUploadJob *mUploadJob;
};

}

#endif
