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
#ifndef KDEPIM_EXCHANGE_CLIENT_H
#define KDEPIM_EXCHANGE_CLIENT_H

#include <qstring.h>
#include <qdatetime.h>
#include <qobject.h>
#include <qhostaddress.h>
#include <qptrlist.h>

#include <kdepimmacros.h>

namespace KCal { 
  class Event;
  class Calendar;
}

namespace KIO { 
  class Job; 
}

namespace KPIM {

class ExchangeAccount;
class ExchangeDownload;
class ExchangeUpload;
class ExchangeDelete;
//class ExchangeMonitor;

class KDE_EXPORT ExchangeClient : public QObject {
    Q_OBJECT
  public:
    ExchangeClient( ExchangeAccount* account, const QString& mTimeZoneId=QString::null );
    ~ExchangeClient();

    /**
     * Associate this client with a window given by @p window.
     */
    void setWindow(QWidget *window);

    /**
     * Returns the window this client is associated with.
     */
    QWidget *window() const;

    /**
     * Set the time zone to use 
     */
    void setTimeZoneId( const QString& timeZoneId );
    QString timeZoneId();
        
    // synchronous functions
    enum { 
      ResultOK,  /** No problem */
      UnknownError, /** Something else happened */
      CommunicationError, /** IO Error, the server could not be reached or returned an HTTP error */
      ServerResponseError,  /** Server did not give a useful response. For download, this
                                means that a SEARCH did not result in anything like an appointment */
      IllegalAppointmentError, /** Reading appointment data from server response failed */
      NonEventError, /** The Incidence that is to be uplaoded to the server is not an Event */
      EventWriteError, /** When writing an event to the server, an error occurred */
      DeleteUnknownEventError /** The event to be deleted does not exist on the server */
    };

    int downloadSynchronous( KCal::Calendar* calendar, const QDate& start, const QDate& end, bool showProgress=false);
    int uploadSynchronous( KCal::Event* event );
    int removeSynchronous( KCal::Event* event );

    // ExchangeMonitor* monitor( int pollMode, const QHostAddress& ownInterface );

    QString detailedErrorString();

  public slots:
    // Asynchronous functions, wait for "finished" signals for result
    // Deprecated: use download() without the Calendar* argument instead
    void download( KCal::Calendar* calendar, const QDate& start, const QDate& end, bool showProgress=false);
    void download( const QDate& start, const QDate& end, bool showProgress=false);
    void upload( KCal::Event* event );
    void remove( KCal::Event* event );
    void test();

  private slots:
    void slotDownloadFinished( ExchangeDownload* worker, int result, const QString& moreInfo );
    void slotDownloadFinished( ExchangeDownload* worker, int result, const QString& moreInfo, QPtrList<KCal::Event>& );
    void slotUploadFinished( ExchangeUpload* worker, int result, const QString& moreInfo );
    void slotRemoveFinished( ExchangeDelete* worker, int result, const QString& moreInfo );
    void slotSyncFinished( int result, const QString& moreInfo );

  signals:
    // Useful for progress dialogs, shows how much still needs to be done.
    // Not used right now, since ExchangeDownload provides its own progress dialog
    void startDownload();
    void finishDownload();

    void downloadFinished( int result, const QString& moreInfo );
    void event( KCal::Event* event, const KURL& url);
    void downloadFinished( int result, const QString& moreInfo, QPtrList<KCal::Event>& events );
    void uploadFinished( int result, const QString& moreInfo );
    void removeFinished( int result, const QString& moreInfo );

  private:
    void test2();

    enum { WaitingForResult, HaveResult, Error };
   
    int mClientState;
    int mSyncResult;
    QString mDetailedErrorString;
    QWidget* mWindow;
    ExchangeAccount* mAccount;
    QString mTimeZoneId;
};

}

#endif
