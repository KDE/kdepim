/*
    Copyright (c) 2005 by Volker Krause <volker.krause@rwth-aachen.de>

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

#include <kio/job.h>

#include <Q3SortedList>

class KNGroupInfo;

namespace KNode {

/** Download and update newsgroups lists */
class GroupFetchJob : public KNJobData
{
  Q_OBJECT
  public:
    GroupFetchJob( KNJobConsumer *c, KNServerInfo *a, KNJobItem *i );

    virtual void execute();

  private slots:
    void slotEntries( KIO::Job *job, const KIO::UDSEntryList &list );
    void slotResult( KIO::Job *job );

  private:
    Q3SortedList<KNGroupInfo> mGroupList;
};



/** Update newsgroup list (convenience wrapper for GroupFetchJob). */
class GroupUpdateJob : public GroupFetchJob
{
  public:
    GroupUpdateJob( KNJobConsumer *c, KNServerInfo *a, KNJobItem *i );
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
    ArticleListJob( KNJobConsumer *c, KNServerInfo *a, KNJobItem *i );

    virtual void execute();

  private slots:
    void slotEntries( KIO::Job *job, const KIO::UDSEntryList &list );
    void slotResult( KIO::Job *job );

  private:
    KIO::UDSEntryList mArticleList;
};

}
#endif
