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
//Added by qt3to4:
#include <QFocusEvent>
#include <QEvent>
#include <QDropEvent>

class KMenu;
class KNNntpAccount;
class KNGroup;
class KNFolder;
class KNCollectionViewItem;

class KNCollectionView : public KFolderTree {

  Q_OBJECT

  public:
    KNCollectionView( QWidget *parent );
    ~KNCollectionView();

    void setActive(Q3ListViewItem *item);

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
    void folderDrop( QDropEvent *e, KNCollectionViewItem *item );

  protected:
    // dnd
    virtual Q3DragObject* dragObject();
    virtual void contentsDropEvent( QDropEvent *e );

    bool eventFilter( QObject *, QEvent * );

  private:
    Q3ListViewItem *mActiveItem;
    KMenu *mPopup;
    int mUnreadPop, mTotalPop;

};

#endif
