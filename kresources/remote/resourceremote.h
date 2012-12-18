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
#ifndef KCAL_RESOURCEREMOTEDIR_H
#define KCAL_RESOURCEREMOTEDIR_H

#include "remote_export.h"
#include <kurl.h>
#include <kdirwatch.h>

#include <kcal/incidence.h>
#include <kcal/calendarlocal.h>
#include <kcal/icalformat.h>

#include <kcal/resourcecached.h>

namespace KIO {
class FileCopyJob;
class Job;
}

namespace KPIM {
class ProgressItem;
}

class KJob;
namespace KCal {

/**
  This class provides a calendar stored as a remote file.
*/
class KCAL_RESOURCEREMOTE_EXPORT ResourceRemote : public ResourceCached
{
    Q_OBJECT

    friend class ResourceRemoteConfig;

  public:
    ResourceRemote();
    /**
      Create resource from configuration information stored in KConfig object.
    */
    explicit ResourceRemote( const KConfigGroup &group );
    /**
      Create remote resource.

      @param downloadUrl URL used to download iCalendar file
      @param uploadUrl   URL used to upload iCalendar file
    */
    explicit ResourceRemote( const KUrl &downloadUrl, const KUrl &uploadUrl = KUrl() );
    virtual ~ResourceRemote();

    void readConfig( const KConfigGroup &group );
    void writeConfig( KConfigGroup &group );

    void setDownloadUrl( const KUrl & );
    KUrl downloadUrl() const;

    void setUploadUrl( const KUrl & );
    KUrl uploadUrl() const;

    void setUseProgressManager( bool useProgressManager );
    bool useProgressManager() const;

    void setUseCacheFile( bool useCacheFile );
    bool useCacheFile() const;

    KABC::Lock *lock();

    bool isSaving();

    void dump() const;

    bool setValue( const QString &key, const QString &value );

  protected Q_SLOTS:
    void slotLoadJobResult( KJob * );
    void slotSaveJobResult( KJob * );

    void slotPercent( KJob *, unsigned long percent );

  protected:
    bool doLoad( bool syncCache );
    bool doSave( bool syncCache );
    bool doSave( bool syncCache, Incidence *incidence );

    void addInfoText( QString & ) const;

  private:
    void init();

    KUrl mDownloadUrl;
    KUrl mUploadUrl;

    bool mUseProgressManager;
    bool mUseCacheFile;

    KIO::FileCopyJob *mDownloadJob;
    KIO::FileCopyJob *mUploadJob;

    KPIM::ProgressItem *mProgress;

    Incidence::List mChangedIncidences;

    KABC::Lock *mLock;

    class Private;
    Private *d;
};

}

#endif
