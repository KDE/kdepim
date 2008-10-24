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

#include <unistd.h>
#include <fcntl.h>

#include <qsocketnotifier.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kio/job.h>
#include <kio/passdlg.h>
#include <ksocks.h>
#include <kapplication.h>

#include "knaccountmanager.h"
#include "knarticle.h"
#include "knmainwidget.h"
#include "knjobdata.h"
#include "knnntpclient.h"
#include "knglobals.h"
#include "knnetaccess.h"
#include "knwidgets.h"

using KPIM::ProgressManager;


KNNetAccess::KNNetAccess(QObject *parent, const char *name )
  : QObject(parent,name), currentNntpJob(0), currentSmtpJob(0)
{
  if ( pipe(nntpInPipe) == -1 || pipe(nntpOutPipe) == -1 ) {
    KMessageBox::error(knGlobals.topWidget, i18n("Internal error:\nFailed to open pipes for internal communication."));
    kapp->exit(1);
  }
  if ( fcntl( nntpInPipe[0], F_SETFL, O_NONBLOCK ) == -1 ||
       fcntl( nntpOutPipe[0], F_SETFL, O_NONBLOCK ) == -1 ) {
    KMessageBox::error(knGlobals.topWidget, i18n("Internal error:\nFailed to open pipes for internal communication."));
    kapp->exit(1);
  }

  nntpNotifier=new QSocketNotifier(nntpInPipe[0], QSocketNotifier::Read);
  connect(nntpNotifier, SIGNAL(activated(int)), this, SLOT(slotThreadSignal(int)));

  // initialize the KSocks stuff in the main thread, otherwise we get
  // strange effects on FreeBSD
  (void) KSocks::self();

  nntpClient=new KNNntpClient(nntpOutPipe[0],nntpInPipe[1],nntp_Mutex);
  nntpClient->start();

  connect( knGlobals.accountManager(), SIGNAL(passwordsChanged()), SLOT(slotPasswordsChanged()) );
}



KNNetAccess::~KNNetAccess()
{
  disconnect(nntpNotifier, SIGNAL(activated(int)), this, SLOT(slotThreadSignal(int)));

  nntpClient->terminateClient();
  triggerAsyncThread(nntpOutPipe[1]);
  nntpClient->wait();

  delete nntpClient;
  delete nntpNotifier;

  if ( ::close(nntpInPipe[0]) == -1 ||
       ::close(nntpInPipe[1]) == -1 ||
       ::close(nntpOutPipe[0]) == -1 ||
       ::close(nntpOutPipe[1]) == -1 )
    kdDebug(5003) << "Can't close pipes" << endl;
}



void KNNetAccess::addJob(KNJobData *job)
{
  // kdDebug(5003) << "KNNetAccess::addJob() : job queued" << endl;
  if(job->account()==0) {
    job->setErrorString(i18n("Internal Error: No account set for this job."));
    job->notifyConsumer();
    return;
  }

  job->createProgressItem();
  connect( job->progressItem(), SIGNAL(progressItemCanceled(KPIM::ProgressItem*)), SLOT(slotCancelJob(KPIM::ProgressItem*)) );
  emit netActive( true );

  // put jobs which are waiting for the wallet into an extra queue
  if ( !job->account()->readyForLogin() ) {
    mWalletQueue.append( job );
    knGlobals.accountManager()->loadPasswordsAsync();
    job->setStatus( i18n( "Waiting for KWallet..." ) );
    return;
  }

  if (job->type()==KNJobData::JTmail) {
    smtpJobQueue.append(job);
    if (!currentSmtpJob)   // no active job, start the new one
      startJobSmtp();
  } else {

    /*
        TODO: the following code doesn't really belong here, it should
              be moved to KNGroupManager, or elsewere...
    */

    // avoid duplicate fetchNewHeader jobs...
    bool duplicate = false;
    if ( job->type() == KNJobData::JTfetchNewHeaders || job->type() == KNJobData::JTsilentFetchNewHeaders ) {
      QValueList<KNJobData*>::ConstIterator it;
      for ( it = nntpJobQueue.begin(); it != nntpJobQueue.end(); ++it ) {
        if ( ( (*it)->type() == KNJobData::JTfetchNewHeaders || (*it)->type() == KNJobData::JTsilentFetchNewHeaders )
          && (*it)->data() == job->data() ) // job works on the same group...
          duplicate = true;
      }
    }

    if (!duplicate) {
      // give a lower priority to fetchNewHeaders and postArticle jobs
      if ( job->type() == KNJobData::JTfetchNewHeaders
           || job->type() == KNJobData::JTsilentFetchNewHeaders
           || job->type() == KNJobData::JTpostArticle ) {
        nntpJobQueue.append( job );
      } else {
        nntpJobQueue.prepend( job );
      }

      if (!currentNntpJob)   // no active job, start the new one
        startJobNntp();
    }
  }
  updateStatus();
}


