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

#include <kapplication.h>
#include <kconfig.h>
#include <kstandarddirs.h>

#include <kurl.h>
#include <kdebug.h>
#include <krfcdate.h>
#include <kio/job.h>

#include <mimelib/string.h>
#include <mimelib/message.h>
#include <mimelib/body.h>
#include <mimelib/bodypart.h>
#include <mimelib/headers.h>
#include <mimelib/mediatyp.h>
#include <mimelib/addrlist.h>
#include <mimelib/mboxlist.h>
#include <mimelib/text.h>

#include <qfile.h>
#include <qinputdialog.h>
#include <qtextstream.h>
#include <qdatastream.h>
#include <qcstring.h>
#include <qregexp.h>

#include <kapp.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kaction.h>
#include <kio/slave.h>
#include <kio/scheduler.h>
#include <kio/slavebase.h>
#include <kio/davjob.h>
#include <kio/http.h>

#include <libkcal/icalformat.h>
#include <libkcal/icalformatimpl.h>
#include <libkcal/calendarlocal.h>

extern "C" {
  #include <ical.h>
}

#include "exchangeaccount.h"
#include "exchangedownload.h"
#include "exchangeprogress.h"

using namespace KPIM;

ExchangeDownload::ExchangeDownload( ExchangeAccount* account, QWidget* window ) :
  mWindow( window )
{
  mAccount = account;
  mMode = Asynchronous;
  mDownloadsBusy = 0;
  mProgress = 0L;
  mCalendar = 0L;
}

ExchangeDownload::~ExchangeDownload()
{
  kdDebug() << "ExchangeDownload destructor" << endl;
}

// Asynchronous functions

void ExchangeDownload::download(KCal::Calendar* calendar, const QDate& start, const QDate& end, bool showProgress)
{
  mCalendar = calendar;
  mMode = Asynchronous;
  initiateDownload( start, end, showProgress );
}

void ExchangeDownload::initiateDownload( const QDate& start, const QDate& end, bool showProgress ) 
{
  // mAccount->authenticate();

  if( showProgress ) {
    //kdDebug() << "Creating progress dialog" << endl;
    mProgress = new ExchangeProgress();
    mProgress->show();
  
    connect( this, SIGNAL(startDownload()), mProgress, SLOT(slotTransferStarted()) );
    connect( this, SIGNAL(finishDownload()), mProgress, SLOT(slotTransferFinished()) );
    // connect( mProgress, SIGNAL(complete( ExchangeProgress* )), this, SLOT(slotComplete( ExchangeProgress* )) );
  }

  QString sql = dateSelectQuery( start, end );
 
  // kdDebug() << "Exchange download query: " << endl << sql << endl;

  increaseDownloads();

  KIO::DavJob *job = KIO::davSearch( mAccount->calendarURL(), "DAV:", "sql", sql, false );
  job->setWindow( mWindow );
  connect(job, SIGNAL(result( KIO::Job * )), this, SLOT(slotSearchResult(KIO::Job *)));
}

QString ExchangeDownload::dateSelectQuery( const QDate& start, const QDate& end )
{
  QString startString;
  startString.sprintf("%04i/%02i/%02i",start.year(),start.month(),start.day());
  QString endString;
  endString.sprintf("%04i/%02i/%02i",end.year(),end.month(),end.day());
  QString sql = 
        "SELECT \"DAV:href\", \"urn:schemas:calendar:instancetype\", \"urn:schemas:calendar:uid\"\r\n"
        "FROM Scope('shallow traversal of \"\"')\r\n"
        "WHERE \"urn:schemas:calendar:dtend\" > '" + startString + "'\r\n"
        "AND \"urn:schemas:calendar:dtstart\" < '" + endString + "'";
  return sql;
}


void ExchangeDownload::slotSearchResult( KIO::Job *job )
{
  if ( job->error() ) {
    kdDebug() << "Error result for search: " << job->error() << endl;
    job->showErrorDialog( 0L );
    decreaseDownloads();
    return;
  }
  QDomDocument& response = static_cast<KIO::DavJob *>( job )->response();

  // kdDebug() << "Search result: " << endl << response.toString() << endl;

  handleAppointments( response, true );
  
  decreaseDownloads();
}

void ExchangeDownload::slotMasterResult( KIO::Job *job )
{
  if ( job->error() ) {
    job->showErrorDialog( 0L );
    decreaseDownloads();
    return;
  }
  QDomDocument& response = static_cast<KIO::DavJob *>( job )->response();

  // kdDebug() << "Search (master) result: " << endl << response.toString() << endl;

  handleAppointments( response, false );
  
  decreaseDownloads();
}

