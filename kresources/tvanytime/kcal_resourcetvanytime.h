 /*
    This file is part of kdepim.

    Copyright (c) 2005 Will Stephenson <wstephenson@kde.org>

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
#ifndef KCAL_RESOURCETVANYTIME_H
#define KCAL_RESOURCETVANYTIME_H

#include <qtimer.h>

#include <libkdepim/progressmanager.h>

#include <libkcal/resourcecached.h>

#include <kabc/locknull.h>
#include <kio/job.h>
#include <kconfig.h>
#include <kdepimmacros.h>

#include "service.h"

class QDomDocument; 
class KTar;
class KTempFile;

typedef QMap< QString, Service > ServiceMap;
typedef QMap< QString, QString > UidMap;

namespace KCal {

class TVAnytimePrefsBase;

/**
  This class provides a resource for accessing a BBC TV and radio schedule as a calendar.
*/
class KDE_EXPORT ResourceTVAnytime : public ResourceCached
{
  Q_OBJECT

  public:
    ResourceTVAnytime();

    ResourceTVAnytime( const KConfig * );
    virtual ~ResourceTVAnytime();

    void readConfig( const KConfig *config );
    void writeConfig( KConfig *config );

    TVAnytimePrefsBase *prefs();

    QStringList subresources() const;
    const QString labelForSubresource( const QString& subresource ) const;
    QString subresourceIdentifier( Incidence *incidence );
    /** Is this subresource active? */
    bool subresourceActive( const QString& ) const;
    /** (De)activate the subresource */
    virtual void setSubresourceActive( const QString &, bool );

    bool doOpen();
    void doClose();

    bool doLoad();
    bool doSave() { return true; }

    KABC::Lock *lock();

  protected:
    void init();
    /**
     * Extract the XML from the given filename within the schedule archive
     */
    QDomDocument archiveFileXml( const QString & name );

    /**
     * Read the tv schedule from the tarball downloaded to mDestination
     */
    bool readSchedule();

    /** 
     * Read the service information map from the supplied xml
     */
    bool readServiceInformation( const QDomDocument & );
    bool readServices() { return true; }
    bool readService( const QString & serviceId );
    bool readProgrammeInformation( const QDomDocument & );

  protected slots:
    void slotJobResult( KIO::Job * );
    void cancelLoad();
    void slotEmitResourceChanged();

  private:
    TVAnytimePrefsBase *mPrefs;
    KABC::LockNull mLock;

    KIO::FileCopyJob *mDownloadJob;
    KPIM::ProgressItem *mProgress;

    bool mIsShowingError;

    QTimer mResourceChangedTimer;
    KTempFile * mDestination;
    KTar * mScheduleArchive;

    ServiceMap mServiceMap;
    UidMap mUidMap;
    //TODO: read this from subresource config
    QStringList mActiveServices;
    
};

}

#endif
