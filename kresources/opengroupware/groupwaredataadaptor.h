/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>

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
#ifndef KPIM_GROUPWAREDATAADAPTOR_H
#define KPIM_GROUPWAREDATAADAPTOR_H

#include <libemailfunctions/idmapper.h>

#include <kurl.h>

#include <qstring.h>

namespace KCal {
class FolderLister;
}

namespace KPIM {

class IdMapper;

class GroupwareDataAdaptor
{
  public:
    GroupwareDataAdaptor();
    virtual ~GroupwareDataAdaptor();

    void setFolderLister( KCal::FolderLister *folderLister )
    {
      mFolderLister = folderLister;
    }
    KCal::FolderLister *folderLister()
    {
      return mFolderLister;
    }

    /**
      Set progress message shown by progress manager during download.
    */
    void setDownloadProgressMessage( const QString &v )
    {
      mDownloadProgressMessage = v;
    }
    /**
      Get download progress message. See setDownloadProgressMessage().
    */
    QString downloadProgressMessage() const
    {
      return mDownloadProgressMessage;
    }

    /**
      Set progress message shown by progress manager during upload.
    */
    void setUploadProgressMessage( const QString &v )
    {
      mUploadProgressMessage = v;
    }
    /**
      Get upload progress message. See setUploadProgressMessage().
    */
    QString uploadProgressMessage() const
    {
      return mUploadProgressMessage;
    }

    /**
      Set user name.
    */
    void setUser( const QString &v )
    {
      mUser = v;
    }
    /**
      Get user. See setUser().
    */
    QString user() const
    {
      return mUser;
    }

    /**
      Set password of user.
    */
    void setPassword( const QString &v )
    {
      mPassword = v;
    }
    /**
      Get password. See setPassword().
    */
    QString password() const
    {
      return mPassword;
    }

    /**
      Set id mapper.
    */
    void setIdMapper( KPIM::IdMapper *v )
    {
      mIdMapper = v;
    }
    /**
      Get idMapper. See setIdMapper().
    */
    KPIM::IdMapper *idMapper() const
    {
      return mIdMapper;
    }

    void setUserPassword( KURL &url );

    virtual void adaptDownloadUrl( KURL &url ) = 0;
    virtual void adaptUploadUrl( KURL &url ) = 0;
    virtual QString mimeType() const = 0;
    virtual bool localItemExists( const QString &localId ) = 0;
    virtual bool localItemHasChanged( const QString &localId ) = 0;
    virtual void deleteItem( const QString &localId ) = 0;
    virtual QString addItem( const QString &rawText,
      const QString &localId, const QString &storageLocation ) = 0;
    virtual QString extractUid( const QString &data ) = 0;
    virtual void clearChange( const QString &uid ) = 0;

  private:
    KCal::FolderLister *mFolderLister;
    QString mDownloadProgressMessage;
    QString mUploadProgressMessage;
    QString mUser;
    QString mPassword;
    KPIM::IdMapper *mIdMapper;
};

}

#endif
