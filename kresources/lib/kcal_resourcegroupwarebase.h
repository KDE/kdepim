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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KCAL_RESOURCEGROUPWAREBASE_H
#define KCAL_RESOURCEGROUPWAREBASE_H

#include <kurl.h>
#include <libkcal/resourcecached.h>
#include <kdepimmacros.h>
#include <kabc/locknull.h>

namespace KIO {
class Job;
}

namespace KPIM {
class GroupwareJob;
class GroupwareDownloadJob;
class GroupwareUploadJob;
class FolderLister;
class GroupwarePrefsBase;
}


namespace KCal {

class CalendarAdaptor;

/**
  This class provides a resource for accessing a Groupware kioslave-based
  calendar.
*/
class KDE_EXPORT ResourceGroupwareBase : public ResourceCached
{
    Q_OBJECT
  public:
    ResourceGroupwareBase();
  
    ResourceGroupwareBase( const KConfig * );
    virtual ~ResourceGroupwareBase();

    void readConfig( const KConfig *config );
    void writeConfig( KConfig *config );

    KPIM::GroupwarePrefsBase *prefs();
    KPIM::FolderLister *folderLister() { return mFolderLister; }

    bool doOpen();
    void doClose();

    bool doLoad();
    bool doSave();

    KABC::Lock *lock();
    
    bool addEvent( Event *event );
    bool addTodo( Todo *todo );
    bool addJournal( Journal *journal );

  protected:
    void init();
    
    bool confirmSave();
    
    KPIM::GroupwarePrefsBase *createPrefs();
    void setPrefs( KPIM::GroupwarePrefsBase *prefs );
    void setFolderLister( KPIM::FolderLister *folderLister );
    void setAdaptor( CalendarAdaptor *adaptor );
    CalendarAdaptor *adaptor() const { return mAdaptor; }

    virtual KPIM::GroupwareDownloadJob *createDownloadJob( CalendarAdaptor *adaptor );
    virtual KPIM::GroupwareUploadJob *createUploadJob( CalendarAdaptor *adaptor );

  protected slots:
    void slotLoginJobResult( KIO::Job *job );
    void slotLogoffJobResult( KIO::Job *job );
    void slotDownloadJobResult( KPIM::GroupwareJob * );
    void slotUploadJobResult( KPIM::GroupwareJob * );

  private:
    void enter_loop();
    
    KPIM::GroupwarePrefsBase *mPrefs;
    KPIM::FolderLister *mFolderLister;
    KABC::LockNull mLock;
    CalendarAdaptor *mAdaptor;

    KPIM::GroupwareDownloadJob *mDownloadJob;
    KPIM::GroupwareUploadJob *mUploadJob;

    bool mIsShowingError;

    bool mLoginFinished; // temp variable for the login job
    
};

}

#endif
