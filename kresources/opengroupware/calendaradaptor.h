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
#ifndef KPIM_CALENDARADAPTOR_H
#define KPIM_CALENDARADAPTOR_H

#include "groupwaredataadaptor.h"

#include <libemailfunctions/idmapper.h>

#include <kurl.h>

#include <qstring.h>

namespace KCal {
class FolderLister;
class ResourceCached;
}

namespace KPIM {

class CalendarAdaptor : public GroupwareDataAdaptor
{
  public:
    CalendarAdaptor();

    /**
      Set resource.
    */
    void setResource( KCal::ResourceCached *v )
    {
      mResource = v;
    }
    /**
      Get resource. See setResource().
    */
    KCal::ResourceCached *resource() const
    {
      return mResource;
    }

    void adaptDownloadUrl( KURL &url );
    void adaptUploadUrl( KURL &url );
    QString mimeType() const;
    bool localItemExists( const QString &localId );
    bool localItemHasChanged( const QString &localId );
    void deleteItem( const QString &localId );
    QString addItem( const QString &rawText,
      const QString &localId, const QString &storageLocation );
    QString CalendarAdaptor::extractUid( const QString &data );
    void CalendarAdaptor::clearChange( const QString &uid );

  private:
    KCal::ResourceCached *mResource;
};

}

#endif
