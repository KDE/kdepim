/*
    This file is part of the KDE-PIM exchange library
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

#include <qfile.h>

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
extern "C" {
  #include <ical.h>
}

#include "exchangeaccount.h"
#include "exchangedownload.h"
#include "exchangeprogress.h"

using namespace KPIM;

ExchangeDownload::ExchangeDownload(KCal::Calendar* calendar, ExchangeAccount* account, QDate& start, QDate& end, bool showProgress) 
{
  mCalendar = calendar;
  mAccount = account;

  account->authenticate();

  if( showProgress ) {
    ExchangeProgress *progress = new ExchangeProgress();
  
    connect( this, SIGNAL(startDownload()), progress, SLOT(slotTransferStarted()) );
    connect( this, SIGNAL(finishDownload()), progress, SLOT(slotTransferFinished()) );
    connect( progress, SIGNAL(complete( ExchangeProgress* )), this, SLOT(slotComplete( ExchangeProgress* )) );
  }
   
  QString startString;
  startString.sprintf("%04i/%02i/%02i",start.year(),start.month(),start.day());
  QString endString;
  endString.sprintf("%04i/%02i/%02i",end.year(),end.month(),end.day());
  QString sql = 
        "SELECT \"DAV:href\", \"urn:schemas:calendar:instancetype\", \"urn:schemas:calendar:uid\"\r\n"
        "FROM Scope('shallow traversal of \"\"')\r\n"
        "WHERE \"urn:schemas:calendar:dtend\" > '" + startString + "'\r\n"
        "AND \"urn:schemas:calendar:dtstart\" < '" + endString + "'";
  
  kdDebug() << "Exchange download query: " << endl << sql << endl;

  emit startDownload();

  KIO::DavJob *job = KIO::davSearch( mAccount->calendarURL(), "DAV:", "sql", sql, false );
  connect(job, SIGNAL(result( KIO::Job * )), this, SLOT(slotSearchResult(KIO::Job *)));
}


ExchangeDownload::~ExchangeDownload()
{
  kdDebug() << "ExchangeDownload destructor" << endl;
}

void ExchangeDownload::slotSearchResult( KIO::Job *job )
{
  if ( job->error() ) {
    job->showErrorDialog( 0L );
    emit finishDownload();
    return;
  }
  QDomDocument& response = static_cast<KIO::DavJob *>( job )->response();

  kdDebug() << "Search result: " << endl << response.toString() << endl;

  handleAppointments( response, true );
  
  emit finishDownload();
}

void ExchangeDownload::slotMasterResult( KIO::Job *job )
{
  if ( job->error() ) {
    job->showErrorDialog( 0L );
    emit finishDownload();
    return;
  }
  QDomDocument& response = static_cast<KIO::DavJob *>( job )->response();

  kdDebug() << "Search (master) result: " << endl << response.toString() << endl;

  handleAppointments( response, false );
  
  emit finishDownload();
}

void ExchangeDownload::handleAppointments( const QDomDocument& response, bool recurrence ) {
  kdDebug() << "Entering handleAppointments" << endl;
  for( QDomElement item = response.documentElement().firstChild().toElement();
       !item.isNull();
       item = item.nextSibling().toElement() )
  {
    kdDebug() << "Current item:" << item.tagName() << endl;
    QDomNodeList propstats = item.elementsByTagNameNS( "DAV:", "propstat" );
    kdDebug() << "Item has " << propstats.count() << " propstat children" << endl; 
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
      kdDebug() << "Instance type: " << instanceType << endl;
    
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
      if ( instancetypeElement.isNull() ) {
        kdDebug() << "Error: no href in Exchange server reply" << endl;
        continue;
      }
      QString href = hrefElement.text();
      KURL url(href);
      url.setProtocol("webdav");
      
      kdDebug() << "GET url: " << url.prettyURL() << endl;
    
      emit startDownload();
      KIO::TransferJob *job2 = KIO::get(url, false, false);
      KIO::Scheduler::scheduleJob(job2);
      job2->addMetaData("davHeader", "Translate: f\r\n");
      connect( job2, SIGNAL(data(KIO::Job *, const QByteArray &)), this, SLOT(slotData(KIO::Job *, const QByteArray &)));
      connect( job2, SIGNAL( result ( KIO::Job * ) ), SLOT ( slotTransferResult( KIO:: Job * ) ) );
    }
  }
}  

void ExchangeDownload::handleRecurrence(QString uid) {
  kdDebug() << "Handling recurrence info for uid=" << uid << endl;
  QString query = 
        "SELECT \"DAV:href\", \"urn:schemas:calendar:instancetype\"\r\n"
        "FROM Scope('shallow traversal of \"\"')\r\n"
        "WHERE \"urn:schemas:calendar:uid\" = '" + uid + "'\r\n"
	" AND (\"urn:schemas:calendar:instancetype\" = 1)\r\n";
//	"      OR \"urn:schemas:calendar:instancetype\" = 3)\r\n" // FIXME: exception are not handled

  kdDebug() << "Exchange master query: " << endl << query << endl;

  emit startDownload();
 
  KIO::DavJob* job = KIO::davSearch( mAccount->calendarURL(), "DAV:", "sql", query, false );
  connect(job, SIGNAL(result( KIO::Job * )), this, SLOT(slotMasterResult(KIO::Job *)));
}

void ExchangeDownload::slotData(KIO::Job *job, const QByteArray &data) {
  KURL url = static_cast<KIO::TransferJob *>(job)->url();
  kdDebug() << "Got data for " << url.prettyURL() << endl;
  
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
    emit finishDownload();
    return;
  }

  // kdDebug() << "Message: " << messageData->c_str() << endl;
  if ( !m_transferJobs.contains( url.url() ) ) {
    kdDebug() << "WARNING: no data!" << endl;
    emit finishDownload();
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
  emit finishDownload();
  kdDebug() << "Finished slotTransferREsult" << endl;
}

void ExchangeDownload::handlePart( DwEntity *part ) {
  // kdDebug() << "part text:" << endl << part->Body().AsString().c_str() << endl;
  DwMediaType contType = part->Headers().ContentType();
  if ( contType.TypeStr()=="text" && contType.SubtypeStr()=="calendar" ) {
    kdDebug() << "CALENDAR!" <<endl;
    kdDebug() << "VCalendar text:" << endl << "---- BEGIN ----" << endl << part->Body().AsString().c_str() << "---- END ---" << endl;
    KCal::ICalFormat *format = new KCal::ICalFormat();
    bool result = format->fromString( mCalendar, part->Body().AsString().c_str() );
    delete format;
    kdDebug() << "Result:" << result << endl;
  } else {
    kdDebug() << contType.TypeStr().c_str() << "/" << contType.SubtypeStr().c_str() << endl;
  }
}

void ExchangeDownload::slotComplete( ExchangeProgress *progress )
{
  kdDebug() << "Entering slotComplete()" << endl;
  mCalendar->setModified( true );
  disconnect( this, 0, progress, 0 );
  disconnect( progress, 0, this, 0 );
  progress->delayedDestruct();

  emit downloadFinished( this );
}

