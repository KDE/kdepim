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

#include <qstring.h>
#include <qregexp.h>
#include <qsocketdevice.h>
#include <qsocketnotifier.h>
#include <qtextstream.h>

#include <kurl.h>
#include <kdebug.h>
#include <krfcdate.h>
#include <kextsock.h>

#include <kio/job.h>
#include <kio/slave.h>
#include <kio/scheduler.h>
#include <kio/slavebase.h>
#include <kio/davjob.h>
#include <kio/http.h>

#include <libkcal/event.h>
#include <libkcal/icalformat.h>
#include <libkcal/icalformatimpl.h>
#include <libkcal/recurrence.h>
#include <libkcal/incidence.h>
#include <libkcal/event.h>

#include "exchangemonitor.h"
#include "exchangeclient.h"
#include "exchangeaccount.h"
#include "utils.h"

extern "C" {
  #include <unistd.h>
}

using namespace KPIM;

QString makeIDString( const ExchangeMonitor::IDList& IDs )
{
  QString result;
  ExchangeMonitor::IDList::ConstIterator it;
  for ( it = IDs.begin(); it != IDs.end(); ++it ) {
    if ( it == IDs.begin() )
      result += QString::number( (*it) );
    else
      result += "," + QString::number( (*it) );
  }
  return result;
}

ExchangeMonitor::IDList makeIDList( const QString& input )
{
  ExchangeMonitor::IDList IDs;
  QStringList numbers = QStringList::split( ",", input );
  QStringList::iterator j;
  for ( j = numbers.begin(); j != numbers.end(); ++j ) {
    ExchangeMonitor::ID id = (*j).toLong();
    IDs.append( id );
  }
  return IDs;
}

ExchangeMonitor::ExchangeMonitor( ExchangeAccount* account, int pollMode, const QHostAddress& ownInterface )
{
  kdDebug() << "Called ExchangeMonitor" << endl;

  mAccount = account;
  mSubscriptionLifetime = 3600; // by default, renew subscription every 3600 seconds or one hour
  mPollMode = pollMode;
  mPollTimer = 0;

  if ( pollMode == CallBack ) {
    mSocket = new QSocketDevice( QSocketDevice::Datagram );
    if ( ! mSocket->bind( ownInterface, 0 ) )
      kdDebug() << "bind() returned false" << endl;
    mSocket->setBlocking( false );
    mNotifier = new QSocketNotifier( mSocket->socket(), QSocketNotifier::Read );
    connect( mNotifier, SIGNAL(activated( int )), this, SLOT( slotActivated(int)));

    //mSocket.setSocketFlags( KExtendedSocket::inetSocket | KExtendedSocket::passiveSocket | KExtendedSocket::datagramSocket | KExtendedSocket::bufferedSocket );
    //mSocket.setHost( "jupiter.tbm.tudelft.nl" );    // Does this work?
    //mSocket.setPort( 0 );                // setting port to 0 will make us bind to a random, free port
    // UDP server socket: no listen
    //if (  int code = mSocket.listen() )
    //   kdError() << "Error in socket listen: " << code << endl;
    //mSocket.enableRead( true );
    kdDebug() << "Port: " << mSocket->port() << endl;
    kdDebug() << "Host: " << mSocket->address().toString() << endl;
    //  mStream = new QTextStream( mSocket );
  }

  if ( mPollMode == Poll ) {
    mPollTimer = new QTimer( this );
    connect( mPollTimer, SIGNAL(timeout()), this, SLOT(slotPollTimer()) );
    mPollTimer->start( 60000 ); // 1 minute timer
  }

  mRenewTimer = new QTimer( this );
  connect( mRenewTimer, SIGNAL(timeout()), this, SLOT(slotRenewTimer()) );
  mRenewTimer->start( mSubscriptionLifetime * 900 ); // 10% early so as to be in time
}

