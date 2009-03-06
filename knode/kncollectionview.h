/*
    KNode, the KDE newsreader
    Copyright (c) 2004-2005 Volker Krause <volker.krause@rwth-aachen.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNODE_KNCOLLECTIONVIEW_H
#define KNODE_KNCOLLECTIONVIEW_H

#include <foldertreewidget.h>

#include <QDropEvent>

class KNNntpAccount;
class KNGroup;
class KNFolder;
class KNCollectionViewItem;

class QContextMenuEvent;

using namespace KPIM;

/** The group/folder tree. */
class KNCollectionView : public FolderTreeWidget
{

  Q_OBJECT

  public:
    KNCollectionView( QWidget *parent );
    ~KNCollectionView();

    /** Selects @p item and set it the current item. */
    void setActive( QTreeWidgetItem *item );

    void readConfig();
    void writeConfig();

  public slots:
    void addAccount(KNNntpAccount* a);
    void removeAccount(KNNntpAccount* a);
    void updateAccount(KNNntpAccount* a);
    void reloadAccounts();

    void addGroup(KNGroup* g);
    void removeGroup(KNGroup* g);
    void updateGroup(KNGroup* g);

    void addFolder(KNFolder* f);
    void removeFolder(KNFolder* f);
    void activateFolder(KNFolder* f);
    void updateFolder(KNFolder* f);
    void addPendingFolders();
    void reloadFolders();

    void reload();

    void nextGroup();
    void prevGroup();

    // KMail like keyboard navigation
    void decCurrentFolder();
    void incCurrentFolder();
    void selectCurrentFolder();

  signals:
    void folderDrop( QDropEvent *e, KNCollectionViewItem *item );
    /**
      This signal is emitted when a context menu should be displayed
      for @p item as global position @p position.
      */
    void contextMenu( QTreeWidgetItem *item, const QPoint &position );

  protected:
    /**
      Emits the signal contextMenu() to display a context menu on
      items of the view.
      Reimplemented from QAbstractScrollArea.
      */
    virtual void contextMenuEvent( QContextMenuEvent *event );

    // dnd
/* TODO
    virtual Q3DragObject* dragObject();
    virtual void contentsDropEvent( QDropEvent *e );
*/

  private:
    /** Loads the column layout. */
    void loadLayout();

    QTreeWidgetItem *mActiveItem;
};

#endif // KNODE_KNCOLLECTIONVIEW_H
