/***************************************************************************
                          knnetaccess.cpp  -  description
                             -------------------

    copyright            : (C) 2000 by Christian Thurner
    email                : cthurner@freepage.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

#include <qsocketnotifier.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kapp.h>
#include <kglobal.h>

#include "knode.h"
#include "knjobdata.h"
#include "knnntpclient.h"
#include "knsmtpclient.h"
#include "knglobals.h"
#include "knnetaccess.h"


KNNetAccess::KNNetAccess(QObject *parent, const char *name )
: QObject(parent,name), currentNntpJob(0L), currentSmtpJob(0L)
{
	if((pipe(nntpInPipe)==-1)||
	   (pipe(nntpOutPipe)==-1)||
	   (pipe(smtpInPipe)==-1)||
	   (pipe(smtpOutPipe)==-1)) {
		KMessageBox::error(0,i18n("Internal error:\nFailed to open pipes for internal communcation!"));
		kapp->exit(1);
	}
	if((fcntl(nntpInPipe[0],F_SETFL,O_NONBLOCK)==-1)||
	   (fcntl(nntpOutPipe[0],F_SETFL,O_NONBLOCK)==-1)||
	   (fcntl(smtpInPipe[0],F_SETFL,O_NONBLOCK)==-1)||
	   (fcntl(smtpOutPipe[0],F_SETFL,O_NONBLOCK)==-1)) {
		KMessageBox::error(0,i18n("Internal error:\nFailed to open pipes for internal communcation!"));
		kapp->exit(1);
	}	

	nntpNotifier=new QSocketNotifier(nntpInPipe[0], QSocketNotifier::Read);
	connect(nntpNotifier, SIGNAL(activated(int)), this, SLOT(slotThreadSignal(int)));
	smtpNotifier=new QSocketNotifier(smtpInPipe[0], QSocketNotifier::Read);
	connect(smtpNotifier, SIGNAL(activated(int)), this, SLOT(slotThreadSignal(int)));

	nntpClient=new KNNntpClient(nntpOutPipe[0],nntpInPipe[1],this);
	smtpClient=new KNSmtpClient(smtpOutPipe[0],smtpInPipe[1],this);
	
	if(pthread_create(&nntpThread, 0,&(nntpClient->startThread), nntpClient)!=0) {
		KMessageBox::error(0, i18n("Internal error:\nCannot create the nntp-network-thread!"));
		kapp->exit(1);
	}
	if(pthread_create(&smtpThread, 0,&(smtpClient->startThread), smtpClient)!=0) {
		KMessageBox::error(0, i18n("Internal error:\nCannot create the smtp-network-thread!"));
		kapp->exit(1);
	}
	
	nntpJobQueue.setAutoDelete(false);		
	smtpJobQueue.setAutoDelete(false);		
	
	actNetStop = new KAction(i18n("Stop &Network"),"stop",0, this, SLOT(slotCancelAllJobs()),
	                         &actionCollection, "net_stop");
	actNetStop->setEnabled(false);
}



KNNetAccess::~KNNetAccess()
{
	disconnect(nntpNotifier, SIGNAL(activated(int)), this, SLOT(slotThreadSignal(int)));
	disconnect(smtpNotifier, SIGNAL(activated(int)), this, SLOT(slotThreadSignal(int)));
	
	if(pthread_cancel(nntpThread)!=0)
		qDebug("KNNetAccess::~KNNetAccess() : cannot cancel thread");
  if (0!=pthread_join(nntpThread,NULL))                         // join is important...
    qDebug("KNNetAccess::~KNNetAccess() : cannot join thread");
	if(pthread_cancel(smtpThread)!=0)
		qDebug("KNNetAccess::~KNNetAccess() : cannot cancel thread");				
  if (0!=pthread_join(smtpThread,NULL))                         // join is important...
    qDebug("KNNetAccess::~KNNetAccess() : cannot join thread");
		
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
    qDebug( "Can't close pipes" );
}



void KNNetAccess::addJob(KNJobData *job)
{
	// qDebug("KNNetAccess::addJob() : job queued");  // too verbose...
	if (job->type()==KNJobData::JTmail) {
		smtpJobQueue.enqueue(job);
		if (!currentSmtpJob)   // no active job, start the new one
			startJobSmtp();
	} else {
		nntpJobQueue.enqueue(job);
		if (!currentNntpJob)   // no active job, start the new one
			startJobNntp();
	}
}


// passes a signal through the ipc-pipe to the net-thread
void KNNetAccess::triggerAsyncThread(int pipeFd)
{
	int signal=0;
	
	// qDebug("KNNetAccess::triggerAsyncThread() : sending signal to net thread"); // too verbose
	write(pipeFd, &signal, sizeof(int));
}


void KNNetAccess::startJobNntp()
{
	if (nntpJobQueue.isEmpty()) {
		qDebug("KNNetAccess::startJobNntp(): job queue is empty?? aborting");
		return;
	}
	currentNntpJob = nntpJobQueue.dequeue();
	nntpClient->insertJob(currentNntpJob);
	triggerAsyncThread(nntpOutPipe[1]);
  actNetStop->setEnabled(true);
	qDebug("KNNetAccess::startJobNntp(): job started");
}



void KNNetAccess::startJobSmtp()
{
	if (smtpJobQueue.isEmpty()) {
		qDebug("KNNetAccess::startJobSmtp(): job queue is empty?? aborting");
		return;
	}
	unshownMsg = QString::null;
	unshownByteCount = QString::null;
	unshownProgress = 0;
	
	currentSmtpJob = smtpJobQueue.dequeue();
	smtpClient->insertJob(currentSmtpJob);
	triggerAsyncThread(smtpOutPipe[1]);
  actNetStop->setEnabled(true);
	qDebug("KNNetAccess::startJobSmtp(): job started");
}



void KNNetAccess::threadDoneNntp()
{
 	KNJobData *tmp;
	if (!currentNntpJob) {
		qDebug("KNNetAccess::threadDoneNntp(): no current job?? aborting");
		return;
	}
 		
 	qDebug("KNNetAccess::threadDoneNntp(): job done");

  tmp = currentNntpJob;
  nntpClient->removeJob();
	currentNntpJob = 0L;
 	if (!currentSmtpJob) {
		actNetStop->setEnabled(false);
 	  knGlobals.progressBar->disableProgressBar();
 	 	knGlobals.top->setStatusMsg();
 	}	else {
  	knGlobals.progressBar->setProgressBar(unshownProgress,unshownByteCount);
  	knGlobals.top->setStatusMsg(unshownMsg);
	}
	
	knGlobals.top->jobDone(tmp); 	

	if (!nntpJobQueue.isEmpty())
		startJobNntp();
}



void KNNetAccess::threadDoneSmtp()
{
 	KNJobData *tmp;
	if (!currentSmtpJob) {
		qDebug("KNNetAccess::threadDoneSmtp(): no current job?? aborting");
		return;
	}
 		
 	qDebug("KNNetAccess::threadDoneSmtp(): job done");

  tmp = currentSmtpJob;
  smtpClient->removeJob();
	currentSmtpJob = 0L;
 	if (!currentNntpJob) {
   	actNetStop->setEnabled(false);
		knGlobals.progressBar->disableProgressBar();
 	 	knGlobals.top->setStatusMsg();
 	}
	
	knGlobals.top->jobDone(tmp); 	

	if (!smtpJobQueue.isEmpty())
		startJobSmtp();
}



void KNNetAccess::slotThreadSignal(int i)
{
	int signal,byteCount;
	QString tmp;
		
	//qDebug("KNNetAccess::slotThreadSignal() : signal received from net thread"); // too verbose
	if(read(i, &signal, sizeof(int))==-1) {
		qDebug("KNNetAccess::slotThreadSignal() : cannot read from pipe");
    return;
	}
		 	
  if (i == nntpInPipe[0]) {      // signal from nntp thread
  	switch(signal) {
  	 	case KNProtocolClient::TSworkDone:
    		threadDoneNntp();
     	break;
    	case KNProtocolClient::TSconnect:
    		knGlobals.top->setStatusMsg(i18n(" Connecting to server ..."));
  	 	break;
    	case KNProtocolClient::TSdownloadGrouplist:
    		knGlobals.top->setStatusMsg(i18n(" Downloading grouplist ..."));
    	break;
  		case KNProtocolClient::TSdownloadNew:
  			knGlobals.top->setStatusMsg(i18n(" Downloading new headers ..."));
  		break;
  		case KNProtocolClient::TSsortNew:
  			knGlobals.top->setStatusMsg(i18n(" Sorting ..."));
  		break;
    	case KNProtocolClient::TSdownloadArticle:
		  	knGlobals.top->setStatusMsg(i18n(" Downloading article ..."));
  		break;
	  	case KNProtocolClient::TSsendArticle:
  			knGlobals.top->setStatusMsg(i18n(" Sending article ..."));
  		break;
  		case KNProtocolClient::TSjobStarted:
      	knGlobals.progressBar->setProgressBar(10, i18n("0 Bytes"));
      break;
  		case KNProtocolClient::TSprogressUpdate:
  		  byteCount = nntpClient->getByteCount();
  		  if (byteCount < 1000)
  		    tmp = i18n("%1 Bytes").arg(KGlobal::locale()->formatNumber(byteCount, 0));
  		  else
  		    tmp = i18n("%1 KB").arg(KGlobal::locale()->formatNumber(byteCount/1000.0, 1));
  		  knGlobals.progressBar->setProgressBar(nntpClient->getProgressValue(),tmp);
  		break;
  	};
  } else {                    // signal from smtp thread
		switch(signal) {
  		case KNProtocolClient::TSworkDone:     		
	  		threadDoneSmtp();
		  break;
  		case KNProtocolClient::TSconnect:
  		  unshownMsg = i18n(" Connecting to server ...");
 		   	if (!currentNntpJob)
  		    knGlobals.top->setStatusMsg(unshownMsg);
	  	break;
    	case KNProtocolClient::TSsendMail:
    	  unshownMsg = i18n(" Sending mail ...");
 		   	if (!currentNntpJob)
  		    knGlobals.top->setStatusMsg(unshownMsg);
  		break;
  		case KNProtocolClient::TSjobStarted:
      	unshownByteCount = i18n("0 Bytes");
      	unshownProgress = 10;
      	if (!currentNntpJob)
    		  knGlobals.progressBar->setProgressBar(unshownProgress,unshownByteCount);
      break;
	  	case KNProtocolClient::TSprogressUpdate:
  	  	byteCount = smtpClient->getByteCount();
  		  if (byteCount < 1000)
  		    unshownByteCount = i18n("%1 Bytes").arg(KGlobal::locale()->formatNumber(byteCount, 0));
  		  else
  		    unshownByteCount = i18n("%1 KB").arg(KGlobal::locale()->formatNumber(byteCount, 1));
  		  unshownProgress = smtpClient->getProgressValue();
 		   	if (!currentNntpJob)
    		  knGlobals.progressBar->setProgressBar(unshownProgress,unshownByteCount);
  		break;
  	}
  }
}



void KNNetAccess::slotCancelAllJobs()
{
	KNJobData *tmp;	
	
	// ** nntp ***********************************************
	
	if (currentNntpJob) {                // stop active job
		currentNntpJob->cancel();
		triggerAsyncThread(nntpOutPipe[1]);
	}
		
	while(!nntpJobQueue.isEmpty()) {     // kill all waiting jobs
	  tmp=nntpJobQueue.dequeue();
		tmp->cancel();
		knGlobals.top->jobDone(tmp);
	}	
	
	// ** smtp ***********************************************	
			
  if (currentSmtpJob) {        // stop active job
		currentSmtpJob->cancel();
		triggerAsyncThread(smtpOutPipe[1]);
	}
		
	while(!smtpJobQueue.isEmpty()) {     // kill all waiting jobs
	  tmp=smtpJobQueue.dequeue();
		tmp->cancel();
		knGlobals.top->jobDone(tmp);
	}
}


//--------------------------------

#include "knnetaccess.moc"
