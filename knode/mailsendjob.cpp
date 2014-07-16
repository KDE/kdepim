/*
    Copyright (c) 2005 by Volker Krause <vkrause@kde.org>

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

#include <mailtransport/transportmanager.h>
#include <mailtransport/transportjob.h>

#include <kdebug.h>
#include <klocale.h>
#include <kio/job.h>

using namespace MailTransport;

KNode::MailSendJob::MailSendJob( KNJobConsumer * c, int transportId, KNJobItem::Ptr i ) :
  KNJobData( KNJobData::JTmail, c, KNServerInfo::Ptr(), i ),
  mTransportId( transportId )
{
}

void KNode::MailSendJob::execute()
{
  KNLocalArticle::Ptr art = boost::static_pointer_cast<KNLocalArticle>( data() );

  TransportJob* job = TransportManager::self()->createTransportJob( mTransportId );
  if ( !job ) {
    setError( KIO::ERR_INTERNAL, i18n("Could not create mail transport job.") );
    emitFinished();
    return;
  }

  job->setData( art->encodedContent( true ) );
  job->setSender( art->from()->addresses().first() );

  // FIXME
  QStringList to;
  foreach ( const QByteArray &b, art->to()->addresses() ) {
    to << QString::fromLatin1( b );
  }
  job->setTo( to );

  connect( job, SIGNAL(result(KJob*)), SLOT(slotResult(KJob*)) );
  setupKJob( job );
  TransportManager::self()->schedule( job );
}

void KNode::MailSendJob::slotResult( KJob * job )
{
  if ( job->error() )
    setError( job->error(), job->errorString() );
  emitFinished();
}

