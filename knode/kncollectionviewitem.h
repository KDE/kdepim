/*
    kncollectionviewitem.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef KNCOLLECTIONVIEWITEM_H
#define KNCOLLECTIONVIEWITEM_H

#include "knlistview.h"

class KNCollection;


class KNCollectionViewItem : public KNLVItemBase  {
  
  public:
    KNCollectionViewItem(KNListView *vi);
    KNCollectionViewItem(KNLVItemBase *it);
    ~KNCollectionViewItem();

    void setNumber(int column, int number);  // cache the values for compare()
    virtual int compare(QListViewItem *i, int col, bool ascending) const;    

    // DND
    virtual QDragObject* dragObject();
    virtual bool acceptDrag(QDropEvent* event) const;
    
    KNCollection *coll;

  protected:
    bool firstColBold();
    virtual QString shortString(QString text, int col, int width, QFontMetrics fm);
    int num[3];
};


#endif
