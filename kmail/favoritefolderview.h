/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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

#ifndef KMAIL_FAVORITEFOLDERVIEW_H
#define KMAIL_FAVORITEFOLDERVIEW_H

#include "kmfoldertree.h"

namespace KMail {

class FavoriteFolderView;

class FavoriteFolderViewItem : public KMFolderTreeItem
{
  Q_OBJECT
  public:
    FavoriteFolderViewItem( FavoriteFolderView *parent, const TQString & name, KMFolder* folder );

  protected:
    bool useTopLevelIcon() const { return false; }
    int iconSize() const { return 22; }

  private slots:
    void nameChanged();

  private:
    TQString mOldName;
};

class FavoriteFolderView : public FolderTreeBase
{
  Q_OBJECT

  public:
    FavoriteFolderView( KMMainWidget *mainWidget, TQWidget *parent = 0 );
    ~FavoriteFolderView();

    void readConfig();
    void writeConfig();

    KMFolderTreeItem* addFolder( KMFolder *folder, const TQString &name = TQString::null,
                                 TQListViewItem *after = 0 );
    void addFolder( KMFolderTreeItem *fti );

  public slots:
    void folderTreeSelectionChanged( KMFolder *folder );
    void checkMail();

  protected:
    bool acceptDrag(TQDropEvent* e) const;
    void contentsDragEnterEvent( TQDragEnterEvent *e );
    void readColorConfig();

  private:
    static TQString prettyName( KMFolderTreeItem* fti );
    KMFolderTreeItem* findFolderTreeItem( KMFolder* folder ) const;
    void handleGroupwareFolder( KMFolderTreeItem *fti );

  private slots:
    void selectionChanged();
    void itemClicked( TQListViewItem *item );
    void folderRemoved( KMFolder *folder );
    void dropped( TQDropEvent *e, TQListViewItem *after );
    void contextMenu( TQListViewItem *item, const TQPoint &point );
    void removeFolder();
    void initializeFavorites();
    void renameFolder();
    void addFolder();
    void notifyInstancesOnChange();
    void refresh();

  private:
    KMFolderTreeItem* mContextMenuItem;
    static TQValueList<FavoriteFolderView*> mInstances;
    bool mReadingConfig;
};

}

#endif
