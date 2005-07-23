/*
    kncollectionviewitem.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2004 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNCOLLECTIONVIEWITEM_H
#define KNCOLLECTIONVIEWITEM_H

#include <kfoldertree.h>

class QPainter;
class QColorGroup;

class KNCollection;


class KNCollectionViewItem : public KFolderTreeItem  {

  public:
    KNCollectionViewItem( KFolderTree *parent, Protocol protocol = NONE, Type type = Root);
    KNCollectionViewItem( KFolderTreeItem *parent, Protocol protocol = NONE,
                          Type type = Other, int unread = 0, int total = 0 );
    ~KNCollectionViewItem();

    void paintCell( QPainter * p, const QColorGroup & cg,
                    int column, int width, int align );

    int compare(QListViewItem *i, int col, bool ascending) const;

    // DND
    virtual bool acceptDrag(QDropEvent* event) const;

    KNCollection *coll;

  protected:
    virtual QString squeezeFolderName( const QString &text,
                                       const QFontMetrics &fm,
                                       uint width ) const;

  private:
    void setIcon();

};

#endif
