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
#if QT_VERSION >= 290
#  include <qptrqueue.h>
#else
// remove after Qt3 becomes mandatory
// don't we need to include only q(ptr)list.h here?
#  include <qqueue.h>
#  define QPtrQueue QQueue
#  define QPtrList QList
#endif

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
    /** type==0 => all jobs */
    void stopJobsNntp(int type);
    /** type==0 => all jobs */
    void stopJobsSmtp(int type);
    void cancelAllJobs();

    /** current statusbar message */
    QString currentMsg()         { return currMsg; }

    pthread_mutex_t* nntpMutex() { return &nntp_Mutex; }

  protected:
    /** passes a signal through the ipc-pipe to the net-thread */
    void triggerAsyncThread(int pipeFd);
    void startJobNntp();
    void startJobSmtp();
    void threadDoneNntp();
    void threadDoneSmtp();

    /** stores the current status message,
	so that it can be restored by the mainwindow */
    QString currMsg;
    /** messages from the nntp-client have priority */
    QString unshownMsg, unshownByteCount;
    /** unshown messages get stored here */
    int unshownProgress;

    KNNntpClient *nntpClient;
    KNSmtpClient *smtpClient;   
    QPtrList<KNJobData> nntpJobQueue, smtpJobQueue;
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
