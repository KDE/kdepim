 /*
    This file is part of libkcal.
    Copyright (c) 1998 Preston Brown
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KCAL_RESOURCELOCAL_H
#define KCAL_RESOURCELOCAL_H

#include <qptrlist.h>
#include <qstring.h>
#include <qdatetime.h>

#include <kurl.h>
#include <kconfig.h>
#include <kdirwatch.h>

#include "incidence.h"
#include "calendarlocal.h"

#include "resourcecached.h"

namespace KCal {

/**
  This class provides a calendar stored as a local file.
*/
class ResourceLocal : public ResourceCached
{
    Q_OBJECT

    friend class ResourceLocalConfig;

  public:
    ResourceLocal( const KConfig * );
    ResourceLocal( const QString& fileName );
    virtual ~ResourceLocal();

    virtual void writeConfig( KConfig* config );

    bool load();

    bool save();

    void dump() const;

  protected slots:
    void reload();

  protected:
    bool doOpen();

    /** clears out the current calendar, freeing all used memory etc. etc. */
    void doClose();

    /** this method should be called whenever a Event is modified directly
     * via it's pointer.  It makes sure that the calendar is internally
     * consistent. */
    virtual void update(IncidenceBase *incidence);
 
  private:
    void init();

    KURL mURL;
    CalFormat* mFormat;

    bool mOpen;

    KDirWatch mDirWatch;
};

}

#endif
