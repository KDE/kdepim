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

#include <tqstring.h>
#include <tqregexp.h>

#include <kurl.h>
#include <kdebug.h>
#include <krfcdate.h>
#include <kio/job.h>
#include <kio/jobclasses.h>

#include <kio/slave.h>
#include <kio/scheduler.h>
#include <kio/slavebase.h>
#include <kio/davjob.h>
#include <kio/http.h>

#include "exchangeclient.h"
#include "exchangeprogress.h"
#include "exchangedelete.h"
#include "exchangeaccount.h"
#include "utils.h"

using namespace KPIM;

// Delete:
// - Find URL for uid
// - Delete URL
// - Can there be multipe URLs, for instance when dealing with 
// recurrent appointments? Maybe, so we just look for Master or Single
// instancetypes

ExchangeDelete::ExchangeDelete( KCal::Event* event, ExchangeAccount* account, TQWidget* window ) :
  mWindow( window )
{
  kdDebug() << "Created ExchangeDelete" << endl;

  mAccount = account;

  findUidSingleMaster( event->uid() );
}

ExchangeDelete::~ExchangeDelete()
{
  kdDebug() << "ExchangeDelete destructor" << endl;
}

void ExchangeDelete::findUidSingleMaster( TQString const& uid )
{
  TQString query = 
        "SELECT \"DAV:href\", \"urn:schemas:calendar:uid\"\r\n"
        "FROM Scope('shallow traversal of \"\"')\r\n"
        "WHERE \"urn:schemas:calendar:uid\" = '" + uid + "'\r\n"
	" AND (\"urn:schemas:calendar:instancetype\" = 0\r\n"
	"      OR \"urn:schemas:calendar:instancetype\" = 1)\r\n";

  KIO::DavJob* job = KIO::davSearch( mAccount->calendarURL(), "DAV:", "sql", query, false );
  job->setWindow( mWindow );
  connect(job, TQT_SIGNAL(result( KIO::Job * )), this, TQT_SLOT(slotFindUidResult(KIO::Job *)));
}

void ExchangeDelete::slotFindUidResult( KIO::Job * job )
{
  if ( job->error() ) {
    job->showErrorDialog( 0L );
    emit finished( this, ExchangeClient::CommunicationError, "IO Error: " + TQString::number(job->error()) + ":" + job->errorString() );
    return;
  }
  TQDomDocument& response = static_cast<KIO::DavJob *>( job )->response();

  TQDomElement item = response.documentElement().firstChild().toElement();
  TQDomElement hrefElement = item.namedItem( "href" ).toElement();
  if ( item.isNull() || hrefElement.isNull() ) {
    // Not found
    emit finished( this, ExchangeClient::DeleteUnknownEventError, "UID of event to be deleted not found on server\n"+response.toString() );
    return;
  }
  // Found the appointment's URL
  TQString href = hrefElement.text();
  KURL url(href);

  startDelete( toDAV( url ) );  
}  

void ExchangeDelete::startDelete( const KURL& url )
{
  KIO::SimpleJob* job = KIO::file_delete( url, false ); // no GUI
  job->setWindow( mWindow );
  connect( job, TQT_SIGNAL( result( KIO::Job * ) ), this, TQT_SLOT( slotDeleteResult( KIO::Job * ) ) );
}

void ExchangeDelete::slotDeleteResult( KIO::Job* job )
{
  kdDebug() << "Finished Delete" << endl;
  if ( job->error() ) {
    job->showErrorDialog( 0L );
    emit finished( this, ExchangeClient::CommunicationError, "IO Error: " + TQString::number(job->error()) + ":" + job->errorString() );
    return;
  }
  emit finished( this, ExchangeClient::ResultOK, TQString::null );
}

#include "exchangedelete.moc"
