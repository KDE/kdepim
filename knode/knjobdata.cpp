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

#include <libkdepim/progressmanager.h>

#include "knarticle.h"
#include "knglobals.h"
#include "knnetaccess.h"
#include "knnntpaccount.h"

#include <qstylesheet.h>

KNJobConsumer::KNJobConsumer()
{
}


KNJobConsumer::~KNJobConsumer()
{
  QValueList<KNJobData*>::Iterator it;
  for ( it = mJobs.begin(); it != mJobs.end(); ++it )
    (*it)->c_onsumer = 0;
}


void KNJobConsumer::emitJob( KNJobData *j )
{
  if ( j ) {
    mJobs.append( j );
    knGlobals.netAccess()->addJob( j );
  }
}


void KNJobConsumer::jobDone( KNJobData *j )
{
  if ( j && mJobs.remove( j ) )
    processJob( j );
}


void KNJobConsumer::processJob( KNJobData *j )
{
  delete j;
}


// the assingment of a_ccount may cause race conditions, check again.... (CG)
KNJobData::KNJobData(jobType t, KNJobConsumer *c, KNServerInfo *a, KNJobItem *i)
 : t_ype(t), d_ata(i), a_ccount(a), c_anceled(false), a_uthError(false), c_onsumer(c),
  mJob( 0 ),
  mProgressItem( 0 )
{
  d_ata->setLocked(true);
}



KNJobData::~KNJobData()
{
  d_ata->setLocked(false);
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
  c_anceled = true;
  if ( mJob ) {
    mJob->kill();
    mJob = 0;
  }
  if ( mProgressItem ) {
    mProgressItem->setStatus( "Canceled" );
    mProgressItem->setComplete();
    mProgressItem = 0;
  }
}

void KNJobData::setJob( KIO::Job *job )
{
  mJob = job;
  if ( job ) {
    connect( job, SIGNAL( percent(KIO::Job*, unsigned long) ),
             SLOT( slotJobPercent(KIO::Job*, unsigned long) ) );
    connect( job, SIGNAL( infoMessage(KIO::Job*, const QString&) ),
             SLOT( slotJobInfoMessage(KIO::Job*, const QString&) ) );
  }
}

void KNJobData::createProgressItem()
{
  if ( mProgressItem )
    return;
  KNNntpAccount *acc = static_cast<KNNntpAccount*>( account() );
  QString msg = i18n( "KNode" );
  if ( type() == JTmail )
    msg = i18n( "Sending message" );
  else {
    if ( acc )
      msg = QStyleSheet::escape( acc->name() );
  }
  bool encr = false;
  if ( acc && acc->encryption() != KNServerInfo::None )
    encr = true;
  mProgressItem = KPIM::ProgressManager::createProgressItem( 0,
      KPIM::ProgressManager::getUniqueID(), msg, i18n( "Waiting..." ), true, encr );
}

void KNJobData::slotJobPercent( KIO::Job*, unsigned long percent )
{
  kdDebug(5003) << k_funcinfo << "Progress: " << percent << endl;
  setProgress( percent );
}

void KNJobData::slotJobInfoMessage( KIO::Job*, const QString &msg )
{
  kdDebug(5003) << k_funcinfo << "Status: " << msg << endl;
  setStatus( msg );
}


#include "knjobdata.moc"
