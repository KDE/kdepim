/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2006 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include <klocale.h>
#include <kdebug.h>
#include <kpassworddialog.h>

#include "knaccountmanager.h"
#include "knarticle.h"
#include "knglobals.h"
#include "kngroupmanager.h"
#include "knjobdata.h"
#include "scheduler.h"
#include "knserverinfo.h"

using namespace KNode;
using KPIM::ProgressManager;
using KPIM::ProgressItem;


Scheduler::Scheduler( QObject *parent ) :
  QObject( parent ),
  currentNntpJob( 0 ),
  currentSmtpJob( 0 )
{
  connect( knGlobals.accountManager(), SIGNAL(passwordsChanged()), SLOT(slotPasswordsChanged()) );
}


Scheduler::~Scheduler()
{
}


void Scheduler::addJob(KNJobData *job)
{
  // kDebug(5003) <<"Scheduler::addJob() : job queued";
  if ( job->type() != KNJobData::JTmail && job->account() == 0 ) {
    job->setError( KIO::ERR_INTERNAL, i18n("Internal Error: No account set for this job.") );
    job->notifyConsumer();
    return;
  }

  job->createProgressItem();
  connect( job->progressItem(), SIGNAL(progressItemCanceled(KPIM::ProgressItem*)), SLOT(slotCancelJob(KPIM::ProgressItem*)) );
  emit netActive( true );

  // put jobs which are waiting for the wallet into an extra queue
  if ( job->account() && !job->account()->readyForLogin() ) {
    kDebug(5003) <<"Job waits for KWallet.";
    mWalletQueue.append( job );
    knGlobals.accountManager()->loadPasswordsAsync();
    job->setStatus( i18n( "Waiting for KWallet..." ) );
    return;
  }

  if (job->type()==KNJobData::JTmail) {
    smtpJobs.append( job );
    startJob( job );
  } else {

    /*
        TODO: the following code doesn't really belong here, it should
              be moved to KNGroupManager, or elsewere...
    */

    // avoid duplicate fetchNewHeader jobs...
    bool duplicate = false;
    if ( job->type() == KNJobData::JTfetchNewHeaders ) {
      QList<KNJobData*>::ConstIterator it;
      for ( it = nntpJobQueue.constBegin(); it != nntpJobQueue.constEnd(); ++it ) {
        if ( ( (*it)->type() == KNJobData::JTfetchNewHeaders )
          && (*it)->data() == job->data() ) // job works on the same group...
          duplicate = true;
      }
    }

    if (!duplicate) {
      // give a lower priority to fetchNewHeaders and postArticle jobs
      if ( job->type() == KNJobData::JTfetchNewHeaders
           || job->type() == KNJobData::JTpostArticle ) {
        nntpJobQueue.append( job );
      } else {
        nntpJobQueue.prepend( job );
      }
    }
  }
  schedule();
  updateStatus();
}


void Scheduler::schedule()
{
  if ( !currentNntpJob && !nntpJobQueue.isEmpty() ) {
    currentNntpJob = nntpJobQueue.first();
    nntpJobQueue.removeFirst();
    startJob( currentNntpJob );
  }
}


void Scheduler::startJob( KNJobData * job )
{
  job->prepareForExecution();
  if ( job->success() ) {
    connect( job, SIGNAL(finished(KNJobData*)),
             SLOT(slotJobFinished(KNJobData*)) );
    job->execute();
  } else
    slotJobFinished( job );
}


void Scheduler::cancelJobs( int type, KPIM::ProgressItem * item )
{
  KNJobData *tmp = 0;
  QList<KNJobData*>::Iterator it;
  for ( it = nntpJobQueue.begin(); it != nntpJobQueue.end();) {
    tmp = *it;
    if ( ( item && tmp->progressItem() == item ) || type == 0 || type == tmp->type() ) {
      it = nntpJobQueue.erase( it );
      tmp->cancel();
      tmp->notifyConsumer();
    } else
      ++it;
  }
  for ( it = smtpJobs.begin(); it != smtpJobs.end();) {
    tmp = *it;
    if ( ( item && tmp->progressItem() == item ) || type == 0 || type == tmp->type() ) {
      it = smtpJobs.erase( it );
      tmp->cancel();
      tmp->notifyConsumer();
    } else
      ++it;
  }
  for ( it = mWalletQueue.begin(); it != mWalletQueue.end();) {
    tmp = *it;
    if ( ( item && tmp->progressItem() == item ) || type == 0 || type == tmp->type() ) {
      it = mWalletQueue.erase( it );
      tmp->cancel();
      tmp->notifyConsumer();
    } else
      ++it;
  }

  if ( currentNntpJob )
    if ( ( item && currentNntpJob->progressItem() == item ) || type == 0 || type == currentNntpJob->type() )
      currentNntpJob->cancel();

  updateStatus();
}


void Scheduler::slotJobFinished( KNJobData * job )
{
  // handle authentication errors, ie. request password and try again
  if ( job->error() == KIO::ERR_COULD_NOT_LOGIN ) {
    kDebug(5003) <<"authentication error";
    KNServerInfo::Ptr account = job->account();
    if ( account ) {
      QString user = account->user();
      QString pass = account->pass();

      KPasswordDialog dlg( 0, KPasswordDialog::ShowUsernameLine );
      dlg.setUsername( user );
      dlg.setPassword( pass );
      dlg.setKeepPassword( false );
      dlg.setPrompt( i18n( "You need to supply a username and a\npassword to access this server" ) );
      dlg.setUsernameReadOnly( false );
      dlg.setCaption( i18n( "Authentication Failed" ) );
      dlg.addCommentLine( i18n( "Server:" ), account->server() );

      if ( dlg.exec() == KDialog::Accepted ) {
        account->setNeedsLogon( true );
        account->setUser( user );
        account->setPass( pass );
        job->setError( 0, QString() );
        // restart job
        job->execute();
        return;
      }
    }
  }

  if ( currentNntpJob && job == currentNntpJob )
    currentNntpJob = 0;
  smtpJobs.removeAll( job );

  job->setComplete();
  job->notifyConsumer();

  schedule();
  updateStatus();
}


void Scheduler::slotPasswordsChanged()
{
  QList<KNJobData*>::ConstIterator it;
  for ( it = mWalletQueue.constBegin(); it != mWalletQueue.constEnd(); ++it ) {
    (*it)->setStatus( i18n("Waiting...") );
    Q_ASSERT( (*it)->type() != KNJobData::JTmail );
    nntpJobQueue.append( (*it) );
  }
  mWalletQueue.clear();
  schedule();
}


void Scheduler::slotCancelJob( KPIM::ProgressItem *item )
{
  cancelJobs( 0, item );
}


void Scheduler::updateStatus( )
{
  if ( nntpJobQueue.isEmpty() && smtpJobs.isEmpty() && !currentNntpJob
       && mWalletQueue.isEmpty() )
    emit netActive( false );
  else
    emit netActive( true );
}


