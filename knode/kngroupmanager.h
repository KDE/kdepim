/*
    kngroupmanager.h

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

#ifndef KNGROUPMANAGER_H
#define KNGROUPMANAGER_H

#include <qlist.h>
#include <qstrlist.h>
#include <qdatetime.h>
#include <qsortedlist.h>
#include <kaction.h>

#include "knjobdata.h"

class KNGroup;
class KNCleanUp;
class KNNntpAccount;
class KNServerInfo;
class KNArticleManager;


//=================================================================================

// helper classes for the group selection dialog
// contains info about a newsgroup (name, description)

class KNGroupInfo {

  public:
    KNGroupInfo();
    KNGroupInfo(const char *n_ame, const char *d_escription, bool n_ewGroup=false, bool s_ubscribed=false );
    ~KNGroupInfo();

    QCString name,description;
    bool newGroup, subscribed;

    bool operator== (const KNGroupInfo &gi2);
    bool operator< (const KNGroupInfo &gi2);
};


class KNGroupListData : public KNJobItem {

  public:
    KNGroupListData();
    ~KNGroupListData();

    bool readIn();
    bool writeOut();
    void merge(QSortedList<KNGroupInfo>* newGroups);

    QSortedList<KNGroupInfo>* extractList();

    QStrList subscribed;
    QString path;
    QSortedList<KNGroupInfo> *groups;
    QDate fetchSince;
    bool getDescriptions;

};

//===============================================================================


class KNGroupManager : public QObject , public KNJobConsumer {

  Q_OBJECT
      
  public:

    KNGroupManager(KNArticleManager *a, QObject * parent=0, const char * name=0);
    ~KNGroupManager();

    void loadGroups(KNNntpAccount *a);
    void getSubscribed(KNNntpAccount *a, QStrList* l);
    void getGroupsOfAccount(KNNntpAccount *a, QList<KNGroup> *l);   
    void showGroupDialog(KNNntpAccount *a, QWidget *parent=0);
    void subscribeGroup(const KNGroupInfo *gi, KNNntpAccount *a);
    void unsubscribeGroup(KNGroup *g=0);
    void showGroupProperties(KNGroup *g=0);
    void checkGroupForNewHeaders(KNGroup *g=0);
    void expireGroupNow(KNGroup *g=0);
    void resortGroup(KNGroup *g=0);
      
    KNGroup* group(const QCString &gName, const KNServerInfo *s);
    KNGroup* currentGroup()               { return c_urrentGroup; }
    bool hasCurrentGroup()                { return (c_urrentGroup!=0); }
    void setCurrentGroup(KNGroup *g);
    
    void checkAll(KNNntpAccount *a);
    void expireAll(KNCleanUp *cup);
    void syncGroups();    
    void jobDone(KNJobData *j);     
  
  public slots:
    void slotLoadGroupList(KNNntpAccount *a);      // load group list from disk (if this fails: ask user if we should fetch the list)
    void slotFetchGroupList(KNNntpAccount *a);     // fetch group list from server
    void slotCheckForNewGroups(KNNntpAccount *a, QDate date);    // check for new groups (created after the given date)
    
  protected:
		void processJob(KNJobData *j); //reimplemented from KNJobConsumer
    QList<KNGroup>  *g_List;
    KNGroup *c_urrentGroup;
    KNArticleManager *a_rticleMgr;

  signals:
    void newListReady(KNGroupListData* d);
      
};



#endif
