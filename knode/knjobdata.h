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

#include <qobject.h>
#include <qvaluelist.h>

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

    /** Send the job to KNNetAccess and append it to the
	joblist */
    void emitJob(KNJobData *j);

    /** Remove the job from the joblist and process it by
	calling @ref processJob */
    void jobDone(KNJobData *j);

    /** Returns true if we are waiting for at least one job
	to be completed */
    bool jobsPending() const { return !mJobs.isEmpty(); }

  protected:
    /** The actual work is done here */
    virtual void processJob(KNJobData *j);
    QValueList<KNJobData*> mJobs;

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
                    JTCheckNewGroups,
                    JTfetchNewHeaders,
                    JTsilentFetchNewHeaders,
                    JTfetchArticle,
                    JTpostArticle,
                    JTmail,
                    JTfetchSource   };

    KNJobData(jobType t, KNJobConsumer *c, KNServerInfo *a, KNJobItem *i);
    ~KNJobData();

    jobType type() const                  { return t_ype; }

    bool net() const                      { return (t_ype!=JTLoadGroups); }
    KNServerInfo* account() const         { return a_ccount; }
    KNJobItem* data() const               { return d_ata; }

    const QString& errorString() const    { return e_rrorString; }
    bool success() const                  { return e_rrorString.isEmpty(); }
    bool canceled() const                 { return c_anceled; }
    bool authError() const                { return a_uthError; }

    void setErrorString(const QString& s) { e_rrorString=s; }
    void cancel();
    void setAuthError(bool b)             { a_uthError=b; }

    void prepareForExecution()           { e_rrorString = d_ata->prepareForExecution(); }
    void notifyConsumer();

    KIO::Job* job() const                { return mJob; }
    void setJob( KIO::Job *job );

    KPIM::ProgressItem* progressItem() const { return mProgressItem; }
    void createProgressItem();

    // safe forwards to the progress item
    void setStatus( const QString &msg ) { if ( mProgressItem ) mProgressItem->setStatus( msg ); }
    void setProgress( unsigned int progress ) { if ( mProgressItem ) mProgressItem->setProgress( progress ); }
    void setComplete() { if ( mProgressItem ) { mProgressItem->setComplete(); mProgressItem = 0; } }

  protected:
    jobType t_ype;
    KNJobItem *d_ata;
    KNServerInfo *a_ccount;
    QString e_rrorString;
    bool c_anceled;
    bool a_uthError;
    KNJobConsumer *c_onsumer;

  private slots:
    void slotJobPercent( KIO::Job *job, unsigned long percent );
    void slotJobInfoMessage( KIO::Job *job, const QString &msg );

  private:
    KIO::Job *mJob;
    KPIM::ProgressItem *mProgressItem;

};


#endif
