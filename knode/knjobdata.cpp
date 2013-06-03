/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/


#include <kdebug.h>
#include <klocale.h>
#include <kio/job.h>

#include <libkdepim/progresswidget/progressmanager.h>

#include "knarticle.h"
#include "knglobals.h"
#include "knnntpaccount.h"
#include "scheduler.h"


#include <QTimer>

KNJobConsumer::KNJobConsumer()
{
}


KNJobConsumer::~KNJobConsumer()
{
  for ( QList<KNJobData*>::Iterator it = mJobs.begin(); it != mJobs.end(); ++it )
    (*it)->c_onsumer = 0;
}


void KNJobConsumer::emitJob( KNJobData *j )
{
  if ( j ) {
    mJobs.append( j );
    knGlobals.scheduler()->addJob( j );
  }
}


void KNJobConsumer::jobDone( KNJobData *j )
{
  if ( j && mJobs.removeAll( j ) )
    processJob( j );
}

void KNJobConsumer::cancelJobs( KNJobItem::Ptr item )
{
  Q_FOREACH( KNJobData *job, mJobs ) {
    if ( job->data() == item ) {
      job->d_ata.reset();
      job->cancel();
    }
  }
}


void KNJobConsumer::processJob( KNJobData *j )
{
  delete j;
}


// the assingment of a_ccount may cause race conditions, check again.... (CG)
KNJobData::KNJobData( jobType t, KNJobConsumer *c, KNServerInfo::Ptr a, KNJobItem::Ptr i ) :
  t_ype(t), d_ata(i), a_ccount(a),
  mError( 0 ),
  mCanceled( false ),
  c_onsumer( c ),
  mJob( 0 ),
  mProgressItem( 0 )
{
  d_ata->setLocked(true);
}



KNJobData::~KNJobData()
{
  if ( d_ata ) {
    d_ata->setLocked( false );
  }
}


void KNJobData::notifyConsumer()
{

  if(c_onsumer)
    c_onsumer->jobDone(this);
  else
    delete this;
}

void KNJobData::cancel()
{
  mCanceled = true;
  if ( mJob ) {
    mJob->kill();
  }
  if ( mProgressItem ) {
    mProgressItem->setStatus( "Canceled" );
    mProgressItem->setComplete();
    mProgressItem = 0;
  }
  emitFinished();
}

void KNJobData::emitFinished()
{
  QTimer::singleShot( 0, this, SLOT(slotEmitFinished()) );
}

void KNJobData::setupKJob(KJob * job)
{
  mJob = job;
  if ( job ) {
    connect( job, SIGNAL(percent(KJob*,ulong)),
             SLOT(slotJobPercent(KJob*,ulong)) );
    connect( job, SIGNAL(infoMessage(KJob*,QString)),
             SLOT(slotJobInfoMessage(KJob*,QString)) );
  }
}

void KNJobData::setupKIOJob( KIO::Job *job )
{
  if ( job ) {
    if ( account() ) {
      if ( account()->encryption() == KNServerInfo::TLS )
        job->addMetaData( "tls", "on" );
      else
        job->addMetaData( "tls", "off" );
    }
  job->setUiDelegate(0);
  setupKJob( job );
  }
}

void KNJobData::createProgressItem()
{
  if ( mProgressItem )
    return;
  KNNntpAccount::Ptr acc = boost::static_pointer_cast<KNNntpAccount>( account() );
  QString msg = i18n( "KNode" );
  if ( type() == JTmail )
    msg = i18n( "Sending message" );
  else {
    if ( acc )
      msg = acc->name();
  }
  bool encr = false;
  if ( acc && acc->encryption() != KNServerInfo::None )
    encr = true;
  mProgressItem = KPIM::ProgressManager::createProgressItem( 0,
      KPIM::ProgressManager::getUniqueID(), msg, i18n( "Waiting..." ), true, encr );
}

void KNJobData::slotJobPercent( KJob*, unsigned long percent )
{
  kDebug(5003) <<"Progress:" << percent;
  setProgress( percent );
}

void KNJobData::slotJobInfoMessage( KJob*, const QString &msg )
{
  kDebug(5003) <<"Status:" << msg;
  setStatus( msg );
}

void KNJobData::slotEmitFinished( )
{
  emit finished( this );
}

KUrl KNJobData::baseUrl() const
{
  KUrl url;
  if ( account()->encryption() == KNServerInfo::SSL )
    url.setProtocol( "nntps" );
  else
    url.setProtocol( "nntp" );
  url.setHost( account()->server() );
  url.setPort( account()->port() );
  if ( account()->needsLogon() ) {
    url.setUser( account()->user() );
    url.setPass( account()->pass() );
  }
  return url;
}

void KNJobData::setError( int err, const QString & errMsg )
{
  mError = err;
  mErrorString = errMsg;
}

#include "knjobdata.moc"
