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

#ifndef KNARTICLEMANAGER_H
#define KNARTICLEMANAGER_H

#include <QList>

#include "knjobdata.h"
#include "knarticle.h"

class Q3ListViewItem;

class KTemporaryFile;

class KNArticle;
class KNHeaderView;
class KNArticleCollection;
class KNGroup;
class KNFolder;
class KNArticleFilter;
class KNFilterManager;
class KNJobData;

namespace KNode {
  class SearchDialog;
}


/** Article manager. */
class KNArticleManager : public QObject, public KNJobConsumer {

  Q_OBJECT

  public:
    KNArticleManager();
    virtual ~KNArticleManager();

    //content handling
    void deleteTempFiles();
    void saveContentToFile(KMime::Content *c, QWidget *parent);
    void saveArticleToFile( KNArticle::Ptr a, QWidget *parent );
    QString saveContentToTemp(KMime::Content *c);
    void openContent(KMime::Content *c);

    //listview handling
    void showHdrs(bool clear=true);
    void updateViewForCollection(KNArticleCollection *c);
    void updateListViewItems();
    void setAllThreadsOpen(bool b=true);

    void updateStatusString();

    //filter
    KNArticleFilter* filter() const   { return f_ilter; }
    void search();

    //collection handling
    void setGroup(KNGroup *g);
    void setFolder(KNFolder *f);
    KNArticleCollection* collection();

    //article loading
    /**
      Loads the full content of the article @p a.
    */
    bool loadArticle( KNArticle::Ptr a);
    bool unloadArticle( KNArticle::Ptr a, bool force=true );

    //article storage
    void copyIntoFolder(KNArticle::List &l, KNFolder *f);
    void moveIntoFolder(KNLocalArticle::List &l, KNFolder *f);
    bool deleteArticles(KNLocalArticle::List &l, bool ask=true);

    //article handling
    void setAllRead( bool read = true, int lastcount = -1 );
    void setRead(KNRemoteArticle::List &l, bool r=true, bool handleXPosts=true);
    /// mark all articles in the current group as not new
    void setAllNotNew();

    // returns false if the changes were reverted (i.e. ignored articles->neutral articles)
    bool toggleWatched(KNRemoteArticle::List &l);
    bool toggleIgnored(KNRemoteArticle::List &l);

    void rescoreArticles(KNRemoteArticle::List &l);

    /** Allow to delay the setup of UI elements, since the knode part may not
     * be available when the config dialog is called.
     */
    void setView(KNHeaderView* v);

  signals:
    /** A newsgroup is about to be shown in the header view.
     * Connect to the header view to adapt to the upcoming content.
     */
    void aboutToShowGroup();
    /** A local folder is about to be shown in the header view.
     * Connect to the header view to adapt to the upcoming content.
     */
    void aboutToShowFolder();

  protected:
    void processJob(KNJobData *j);
    void createThread( KNRemoteArticle::Ptr a );
    void createCompleteThread( KNRemoteArticle::Ptr a );

    KNHeaderView *v_iew;
    KNGroup *g_roup;
    KNFolder *f_older;
    KNArticleFilter *f_ilter;
    KNFilterManager *f_ilterMgr;
    KNode::SearchDialog *s_earchDlg;
    QList<KTemporaryFile*> mTempFiles;
    bool d_isableExpander;

  public slots:
    void slotFilterChanged(KNArticleFilter *f);
    void slotSearchDialogDone();
    void slotItemExpanded(Q3ListViewItem *p);

};

#endif
