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

#ifndef KNJOBDATA_H
#define KNJOBDATA_H

#include <kurl.h>

#include <qobject.h>
#include <q3valuelist.h>

#include <libkdepim/progressmanager.h>

namespace KIO {
  class Job;
}

class KNJobData;
class KNServerInfo;


class KNJobConsumer {

  public:
    KNJobConsumer();
    virtual ~KNJobConsumer();

    /** Send the job to the scheduler and append it to the
     *  job queue.
     */
    void emitJob(KNJobData *j);

    /** Remove the job from the joblist and process it by
     *  calling @ref processJob
     */
    void jobDone(KNJobData *j);

    /** Returns true if we are waiting for at least one job
     *  to be completed
     */
    bool jobsPending() const { return !mJobs.isEmpty(); }

  protected:
    /** The actual work is done here */
    virtual void processJob(KNJobData *j);
    Q3ValueList<KNJobData*> mJobs;

};


class KNJobItem {

  public:
    KNJobItem()           {}
    virtual ~KNJobItem()  {}

    virtual bool isLocked()         { return false; }
    virtual void setLocked(bool)    { }

    virtual QString prepareForExecution() { return QString::null; }

};


class KNJobData : public QObject
{
  Q_OBJECT

  public:

    friend class KNJobConsumer;

    enum jobType {  JTLoadGroups=1,
                    JTFetchGroups,
                    JTfetchNewHeaders,
                    JTfetchArticle,
                    JTpostArticle,
                    JTmail,
                    JTfetchSource   };

    KNJobData(jobType t, KNJobConsumer *c, KNServerInfo *a, KNJobItem *i);
    ~KNJobData();

    jobType type() const                  { return t_ype; }

    KNServerInfo* account() const         { return a_ccount; }
    KNJobItem* data() const               { return d_ata; }

    const QString& errorString() const    { return e_rrorString; }
    bool success() const                  { return e_rrorString.isEmpty(); }
    bool canceled() const                 { return c_anceled; }
    bool authError() const                { return a_uthError; }

    void setErrorString(const QString& s) { e_rrorString=s; }

    /** Cancels this job.
     *  If the job is currently active, this cancels the assosiated KIO job and
     *  emits the finished signal.
     */
    void cancel();
    void setAuthError(bool b)             { a_uthError=b; }

    void prepareForExecution()           { e_rrorString = d_ata->prepareForExecution(); }
    void notifyConsumer();

    /** Performs the actual operation of a job, needs to be reimplemented for
     *  every job.
     *  Note that a job might be executed multiple times e.g. in case of an
     *  authentication error.
     */
    virtual void execute() = 0;

    /** Returns the progress item for this job. */
    KPIM::ProgressItem* progressItem() const { return mProgressItem; }
    /** Creates a KPIM::ProgressItem for this job. */
    void createProgressItem();

    // safe forwards to the progress item
    void setStatus( const QString &msg ) { if ( mProgressItem ) mProgressItem->setStatus( msg ); }
    void setProgress( unsigned int progress ) { if ( mProgressItem ) mProgressItem->setProgress( progress ); }
    void setComplete() { if ( mProgressItem ) { mProgressItem->setComplete(); mProgressItem = 0; } }

  signals:
    /** Emitted when a job has been finished.
     *  It's recommended to to emit it via emitFinished().
     */
    void finished( KNJobData* );

  protected:
    /** Emits the finished() signal via a single-shot timer. */
    void emitFinished();

    /** Returns a correctly set up KURL according to the encryption and
     *  authentication settings for KIO slave operations.
     */
    KURL baseUrl() const;

    /** Sets TLS metadata and connects the given KIO job to the progress item.
     *  @param job The KIO job to setup.
     */
    void setupKIOJob( KIO::Job *job );

  protected:
    jobType t_ype;
    KNJobItem *d_ata;
    KNServerInfo *a_ccount;
    QString e_rrorString;
    bool c_anceled;
    bool a_uthError;
    KNJobConsumer *c_onsumer;
    KIO::Job *mJob;
    KPIM::ProgressItem *mProgressItem;

  private slots:
    void slotJobPercent( KIO::Job *job, unsigned long percent );
    void slotJobInfoMessage( KIO::Job *job, const QString &msg );
    void slotEmitFinished();

};


#endif