void KNNetAccess::cancelCurrentNntpJob( int type )
{
  if ((currentNntpJob && !currentNntpJob->canceled()) && ((type==0)||(currentNntpJob->type()==type))) {   // stop active job
    currentNntpJob->cancel();
    triggerAsyncThread(nntpOutPipe[1]);
  }
}


void KNNetAccess::stopJobsNntp( int type )
{
  cancelCurrentNntpJob( type );
  KNJobData *tmp = 0;
  QValueList<KNJobData*>::Iterator it;
  for ( it = nntpJobQueue.begin(); it != nntpJobQueue.end();) {
    tmp = *it;
    if ( type == 0 || tmp->type() == type ) {
      it = nntpJobQueue.remove( it );
      tmp->cancel();
      tmp->notifyConsumer();
    } else
      ++it;
  }
  for ( it = mWalletQueue.begin(); it != mWalletQueue.end();) {
      tmp = *it;
    if ( type == 0 || tmp->type() == type ) {
      it = mWalletQueue.remove( it );
      tmp->cancel();
      tmp->notifyConsumer();
    } else
      ++it;
  }
  updateStatus();
}



// type==0 => all jobs
void KNNetAccess::cancelCurrentSmtpJob( int type )
{
  if ((currentSmtpJob && !currentSmtpJob->canceled()) && ((type==0)||(currentSmtpJob->type()==type))) {    // stop active job
    currentSmtpJob->cancel();
    threadDoneSmtp();
  }
}


void KNNetAccess::stopJobsSmtp( int type )
{
  cancelCurrentSmtpJob( type );
  KNJobData *tmp = 0;
  QValueList<KNJobData*>::Iterator it;
  for ( it = smtpJobQueue.begin(); it != smtpJobQueue.end();) {
    tmp = *it;
    if ( type == 0 || tmp->type() == type ) {
      it = smtpJobQueue.remove( it );
      tmp->cancel();
      tmp->notifyConsumer();
    } else
      ++it;
  }
  updateStatus();
}



// passes a signal through the ipc-pipe to the net-thread
void KNNetAccess::triggerAsyncThread(int pipeFd)
{
  int signal=0;

  // kdDebug(5003) << "KNNetAccess::triggerAsyncThread() : sending signal to net thread" << endl;
  write(pipeFd, &signal, sizeof(int));
}



void KNNetAccess::startJobNntp()
{
  if ( nntpJobQueue.isEmpty() )
    return;

  currentNntpJob = nntpJobQueue.first();
  nntpJobQueue.remove( nntpJobQueue.begin() );
  currentNntpJob->prepareForExecution();
  if (currentNntpJob->success()) {
    nntpClient->insertJob(currentNntpJob);
    triggerAsyncThread(nntpOutPipe[1]);
    kdDebug(5003) << "KNNetAccess::startJobNntp(): job started" << endl;
  } else {
    threadDoneNntp();
  }
}



void KNNetAccess::startJobSmtp()
{
  if ( smtpJobQueue.isEmpty() )
    return;

  currentSmtpJob = smtpJobQueue.first();
  smtpJobQueue.remove( smtpJobQueue.begin() );
  currentSmtpJob->prepareForExecution();
  if (currentSmtpJob->success()) {
    KNLocalArticle *art = static_cast<KNLocalArticle*>( currentSmtpJob->data() );
    // create url query part
    QString query("headers=0&from=");
    query += KURL::encode_string( art->from()->email() );
    QStrList emails;
    art->to()->emails( &emails );
    for ( char *e = emails.first(); e; e = emails.next() ) {
      query += "&to=" + KURL::encode_string( e );
    }
    // create url
    KURL destination;
    KNServerInfo *account = currentSmtpJob->account();
    if ( account->encryption() == KNServerInfo::SSL )
      destination.setProtocol( "smtps" );
    else
      destination.setProtocol( "smtp" );
    destination.setHost( account->server() );
    destination.setPort( account->port() );
    destination.setQuery( query );
    if ( account->needsLogon() ) {
      destination.setUser( account->user() );
      destination.setPass( account->pass() );
    }
    KIO::Job* job = KIO::storedPut( art->encodedContent(true), destination, -1, false, false, false );
    connect( job, SIGNAL( result(KIO::Job*) ),
             SLOT( slotJobResult(KIO::Job*) ) );
    if ( account->encryption() == KNServerInfo::TLS )
      job->addMetaData( "tls", "on" );
    else
      job->addMetaData( "tls", "off" );
    currentSmtpJob->setJob( job );

    kdDebug(5003) << "KNNetAccess::startJobSmtp(): job started" << endl;
  } else {
    threadDoneSmtp();
  }
}



