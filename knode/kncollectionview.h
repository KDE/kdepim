/*
    KNode, the KDE newsreader
    Copyright (c) 2004-2005 Volker Krause <vkrause@kde.org>
    Copyright (c) 2009 Olivier Trichet <nive@nivalis.org>

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

#include "kngroup.h"
#include "knfolder.h"
#include "knnntpaccount.h"

#include <foldertreewidget.h>

class QContextMenuEvent;
class QDropEvent;

using namespace KPIM;

/** The group/folder tree. */
class KNCollectionView : public FolderTreeWidget
{

  Q_OBJECT

  public:
    explicit KNCollectionView( QWidget *parent );
    ~KNCollectionView();

    /** Selects @p item and set it the current item. */
    void setActive( QTreeWidgetItem *item );

    void readConfig();
    void writeConfig();

  public slots:
    void addAccount( KNNntpAccount::Ptr a );
    void removeAccount( KNNntpAccount::Ptr a );
    void updateAccount( KNNntpAccount::Ptr a );
    void reloadAccounts();

    void addGroup( KNGroup::Ptr g );
    void removeGroup( KNGroup::Ptr g );
    void updateGroup( KNGroup::Ptr g );

    void addFolder( KNFolder::Ptr f );
    void removeFolder( KNFolder::Ptr f );
    void activateFolder( KNFolder::Ptr f );
    void updateFolder( KNFolder::Ptr f );
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


    virtual void paintEvent( QPaintEvent *event );


    // Drag and drop
    /**
      Reimplement to only accept MIME types which are accepted in dropEvent.
    */
    virtual void dragEnterEvent ( QDragEnterEvent *event );
    /**
      Reimplemented to hide the drop indicator when the drag leave the view.
    */
    virtual void dragLeaveEvent ( QDragLeaveEvent *event );
    /**
      Reimplemented to accept the @p event only when the target folder can be
      dropped articles or folders.
    */
    virtual void dragMoveEvent( QDragMoveEvent *event );
    /**
      Handles actual droping of articles or folder.
    */
    virtual void dropEvent( QDropEvent *event );
    /**
      Called by dragMoveEvent() and dropEvent().
      @param event The drop/drag-move event.
      @param enforceDrop If true the actual move of folder or move/copy of articles happens.
    */
    void handleDragNDropEvent( QDropEvent *event, bool enforceDrop );

    /**
      Returns the "x-knode-drag/folder" mime-type for drag&droping of folders within this view.
    */
    virtual QStringList mimeTypes() const;

    /**
      Reimplemented to perform the actual drag operations of folders.
      This takes care of showing a nice drag icon/cursor instead of the default that consists
      of the whole item widget (cells of label & counts).
      @param supportedActions unused parameter.
    */
    virtual void startDrag( Qt::DropActions supportedActions );


  private:
    /** Loads the column layout. */
    void loadLayout();

    QTreeWidgetItem *mActiveItem;

    /**
      Current target of a drag & drop operation.
      Used to highlight this item.
    */
    QTreeWidgetItem *mDragTargetItem;
};

#endif // KNODE_KNCOLLECTIONVIEW_H
