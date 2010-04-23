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
    /**
     * Shared pointer to a KNFolder. To be used instead of raw KNFolder*.
     */
    typedef boost::shared_ptr<KNFolder> Ptr;
    /**
     * List of folders.
     */
    typedef QList<KNFolder::Ptr> List;

    KNFolder();
    KNFolder(int id, const QString &name, KNFolder::Ptr parent = KNFolder::Ptr() );
    KNFolder( int id, const QString &name, const QString &prefix, KNFolder::Ptr parent = KNFolder::Ptr() );
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
    void writeConfig();

    //article access
    KNLocalArticle::Ptr at( int i )
      { return boost::static_pointer_cast<KNLocalArticle>( KNArticleCollection::at( i ) ); }
    KNLocalArticle::Ptr byId( int id )
      { return boost::static_pointer_cast<KNLocalArticle>( KNArticleCollection::byId( id ) ); }
    KNLocalArticle::Ptr byMessageId( const QByteArray &mId )
      { return boost::static_pointer_cast<KNLocalArticle>( KNArticleCollection::byMessageId( mId ) ); }

    //parent
    void setParent( KNCollection::Ptr p );

    //load, save and delete
    bool loadHdrs();
    bool unloadHdrs(bool force=true);
    /**
      Load the full content of an article.
      @param a the article to load.
      @return true if the article is successfully loaded.
    */
    bool loadArticle( KNLocalArticle::Ptr a );
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
          void setData( KNLocalArticle::Ptr a );
          void getData( KNLocalArticle::Ptr a );

          int id,
              so,
              eo,
              sId;
          time_t ti;
          bool flags[6];
      };

  private:
    /**
     * Returns a shared pointer pointing to this folder.
     */
    KNFolder::Ptr thisFolderPtr();

    /**
     * Reimplemented from KNArticleCollection::selfPtr().
     */
    virtual KNCollection::Ptr selfPtr()
    {
      return thisFolderPtr();
    }

};

#endif
