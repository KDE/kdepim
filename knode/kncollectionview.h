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
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef KNCOLLECTIONTREE_H
#define KNCOLLECTIONTREE_H

#include <kfoldertree.h>

class KNNntpAccount;
class KNGroup;
class KNFolder;
class KNCollectionViewItem;

class KNCollectionView : public KFolderTree {

  Q_OBJECT

  public:
    KNCollectionView(QWidget *parent, const char *name = 0);

    void setActive(QListViewItem *item);

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

    void nextGroup();
    void prevGroup();

    // KMail like keyboard navigation
    void decCurrentFolder();
    void incCurrentFolder();
    void selectCurrentFolder();

  signals:
    void folderDrop( QDropEvent *e, KNCollectionViewItem *item );

    void focusChanged( QFocusEvent* );
    void focusChangeRequest( QWidget* );

  protected:
    // dnd
    virtual QDragObject* dragObject();
    virtual void contentsDropEvent( QDropEvent *e );

    bool eventFilter( QObject *, QEvent * );
    void focusInEvent( QFocusEvent *e );
    void focusOutEvent( QFocusEvent *e );

  private:
    QListViewItem *mActiveItem;

  private slots:
    void slotSizeChanged(int section, int, int newSize);

};

#endif
