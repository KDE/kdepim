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

#ifndef KNODE_SCHEDULER_H
#define KNODE_SCHEDULER_H

#include <QObject>
#include <QMutex>
#include <QList>

namespace KPIM {
  class ProgressItem;
}

class KNJobData;

namespace KNode {

/** The job scheduler.
 * This class manages three diffrent queues of all created jobs:
 * - a queue for all NNTP jobs
 * - a queue for all SMTP jobs
 * - a queue for jobs that are waiting for KWallet
 * At most two jobs (one NNTP, one SMTP job) are executed at once.
 *
 * @todo Add per-account queues and execute one job per account.
 *
 * @see KNGlobals::scheduler()
 */
class Scheduler : public QObject  {

  Q_OBJECT

  public:

    /** Create a new Scheduler object
     * @param parent The parent QObject.
     */
    explicit Scheduler( QObject *parent = 0 );
    ~Scheduler();

    /** Adds a new job to the scheduler queue.
     * @param job The new job.
     */
    void addJob(KNJobData *job);

    QMutex& nntpMutex() { return nntp_Mutex; }

    /** Cancel the selected jobs.
     *  @param type Cancel all jobs of the given type, 0 means all.
     *  @param item Cancel the job associated with this progress item.
     *              If item is 0, only the job type is evaluated.
     */
    void cancelJobs( int type = 0, KPIM::ProgressItem* item = 0 );

  protected:
    /// the currently active NNTP job
    KNJobData *currentNntpJob;
    /// the currently active SMTP job
    KNJobData *currentSmtpJob;
    QMutex nntp_Mutex;

  signals:
    /** Indicates whether there are currently active jobs, useful to e.g. enable
     * a cancel button.
     */
    void netActive(bool);

  private:
    /** Checks if there is a free slot available for a waiting job and executes
     *  it.
     */
    void schedule();

    /** Starts the given job.
     *  @param job The job to start.
     */
    void startJob( KNJobData *job );

    /** Update activitiy status, i.e. emit netActive signal. */
    void updateStatus();

  private slots:
    /** Connected to the finished() signal of all active jobs.
     *  @param job The job that has been finished.
     */
    void slotJobFinished( KNJobData *job );

    /** Connected to the cancel signal of the progress item of each job.
     *  @param item The progress item which has been canceled.
     */
    void slotCancelJob( KPIM::ProgressItem *item );

    void slotPasswordsChanged();

  private:
    /// queue for NNTP jobs
    QList<KNJobData*> nntpJobQueue;
    /// all SMTP jobs
    QList<KNJobData*> smtpJobs;
    /// jobs waiting for async wallet loading
    QList<KNJobData*> mWalletQueue;

};

}

#endif
