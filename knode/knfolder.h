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

#ifndef KNFOLDER_H
#define KNFOLDER_H

#include "utilities.h"
#include "knarticle.h"
#include "knarticlecollection.h"

#include <QByteArray>


/** Representation of a folder. This includes:
 * - Information about the folder (eg. name, parent)
 * - Methods to load the folder content from a mbox file.
 * - Methods to store the folder content in a mbox file.
 */
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
    int id() const                           { return i_d; }
    void setId(int i)                   { i_d=i; }
    int parentId() const                     { return p_arentId; }
    bool isStandardFolder()             { return (i_d > 0) && (i_d <=3); }
    bool isRootFolder()                 { return i_d==0; }

    //list item handling
    void updateListItem();
    bool wasOpen()const                      { return w_asOpen; }

    //info
    QString path();
    bool readInfo(const QString &confPath);
    bool readInfo();
    void saveInfo();

    //article access
    KNLocalArticle* at(int i)           { return static_cast<KNLocalArticle*>(KNArticleCollection::at(i)); }
    KNLocalArticle* byId(int id)        { return static_cast<KNLocalArticle*>(KNArticleCollection::byId(id)); }
    KNLocalArticle* byMessageId( const QByteArray &mid )
                                        { return static_cast<KNLocalArticle*>(KNArticleCollection::byMessageId(mid)); }

    //parent
    void setParent(KNCollection *p);

    //load, save and delete
    bool loadHdrs();
    bool unloadHdrs(bool force=true);
    bool loadArticle(KNLocalArticle *a);
    bool saveArticles( KNLocalArticle::List &l );
    void removeArticles( KNLocalArticle::List &l, bool del = true );
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
      QFile m_boxFile;
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
