/***************************************************************************
                          knnetaccess.h  -  description
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


#ifndef KNNETACCESS_H
#define KNNETACCESS_H

#include <qobject.h>
#include <qqueue.h>
#include <qsocketnotifier.h>
#include <pthread.h>

#include "knnntpclient.h"
#include "knsmtpclient.h"
#include "knjobdata.h"

class KNNetAccess : public QObject  {

  Q_OBJECT

  public:

		KNNetAccess(QObject *parent=0, const char *name=0);
		~KNNetAccess();
		
		void addJob(KNJobData *job);
		void cancelAllJobs();

	protected:
		void triggerAsyncThread(int pipeFd);     // passes a signal through the ipc-pipe to the net-thread
	  void startJobNntp();
	  void startJobSmtp();
	  void threadDoneNntp();
	  void threadDoneSmtp();
	   	  	  						
		QString unshownMsg, unshownByteCount;    // messages from the nntp-client have priority
		int unshownProgress;                     // unshown messages get stored here
	
		KNNntpClient *nntpClient;
		KNSmtpClient *smtpClient;		
	  QQueue<KNJobData> nntpJobQueue, smtpJobQueue;
	  KNJobData *currentNntpJob, *currentSmtpJob;
	  pthread_t nntpThread, smtpThread;
	  int nntpInPipe[2], nntpOutPipe[2], smtpInPipe[2], smtpOutPipe[2];
	  QSocketNotifier *nntpNotifier,*smtpNotifier;
	
	protected slots:
		void slotThreadSignal(int i);
};

#endif
