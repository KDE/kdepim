/*
    kngroup.h

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

#ifndef KNGROUP_H
#define KNGROUP_H

#include "knarticlecollection.h"
#include "knmime.h"
#include "knjobdata.h"

class KNProtocolClient;
class KNNntpAccount;
class QStrList;

namespace KNConfig {
class Identity;
};

class KNGroup : public KNArticleCollection , public KNJobItem  {
  
  public:
    enum Status { unknown=0, readOnly=1, postingAllowed=2, moderated=3 };

    KNGroup(KNCollection *p=0);
    ~KNGroup();

    // type
    collectionType type()               { return CTgroup; }

    // list-item handling
    void updateListItem();

    // info
    QString path();
    bool readInfo(const QString &confPath);
    void saveInfo();

    // name
    bool hasName()                          { return (!n_ame.isEmpty()); }
    const QString& name();
    const QString& groupname()              { return g_roupname; }
    void setGroupname(const QString &s)     { g_roupname=s; }
    const QString& description()            { return d_escription; }
    void setDescription(const QString &s)   { d_escription=s; }

    // count + numbers
    int newCount()                { return n_ewCount; }
    void setNewCount(int i)       { n_ewCount=i; }
    void incNewCount(int i=1)     { n_ewCount+=i; }
    void decNewCount(int i=1)     { n_ewCount-=i; }
    int firstNewIndex()           { return f_irstNew; }
    void setFirstNewIndex(int i)  { f_irstNew=i; }


    int readCount()               { return r_eadCount; }
    void setReadCount(int i)      { r_eadCount=i; }
    void incReadCount(int i=1)    { r_eadCount+=i; }
    void decReadCount(int i=1)    { r_eadCount-=i; }

    int firstNr()                 { return f_irstNr; }
    void setFirstNr(int i)        { f_irstNr=i; }
    int lastNr()                  { return l_astNr; }
    void setLastNr(int i)         { l_astNr=i; }
    int maxFetch()                { return m_axFetch; }
    void setMaxFetch(int i)       { m_axFetch=i; }

    int statThrWithNew();
    int statThrWithUnread();

    // article access
    KNRemoteArticle* at(int i)          { return static_cast<KNRemoteArticle*> (KNArticleCollection::at(i)); }
    KNRemoteArticle* byId(int id)       { return static_cast<KNRemoteArticle*> (KNArticleCollection::byId(id)); }
    KNRemoteArticle* byMessageId(const QCString &mId)
                                        { return static_cast<KNRemoteArticle*> (KNArticleCollection::byMessageId(mId)); }
    // load + save
    bool loadHdrs();
    void insortNewHeaders(QStrList *hdrs, KNProtocolClient *client=0);
    int saveStaticData(int cnt,bool ovr=false);
    void saveDynamicData(int cnt,bool ovr=false);
    void syncDynamicData();

    // mark articles with this id as read when we later load the headers / fetch new articles
    void appendXPostID(const QString &id);
    void processXPostBuffer(bool deleteAfterwards);

    // article handling
    void updateThreadInfo();
    void reorganize();
    void scoreArticles(bool onlynew=true);

    // locking
    bool isLocked()             { return l_ocked; }
    void setLocked(bool l)      { l_ocked=l; }

    // charset-handling
    const QCString defaultCharset()           { return d_efaultChSet; }
    void setDefaultCharset(const QCString &s) { d_efaultChSet=s; }
    bool useCharset()                         { return ( u_seCharset && !d_efaultChSet.isEmpty() ); }
    void setUseCharset(bool b)                { u_seCharset=b; }

    // misc
    KNNntpAccount* account();
    KNConfig::Identity* identity()          { return i_dentity; }
    void setIdentity(KNConfig::Identity *i) { i_dentity=i; }
    Status status()                         { return s_tatus; }
    void setStatus(Status s)                { s_tatus=s; }
    void showProperties();


  protected:
    void buildThreads(int cnt, KNProtocolClient *client=0);
    KNRemoteArticle* findReference(KNRemoteArticle *a);
        
    int       n_ewCount,
              r_eadCount,
              f_irstNr,
              l_astNr,
              m_axFetch,
              d_ynDataFormat,
              f_irstNew;

    QCString  d_efaultChSet;
    QString   g_roupname,
              d_escription;

    bool      l_ocked,
              u_seCharset;

    Status    s_tatus;

    QStringList c_rosspostIDBuffer;

    KNConfig::Identity *i_dentity;

    class dynDataVer0 {
      
      public:
        dynDataVer0()     { id=-1; idRef=-1; read=0; thrLevel=0; score=50; }
        ~dynDataVer0()    {}
        void setData(KNRemoteArticle *a);
        void getData(KNRemoteArticle *a);
      
        int id;
        int idRef;
        bool read;
        short thrLevel, score;
    };

    class dynDataVer1 {

      public:
        dynDataVer1()     { id=-1; idRef=-1; read=0; thrLevel=0; score=0, ignoredWatched=0; }
        ~dynDataVer1()    {}
        void setData(KNRemoteArticle *a);
        void getData(KNRemoteArticle *a);

        int id;
        int idRef;
        bool read;
        short thrLevel, score;
        char ignoredWatched;
    };
    
};

#endif
