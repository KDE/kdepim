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

#include "exchangeclient.h"
#include "exchangeprogress.h"
#include "exchangeupload.h"
#include "exchangedownload.h"
#include "utils.h"

using namespace KPIM;

ExchangeClient::ExchangeClient( ExchangeAccount* account ) 
{
  kdDebug() << "Creating Exchange...\n";
  mAccount = account;
}

ExchangeClient::~ExchangeClient()
{
  kdDebug() << "Entering ExchangeClient destructor" << endl;
  kdDebug() << "Finished ExchangeClient destructor" << endl;
}

void ExchangeClient::test()
{
  kdDebug() << "Entering test()" << endl;
  KURL baseURL = KURL( "http://mail.tbm.tudelft.nl/janb/Calendar" );
  KURL url( "webdav://mail.tbm.tudelft.nl/janb/Calendar/TestTest.EML" );
/*
  kdDebug() << "GET url: " << url.prettyURL() << endl;
    
  KIO::TransferJob *job2 = KIO::get(url, false, false);
  KIO::Scheduler::scheduleJob(job2);
  job2->addMetaData("davHeader", "Translate: f\r\n");
  connect( job2, SIGNAL(data(KIO::Job *, const QByteArray &)), this, SLOT(slotData(KIO::Job *, const QByteArray &)));
  connect( job2, SIGNAL( result ( KIO::Job * ) ), SLOT ( slotTransferResult( KIO:: Job * ) ) );
*/ 
/*
  QString query = // "<D:sql>\r\n"
        "SELECT *\r\n" 
        "FROM Scope('shallow traversal of \"\"')\r\n"
        "WHERE \"DAV:displayname\" = \"IWB.EML\"\r\n";
        // "</D:sql>\r\n";

  kdDebug() << query << endl;
  KIO::DavJob* job = KIO::davSearch( baseURL, "DAV:", "sql", query, false );
*/
  QDomDocument doc;
  QDomElement root = addElement( doc, doc, "DAV:", "propfind" );
  addElement( doc, root, "DAV:", "allprop" );
//  QDomElement prop = addElement( doc, root, "DAV:", "prop" );
//  addElement( doc, prop, "urn:schemas:calendar:", "rrule" );
//  addElement( doc, prop, "urn:schemas:calendar:", "uid" );
//  addElement( doc, prop, "DAV:", "displayname" );


  KIO::DavJob* job = KIO::davPropFind( url, doc, "0", false );
  job->addMetaData( "errorPage", "false" );
  connect(job, SIGNAL(result( KIO::Job * )), this, SLOT(slotTestResult(KIO::Job *)));
}

void ExchangeClient::slotTestResult( KIO::Job * job )
{
  if ( job->error() ) {
    job->showErrorDialog( 0L );
    return;
  }
  QDomDocument& response = static_cast<KIO::DavJob *>( job )->response();

  kdDebug() << "Test result: " << endl << response.toString() << endl;
}
 
void ExchangeClient::test2()
{
  kdDebug() << "Entering test2()" << endl;
}

QPtrList<KCal::Event> ExchangeClient::events( KCal::Calendar* calendar, const QDate &qd )
{
   // QString query = dateSelectQuery( qd, qd, true /* inclusive */ );
  ExchangeDownload download( mAccount );
  return download.eventsForDate( calendar, qd );
   // QDomDocument response = NetDavAccess::davSearch( mAccount->calendarURL(), "DAV:", "sql", query );  
}
/*
QPtrList<KCal::Event> ExchangeClient::events( const QDate &qd )
{
  KCal::CalendarLocal* 
   // QString query = dateSelectQuery( qd, qd, true );
  ExchangeDownload download( mAccount );
  return download.eventsForDate( qd );
   // QDomDocument response = NetDavAccess::davSearch( mAccount->calendarURL(), "DAV:", "sql", query );  
}
*/

void ExchangeClient::download( KCal::Calendar* calendar, QDate& start, QDate& end, bool showProgress )
{
  ExchangeDownload* worker = new ExchangeDownload( mAccount );
  worker->download( calendar, start, end, showProgress );
  connect( worker, SIGNAL( finished( ExchangeDownload* ) ), this, SLOT( slotDownloadFinished( ExchangeDownload* ) ) );
}

void ExchangeClient::upload( KCal::Event* event )
{
  ExchangeUpload* worker = new ExchangeUpload( event, mAccount );
  connect( worker, SIGNAL( finished( ExchangeUpload* ) ), this, SLOT( slotUploadFinished( ExchangeUpload* ) ) );
}


void ExchangeClient::slotDownloadFinished( ExchangeDownload* worker ) 
{
  delete worker;
}

void ExchangeClient::slotUploadFinished( ExchangeUpload* worker ) 
{
  delete worker;
}

