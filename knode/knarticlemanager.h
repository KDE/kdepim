/*
    knarticlemanager.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2000 the KNode authors.
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
#include <kurl.h>
#include <qfile.h>
#include <qlistview.h>

#include "knmime.h"

class KNArticle;
class KTempFile;
class KNListView;
class KNThread;
class KNArticleCollection;
class KNGroup;
class KNFolder;
class KNArticleFilter;
class KNFilterManager;
class KNSearchDialog;


//===============================================================================


// handles file saving for KNArticleManager => no duplicated code
class KNSaveHelper {

public:
  
  KNSaveHelper(QString saveName, QWidget *parent);
  ~KNSaveHelper();
  
  // returns a file open for writing
  QFile* getFile(QString dialogTitle);
  
private:

  QWidget *p_arent;
  QString s_aveName;
  KURL url;
  QFile* file;
  KTempFile* tmpFile;
  static QString lastPath;

};


//===============================================================================


class KNArticleManager : public QObject {

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
    void setAllThreadsOpen(bool b=true);
    bool showThreads()                { return s_howThreads; }
    void toggleShowThreads()          { s_howThreads=!s_howThreads; showHdrs(true); }
    void setShowThreads(bool b=true)  { s_howThreads=b; showHdrs(true); }

    //filter
    KNArticleFilter* filter() const { return f_ilter; }
    void search();

    //collection handling
    void setGroup(KNGroup *g);
    void setFolder(KNFolder *f);
    KNArticleCollection* collection();

    //pgp signature check
    void verifyPGPSignature(KNArticle* a);

    //article handling - RemoteArticles
    void setAllRead(bool r=true);
    void setRead(KNRemoteArticle::List *l, bool r=true);
    void toggleWatched(KNRemoteArticle::List *l);
    void toggleIgnored(KNRemoteArticle::List *l);
    void setScore(KNRemoteArticle::List *a, int score=-1);

    //article handling - LocalArticles
    //soon to come ..

  protected:  
    void createHdrItem(KNRemoteArticle *a);
    void createThread(KNRemoteArticle *a);
    void updateStatusString();

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
