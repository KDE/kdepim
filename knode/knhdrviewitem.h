/*
    knhdrviewitem.h

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

#ifndef KNHDRVIEWITEM_H
#define KNHDRVIEWITEM_H

#include <qfont.h>
#include <qcache.h>

#include "knlistview.h"

class KNArticle;


class KNHdrViewItem : public KNLVItemBase  {
  
  public:
    KNHdrViewItem(KNListView *ref, KNArticle *a=0);
    KNHdrViewItem(KNLVItemBase *ref, KNArticle *a=0);
    ~KNHdrViewItem();

    virtual int compare(QListViewItem *i, int col, bool ascending) const;

    // DND
    virtual QDragObject* dragObject();

    KNArticle *art;
    bool firstColBold();
    virtual int countUnreadInThread();

  protected:
    bool greyOut();
    QColor normalColor();
    QColor greyColor();

};

#endif
