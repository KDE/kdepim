/*
    knnetaccess.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2004 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include <unistd.h>
#include <fcntl.h>

#include <qsocketnotifier.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kio/passdlg.h>
#include <ksocks.h>
#include <kapplication.h>

#include "progressmanager.h"

#include "knmainwidget.h"
#include "knjobdata.h"
#include "knnntpclient.h"
#include "knsmtpclient.h"
#include "knglobals.h"
#include "knnetaccess.h"
#include "knwidgets.h"

using KPIM::ProgressManager;


KNNetAccess::KNNetAccess(QObject *parent, const char *name )
  : QObject(parent,name), currentNntpJob(0), currentSmtpJob(0),
    mNNTPProgressItem(0), mSMTPProgressItem(0)
{
  if((pipe(nntpInPipe)==-1)||
     (pipe(nntpOutPipe)==-1)||
     (pipe(smtpInPipe)==-1)||
     (pipe(smtpOutPipe)==-1)) {
    KMessageBox::error(knGlobals.topWidget, i18n("Internal error:\nFailed to open pipes for internal communication."));
    kapp->exit(1);
  }
  if((fcntl(nntpInPipe[0],F_SETFL,O_NONBLOCK)==-1)||
     (fcntl(nntpOutPipe[0],F_SETFL,O_NONBLOCK)==-1)||
     (fcntl(smtpInPipe[0],F_SETFL,O_NONBLOCK)==-1)||
     (fcntl(smtpOutPipe[0],F_SETFL,O_NONBLOCK)==-1)) {
    KMessageBox::error(knGlobals.topWidget, i18n("Internal error:\nFailed to open pipes for internal communication."));
    kapp->exit(1);
  }

  nntpNotifier=new QSocketNotifier(nntpInPipe[0], QSocketNotifier::Read);
  connect(nntpNotifier, SIGNAL(activated(int)), this, SLOT(slotThreadSignal(int)));
  smtpNotifier=new QSocketNotifier(smtpInPipe[0], QSocketNotifier::Read);
  connect(smtpNotifier, SIGNAL(activated(int)), this, SLOT(slotThreadSignal(int)));

  // initialize the KSocks stuff in the main thread, otherwise we get
  // strange effects on FreeBSD
  (void) KSocks::self();

  nntpClient=new KNNntpClient(nntpOutPipe[0],nntpInPipe[1],nntp_Mutex);
  smtpClient=new KNSmtpClient(smtpOutPipe[0],smtpInPipe[1]);

  nntpClient->start();
  smtpClient->start();

  nntpJobQueue.setAutoDelete(false);
  smtpJobQueue.setAutoDelete(false);
}



KNNetAccess::~KNNetAccess()
{
  disconnect(nntpNotifier, SIGNAL(activated(int)), this, SLOT(slotThreadSignal(int)));
  disconnect(smtpNotifier, SIGNAL(activated(int)), this, SLOT(slotThreadSignal(int)));

  if(mNNTPProgressItem)
    mNNTPProgressItem->setComplete();
  if(mSMTPProgressItem)
    mSMTPProgressItem->setComplete();

  nntpClient->terminate();
  nntpClient->wait();
  smtpClient->terminate();
  smtpClient->wait();

  delete nntpClient;
  delete smtpClient;
  delete nntpNotifier;
  delete smtpNotifier;

  if ((::close(nntpInPipe[0]) == -1)||
      (::close(nntpInPipe[1]) == -1)||
      (::close(nntpOutPipe[0]) == -1)||
      (::close(nntpOutPipe[1]) == -1)||
      (::close(smtpInPipe[0]) == -1)||
      (::close(smtpInPipe[1]) == -1)||
      (::close(smtpOutPipe[0]) == -1)||
      (::close(smtpOutPipe[1]) == -1))
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

  // make sure the account has its password loaded
  // this allows on-demand wallet opening
  job->account()->pass();

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
    if (job->type()==KNJobData::JTfetchNewHeaders || job->type()==KNJobData::JTsilentFetchNewHeaders) {
      for (KNJobData *j = nntpJobQueue.first(); j; j = nntpJobQueue.next())
        if ((j->type()==KNJobData::JTfetchNewHeaders || j->type()==KNJobData::JTsilentFetchNewHeaders) &&
             j->data() == job->data())     // job works on the same group...
          duplicate = true;
    }

    if (!duplicate) {
      // give a lower priority to fetchNewHeaders and postArticle jobs
      if (job->type()==KNJobData::JTfetchNewHeaders || job->type()==KNJobData::JTsilentFetchNewHeaders || job->type()==KNJobData::JTpostArticle)
        nntpJobQueue.append(job);
      else
        nntpJobQueue.prepend(job);

      if (!currentNntpJob)   // no active job, start the new one
        startJobNntp();
    }
  }
}


// type==0 => all jobs
void KNNetAccess::stopJobsNntp(int type)
{
  if ((currentNntpJob && !currentNntpJob->canceled()) && ((type==0)||(currentNntpJob->type()==type))) {   // stop active job
    currentNntpJob->cancel();
    triggerAsyncThread(nntpOutPipe[1]);
  }

  KNJobData *tmp;                          // kill waiting jobs
  KNJobData *start = nntpJobQueue.first();
  do {
    if (!nntpJobQueue.isEmpty()) {
      tmp=nntpJobQueue.take(0);
      if ((type==0)||(tmp->type()==type)) {
        tmp->cancel();
        tmp->notifyConsumer();
      } else
        nntpJobQueue.append(tmp);
    }
  } while(!nntpJobQueue.isEmpty() && (start != nntpJobQueue.first()));
}



// type==0 => all jobs
void KNNetAccess::stopJobsSmtp(int type)
{
  if ((currentSmtpJob && !currentSmtpJob->canceled()) && ((type==0)||(currentSmtpJob->type()==type))) {    // stop active job
    currentSmtpJob->cancel();
    triggerAsyncThread(smtpOutPipe[1]);
  }

  KNJobData *tmp;                          // kill waiting jobs
  KNJobData *start = smtpJobQueue.first();
  do {
    if (!smtpJobQueue.isEmpty()) {
      tmp=smtpJobQueue.take(0);
      if ((type==0)||(tmp->type()==type)) {
        tmp->cancel();
        tmp->notifyConsumer();
      } else
        smtpJobQueue.append(tmp);
    }
  } while(!smtpJobQueue.isEmpty() && (start != smtpJobQueue.first()));
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
  if (nntpJobQueue.isEmpty()) {
    kdWarning(5003) << "KNNetAccess::startJobNntp(): job queue is empty?? aborting" << endl;
    return;
  }

  mNNTPProgressItem = ProgressManager::createProgressItem(
      0, "NNTP", i18n("KNode NNTP"), QString::null, true, false );
  connect(mNNTPProgressItem, SIGNAL(progressItemCanceled(KPIM::ProgressItem*)), SLOT(slotCancelNNTPJobs()));

  currentNntpJob = nntpJobQueue.take(0);
  currentNntpJob->prepareForExecution();
  if (currentNntpJob->success()) {
    nntpClient->insertJob(currentNntpJob);
    triggerAsyncThread(nntpOutPipe[1]);
    emit netActive(true);
    kdDebug(5003) << "KNNetAccess::startJobNntp(): job started" << endl;
  } else {
    threadDoneNntp();
  }
}



void KNNetAccess::startJobSmtp()
{
  if (smtpJobQueue.isEmpty()) {
    kdWarning(5003) << "KNNetAccess::startJobSmtp(): job queue is empty?? aborting" << endl;
    return;
  }
  unshownMsg = QString::null;

  mSMTPProgressItem = ProgressManager::createProgressItem(
      0, "SMTP", i18n("KNode SMTP"), QString::null, true, false );
  connect(mSMTPProgressItem, SIGNAL(progressItemCanceled(KPIM::ProgressItem*)), SLOT(slotCancelSMTPJobs()));

  currentSmtpJob = smtpJobQueue.take(0);
  currentSmtpJob->prepareForExecution();
  if (currentSmtpJob->success()) {
    smtpClient->insertJob(currentSmtpJob);
    triggerAsyncThread(smtpOutPipe[1]);
    emit netActive(true);
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
                                 kapp->makeStdCaption(i18n("Authorization Dialog")),info->server(),i18n("Server:"))) {
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
  if (!currentSmtpJob) {
    emit netActive(false);
    currMsg = QString::null;
    knGlobals.setStatusMsg();
  } else {
    currMsg = unshownMsg;
    knGlobals.setStatusMsg(currMsg);
  }
  mNNTPProgressItem->setComplete();
  mNNTPProgressItem = 0;

  tmp->notifyConsumer();

  if (!nntpJobQueue.isEmpty())
    startJobNntp();
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
  smtpClient->removeJob();
  currentSmtpJob = 0L;
  if (!currentNntpJob) {
    emit netActive(false);
    currMsg = QString::null;
    knGlobals.setStatusMsg();
  }
  mSMTPProgressItem->setComplete();
  mSMTPProgressItem = 0;

  tmp->notifyConsumer();

  if (!smtpJobQueue.isEmpty())
    startJobSmtp();
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
        mNNTPProgressItem->setStatus(currMsg);
      break;
      case KNProtocolClient::TSloadGrouplist:
        currMsg = i18n(" Loading group list from disk...");
        knGlobals.setStatusMsg(currMsg);
        mNNTPProgressItem->setStatus(currMsg);
      break;
      case KNProtocolClient::TSwriteGrouplist:
        currMsg = i18n(" Writing group list to disk...");
        knGlobals.setStatusMsg(currMsg);
        mNNTPProgressItem->setStatus(currMsg);
      break;
      case KNProtocolClient::TSdownloadGrouplist:
        currMsg = i18n(" Downloading group list...");
        knGlobals.setStatusMsg(currMsg);
        mNNTPProgressItem->setStatus(currMsg);
      break;
      case KNProtocolClient::TSdownloadNewGroups:
        currMsg = i18n(" Looking for new groups...");
        knGlobals.setStatusMsg(currMsg);
        mNNTPProgressItem->setStatus(currMsg);
      break;
      case KNProtocolClient::TSdownloadDesc:
        currMsg = i18n(" Downloading group descriptions...");
        knGlobals.setStatusMsg(currMsg);
        mNNTPProgressItem->setStatus(currMsg);
      break;
      case KNProtocolClient::TSdownloadNew:
        currMsg = i18n(" Downloading new headers...");
        knGlobals.setStatusMsg(currMsg);
        mNNTPProgressItem->setStatus(currMsg);
      break;
      case KNProtocolClient::TSsortNew:
        currMsg = i18n(" Sorting...");
        knGlobals.setStatusMsg(currMsg);
        mNNTPProgressItem->setStatus(currMsg);
      break;
      case KNProtocolClient::TSdownloadArticle:
        currMsg = i18n(" Downloading article...");
        knGlobals.setStatusMsg(currMsg);
        mNNTPProgressItem->setStatus(currMsg);
      break;
      case KNProtocolClient::TSsendArticle:
        currMsg = i18n(" Sending article...");
        knGlobals.setStatusMsg(currMsg);
        mNNTPProgressItem->setStatus(currMsg);
      break;
      case KNProtocolClient::TSjobStarted:
        mNNTPProgressItem->setProgress(0);
      break;
      case KNProtocolClient::TSprogressUpdate:
          mNNTPProgressItem->setProgress(nntpClient->getProgressValue()/10);
      break;
    };
  } else {                    // signal from smtp thread
    switch(signal) {
      case KNProtocolClient::TSworkDone:
        threadDoneSmtp();
      break;
      case KNProtocolClient::TSconnect:
        unshownMsg = i18n(" Connecting to server...");
        if (!currentNntpJob) {
          currMsg = unshownMsg;
          knGlobals.setStatusMsg(currMsg);
        }
        mSMTPProgressItem->setStatus(unshownMsg);
      break;
      case KNProtocolClient::TSsendMail:
        unshownMsg = i18n(" Sending mail...");
        if (!currentNntpJob) {
          currMsg = unshownMsg;
          knGlobals.setStatusMsg(currMsg);
        }
        mSMTPProgressItem->setStatus(unshownMsg);
      break;
      case KNProtocolClient::TSjobStarted:
        mSMTPProgressItem->setProgress(0);
      break;
      case KNProtocolClient::TSprogressUpdate:
        mSMTPProgressItem->setProgress(smtpClient->getProgressValue()/10);
      break;
    }
  }
}


//--------------------------------

#include "knnetaccess.moc"
