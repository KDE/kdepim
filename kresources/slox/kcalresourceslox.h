/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifndef KCALRESOURCESLOX_H
#define KCALRESOURCESLOX_H

#include "webdavhandler.h"

#include <qptrlist.h>
#include <qstring.h>
#include <qdatetime.h>
#include <qdom.h>

#include <kurl.h>
#include <kconfig.h>
#include <kdirwatch.h>

#include <libkcal/incidence.h>
#include <libkcal/todo.h>
#include <libkcal/calendarlocal.h>
#include <libkcal/icalformat.h>
#include <libkcal/resourcecached.h>

namespace KIO {
class FileCopyJob;
class Job;
class DavJob;
}

namespace KCal {
class SloxPrefs;
}

namespace KPIM {
class ProgressItem;
}

/**
  This class provides a calendar stored as a remote file.
*/
class KCalResourceSlox : public KCal::ResourceCached
{
    Q_OBJECT

    friend class KCalResourceSloxConfig;

  public:
    /**
      Reload policy.
      
      @see setReloadPolicy(), reloadPolicy()
    */
    enum { ReloadNever, ReloadOnStartup, ReloadOnceADay, ReloadAlways };
  
    /**
      Create resource from configuration information stored in KConfig object.
    */
    KCalResourceSlox( const KConfig * );
    KCalResourceSlox( const KURL &url );
    ~KCalResourceSlox();

    void readConfig( const KConfig *config );
    void writeConfig( KConfig *config );

    KCal::SloxPrefs *prefs() const { return mPrefs; }

    /**
      Return name of file used as cache for remote file.
    */
    QString cacheFile();

    KABC::Lock *lock();

    bool isSaving();

    void dump() const;

  protected slots:
    void slotLoadEventsResult( KIO::Job * );
    void slotLoadTodosResult( KIO::Job * );
    void slotUploadResult( KIO::Job * );
    
    void slotEventsProgress( KIO::Job *job, unsigned long percent );
    void slotTodosProgress( KIO::Job *job, unsigned long percent );
    void slotUploadProgress( KIO::Job *job, unsigned long percent );

    void cancelLoadEvents();
    void cancelLoadTodos();
    void cancelUpload();

  protected:
    bool doOpen();
    void doClose();
    bool doLoad();
    bool doSave();

    void requestEvents();
    void requestTodos();

    void uploadIncidences();
 
    void parseMembersAttribute( const QDomElement &e,
                                KCal::Incidence *incidence );
    void parseIncidenceAttribute( const QDomElement &e,
                                  KCal::Incidence *incidence );
    void parseTodoAttribute( const QDomElement &e, KCal::Todo *todo );
    void parseEventAttribute( const QDomElement &e, KCal::Event *event );
    void parseRecurrence( const QDomNode &n, KCal::Event *event );

    void createIncidenceAttributes( QDomDocument &doc,
                                    QDomElement &parent,
                                    KCal::Incidence *incidence );
    void createEventAttributes( QDomDocument &doc,
                                QDomElement &parent,
                                KCal::Event *event );
    void createTodoAttributes( QDomDocument &doc,
                               QDomElement &parent,
                               KCal::Todo *todo );

    bool confirmSave();

    QString sloxIdToEventUid( const QString &sloxId );
    QString sloxIdToTodoUid( const QString &sloxId );

  private:
    void init();

    KCal::SloxPrefs *mPrefs;

    bool mOpen;

    KIO::DavJob *mLoadEventsJob;
    KIO::DavJob *mLoadTodosJob;
    KIO::DavJob *mUploadJob;

    KPIM::ProgressItem *mLoadEventsProgress;
    KPIM::ProgressItem *mLoadTodosProgress;
    KPIM::ProgressItem *mUploadProgress;

    KCal::Incidence *mUploadedIncidence;
    bool mUploadIsDelete;

    KABC::Lock *mLock;

    WebdavHandler mWebdavHandler;
};

#endif
