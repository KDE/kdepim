/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef KDEPIM_EXCHANGE_DOWNLOAD_H
#define KDEPIM_EXCHANGE_DOWNLOAD_H

#include <qstring.h>
#include <qdom.h>
#include <qmap.h>
#include <kio/job.h>

#include <libkcal/event.h>
#include <libkcal/icalformat.h>
#include <libkcal/incidence.h>
#include <libkcal/calendar.h>

class DwString;
class DwEntity;

namespace KPIM {
	
class ExchangeProgress;
class ExchangeAccount;

enum DownloadMode { Synchronous, Asynchronous };
enum DownloadState { WaitingForResult, HaveResult };

class ExchangeDownload : public QObject {
    Q_OBJECT
  public:
    ExchangeDownload( ExchangeAccount* account );
   ~ExchangeDownload();

    // Synchronous functions
    QPtrList<KCal::Event> eventsForDate( const QDate &qd );

    // Asynchronous functions
    void download( KCal::Calendar* calendar, 
         const QDate& start, const QDate& end, bool showProgress);
 
  private slots:
    // void slotPatchResult( KIO::Job * );
    // void slotPropFindResult( KIO::Job * );
    // void slotComplete( ExchangeProgress * );
 
    void slotSearchResult( KIO::Job *job );
    void slotMasterResult( KIO::Job* job );
    void slotData( KIO::Job *job, const QByteArray &data );
    void slotTransferResult( KIO::Job *job );

    void slotDownloadFinished( ExchangeDownload * );
    void slotSyncResult( KIO::Job * job );

  signals:
    void startDownload();
    void finishDownload();

    void downloadFinished( ExchangeDownload* );

  private:
    void handleAppointments( const QDomDocument &, bool recurrence );
    void handleRecurrence( QString uid );
    void handlePart( DwEntity *part );

    void initiateDownload( const QDate& start, const QDate& end, bool showProgress );

    void increaseDownloads();
    void decreaseDownloads();

    QString dateSelectQuery( const QDate& start, const QDate& end );
    
    KCal::Calendar *mCalendar;
    ExchangeAccount *mAccount;
    ExchangeProgress *mProgress;
    int mDownloadsBusy;
    DownloadMode mMode;
    DownloadState mState;
    QDomDocument mResponse;

    QMap<QString,int> m_uids; // This keeps track of uids we already covered. Especially useful for
    	// recurring events.
    QMap<QString,DwString *> m_transferJobs; // keys are URLs
};

}

#endif

