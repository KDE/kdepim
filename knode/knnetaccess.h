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

#include <pthread.h>

#include <qobject.h>
#include <qqueue.h>

#include <kaction.h>

class QSocketNotifier;

class KNJobData;
class KNNntpClient;
class KNSmtpClient;


class KNNetAccess : public QObject  {

  Q_OBJECT

  public:

    KNNetAccess(QObject *parent=0, const char *name=0);
    ~KNNetAccess();
    
    const KActionCollection& actions()    { return actionCollection; }  
    
    void addJob(KNJobData *job);
    void stopJobsNntp(int type);         // type==0 => all jobs
    void stopJobsSmtp(int type);         // type==0 => all jobs

    pthread_mutex_t* nntpMutex() { return &nntp_Mutex; }

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
    pthread_mutex_t nntp_Mutex;
    int nntpInPipe[2], nntpOutPipe[2], smtpInPipe[2], smtpOutPipe[2];
    QSocketNotifier *nntpNotifier,*smtpNotifier;
    KAction* actNetStop;
    KActionCollection actionCollection;
  
  protected slots:
    void slotThreadSignal(int i);
    void slotCancelAllJobs();
    
};

#endif
