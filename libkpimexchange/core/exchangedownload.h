/*
    This file is part of libkpimexchange
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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
#ifndef KDEPIM_EXCHANGE_DOWNLOAD_H
#define KDEPIM_EXCHANGE_DOWNLOAD_H

#include <qstring.h>
#include <qptrlist.h>
#include <qdatetime.h>
#include <qdom.h>
#include <qmap.h>
#include <kio/job.h>

#include <libkcal/calendar.h>
#include <libkcal/icalformat.h>

namespace KPIM {
	
class ExchangeProgress;
class ExchangeAccount;

class ExchangeDownload : public QObject {
    Q_OBJECT
  public:
    ExchangeDownload( ExchangeAccount* account, QWidget* window=0 );
   ~ExchangeDownload();

    void download( KCal::Calendar* calendar, 
         const QDate& start, const QDate& end, bool showProgress );
    void download( const QDate& start, const QDate& end, bool showProgress );
 
  signals:
    void startDownload();
    void finishDownload();

    void gotEvent( KCal::Event* event, const KURL& url );
    void finished( ExchangeDownload*, int result, const QString& moreInfo );
    void finished( ExchangeDownload*, int result, const QString& moreInfo, QPtrList<KCal::Event>& events );

  private slots:
    void slotSearchResult( KIO::Job *job );
    void slotMasterResult( KIO::Job* job );
    void slotPropFindResult( KIO::Job * );

  private:
    void handleAppointments( const QDomDocument &, bool recurrence );
    void readAppointment( const KURL& url );
    void handleRecurrence( QString uid );
    void finishUp( int result, const QString& moreInfo=QString::null );
    void finishUp( int result, KIO::Job* job );

    void increaseDownloads();
    void decreaseDownloads();

    QString dateSelectQuery( const QDate& start, const QDate& end );
    
    KCal::Calendar *mCalendar;
    KCal::ICalFormat *mFormat;
    QPtrList<KCal::Event> *mEvents;
    ExchangeAccount *mAccount;
    ExchangeProgress *mProgress;
    int mDownloadsBusy;
    QDomDocument mResponse;

    QMap<QString,int> m_uids; // This keeps track of uids we already covered. Especially useful for
    	// recurring events.
    QWidget* mWindow;
};

}

#endif