void KNNetAccess::threadDoneNntp()
{
  KNJobData *tmp;
  if (!currentNntpJob) {
    kdWarning(5003) << "KNNetAccess::threadDoneNntp(): no current job?? aborting" << endl;
    return;
  }

  kdDebug(5003) << "KNNetAccess::threadDoneNntp(): job done" << endl;

  tmp = currentNntpJob;

  if (!tmp->success() && tmp->authError()) {
    kdDebug(5003) << "KNNetAccess::threadDoneNntp(): authentication error" << endl;
    KNServerInfo *info = tmp->account();
    if (info) {
      QString user = info->user();
      QString pass = info->pass();
      bool keep=false;
      if (KDialog::Accepted == KIO::PasswordDialog::getNameAndPassword(user, pass, &keep,
                                 i18n("You need to supply a username and a\npassword to access this server"), false,
                                 kapp->makeStdCaption(i18n("Authentication Failed")),info->server(),i18n("Server:"))) {
        info->setNeedsLogon(true);
        info->setUser(user);
        info->setPass(pass);
        tmp->setAuthError(false);
        tmp->setErrorString(QString::null);

        kdDebug(5003) << "KNNetAccess::threadDoneNntp(): trying again with authentication data" << endl;

        // restart job...
        triggerAsyncThread(nntpOutPipe[1]);
        return;
      }
    }
  }

  nntpClient->removeJob();
  currentNntpJob = 0L;

  currMsg = QString::null;
  knGlobals.setStatusMsg();
  tmp->setComplete();

  tmp->notifyConsumer();

  if (!nntpJobQueue.isEmpty())
    startJobNntp();

  updateStatus();
}



void KNNetAccess::threadDoneSmtp()
{
  KNJobData *tmp;
  if (!currentSmtpJob) {
    kdWarning(5003) << "KNNetAccess::threadDoneSmtp(): no current job?? aborting" << endl;
    return;
  }

  kdDebug(5003) << "KNNetAccess::threadDoneSmtp(): job done" << endl;

  tmp = currentSmtpJob;
  currentSmtpJob = 0L;
  if (!currentNntpJob) {
    currMsg = QString::null;
    knGlobals.setStatusMsg();
  }
  tmp->setComplete();

  tmp->notifyConsumer();

  if (!smtpJobQueue.isEmpty())
    startJobSmtp();

  updateStatus();
}


void KNNetAccess::cancelAllJobs()
{
  stopJobsNntp(0);
  stopJobsSmtp(0);
}



