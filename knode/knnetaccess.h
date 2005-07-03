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

#ifndef KNNETACCESS_H
#define KNNETACCESS_H

#include <qobject.h>
#include <qmutex.h>
#include <qvaluelist.h>

class QSocketNotifier;

namespace KIO {
  class Job;
  class StoredTransferJob;
}

namespace KPIM {
  class ProgressItem;
}
using KPIM::ProgressItem;

class KNJobData;
class KNNntpClient;


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
    QString currentMsg() const        { return currMsg; }

    QMutex& nntpMutex() { return nntp_Mutex; }

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
    QString unshownMsg;

    KNNntpClient *nntpClient;
    QValueList<KNJobData*> nntpJobQueue, smtpJobQueue;
    KNJobData *currentNntpJob, *currentSmtpJob;
    QMutex nntp_Mutex;
    int nntpInPipe[2], nntpOutPipe[2];
    QSocketNotifier *nntpNotifier;

  protected slots:
    void slotThreadSignal(int i);
    void slotCancelNNTPJobs() { stopJobsNntp(0); }
    void slotCancelSMTPJobs() { stopJobsSmtp(0); }

  signals:
    void netActive(bool);

  private slots:
    void slotJobResult( KIO::Job *job );
    void slotJobPercent( KIO::Job *job, unsigned long percent );
    void slotJobInfoMessage( KIO::Job *job, const QString &msg );

  private:
    ProgressItem *mNNTPProgressItem, *mSMTPProgressItem;
    KIO::StoredTransferJob *mSmtpJob;

};

#endif
