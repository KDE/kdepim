 /*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KCAL_RESOURCEGROUPWARE_H
#define KCAL_RESOURCEGROUPWARE_H

#include <libkdepim/progressmanager.h>

#include <libkcal/resourcecached.h>

#include <kabc/locknull.h>
#include <kio/job.h>
#include <kconfig.h>

namespace KCal {

class GroupwarePrefsBase;

/**
  This class provides a resource for accessing a Groupware kioslave-based
  calendar.
*/
class ResourceGroupware : public ResourceCached
{
    Q_OBJECT
  public:
    ResourceGroupware();
  
    ResourceGroupware( const KConfig * );
    virtual ~ResourceGroupware();

    void readConfig( const KConfig *config );
    void writeConfig( KConfig *config );

    GroupwarePrefsBase *prefs();

    bool doOpen();
    void doClose();

    bool doLoad();
    bool doSave();

    KABC::Lock *lock();

  protected:
    void init();

    bool confirmSave();

  protected slots:
    void loadFinished();

    void slotJobResult( KIO::Job * );
    void slotJobData( KIO::Job *, const QByteArray & );

    void cancelLoad();

  private:
    GroupwarePrefsBase *mPrefs;
    KABC::LockNull mLock;

    KIO::TransferJob *mDownloadJob;
    KPIM::ProgressItem *mProgress;
    QString mJobData;

    bool mIsShowingError;
};

}

#endif
