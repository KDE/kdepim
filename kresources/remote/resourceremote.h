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

#include <q3ptrlist.h>
#include <QString>
#include <qdatetime.h>

#include <kurl.h>
#include <kconfig.h>
#include <kdirwatch.h>
#include <kdepimmacros.h>

#include <libkdepim/progressmanager.h>

#include <libkcal/incidence.h>
#include <libkcal/calendarlocal.h>
#include <libkcal/icalformat.h>

#include <libkcal/resourcecached.h>

namespace KIO {
class FileCopyJob;
class Job;
}

class KJob;
namespace KCal {

/**
  This class provides a calendar stored as a remote file.
*/
class KDE_EXPORT ResourceRemote : public ResourceCached
{
    Q_OBJECT

    friend class ResourceRemoteConfig;

  public:
    /**
      Create resource from configuration information stored in KConfig object.
    */
    ResourceRemote( const KConfig * );
    /**
      Create remote resource.
      
      @param downloadUrl URL used to download iCalendar file
      @param uploadUrl   URL used to upload iCalendar file
    */
    ResourceRemote( const KUrl &downloadUrl, const KUrl &uploadUrl = KUrl() );
    virtual ~ResourceRemote();

    void readConfig( const KConfig *config );
    void writeConfig( KConfig *config );

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


  protected slots:
    void slotLoadJobResult( KJob * );
    void slotSaveJobResult( KJob * );

    void slotPercent( KJob *, unsigned long percent );

  protected:
    bool doLoad();
    bool doSave();

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
