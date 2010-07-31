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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KDEPIM_EXCHANGE_DOWNLOAD_H
#define KDEPIM_EXCHANGE_DOWNLOAD_H

#include <tqstring.h>
#include <tqptrlist.h>
#include <tqdatetime.h>
#include <tqdom.h>
#include <tqmap.h>
#include <kio/job.h>

#include <libkcal/calendar.h>
#include <libkcal/icalformat.h>

namespace KPIM {
	
class ExchangeProgress;
class ExchangeAccount;

class ExchangeDownload : public TQObject {
    Q_OBJECT
  public:
    ExchangeDownload( ExchangeAccount* account, TQWidget* window=0 );
   ~ExchangeDownload();

    void download( KCal::Calendar* calendar, 
         const TQDate& start, const TQDate& end, bool showProgress );
    void download( const TQDate& start, const TQDate& end, bool showProgress );
 
  signals:
    void startDownload();
    void finishDownload();

    void gotEvent( KCal::Event* event, const KURL& url );
    void finished( ExchangeDownload*, int result, const TQString& moreInfo );
    void finished( ExchangeDownload*, int result, const TQString& moreInfo, TQPtrList<KCal::Event>& events );

  private slots:
    void slotSearchResult( KIO::Job *job );
    void slotMasterResult( KIO::Job* job );
    void slotPropFindResult( KIO::Job * );

  private:
    void handleAppointments( const TQDomDocument &, bool recurrence );
    void readAppointment( const KURL& url );
    void handleRecurrence( TQString uid );
    void finishUp( int result, const TQString& moreInfo=TQString::null );
    void finishUp( int result, KIO::Job* job );

    void increaseDownloads();
    void decreaseDownloads();

    TQString dateSelectQuery( const TQDate& start, const TQDate& end );
    
    KCal::Calendar *mCalendar;
    KCal::ICalFormat *mFormat;
    TQPtrList<KCal::Event> *mEvents;
    ExchangeAccount *mAccount;
    ExchangeProgress *mProgress;
    int mDownloadsBusy;
    TQDomDocument mResponse;

    TQMap<TQString,int> m_uids; // This keeps track of uids we already covered. Especially useful for
    	// recurring events.
    TQWidget* mWindow;
};

}

#endif

