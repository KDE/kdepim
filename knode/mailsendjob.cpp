/*
    Copyright (c) 2005 by Volker Krause <volker.krause@rwth-aachen.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include "mailsendjob.h"

#include "knarticle.h"
#include "knserverinfo.h"

#include <kdebug.h>

#include <Q3StrList>

KNode::MailSendJob::MailSendJob( KNJobConsumer * c, KNServerInfo * a, KNJobItem * i ) :
  KNJobData( KNJobData::JTmail, c, a, i )
{
}

void KNode::MailSendJob::execute()
{
  KNLocalArticle *art = static_cast<KNLocalArticle*>( data() );

  // create url query part
  QStringList query;
  query << "headers=0";
  query << "from=" + KURL::encode_string( art->from()->email() );
  Q3StrList emails;
  art->to()->emails( &emails );
  for ( char *e = emails.first(); e; e = emails.next() )
    query << "to=" + KURL::encode_string( e );

  // create url
  KURL destination = baseUrl();
  if ( account()->encryption() == KNServerInfo::SSL )
    destination.setProtocol( "smtps" );
  else
    destination.setProtocol( "smtp" );
  destination.setQuery( query.join( "&" ) );
  kdDebug(5003) << k_funcinfo << destination << endl;

  // create job
  KIO::Job* job = KIO::storedPut( art->encodedContent( true ), destination, -1, false, false, false );
  connect( job, SIGNAL( result(KIO::Job*) ), SLOT( slotResult(KIO::Job*) ) );
  if ( account()->encryption() == KNServerInfo::TLS )
    job->addMetaData( "TLS", "on" );
  else
    job->addMetaData( "TLS", "off" );
  setJob( job );
}

void KNode::MailSendJob::slotResult( KIO::Job * job )
{
  if ( job->error() )
    setErrorString( job->errorString() );
  emitFinished();
}

#include "mailsendjob.moc"
