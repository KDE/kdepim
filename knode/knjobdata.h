/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2006 the KNode authors.
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

#include "knserverinfo.h"

#include <QPointer>
#include <kurl.h>
#include <kio/global.h>
#include <QObject>
#include <QList>
#include <libkdepim/progresswidget/progressmanager.h>


class KJob;

namespace KIO {
class Job;
}

class KNJobItem;
class KNJobData;


/** Base class for classes that want to create and schedule jobs. */
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

    /** Find any job related to a job item and cancel it.
     */
    void cancelJobs( boost::shared_ptr<KNJobItem> item );

  protected:
    /** The actual work is done here */
    virtual void processJob(KNJobData *j);
    /** List of all active jobs. */
    QList<KNJobData*> mJobs;

};


/** Base class for data structures used in jobs. */
class KNJobItem {

  public:
    /**
      Shared pointer to a KNJobItem. To be used instead of raw KNJobItem*.
    */
    typedef boost::shared_ptr<KNJobItem> Ptr;

    KNJobItem()           {}
    virtual ~KNJobItem()  {}

    virtual bool isLocked()         { return false; }
    virtual void setLocked(bool)    { }

    virtual QString prepareForExecution() { return QString(); }

};


/** Abstract base class for all KNode internal jobs.
 *  This class takes care of:
 *  - progress/status reporting and user interaction (cancellation).
 *  - error handling/reporting.
 *  - easy handling of associated KIO jobs.
 *  To imlpement a new job class, you need to sub-class this class and
 *  implement the execute() method.
 */
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

    KNJobData( jobType t, KNJobConsumer *c, KNServerInfo::Ptr a, KNJobItem::Ptr i );
    ~KNJobData();

    jobType type() const                  { return t_ype; }

    KNServerInfo::Ptr account() const { return a_ccount; }
    KNJobItem::Ptr data() const           { return d_ata; }

    /** Returns the error code (see KIO::Error). */
    int error() const { return mError; }
    /** Returns the error message. */
    QString errorString() const { return mErrorString; }
    /** Returns true if the job finished successfully. */
    bool success() const { return mErrorString.isEmpty() && mError == 0; }
    /** Returns true if the job has been canceled by the user. */
    bool canceled() const { return mCanceled; }

    /** Cancels this job.
     *  If the job is currently active, this cancels the associated KIO job and
     *  emits the finished signal.
     */
    void cancel();

    /** Set job error information.
     *  @param err The error code (see KIO::Error).
     *  @param errMsg A translated error message.
     */
    void setError( int err, const QString &errMsg );

    void prepareForExecution()           { mErrorString = d_ata->prepareForExecution(); }
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

    /** Set the status message of the progress item if available.
     *  @param msg The new status message.
     */
    void setStatus( const QString &msg ) { if ( mProgressItem ) mProgressItem->setStatus( msg ); }
    /** Set the progress value of the progress item if available.
     *  @param progress The new progress value.
     */
    void setProgress( unsigned int progress ) { if ( mProgressItem ) mProgressItem->setProgress( progress ); }
    /** Tells the progress item to indicate that the job has finished if
     *  available. This causes the destruction of the progress item.
     */
    void setComplete() { if ( mProgressItem ) { mProgressItem->setComplete(); mProgressItem = 0; } }

  signals:
    /** Emitted when a job has been finished.
     *  It's recommended to to emit it via emitFinished().
     */
    void finished( KNJobData* );

  protected:
    /** Emits the finished() signal via a single-shot timer. */
    void emitFinished();

    /** Returns a correctly set up KUrl according to the encryption and
     *  authentication settings for KIO slave operations.
     */
    KUrl baseUrl() const;

    /**
      Connects progress signals.
      @param job The KJob to setup.
    */
    void setupKJob( KJob *job );

    /** Sets TLS metadata and connects the given KIO job to the progress item.
     *  @param job The KIO job to setup.
     */
    void setupKIOJob( KIO::Job *job );

  protected:
    jobType t_ype;
    KNJobItem::Ptr d_ata;
    KNServerInfo::Ptr a_ccount;
    /** The job error code (see KIO::Error). */
    int mError;
    /** The error message. */
    QString mErrorString;
    /** Cancel status flag. */
    bool mCanceled;
    KNJobConsumer *c_onsumer;
    /** An associated KJob. */
    QPointer<KJob> mJob;
    /** The progress item representing this job to the user. */
    KPIM::ProgressItem *mProgressItem;

  private slots:
    /** Connected to the progress signal of mJob to update the progress item. */
    void slotJobPercent( KJob *job, unsigned long percent );
    /** Connected to the info message signal if mJob to update the progress item. */
    void slotJobInfoMessage( KJob *job, const QString &msg );
    /** Emits the finished signal. @see emitFinished() */
    void slotEmitFinished();

};


#endif
