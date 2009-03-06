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

#ifndef KNCOLLECTIONVIEWITEM_H
#define KNCOLLECTIONVIEWITEM_H

#include <foldertreewidget.h>

#include <QDropEvent>

class KNCollection;

using namespace KPIM;

/** Folder tree item. */
class KNCollectionViewItem : public FolderTreeWidgetItem  {

  public:
    KNCollectionViewItem( FolderTreeWidget *parent, Protocol protocol = NONE, FolderType type = Root);
    KNCollectionViewItem( FolderTreeWidgetItem *parent, Protocol protocol = NONE,
                          FolderType type = Other, int unread = 0, int total = 0 );
    ~KNCollectionViewItem();

    /**
      Sort newsgroups before local folders.
      Reimplemented from FolderTreeWidgetItem.
     */
    bool operator<( const QTreeWidgetItem &other ) const;

    // DND
    virtual bool acceptDrag(QDropEvent* event) const;

    KNCollection *coll;

  protected:
    /**
      Elid names of group according to usenet habit.
      (e.g. fr.comp.lang.perl is elided to f.c.lang.perl)
     */
    virtual QString elidedLabelText( const QFontMetrics &fm, unsigned int width ) const;
  private:
    /** Inialize this item. */
    void setUp();

};

#endif
