 /*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>
    Copyright (c) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KABC_ADDRESSBOOKADAPTOR_H
#define KABC_ADDRESSBOOKADAPTOR_H

#include "groupwaredataadaptor.h"

#include <kabc/addressee.h>
#include <kdepimmacros.h>
#include <kurl.h>

namespace KABC {
class ResourceCached;

class AddressBookUploadItem : public KPIM::GroupwareUploadItem
{
  public:
    AddressBookUploadItem( KPIM::GroupwareDataAdaptor *adaptor, 
                           KABC::Addressee addr, UploadType type );
    virtual ~AddressBookUploadItem() {}
  protected:
    AddressBookUploadItem( UploadType type ) 
                                          : KPIM::GroupwareUploadItem( type ) {}
};

class KDE_EXPORT AddressBookAdaptor : public KPIM::GroupwareDataAdaptor
{
  public:
    AddressBookAdaptor();

    QValueList<KPIM::FolderLister::ContentType> supportedTypes()
    {
      QValueList<KPIM::FolderLister::ContentType> types;
      types << KPIM::FolderLister::Contact;
      return types;
    }
    
    /**
      Set resource.
    */
    void setResource( KABC::ResourceCached *v )
    {
      mResource = v;
    }
    /**
      Get resource. See setResource().
    */
    KABC::ResourceCached *resource() const
    {
      return mResource;
    }

    QString mimeType() const;
    bool localItemExists( const QString &localId );
    bool localItemHasChanged( const QString &localId );
    void deleteItem( const QString &localId );
    void addItem( KABC::Addressee addr );
    void clearChange( const QString &uid );

    virtual KPIM::GroupwareUploadItem *newUploadItem( KABC::Addressee addr,
                     KPIM::GroupwareUploadItem::UploadType type );
    virtual void addressbookItemDownloaded( KABC::Addressee addr,
                const QString &newLocalId, const KURL &remoteId,
                const QString &fingerprint, const QString &storagelocation );
    
  private:
    KABC::ResourceCached *mResource;
};

}

#endif
