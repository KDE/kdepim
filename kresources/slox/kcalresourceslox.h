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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef KCALRESOURCESLOX_H
#define KCALRESOURCESLOX_H

#include "sloxbase.h"
#include "webdavhandler.h"
#include "slox_export.h"

#include <QString>
#include <QDateTime>
#include <qdom.h>

#include <kurl.h>
#include <kdirwatch.h>

#include <kcal/incidence.h>
#include <kcal/todo.h>
#include <kcal/calendarlocal.h>
#include <kcal/icalformat.h>
#include <kcal/resourcecached.h>

namespace KIO {
class Job;
class DavJob;
}

class KJob;
namespace KCal {
class SloxPrefs;
}

namespace KPIM {
class ProgressItem;
}

class SloxAccounts;

/**
  This class provides a calendar stored as a remote file.
*/
class KCAL_SLOX_EXPORT KCalResourceSlox : public KCal::ResourceCached, public SloxBase
{
    Q_OBJECT

    friend class KCalResourceSloxConfig;

  public:
    /**
      Reload policy.

      @see setReloadPolicy(), reloadPolicy()
    */
    enum { ReloadNever, ReloadOnStartup, ReloadOnceADay, ReloadAlways };

    KCalResourceSlox();
    /**
      Create resource from configuration information stored in KConfig object.
    */
    KCalResourceSlox( const KConfigGroup &group );
    KCalResourceSlox( const KUrl &url );
    ~KCalResourceSlox();

    void readConfig( const KConfigGroup &group );
    void writeConfig( KConfigGroup &group );

    KCal::SloxPrefs *prefs() const { return mPrefs; }

    KABC::Lock *lock();

    bool isSaving();

    void dump() const;

  protected slots:
    void slotLoadEventsResult( KJob * );
    void slotLoadTodosResult( KJob * );
    void slotUploadResult( KJob * );

    void slotEventsProgress( KJob *job, unsigned long percent );
    void slotTodosProgress( KJob *job, unsigned long percent );
    void slotUploadProgress( KJob *job, unsigned long percent );

    void cancelLoadEvents();
    void cancelLoadTodos();
    void cancelUpload();

  protected:
    void doClose();
    bool doLoad( bool syncCache );
    bool doSave( bool syncCache );
    bool doSave( bool syncCache, KCal::Incidence *incidence );

    void requestEvents();
    void requestTodos();

    void uploadIncidences();

    void parseMembersAttribute( const QDomElement &e,
                                KCal::Incidence *incidence );
    void parseReadRightsAttribute( const QDomElement &e,
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
    void createRecurrenceAttributes( QDomDocument &doc,
                                     QDomElement &parent,
                                     KCal::Incidence *incidence );

    bool confirmSave();

    QString sloxIdToEventUid( const QString &sloxId );
    QString sloxIdToTodoUid( const QString &sloxId );

  private:
    void init();

    KCal::SloxPrefs *mPrefs;

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

    SloxAccounts *mAccounts;
};

#endif
