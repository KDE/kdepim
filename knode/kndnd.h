/*
    kndnd.h

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

#ifndef KNDND_H
#define KNDND_H

#include <qdragobject.h>
#include <qlistview.h>



class KNDragHandler {

  public:
    KNDragHandler() { w_idget=0; }
    virtual ~KNDragHandler() {}

    // managed widget
    QWidget* widget()           { return w_idget; }
    void setWidget(QWidget *w)  { w_idget=w; }

    // drag handling
    virtual void startDrag(QListViewItem*)  { }


    // drop handling
    virtual bool accept(QDragEnterEvent*)                 { return false; }
    virtual bool accept(QDragMoveEvent*, QListViewItem*)  { return false; }
    virtual bool accept(QDropEvent*, QListViewItem*)      { return false; }

  protected:
    QWidget *w_idget;


};


class KNArticleDragHandler : public KNDragHandler {

  public:
    KNArticleDragHandler() : KNDragHandler() {}
    ~KNArticleDragHandler() {}

    // drag handling
    void startDrag(QListViewItem *i);
};


class KNCollectionDragHandler : public KNDragHandler {

  public:
    KNCollectionDragHandler() : KNDragHandler() {}
    ~KNCollectionDragHandler() {}

    // drag handling
    void startDrag(QListViewItem *i);

    // drop handling
    bool accept(QDragEnterEvent *e);
    bool accept(QDragMoveEvent *e, QListViewItem *i);
    bool accept(QDropEvent *e, QListViewItem *i);

};


#endif

