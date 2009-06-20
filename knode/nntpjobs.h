/*
    Copyright (c) 2005 by Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNODE_NNTPJOBS_H
#define KNODE_NNTPJOBS_H

#include "knjobdata.h"
#include "kngroupmanager.h"

#include <kio/job.h>

#include <QList>

class KJob;

namespace KNode {

/** Download and update newsgroups lists */
class GroupListJob : public KNJobData
{
  Q_OBJECT
  public:
    GroupListJob( KNJobConsumer *c, KNServerInfo *a, KNJobItem *i, bool incremental = false );

    virtual void execute();

  private slots:
    void slotEntries( KIO::Job *job, const KIO::UDSEntryList &list );
    void slotResult( KJob *job );

  private:
    QList<KNGroupInfo> mGroupList;
    bool mIncremental;
};



/** Loads the newsgroup list from the disk. */
class GroupLoadJob : public KNJobData
{
  public:
    GroupLoadJob( KNJobConsumer *c, KNServerInfo *a, KNJobItem *i );

    virtual void execute();
};



/** Downloads all or a selected part of the article list for a specific
 *  newsgroup.
 */
class ArticleListJob : public KNJobData
{
  Q_OBJECT
  public:
    ArticleListJob( KNJobConsumer *c, KNServerInfo *a, KNJobItem *i, bool silent = false );

    virtual void execute();
    /** Returns whether an error message should be shown. */
    bool silent() { return mSilent; }

  private slots:
    void slotEntries( KIO::Job *job, const KIO::UDSEntryList &list );
    void slotResult( KJob *_job );

  private:
    KIO::UDSEntryList mArticleList;
    bool mSilent;
};



/** Downloads one specific article from the news server. */
class ArticleFetchJob : public KNJobData
{
  Q_OBJECT
  public:
    ArticleFetchJob( KNJobConsumer *c, KNServerInfo *a, KNJobItem *i, bool parse = true );

    virtual void execute();

  private slots:
    void slotResult( KJob *job );

  private:
    bool mParseArticle;
};



/** Post a article to the given news server. */
class ArticlePostJob : public KNJobData
{
  Q_OBJECT
  public:
    ArticlePostJob( KNJobConsumer *c, KNServerInfo *a, KNJobItem *i );

    virtual void execute();

  private slots:
    void slotResult( KJob *job );
};

}
#endif
