/***************************************************************************
                          kngroup.h  -  description
                             -------------------

    copyright            : (C) 2000 by Christian Thurner
    email                : cthurner@freepage.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef KNGROUP_H
#define KNGROUP_H

#include "knarticlecollection.h"
#include "knmime.h"
#include "knjobdata.h"


class KNNntpAccount;
class QStrList;

namespace KNConfig {
class Identity;
};

class KNGroup : public KNArticleCollection , public KNJobItem  {
  
  public:
    KNGroup(KNCollection *p=0);
    ~KNGroup();
    
    void updateListItem();
    
    bool readInfo(const QString &confPath);
    void saveInfo();

    void showProperties();
    bool loadHdrs();
    void insortNewHeaders(QStrList *hdrs);
    int saveStaticData(int cnt,bool ovr=false);
    void saveDynamicData(int cnt,bool ovr=false);
    void syncDynamicData();
    void updateThreadInfo();
    void resort();
    
    //get
    collectionType type()               { return CTgroup; }
    QString path();
    KNNntpAccount* account();
    KNRemoteArticle* at(int i)           { return static_cast<KNRemoteArticle*> (list[i]); }
    KNRemoteArticle* byId(int id);
    KNRemoteArticle* byMessageId(const QCString &mId);
    int newCount()                      { return n_ewCount; }
    int readCount()                     { return r_eadCount; }
    int lastNr()                        { return l_astNr; }
    int maxFetch()                      { return m_axFetch; }
    const QString& name();
    const QCString& groupname()         { return g_roupname; }
    const QCString& description()       { return d_escription; }
    KNConfig::Identity* identity()      { return i_dentity; }
    bool hasName()                      { return (!n_ame.isEmpty()); }
    int statThrWithNew();
    int statThrWithUnread();

    bool isLocked()                     { return l_ocked; }


    
    //set
    void setGroupname(const QCString &s)    { g_roupname=s; }
    void setDescription(const QCString &s)  { d_escription=s; }
    void setNewCount(int i)                 { n_ewCount=i; }
    void incNewCount(int i=1)               { n_ewCount+=i; }
    void decNewCount(int i=1)               { n_ewCount-=i; }
    void setReadCount(int i)                { r_eadCount=i; }
    void incReadCount(int i=1)              { r_eadCount+=i; }
    void decReadCount(int i=1)              { r_eadCount-=i; }
    void setLastNr(int i)                   { l_astNr=i; }
    void setMaxFetch(int i)                 { m_axFetch=i; }
    void setIdentity(KNConfig::Identity *i) { i_dentity=i; }
    void setLocked(bool l)                  { l_ocked=l; }

                
  protected:
    void sortHdrs(int cnt);
    int findRef(KNRemoteArticle *a, int from, int to, bool reverse=false);
        
    int n_ewCount, r_eadCount, l_astNr, m_axFetch;
    QCString g_roupname, d_escription;
    bool l_ocked;
    int l_oading;
    KNConfig::Identity *i_dentity;

    class dynData {
      
      public:
        dynData()     { id=-1; idRef=-1; read=0; thrLevel=0; score=50; }
        ~dynData()    {}  
        void setData(KNRemoteArticle *a);
      
        int id;
        int idRef;
        bool read;
        short thrLevel, score;
    };
    
};

#endif
