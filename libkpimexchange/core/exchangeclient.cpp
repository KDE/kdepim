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
#include <kurl.h>
#include <kdebug.h>
#include <kcursor.h>

// These two for test() method
#include <kio/http.h>
#include <kio/davjob.h>

#include "exchangeclient.h"
#include "exchangeaccount.h"
#include "exchangeprogress.h"
#include "exchangeupload.h"
#include "exchangedownload.h"
#include "exchangedelete.h"
#include "utils.h"

using namespace KPIM;

ExchangeClient::ExchangeClient( ExchangeAccount* account, const QString& timeZoneId ) :
  mWindow( 0 ), mTimeZoneId( timeZoneId )
{
  kdDebug() << "Creating ExchangeClient...\n";
  mAccount = account;
  if ( timeZoneId.isNull() ) {
    setTimeZoneId( "UTC" );
  }
}

ExchangeClient::~ExchangeClient()
{
  kdDebug() << "ExchangeClient destructor" << endl;
}

void ExchangeClient::setWindow(QWidget *window)
{
  mWindow = window;
}

QWidget *ExchangeClient::window() const
{
  return mWindow;
}

void ExchangeClient::setTimeZoneId( const QString& timeZoneId )
{
  mTimeZoneId = timeZoneId;
}

QString ExchangeClient::timeZoneId()
{
  return mTimeZoneId;
}

void ExchangeClient::test()
{
  kdDebug() << "Entering test()" << endl;
  KURL baseURL = KURL( "http://mail.tbm.tudelft.nl/janb/Calendar" );
  KURL url( "webdav://mail.tbm.tudelft.nl/janb/" );
/*
  QString query = 
  "<propfind xmlns=\"DAV:\" xmlns:h=\"urn:schemas:httpmail:\">\r\n"
  "  <prop>\r\n"
  "    <h:calendar/>\r\n"
  "    <h:contacts/>\r\n"
  "    <h:deleteditems/>\r\n"
  "    <h:drafts/>\r\n"
  "    <h:inbox/>\r\n"
  "    <h:journal/>\r\n"
  "    <h:notes/>\r\n"
  "    <h:outbox/>\r\n"
  "    <h:sentitems/>\r\n"
  "    <h:tasks/>\r\n"
  "    <h:msgfolderroot/>\r\n"
  "    <h:sendmsg/>\r\n"
  "  </prop>\r\n"
  "</propfind>\r\n";

  KIO::DavJob* job = new KIO::DavJob( url, (int) KIO::DAV_PROPFIND, query, false );
  job->addMetaData( "davDepth", "0" );
*/
  QString sql = 
    "select \"DAV:displayname\",\r\n"
    "  \"http://schemas.microsoft.com/exchange/outlookfolderclass\",\r\n"
    "  \"urn:schemas:httpmail:unreadcount\",\r\n"
    "  \"DAV:hassubs\"\r\n"
    "from scope('hierarchical traversal of \"/janb/\"')\r\n";
  KIO::DavJob *job = KIO::davSearch( url, "DAV:", "sql", sql, false );
}

void ExchangeClient::test2()
{
  kdDebug() << "Entering test2()" << endl;
}

void ExchangeClient::download( KCal::Calendar* calendar, const QDate& start, const QDate& end, bool showProgress )
{
  mAccount->authenticate( mWindow );
  ExchangeDownload* worker = new ExchangeDownload( mAccount, mWindow );
  worker->download( calendar, start, end, showProgress );
  connect( worker, SIGNAL( finished( ExchangeDownload*, int, const QString& ) ), this, SLOT( slotDownloadFinished( ExchangeDownload*, int, const QString& ) ) );
}

void ExchangeClient::upload( KCal::Event* event )
{
  mAccount->authenticate( mWindow );
  ExchangeUpload* worker = new ExchangeUpload( event, mAccount, mTimeZoneId, mWindow );
  connect( worker, SIGNAL( finished( ExchangeUpload*, int, const QString& ) ), this, SLOT( slotUploadFinished( ExchangeUpload*, int, const QString& ) ) );
}

