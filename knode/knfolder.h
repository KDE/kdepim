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

#include "knarticlecollection.h"
#include "knmime.h"
#include <time.h>


class KNFolder : public KNArticleCollection  {
  
  friend class KNCleanUp; 

  public:
    KNFolder(int id, const QString &name, KNCollection *parent=0);
    KNFolder(int id, const QString &name, const QString &prefix, KNCollection *parent=0);
    ~KNFolder();

    //type
    collectionType type()               { return CTfolder; }

    //id
    int id()                            { return i_d; }
    void setId(int i)                   { i_d=i; }

    //list item handling
    void updateListItem();

    //info
    QString path();
    bool readInfo(const QString &confPath);
    void saveInfo();

    //article access
    KNLocalArticle* at(int i)           { return static_cast<KNLocalArticle*>(list[i]); }
    KNLocalArticle* byId(int id);

    //load, save and delete
    bool loadHdrs();
    bool loadArticle(KNLocalArticle *a);
    bool saveArticles(KNLocalArticle::List *l);
    void removeArticles(KNLocalArticle::List *l, bool del=true);
    void deleteAll();

    //index synchronization
    void syncIndex(bool force=false);

        
    protected:
      void closeFiles();
      int i_d;            //unique id
      bool i_ndexDirty;   //do we need to sync?
      KNFile m_boxFile;
      QFile i_ndexFile;

      /* helper-class: stores index-data of an article */
      class DynData {
        public:
          DynData()  {}
          ~DynData() {}
          void setData(KNLocalArticle *a);
          
          int id,
              so,
              eo,
              sId;
          time_t ti;
          bool flags[6];
      };
};



#endif
