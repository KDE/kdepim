/*
    knfolder.h

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

#ifndef KNFOLDER_H
#define KNFOLDER_H

#include <time.h>

#include "utilities.h"
#include "knarticle.h"
#include "knarticlecollection.h"


class KNFolder : public KNArticleCollection  {
  
  friend class KNCleanUp; 

  public:
    KNFolder();
    KNFolder(int id, const QString &name, KNFolder *parent=0);
    KNFolder(int id, const QString &name, const QString &prefix, KNFolder *parent=0);
    ~KNFolder();

    //type
    collectionType type()               { return CTfolder; }

    //id
    int id()                            { return i_d; }
    void setId(int i)                   { i_d=i; }
    int parentId()                      { return p_arentId; }
    bool isStandardFolder()             { return (i_d > 0) && (i_d <=3); }
    bool isRootFolder()                 { return i_d==0; }

    //list item handling
    void updateListItem();
    bool wasOpen()                      { return w_asOpen; }

    //info
    QString path();
    bool readInfo(const QString &confPath);
    bool readInfo();
    void saveInfo();

    //article access
    KNLocalArticle* at(int i)           { return static_cast<KNLocalArticle*>(KNArticleCollection::at(i)); }
    KNLocalArticle* byId(int id)        { return static_cast<KNLocalArticle*>(KNArticleCollection::byId(id)); }
    KNLocalArticle* byMessageId(const QCString &mid)
                                        { return static_cast<KNLocalArticle*>(KNArticleCollection::byMessageId(mid)); }

    //parent
    void setParent(KNCollection *p);

    //load, save and delete
    bool loadHdrs();
    bool unloadHdrs(bool force=true);
    bool loadArticle(KNLocalArticle *a);
    bool saveArticles(KNLocalArticle::List *l);
    void removeArticles(KNLocalArticle::List *l, bool del=true);
    void deleteAll();
    void deleteFiles();

    //index synchronization
    void syncIndex(bool force=false);

    protected:
      void closeFiles();
      int i_d;            // unique id: 0: root folder 1-3: standard folders
      int p_arentId;      // -1 for the root folder
      bool i_ndexDirty;   // do we need to sync?
      bool w_asOpen;      // was this folder open in the listview on the last shutdown?
      KNFile m_boxFile;
      QFile i_ndexFile;
      QString i_nfoPath;

      /* helper-class: stores index-data of an article */
      class DynData {
        public:
          DynData()  {}
          ~DynData() {}
          void setData(KNLocalArticle *a);
          void getData(KNLocalArticle *a);
          
          int id,
              so,
              eo,
              sId;
          time_t ti;
          bool flags[6];
      };
};

#endif
