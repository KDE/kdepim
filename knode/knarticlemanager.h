/*
    knarticlemanager.h

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

#ifndef KNARTICLEMANAGER_H
#define KNARTICLEMANAGER_H

#include <qptrlist.h>

#include "knjobdata.h"
#include "knarticle.h"

class QListViewItem;

class KTempFile;

class KNArticle;
class KNHeaderView;
class KNThread;
class KNArticleCollection;
class KNGroup;
class KNFolder;
class KNArticleFilter;
class KNFilterManager;
class KNSearchDialog;
class KNJobData;


class KNArticleManager : public QObject, public KNJobConsumer {

  Q_OBJECT

  public:
    KNArticleManager();
    virtual ~KNArticleManager();

    //content handling
    void deleteTempFiles();
    void saveContentToFile(KMime::Content *c, QWidget *parent);
    void saveArticleToFile(KNArticle *a, QWidget *parent);
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
    bool loadArticle(KNArticle *a);
    bool unloadArticle(KNArticle *a, bool force=true);

    //article storage
    void copyIntoFolder(KNArticle::List &l, KNFolder *f);
    void moveIntoFolder(KNLocalArticle::List &l, KNFolder *f);
    bool deleteArticles(KNLocalArticle::List &l, bool ask=true);

    //article handling
    void setAllRead(bool r=true);
    void setAllRead(int lastcount, bool r=true);
    void setRead(KNRemoteArticle::List &l, bool r=true, bool handleXPosts=true);

    // returns false if the changes were reverted (i.e. ignored articles->neutral articles)
    bool toggleWatched(KNRemoteArticle::List &l);
    bool toggleIgnored(KNRemoteArticle::List &l);

    void rescoreArticles(KNRemoteArticle::List &l);

    // Allow to delay the setup of UI elements, since the knode part may not 
    // be available when the config dialog is called
    void setView(KNHeaderView* v);

  protected:
    void processJob(KNJobData *j);
    void createThread(KNRemoteArticle *a);
    void createCompleteThread(KNRemoteArticle *a);

    KNHeaderView *v_iew;
    KNGroup *g_roup;
    KNFolder *f_older;
    KNArticleFilter *f_ilter;
    KNFilterManager *f_ilterMgr;
    KNSearchDialog *s_earchDlg;
    QPtrList<KTempFile> t_empFiles;
    bool d_isableExpander;

  public slots:
    void slotFilterChanged(KNArticleFilter *f);
    void slotSearchDialogDone();
    void slotItemExpanded(QListViewItem *p);

};

#endif
