/*
    kncollectionview.h

    KNode, the KDE newsreader
    Copyright (c) 2004 Volker Krause <volker.krause@rwth-aachen.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNCOLLECTIONTREE_H
#define KNCOLLECTIONTREE_H

#include <kfoldertree.h>

class KPopupMenu;
class KNNntpAccount;
class KNGroup;
class KNFolder;
class KNCollectionViewItem;

class KNCollectionView : public KFolderTree {

  Q_OBJECT

  public:
    KNCollectionView(TQWidget *parent, const char *name = 0);
    ~KNCollectionView();

    void setActive(TQListViewItem *item);

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

    void toggleUnreadColumn();
    void toggleTotalColumn();
    void updatePopup() const;

  signals:
    void folderDrop( TQDropEvent *e, KNCollectionViewItem *item );

    void focusChanged( TQFocusEvent* );
    void focusChangeRequest( TQWidget* );

  protected:
    // dnd
    virtual TQDragObject* dragObject();
    virtual void contentsDropEvent( TQDropEvent *e );

    bool eventFilter( TQObject *, TQEvent * );
    void focusInEvent( TQFocusEvent *e );
    void focusOutEvent( TQFocusEvent *e );

  private:
    TQListViewItem *mActiveItem;
    KPopupMenu *mPopup;
    int mUnreadPop, mTotalPop;

};

#endif
