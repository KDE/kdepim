/*
    This file is part of libkcal.

    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KCAL_RESOURCELOCAL_H
#define KCAL_RESOURCELOCAL_H

#include <qstring.h>
#include <qdatetime.h>

#include <kurl.h>
#include <kdirwatch.h>
#include <kdepimmacros.h>
class KConfig;

#include "calendarlocal.h"
#include "libkcal_export.h"

#include "resourcecached.h"

namespace KCal {

class CalFormat;

/**
  This class provides a calendar resource stored as a local file.
*/
class LIBKCAL_EXPORT ResourceLocal : public ResourceCached
{
    Q_OBJECT

    friend class ResourceLocalConfig;

  public:
    /**
      Create resource from configuration information stored in a KConfig object.
    */
    ResourceLocal( const KConfig * );
    /**
      Create resource for file named @a fileName.
    */
    ResourceLocal( const QString& fileName );
    virtual ~ResourceLocal();

    virtual void writeConfig( KConfig* config );

    KABC::Lock *lock();

    QString fileName() const;
    bool setFileName( const QString &fileName );
    bool setValue( const QString &key, const QString &value );


    void dump() const;

  protected slots:
    void reload();

  protected:
    virtual bool doLoad();
    virtual bool doSave();
    /**
      Called by reload() to reload the resource, if it is already open.
      @return true if successful, else false. If true is returned,
              reload() will emit a resourceChanged() signal.
    */
    virtual bool doReload();

    QDateTime readLastModified();

  private:
    void init();

    KURL mURL;
    CalFormat *mFormat;

    KDirWatch mDirWatch;

    KABC::Lock *mLock;

    class Private;
    Private *d;
};

}

#endif
