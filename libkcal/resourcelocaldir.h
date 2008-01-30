/*
    This file is part of libkcal.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KCAL_RESOURCELOCALDIRDIR_H
#define KCAL_RESOURCELOCALDIRDIR_H

#include <kurl.h>
#include <kdirwatch.h>
#include <kdepimmacros.h>

#include "resourcecached.h"

#include "libkcal_export.h"

class QString;
class KConfig;

namespace KCal {

class CalendarLocal;
class Incidence;

/**
  \internal

  This class provides a calendar stored as a file per incidence in a directory.
*/
class LIBKCAL_EXPORT ResourceLocalDir : public ResourceCached
{
    Q_OBJECT

    friend class ResourceLocalDirConfig;

  public:
    ResourceLocalDir( const KConfig * );
    ResourceLocalDir( const QString& fileName );
    virtual ~ResourceLocalDir();

    void readConfig( const KConfig *config );
    void writeConfig( KConfig* config );

    KABC::Lock *lock();

    /** deletes an event from this calendar. */
    bool deleteEvent(Event *);

    /**
      Remove a todo from the todolist.
    */
    bool deleteTodo( Todo * );

    /**
      Remove a journal from the journallist.
    */
    bool deleteJournal( Journal * );

    void dump() const;

  protected slots:
    void reload( const QString & );

  protected:
    virtual bool doOpen();
    virtual bool doLoad();
    virtual bool doSave();
    bool doSave( Incidence * );
    virtual bool doFileLoad( CalendarLocal &, const QString &fileName );

  private:
    void init();
    bool deleteIncidenceFile(Incidence *incidence);

    KURL mURL;
//     ICalFormat mFormat;

    KDirWatch mDirWatch;

    KABC::Lock *mLock;

    class Private;
    Private *d;
};

}

#endif