void ExchangeDownload::handleAppointments( const QDomDocument& response, bool recurrence ) {
  //kdDebug() << "Entering handleAppointments" << endl;
  for( QDomElement item = response.documentElement().firstChild().toElement();
       !item.isNull();
       item = item.nextSibling().toElement() )
  {
    //kdDebug() << "Current item:" << item.tagName() << endl;
    QDomNodeList propstats = item.elementsByTagNameNS( "DAV:", "propstat" );
    // kdDebug() << "Item has " << propstats.count() << " propstat children" << endl; 
    for( uint i=0; i < propstats.count(); i++ )
    {
      QDomElement propstat = propstats.item(i).toElement();
      QDomElement prop = propstat.namedItem( "prop" ).toElement();
      if ( prop.isNull() )
      {
        kdDebug() << "Error: no <prop> in response" << endl;
	continue;
      }

      QDomElement instancetypeElement = prop.namedItem( "instancetype" ).toElement();
      if ( instancetypeElement.isNull() ) {
        kdDebug() << "Error: no instance type in Exchange server reply" << endl;
        continue;
      }
      int instanceType = instancetypeElement.text().toInt();
      //kdDebug() << "Instance type: " << instanceType << endl;
    
      if ( recurrence && instanceType > 0 ) {
        QDomElement uidElement = prop.namedItem( "uid" ).toElement();
        if ( uidElement.isNull() ) {
          kdDebug() << "Error: no uid in Exchange server reply" << endl;
          continue;
        }
        QString uid = uidElement.text();
        if ( ! m_uids.contains( uid ) ) {
          m_uids[uid] = 1;
          handleRecurrence(uid);
        }
        continue;
      }

      QDomElement hrefElement = prop.namedItem( "href" ).toElement();
      if ( hrefElement.isNull() ) {
        kdDebug() << "Error: no href in Exchange server reply" << endl;
        continue;
      }
      QString href = hrefElement.text();
      KURL url(href);
      url.setProtocol("webdav");
      
      kdDebug() << "GET url: " << url.prettyURL() << endl;
    
      increaseDownloads();
      KIO::TransferJob *job = KIO::get(url, false, false);
      KIO::Scheduler::scheduleJob(job);
      job->setWindow( mWindow );
      job->addMetaData("davHeader", "Translate: f\r\n");
      connect( job, SIGNAL(data(KIO::Job *, const QByteArray &)), this, SLOT(slotData(KIO::Job *, const QByteArray &)));
      connect( job, SIGNAL( result ( KIO::Job * ) ), SLOT ( slotTransferResult( KIO:: Job * ) ) );
    }
  }
}  

void ExchangeDownload::handleRecurrence(QString uid) {
  // kdDebug() << "Handling recurrence info for uid=" << uid << endl;
  QString query = 
        "SELECT \"DAV:href\", \"urn:schemas:calendar:instancetype\"\r\n"
        "FROM Scope('shallow traversal of \"\"')\r\n"
        "WHERE \"urn:schemas:calendar:uid\" = '" + uid + "'\r\n"
	" AND (\"urn:schemas:calendar:instancetype\" = 1)\r\n";
//	"      OR \"urn:schemas:calendar:instancetype\" = 3)\r\n" // FIXME: exception are not handled

  // kdDebug() << "Exchange master query: " << endl << query << endl;

  increaseDownloads();
 
  KIO::DavJob* job = KIO::davSearch( mAccount->calendarURL(), "DAV:", "sql", query, false );
  job->setWindow( mWindow );
  KIO::Scheduler::scheduleJob(job);
  connect(job, SIGNAL(result( KIO::Job * )), this, SLOT(slotMasterResult(KIO::Job *)));
}

void ExchangeDownload::slotData(KIO::Job *job, const QByteArray &data) {
  KURL url = static_cast<KIO::TransferJob *>(job)->url();
  // kdDebug() << "Got data for " << url.prettyURL() << endl;
  
  if(data.size() != 0)
  {
    DwString *messageData;
    if ( !m_transferJobs.contains( url.url() ) ) 
    { 
      messageData = new DwString();
      m_transferJobs[url.url()] = messageData;
    } else {
      messageData = m_transferJobs[url.url()];
    }

    // DwString string(data.data(), data.size());
    messageData->append(data.data(), data.size());
    // kdDebug() << messageData->c_str() << endl;
    // delete string;
  }
}
   
