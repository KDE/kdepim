/*
    Copyright (c) 2008 Pradeepto K. Bhattacharya <pradeepto@kde.org>
      ( adapted from kdepim/kmail/kmfolderseldlg.cpp and simplefoldertree.h  )

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef KMAIL_TREEBASE_H
#define KMAIL_TREEBASE_H

#include "kmfolder.h"
#include "kmfoldertree.h"

#include <kdebug.h>
#include <klistview.h>

namespace KMail {

class TreeItemBase;

class TreeBase : public KListView
{
    Q_OBJECT
    public:
    TreeBase( TQWidget * parent, KMFolderTree *folderTree,
        const TQString &preSelection, bool mustBeReadWrite );
	
    virtual  ~TreeBase() {}

     const KMFolder * folder() const;
    /** Set the current folder */
    void setFolder( KMFolder *folder );

    inline void setFolder( const TQString& idString )
    {
      setFolder( kmkernel->findFolderById( idString ) );
    }

    void reload( bool mustBeReadWrite, bool showOutbox, bool showImapFolders,
                   const TQString& preSelection = TQString::null );

    int folderColumn() const {  return mFolderColumn; }
    void setFolderColumn( const int folderCol ) { mFolderColumn = folderCol; }
    int pathColumn() const { return mPathColumn; }
    void setPathColumn( const int pathCol ) { mPathColumn = pathCol; }

    public slots:
	void addChildFolder();
    protected slots:
	void slotContextMenuRequested( TQListViewItem *lvi,
					      const TQPoint &p );
    void recolorRows();
protected:
    virtual TQListViewItem* createItem( TQListView* ) = 0;
    virtual TQListViewItem* createItem( TQListView*, TQListViewItem* ) = 0;
    virtual TQListViewItem* createItem( TQListViewItem* ) = 0;
    virtual TQListViewItem* createItem( TQListViewItem*, TQListViewItem* ) = 0;

  protected:
      KMFolderTree* mFolderTree;
      TQString mFilter;
      bool mLastMustBeReadWrite;
      bool mLastShowOutbox;
      bool mLastShowImapFolders;
      /** Folder and path column IDs. */
      int mFolderColumn;
      int mPathColumn;
     
};
}
#endif