ExchangeMonitor::~ExchangeMonitor()
{
  kdDebug() << "Entering ExchangeMonitor destructor" << endl;
  delete mNotifier;
  delete mSocket;
  if ( mPollTimer ) delete mPollTimer;
  if ( mRenewTimer ) delete mRenewTimer;
  if ( ! mSubscriptionMap.isEmpty() ) {
    QString headers = "Subscription-ID: " + makeIDString( mSubscriptionMap.keys() );
    kdDebug() << "Subsubscribing all watches, headers:" << endl << headers << endl;
    KIO::DavJob *job = new KIO::DavJob( mAccount->calendarURL(), (int) KIO::DAV_UNSUBSCRIBE, QString::null, false );
    job->addMetaData( "customHTTPHeader", headers );
    // Can't do, this is a destructor!
    // job->addMetaData( "PropagateHttpHeader", "true" );
    // connect(job, SIGNAL(result( KIO::Job * )), this, SLOT(slotUnsubscribeResult(KIO::Job *)));
  }
  kdDebug() << "Finished ExchangeMonitor destructor" << endl;

}

void ExchangeMonitor::addWatch( const KURL &url, int mode, int depth )
{
  QString headers = "Notification-type: ";
  switch( mode ) {
    case Delete: headers += "delete\r\n"; break;
    case Move: headers += "move\r\n"; break;
    case Newmail: headers += "pragma/<http://schemas.microsoft.com/exchange/newmail>\r\n"; break;
    case Update: headers += "update\r\n"; break;
    case UpdateNewMember: headers += "update/newmember\r\n"; break;
  }

  headers += "Depth: " + QString::number( depth );

  if (mPollMode == CallBack )
    headers += "\r\nCall-Back: httpu://" + mSocket->address().toString() + ":" + QString::number(mSocket->port());

  kdDebug() << "Headers: " << headers << endl;

  KURL myURL = toDAV( url );
  KIO::DavJob *job = new KIO::DavJob( myURL, (int) KIO::DAV_SUBSCRIBE, QString::null, false );
  job->addMetaData( "customHTTPHeader", headers );
  job->addMetaData( "PropagateHttpHeader", "true" );
  connect(job, SIGNAL(result( KIO::Job * )), this, SLOT(slotSubscribeResult(KIO::Job *)));
}

void ExchangeMonitor::removeWatch( const KURL &url ) 
{
  KURL myURL = toDAV( url );
  QMap<ID,KURL>::Iterator it;
  for ( it = mSubscriptionMap.begin(); it != mSubscriptionMap.end(); ++it ) {
    if ( it.data() == myURL ) {
      removeWatch( it.key() );
      return;
    }
  }
  kdWarning() << "Trying to remove unknown watch " << myURL.prettyURL() << ", failed." << endl;
}

void ExchangeMonitor::removeWatch( ID id )
{
  KIO::DavJob *job = new KIO::DavJob( mAccount->calendarURL(), (int) KIO::DAV_UNSUBSCRIBE, QString::null, false );
  job->addMetaData( "customHTTPHeader", "Subscription-id: " + QString::number( id ));
  job->addMetaData( "PropagateHttpHeader", "true" );
  connect(job, SIGNAL(result( KIO::Job * )), this, SLOT(slotUnsubscribeResult(KIO::Job *)));
}