void KNNetAccess::slotThreadSignal(int i)
{
  int signal;
  QString tmp;

  //kdDebug(5003) << "KNNetAccess::slotThreadSignal() : signal received from net thread" << endl;
  if(read(i, &signal, sizeof(int))==-1) {
    kdDebug(5003) << "KNNetAccess::slotThreadSignal() : cannot read from pipe" << endl;
    return;
  }

  if (i == nntpInPipe[0]) {      // signal from nntp thread
    switch(signal) {
      case KNProtocolClient::TSworkDone:
        threadDoneNntp();
      break;
      case KNProtocolClient::TSconnect:
        currMsg = i18n(" Connecting to server...");
        knGlobals.setStatusMsg(currMsg);
        currentNntpJob->setStatus(currMsg);
      break;
      case KNProtocolClient::TSloadGrouplist:
        currMsg = i18n(" Loading group list from disk...");
        knGlobals.setStatusMsg(currMsg);
        currentNntpJob->setStatus(currMsg);
      break;
      case KNProtocolClient::TSwriteGrouplist:
        currMsg = i18n(" Writing group list to disk...");
        knGlobals.setStatusMsg(currMsg);
        currentNntpJob->setStatus(currMsg);
      break;
      case KNProtocolClient::TSdownloadGrouplist:
        currMsg = i18n(" Downloading group list...");
        knGlobals.setStatusMsg(currMsg);
        currentNntpJob->setStatus(currMsg);
      break;
      case KNProtocolClient::TSdownloadNewGroups:
        currMsg = i18n(" Looking for new groups...");
        knGlobals.setStatusMsg(currMsg);
        currentNntpJob->setStatus(currMsg);
      break;
      case KNProtocolClient::TSdownloadDesc:
        currMsg = i18n(" Downloading group descriptions...");
        knGlobals.setStatusMsg(currMsg);
        currentNntpJob->setStatus(currMsg);
      break;
      case KNProtocolClient::TSdownloadNew:
        currMsg = i18n(" Downloading new headers...");
        knGlobals.setStatusMsg(currMsg);
        currentNntpJob->setStatus(currMsg);
      break;
      case KNProtocolClient::TSsortNew:
        currMsg = i18n(" Sorting...");
        knGlobals.setStatusMsg(currMsg);
        currentNntpJob->setStatus(currMsg);
      break;
      case KNProtocolClient::TSdownloadArticle:
        currMsg = i18n(" Downloading article...");
        knGlobals.setStatusMsg(currMsg);
        currentNntpJob->setStatus(currMsg);
      break;
      case KNProtocolClient::TSsendArticle:
        currMsg = i18n(" Sending article...");
        knGlobals.setStatusMsg(currMsg);
        currentNntpJob->setStatus(currMsg);
      break;
      case KNProtocolClient::TSjobStarted:
        currentNntpJob->setProgress(0);
      break;
      case KNProtocolClient::TSprogressUpdate:
          currentNntpJob->setProgress(nntpClient->getProgressValue()/10);
      break;
    };
  }
}


void KNNetAccess::slotJobResult( KIO::Job *job )
{
  if ( job == currentSmtpJob->job() ) {
    if ( job->error() )
      currentSmtpJob->setErrorString( job->errorString() );
    threadDoneSmtp();
    return;
  }
  if ( job == currentNntpJob->job() ) {
    // TODO
    return;
  }
  kdError(5003) << k_funcinfo << "unknown job" << endl;
}


void KNNetAccess::slotPasswordsChanged()
{
  QValueList<KNJobData*>::ConstIterator it;
  for ( it = mWalletQueue.begin(); it != mWalletQueue.end(); ++it ) {
    (*it)->setStatus( i18n("Waiting...") );
    if ( (*it)->type() == KNJobData::JTmail )
      smtpJobQueue.append( (*it) );
    else
      nntpJobQueue.append( (*it) );
  }
  mWalletQueue.clear();
  if ( !currentNntpJob )
    startJobNntp();
  if ( !currentSmtpJob )
    startJobSmtp();
}


void KNNetAccess::slotCancelJob( KPIM::ProgressItem *item )
{
  KNJobData *tmp = 0;
  QValueList<KNJobData*>::Iterator it;
  for ( it = nntpJobQueue.begin(); it != nntpJobQueue.end();) {
    tmp = *it;
    if ( tmp->progressItem() == item ) {
      it = nntpJobQueue.remove( it );
      tmp->cancel();
      tmp->notifyConsumer();
    } else
      ++it;
  }
  for ( it = smtpJobQueue.begin(); it != smtpJobQueue.end();) {
    tmp = *it;
    if ( tmp->progressItem() == item ) {
      it = smtpJobQueue.remove( it );
      tmp->cancel();
      tmp->notifyConsumer();
    } else
      ++it;
  }
  for ( it = mWalletQueue.begin(); it != mWalletQueue.end();) {
    tmp = *it;
    if ( tmp->progressItem() == item ) {
      it = mWalletQueue.remove( it );
      tmp->cancel();
      tmp->notifyConsumer();
    } else
      ++it;
  }

  if ( currentNntpJob && currentNntpJob->progressItem() == item )
    cancelCurrentNntpJob();
  if ( currentSmtpJob && currentSmtpJob->progressItem() == item )
    cancelCurrentSmtpJob();

  updateStatus();
}

void KNNetAccess::updateStatus( )
{
  if ( nntpJobQueue.isEmpty() && smtpJobQueue.isEmpty() && !currentNntpJob
       && !currentSmtpJob && mWalletQueue.isEmpty() )
    emit netActive( false );
  else
    emit netActive( true );
}

//--------------------------------

#include "knnetaccess.moc"
