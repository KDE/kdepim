/*
    knhdrviewitem.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2004 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef KNHDRVIEWITEM_H
#define KNHDRVIEWITEM_H

#include <klistview.h>
#include "headerview.h"

class KNArticle;
class KNHeaderView;


class KNHdrViewItem : public KListViewItem  {

  public:
    KNHdrViewItem( KNHeaderView *ref, KNArticle *a = 0 );
    KNHdrViewItem( KNHdrViewItem *ref, KNArticle *a = 0 );
    ~KNHdrViewItem();

    virtual int compare(QListViewItem *i, int col, bool ascending) const;

    void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment);
    int width(const QFontMetrics &fm, const QListView *lv, int column);

    virtual QString text( int col ) const;

    void expandChildren();

    void setActive( bool b )  { mActive = b; }
    bool isActive() const     { return mActive; }

    // DND
    QDragObject* dragObject();

    KNArticle *art;
    int countUnreadInThread();

  private:
    bool greyOut();
    bool firstColBold();
    QColor normalColor();
    QColor greyColor();

    bool mActive;

};

#endif