void ExchangeMonitor::slotSubscribeResult( KIO::Job * job )
{
  if ( job->error() ) {
    job->showErrorDialog( 0L );
    emit error( ExchangeClient::CommunicationError, "IO Error: " + QString::number(job->error()) + ":" + job->errorString() );
    return;
  }

  ID id;
  KURL url;
  bool gotID = false;
  bool gotURL = false;

  QStringList headers = QStringList::split( "\n", job->queryMetaData( "HTTP-Headers" ) );
  for ( QStringList::Iterator it = headers.begin(); it != headers.end(); ++it ) {
    int colon = (*it).find( ": " );
    if ( colon<0 ) continue;
    QString tag = (*it).left( colon ).stripWhiteSpace().lower();
    QString value = (*it).mid( colon+1 ).stripWhiteSpace();
    if ( tag == "subscription-lifetime" ) {
      int lifetime = value.toInt();
      if ( lifetime < mSubscriptionLifetime ) {
        mSubscriptionLifetime = lifetime;
        mRenewTimer->changeInterval( lifetime * 900 );
        slotRenewTimer();
      }
    } else if ( tag == "subscription-id" ) {
      id = value.toLong();
      gotID = true;
    } else if ( tag == "content-location" ) {
      url = toDAV( value );
      gotURL = true;
    }
  }

  if ( mSubscriptionLifetime < 60 ) {
    kdWarning() << "Exchange server gave subscription a lifetime of " << mSubscriptionLifetime << ", changing to 60 seconds." << endl;
    mSubscriptionLifetime = 60;
    return;
  }

  if ( ! gotID ) {
    kdError() << "Error: Exchange server didn't give a subscription ID" << endl;
    emit error( ExchangeClient::ServerResponseError, "No subscription ID in SUBSCRIBE response headers: " + headers.join(", ") );
    return;
  }

  if ( ! gotURL ) {
    kdError() << "Error: Exchange server didn't return content-location" << endl;
    emit error( ExchangeClient::ServerResponseError, "No content-location in SUBSCRIBE response headers: " + headers.join(", ") );
    return;
  }

  kdDebug() << "Lifetime: " << mSubscriptionLifetime << endl;
  kdDebug() << "ID: " << id << endl;
  kdDebug() << "URL: " << url.prettyURL() << endl;

  mSubscriptionMap.insert( id, url );
}

void ExchangeMonitor::slotUnsubscribeResult( KIO::Job * job )
{
  if ( job->error() ) {
    job->showErrorDialog( 0L );
    emit error( ExchangeClient::CommunicationError, "IO Error: " + QString::number(job->error()) + ":" + job->errorString() );
    return;
  }

  QDomDocument& response = static_cast<KIO::DavJob *>( job )->response();
  kdDebug() << "UNSUBSCRIBE result: " << endl << response.toString() << endl;

  QDomElement status = response.documentElement().namedItem( "response" ).namedItem( "status" ).toElement();
  QDomElement subscriptionID = response.documentElement().namedItem( "response" ).namedItem( "subscriptionID" ).toElement();
  kdDebug() << "Subscription ID.text(): " << subscriptionID.text() << endl;
  bool ok;
  ID id = subscriptionID.text().toLong( &ok );
  if ( ! status.text().contains( "200" ) || !ok) {
    kdError() << "UNSUBSCRIBE result is not 200 or no subscription ID found" << endl;
    emit error( ExchangeClient::ServerResponseError, "UNSUBSCRIBE yields an error response: \n" + response.toString() );
  }

  mSubscriptionMap.remove( id );
}

void ExchangeMonitor::slotPollTimer()
{
  kdDebug() << "ExchangeMonitor::slotPollTimer()" << endl;
  poll( mSubscriptionMap.keys() );
}

void ExchangeMonitor::slotActivated( int )
{
  kdDebug() << "ExchangeMonitor::slotActivated()" << endl;

  kdDebug() << "Bytes available: " << mSocket->bytesAvailable() << endl;
  int maxLen = mSocket->bytesAvailable();
  if ( maxLen == 0 )
    return;

  QCString response( maxLen+2 );
  Q_LONG len = mSocket->readBlock ( response.data(), maxLen+1 );

  if ( len <= 0 ) {
    kdDebug() << "Error: len<=0" << endl;
    kdDebug() << "Error: " << mSocket->error() << endl;
    return;
  }
  kdDebug() << "Got data of " << len << " bytes." << endl;
  kdDebug() << response << endl;

  QString s(response);
  IDList IDs;

  QStringList lines = QStringList::split( "\n", s );
  QStringList::iterator it;
  for ( it = lines.begin(); it != lines.end(); ++it ) {
    QString line = (*it).stripWhiteSpace().lower();
    if ( line.startsWith( "subscription-id: " ) )
      IDs = makeIDList( line.section(":",1).stripWhiteSpace() );
  }

  if ( IDs.isEmpty() ) {
    kdWarning() << "Did not find any subscriptions in NOTIFY!" << response << endl;
  } else {
    poll( IDs );
  }

}