void ExchangeDownload::slotTransferResult(KIO::Job *job) {
  KURL url = static_cast<KIO::TransferJob *>(job)->url();
  kdDebug() << "Transfer " << url.prettyURL() << " finished" << endl;
  
  if ( job->error() ) {
    job->showErrorDialog( 0L );
    if ( m_transferJobs.contains( url.url() ) ) {
      delete m_transferJobs[url.url()];
      m_transferJobs.remove( url.url() );
    }
    decreaseDownloads();
    return;
  }

  // kdDebug() << "Message: " << messageData->c_str() << endl;
  if ( !m_transferJobs.contains( url.url() ) ) {
    kdDebug() << "WARNING: no data!" << endl;
    decreaseDownloads();
    return;
  }
  
  DwString *messageData = m_transferJobs[url.url()];
  DwMessage msg( *messageData );
  msg.Parse();
  handlePart(&msg);
  DwBody body = msg.Body();
  for ( DwBodyPart *part=body.FirstBodyPart(); part; part = part->Next() )
  {
    handlePart(part);
  }
  m_transferJobs.remove( url.url() );
  delete messageData;
  decreaseDownloads();
  // kdDebug() << "Finished slotTransferREsult" << endl;
}

void ExchangeDownload::handlePart( DwEntity *part ) {
  // kdDebug() << "part text:" << endl << part->Body().AsString().c_str() << endl;
  DwMediaType contType = part->Headers().ContentType();
  if ( contType.TypeStr()=="text" && contType.SubtypeStr()=="calendar" ) {
    // kdDebug() << "CALENDAR!" <<endl;
    // kdDebug() << "VCalendar text:" << endl << "---- BEGIN ----" << endl << part->Body().AsString().c_str() << "---- END ---" << endl;
    KCal::ICalFormat *format = new KCal::ICalFormat();
    bool result = format->fromString( mCalendar, part->Body().AsString().c_str() );
    if ( mMode == Synchronous )
    {
      // mEvents.add();
    }
    delete format;
    // kdDebug() << "Result:" << result << endl;
  } else {
    // kdDebug() << contType.TypeStr().c_str() << "/" << contType.SubtypeStr().c_str() << endl;
  }
}

void ExchangeDownload::increaseDownloads()
{
  mDownloadsBusy++;
  emit startDownload();
}

void ExchangeDownload::decreaseDownloads()
{
  mDownloadsBusy--;
  // kdDebug() << "Download finished, waiting for " << mDownloadsBusy << " more" << endl;
  emit finishDownload();
  if ( mDownloadsBusy == 0 ) {
    kdDebug() << "All downloads finished" << endl;
    mCalendar->setModified( true );
    if ( mProgress ) {
      disconnect( this, 0, mProgress, 0 );
      disconnect( mProgress, 0, this, 0 );
      mProgress->delayedDestruct();
    }
    emit finished( this );
  }
}

// Synchronous functions

QPtrList<KCal::Event> ExchangeDownload::eventsForDate( KCal::Calendar* calendar, const QDate &qd )
{
  // kdDebug() << "Entering ExchangeDownload::eventsForDate()" << endl;

  mState = WaitingForResult;
  mMode = Synchronous;
  mCalendar = calendar;
  mEvents = QPtrList<KCal::Event>();

  // kdDebug() << "Initiating download..." << endl;
  initiateDownload( qd, qd.addDays( 1 ), false );

  connect(this, SIGNAL(finished( ExchangeDownload * )), 
		  this, SLOT(slotDownloadFinished( ExchangeDownload *)));
  do {
    qApp->processEvents();
  } while ( mState==WaitingForResult );

  // kdDebug() << "Finished downloading events for date" << endl;
  return mEvents;

  /*
  mAccount->authenticate();
  kdDebug() << "Authenticated" << endl;
  mMode = Synchronous;

  kdDebug() << "Constructing query..." << endl;
  QString sql = dateSelectQuery( qd, qd );
 
  kdDebug() << "Exchange download query: " << endl << sql << endl;

  mState = WaitingForResult;
  KIO::DavJob *job = KIO::davSearch( mAccount->calendarURL(), "DAV:", "sql", sql, false );
  connect(job, SIGNAL(result( KIO::Job * )), this, SLOT(slotSyncResult(KIO::Job *)));
  do {
    qApp->processEvents();
  } while ( mState==WaitingForResult );

  // QDomDocument& response = job->response();

  kdDebug() << "Search result: " << endl << mResponse.toString() << endl;
  QPtrList<KCal::Event> list;
  return list;
  */
}

void ExchangeDownload::slotDownloadFinished( ExchangeDownload *download )
{
  // kdDebug() << "slotDownloadFinished" << endl;
  mState = HaveResult;
}

void ExchangeDownload::slotSyncResult( KIO::Job * job )
{
  // kdDebug() << "slotSyncResult(): error=" << job->error() << endl;
  if ( job->error() ) {
    job->showErrorDialog( 0L );
  }

  mState = HaveResult;
  mResponse = static_cast< KIO::DavJob * >(job)->response();
}

#include "exchangedownload.moc"
