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
#include <kio/job.h>


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
  query << "from=" + KUrl::toPercentEncoding( art->from()->addresses().first() );
  QList<QByteArray> emails = art->to()->addresses();
  foreach ( QByteArray to, emails )
    query << "to=" + KUrl::toPercentEncoding( to );

  // create url
  KUrl destination = baseUrl();
  if ( account()->encryption() == KNServerInfo::SSL )
    destination.setProtocol( "smtps" );
  else
    destination.setProtocol( "smtp" );
  destination.setQuery( query.join( "&" ) );
  kDebug(5003) << k_funcinfo << destination << endl;

  // create job
  KIO::Job* job = KIO::storedPut( art->encodedContent( true ), destination, -1, false, false, false );
  connect( job, SIGNAL( result(KJob*) ), SLOT( slotResult(KJob*) ) );
  setupKIOJob( job );
}

void KNode::MailSendJob::slotResult( KJob * job )
{
  if ( job->error() )
    setError( job->error(), job->errorString() );
  emitFinished();
}

#include "mailsendjob.moc"