void ExchangeMonitor::poll( const IDList& IDs ) {
  // FIXME: Check what did subscription means
//  if ( id != mSubscriptionId ) {
//    kdDebug() << "Don't know subscription id " << id << endl;
//  }

  // confirm it
  KIO::DavJob *job = new KIO::DavJob( mAccount->calendarURL(), (int) KIO::DAV_POLL, QString::null, false );
  job->addMetaData( "customHTTPHeader", "Subscription-ID: " + makeIDString( IDs ) );
  connect(job, SIGNAL(result( KIO::Job * )), this, SLOT(slotPollResult(KIO::Job *)));
}

void ExchangeMonitor::slotPollResult( KIO::Job * job )
{
  if ( job->error() ) {
    job->showErrorDialog( 0L );
    emit error( ExchangeClient::CommunicationError, "IO Error: " + QString::number(job->error()) + ":" + job->errorString() );
    return;
  }
  QDomDocument& response = static_cast<KIO::DavJob *>( job )->response();
  kdDebug() << "POLL result: " << endl << response.toString() << endl;

  // Multiple results!
  QDomNodeList responses = response.documentElement().elementsByTagName( "response" );
  if ( responses.count() == 0 ) {
    emit error( ExchangeClient::ServerResponseError, "Poll result is wrong: \n" +  response.toString() );
    return;
  }
  for( uint i=0; i<responses.count(); i++ ) {
    QDomElement item = responses.item( i ).toElement();
    QDomElement status = item.namedItem( "status" ).toElement();
    QDomElement subscriptionID = item.namedItem( "subscriptionID" ).toElement();
    if ( status.text().contains( "200" ) ) {
      kdDebug() << "subscriptionID: " << subscriptionID.text() << endl;
      IDList IDs = makeIDList( subscriptionID.text() );
      QValueList<KURL> urls;
      IDList::ConstIterator it;
      for ( it = IDs.begin(); it != IDs.end(); ++it ) {
        urls += mSubscriptionMap[ *it ];
      }
      emit notify( IDs, urls );
    } else if ( ! status.text().contains( "204" ) ) {
      kdWarning() << "POLL result is not 200 or 204, what's up?" << endl;
      emit error( ExchangeClient::ServerResponseError, "Poll result is wrong: \n" +  response.toString() );
    }
  }
}

void ExchangeMonitor::slotRenewTimer()
{
  kdDebug() << "ExchangeMonitor::slotRenewTimer()" << endl;

  KIO::DavJob *job = new KIO::DavJob( mAccount->calendarURL(), (int) KIO::DAV_SUBSCRIBE, QString::null, false );
  job->addMetaData( "customHTTPHeader", "Subscription-id: " + makeIDString( mSubscriptionMap.keys() ) );
  connect(job, SIGNAL(result( KIO::Job * )), this, SLOT(slotRenewResult(KIO::Job *)));
}

void ExchangeMonitor::slotRenewResult( KIO::Job* job )
{
  if ( job->error() ) {
    job->showErrorDialog( 0L );
    emit error( ExchangeClient::CommunicationError, "IO Error: " + QString::number(job->error()) + ":" + job->errorString() );
    return;
  }
  kdDebug() << "ExchangeMonitor::slotRenewResult()" << endl;

  // FIXME: check for new subscription lifetime
}

#include "exchangemonitor.moc"
