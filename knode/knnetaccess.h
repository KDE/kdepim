/*
    knnetaccess.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef KNNETACCESS_H
#define KNNETACCESS_H

#include <pthread.h>

#include <qobject.h>
#include <qqueue.h>

class QSocketNotifier;
class KNJobData;
class KNNntpClient;
class KNSmtpClient;


class KNNetAccess : public QObject  {

  Q_OBJECT

  public:

    KNNetAccess(QObject *parent=0, const char *name=0);
    ~KNNetAccess();

    void addJob(KNJobData *job);
    void stopJobsNntp(int type);         // type==0 => all jobs
    void stopJobsSmtp(int type);         // type==0 => all jobs
    void cancelAllJobs();

    QString currentMsg()         { return currMsg; }      // current statusbar message

    pthread_mutex_t* nntpMutex() { return &nntp_Mutex; }

  protected:
    void triggerAsyncThread(int pipeFd);     // passes a signal through the ipc-pipe to the net-thread
    void startJobNntp();
    void startJobSmtp();
    void threadDoneNntp();
    void threadDoneSmtp();

    QString currMsg;                       // stores the current status message,
                                           // so that it can be restored by the mainwindow
    QString unshownMsg, unshownByteCount;  // messages from the nntp-client have priority
    int unshownProgress;                   // unshown messages get stored here

    KNNntpClient *nntpClient;
    KNSmtpClient *smtpClient;   
    QList<KNJobData> nntpJobQueue, smtpJobQueue;
    KNJobData *currentNntpJob, *currentSmtpJob;
    pthread_t nntpThread, smtpThread;
    pthread_mutex_t nntp_Mutex;
    int nntpInPipe[2], nntpOutPipe[2], smtpInPipe[2], smtpOutPipe[2];
    QSocketNotifier *nntpNotifier,*smtpNotifier;

  protected slots:
    void slotThreadSignal(int i);

  signals:
    void netActive(bool);

};

#endif