void ExchangeClient::remove( KCal::Event* event )
{
  mAccount->authenticate( mWindow );
  ExchangeDelete* worker = new ExchangeDelete( event, mAccount, mWindow );
  connect( worker, SIGNAL( finished( ExchangeDelete*, int, const QString& ) ), this, SLOT( slotRemoveFinished( ExchangeDelete*, int, const QString& ) ) );
}

void ExchangeClient::slotDownloadFinished( ExchangeDownload* worker, int result, const QString& moreInfo ) 
{
  emit downloadFinished( result, moreInfo );
  worker->deleteLater();
}

void ExchangeClient::slotUploadFinished( ExchangeUpload* worker, int result, const QString& moreInfo ) 
{
  kdDebug() << "ExchangeClient::slotUploadFinished()" << endl;
  emit uploadFinished( result, moreInfo );
  worker->deleteLater();
}

void ExchangeClient::slotRemoveFinished( ExchangeDelete* worker, int result, const QString& moreInfo ) 
{
  kdDebug() << "ExchangeClient::slotRemoveFinished()" << endl;
  emit removeFinished( result, moreInfo );
  worker->deleteLater();
}

int ExchangeClient::downloadSynchronous( KCal::Calendar* calendar, const QDate& start, const QDate& end, bool showProgress)
{
  mClientState = WaitingForResult;
  connect(this, SIGNAL(downloadFinished( int, const QString& )), 
		  this, SLOT(slotSyncFinished( int, const QString& )));

  download( calendar, start, end, showProgress );

  QApplication::setOverrideCursor( KCursor::waitCursor() );
  do {
    qApp->processEvents();
  } while ( mClientState==WaitingForResult );
  QApplication::restoreOverrideCursor();  
  disconnect(this, SIGNAL(downloadFinished( int, const QString& )), 
		  this, SLOT(slotSyncFinished( int, const QString& )));
  return mSyncResult;
}

int ExchangeClient::uploadSynchronous( KCal::Event* event )
{
  mClientState = WaitingForResult;
  connect(this, SIGNAL(uploadFinished( int, const QString& )), 
		  this, SLOT(slotSyncFinished( int, const QString& )));

  upload( event );

  QApplication::setOverrideCursor( KCursor::waitCursor() );
  do {
    qApp->processEvents();
  } while ( mClientState==WaitingForResult );
  QApplication::restoreOverrideCursor();  
  disconnect(this, SIGNAL(uploadFinished( int, const QString& )), 
		  this, SLOT(slotSyncFinished( int, const QString& )));
  return mSyncResult;
}

int ExchangeClient::removeSynchronous( KCal::Event* event )
{
  mClientState = WaitingForResult;
  connect(this, SIGNAL(removeFinished( int, const QString& )), 
		  this, SLOT(slotSyncFinished( int, const QString& )));

  remove( event );

  QApplication::setOverrideCursor( KCursor::waitCursor() );
  do {
    qApp->processEvents();
  } while ( mClientState==WaitingForResult );
  QApplication::restoreOverrideCursor();  
  disconnect(this, SIGNAL(removeFinished( int, const QString& )), 
		  this, SLOT(slotSyncFinished( int, const QString& )));
  return mSyncResult;
}

void ExchangeClient::slotSyncFinished( int result, const QString& moreInfo )
{
  kdDebug() << "Exchangeclient::slotSyncFinished("<<result<<","<<moreInfo<<")" << endl;
  if ( mClientState == WaitingForResult ) {
    mClientState = HaveResult;
    mSyncResult = result;
    mDetailedErrorString = moreInfo;
  }
}

QString ExchangeClient::detailedErrorString()
{
  return mDetailedErrorString;
}

#include "exchangeclient.moc"
