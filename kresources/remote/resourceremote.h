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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KCAL_RESOURCEREMOTEDIR_H
#define KCAL_RESOURCEREMOTEDIR_H

#include <qptrlist.h>
#include <qstring.h>
#include <qdatetime.h>

#include <kurl.h>
#include <kconfig.h>
#include <kdirwatch.h>

#include <libkdepim/progressmanager.h>

#include <libkcal/incidence.h>
#include <libkcal/calendarlocal.h>
#include <libkcal/icalformat.h>

#include <libkcal/resourcecached.h>

namespace KIO {
class FileCopyJob;
class Job;
}

namespace KCal {

/**
  This class provides a calendar stored as a remote file.
*/
class ResourceRemote : public ResourceCached
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
    ResourceRemote( const KURL &downloadUrl, const KURL &uploadUrl = KURL() );
    virtual ~ResourceRemote();

    void readConfig( const KConfig *config );
    void writeConfig( KConfig *config );

    void setDownloadUrl( const KURL & );
    KURL downloadUrl() const;
    
    void setUploadUrl( const KURL & );
    KURL uploadUrl() const;

    void setUseProgressManager( bool useProgressManager );
    bool useProgressManager() const;

    void setUseCacheFile( bool useCacheFile );
    bool useCacheFile() const;

    KABC::Lock *lock();

    bool isSaving();

    void dump() const;

  protected slots:
    void slotLoadJobResult( KIO::Job * );
    void slotSaveJobResult( KIO::Job * );

    void slotPercent( KIO::Job *, unsigned long percent );

  protected:
    bool doLoad();
    bool doSave();

    void addInfoText( QString & ) const;

  private:
    void init();

    KURL mDownloadUrl;
    KURL mUploadUrl;

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
