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

#include "exchangeclient.h"
#include "exchangeaccount.h"
#include "exchangeprogress.h"
#include "exchangeupload.h"
#include "exchangedownload.h"
#include "exchangedelete.h"
#include "utils.h"

using namespace KPIM;

ExchangeClient::ExchangeClient( ExchangeAccount* account ) :
  mWindow( 0 )
{
  kdDebug() << "Creating ExchangeClient...\n";
  mAccount = account;
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

void ExchangeClient::test()
{
  kdDebug() << "Entering test()" << endl;
  KURL baseURL = KURL( "http://mail.tbm.tudelft.nl/janb/Calendar" );
  KURL url( "webdav://mail.tbm.tudelft.nl/janb/Calendar/TestTest.EML" );
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
  connect( worker, SIGNAL( finished( ExchangeDownload* ) ), this, SLOT( slotDownloadFinished( ExchangeDownload* ) ) );
}

void ExchangeClient::upload( KCal::Event* event )
{
  mAccount->authenticate( mWindow );
  ExchangeUpload* worker = new ExchangeUpload( event, mAccount, mWindow );
  connect( worker, SIGNAL( finished( ExchangeUpload* ) ), this, SLOT( slotUploadFinished( ExchangeUpload* ) ) );
}

void ExchangeClient::remove( KCal::Event* event )
{
  mAccount->authenticate( mWindow );
  ExchangeDelete* worker = new ExchangeDelete( event, mAccount, mWindow );
  connect( worker, SIGNAL( finished( ExchangeDelete* ) ), this, SLOT( slotRemoveFinished( ExchangeDelete* ) ) );
}

void ExchangeClient::slotDownloadFinished( ExchangeDownload* worker ) 
{
  emit downloadFinished( ResultOK );
  worker->deleteLater();
}

void ExchangeClient::slotUploadFinished( ExchangeUpload* worker ) 
{
  kdDebug() << "ExchangeClient::slotUploadFinished()" << endl;
  emit uploadFinished( ResultOK );
  worker->deleteLater();
}

void ExchangeClient::slotRemoveFinished( ExchangeDelete* worker ) 
{
  kdDebug() << "ExchangeClient::slotRemoveFinished()" << endl;
  emit removeFinished( ResultOK );
  worker->deleteLater();
}

int ExchangeClient::downloadSynchronous( KCal::Calendar* calendar, const QDate& start, const QDate& end, bool showProgress)
{
  mClientState = WaitingForResult;
  connect(this, SIGNAL(downloadFinished( int )), 
		  this, SLOT(slotSyncFinished( int )));

  download( calendar, start, end, showProgress );

  QApplication::setOverrideCursor( KCursor::waitCursor() );
  do {
    qApp->processEvents();
  } while ( mClientState==WaitingForResult );
  QApplication::restoreOverrideCursor();  
  disconnect(this, SIGNAL(downloadFinished( int )), 
		  this, SLOT(slotSyncFinished( int )));
  return mSyncResult;
}

int ExchangeClient::uploadSynchronous( KCal::Event* event )
{
  mClientState = WaitingForResult;
  connect(this, SIGNAL(uploadFinished( int )), 
		  this, SLOT(slotSyncFinished( int )));

  upload( event );

  QApplication::setOverrideCursor( KCursor::waitCursor() );
  do {
    qApp->processEvents();
  } while ( mClientState==WaitingForResult );
  QApplication::restoreOverrideCursor();  
  disconnect(this, SIGNAL(uploadFinished( int )), 
		  this, SLOT(slotSyncFinished( int )));
  return mSyncResult;
}

int ExchangeClient::removeSynchronous( KCal::Event* event )
{
  mClientState = WaitingForResult;
  connect(this, SIGNAL(removeFinished( int )), 
		  this, SLOT(slotSyncFinished( int )));

  remove( event );

  QApplication::setOverrideCursor( KCursor::waitCursor() );
  do {
    qApp->processEvents();
  } while ( mClientState==WaitingForResult );
  QApplication::restoreOverrideCursor();  
  disconnect(this, SIGNAL(removeFinished( int )), 
		  this, SLOT(slotSyncFinished( int )));
  return mSyncResult;
}

void ExchangeClient::slotSyncFinished( int result )
{
  mClientState = HaveResult;
  mSyncResult = result;
}


#include "exchangeclient.moc"
