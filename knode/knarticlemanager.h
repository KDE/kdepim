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

#include <qlist.h>

#include "knjobdata.h"
#include "knmime.h"

class QListViewItem;

class KTempFile;

class KNArticle;
class KNListView;
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
    KNArticleManager(KNListView *v, KNFilterManager *f);
    virtual ~KNArticleManager();

    //content handling
    void deleteTempFiles();
    void saveContentToFile(KNMimeContent *c, QWidget *parent);
    void saveArticleToFile(KNArticle *a, QWidget *parent);
    QString saveContentToTemp(KNMimeContent *c);
    void openContent(KNMimeContent *c);

    //listview handling
    void showHdrs(bool clear=true);
    void updateViewForCollection(KNArticleCollection *c);
    void updateListViewItems();
    void setAllThreadsOpen(bool b=true);
    bool showThreads()                { return s_howThreads; }
    void toggleShowThreads()          { s_howThreads=!s_howThreads; showHdrs(true); }
    void setShowThreads(bool b=true)  { s_howThreads=b; showHdrs(true); }

    void updateStatusString();

    //filter
    KNArticleFilter* filter() const   { return f_ilter; }
    void search();

    //collection handling
    void setGroup(KNGroup *g);
    void setFolder(KNFolder *f);
    KNArticleCollection* collection();

    //pgp signature check
    void verifyPGPSignature(KNArticle* a);

    //article loading
    bool loadArticle(KNArticle *a);
    bool unloadArticle(KNArticle *a, bool force=true);

    //article storage
    void saveInFolder(KNRemoteArticle::List &l, KNFolder *f);
    void saveInFolder(KNLocalArticle::List &l, KNFolder *f);
    bool deleteArticles(KNLocalArticle::List &l, bool ask=true);

    //article handling
    void setAllRead(bool r=true);
    void setRead(KNRemoteArticle::List &l, bool r=true, bool handleXPosts=true);

    void toggleWatched(KNRemoteArticle::List &l);
    void toggleIgnored(KNRemoteArticle::List &l);

    void rescoreArticles(KNRemoteArticle::List &l);

  protected:  
    void processJob(KNJobData *j);
    void createHdrItem(KNRemoteArticle *a);
    void createThread(KNRemoteArticle *a);

    KNListView *v_iew;
    KNGroup *g_roup;
    KNFolder *f_older;
    KNArticleFilter *f_ilter;
    KNFilterManager *f_ilterMgr;
    KNSearchDialog *s_earchDlg;
    QList<KTempFile> t_empFiles;
    bool s_howThreads;

  public slots:
    void slotFilterChanged(KNArticleFilter *f);
    void slotSearchDialogDone();
    void slotItemExpanded(QListViewItem *p);

};

#endif
