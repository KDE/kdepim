/*
    knlistview.h

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

#ifndef KNLISTVIEW_H
#define KNLISTVIEW_H

#include <qlistview.h>
#include <qlist.h>
#include <kaction.h>

class KNListView;
class KNDragHandler;


class KNLVItemBase : public QListViewItem  {
  
  public:
    KNLVItemBase(KNListView *view);      // restricted to KNListView to prevent that the
    KNLVItemBase(KNLVItemBase *item);    // static_cast in ~KNLVItemBase fails.
    ~KNLVItemBase();

    void setActive(bool b)  { a_ctive = b; };
    bool isActive()         { return a_ctive; }

    void hover(bool b)      { h_over=b; }

    void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment);
    int width(const QFontMetrics &fm, const QListView *lv, int column);
    void paintFocus(QPainter *, const QColorGroup & cg, const QRect & r);
    void sortChildItems(int column, bool a);
    void expandChildren();

  protected:
    virtual bool greyOut()          { return false; }
    virtual bool firstColBold()     { return false; }
    virtual QColor normalColor();
    virtual QColor greyColor();
    virtual QString shortString(QString text, int col, int width, QFontMetrics fm);
    virtual const QFont& fontForColumn(int, const QFont &font)    { return font; }

  private:
    bool a_ctive, h_over;

};


class KNListView : public QListView  {

  Q_OBJECT

  friend class KNLVItemBase;

  public:
    KNListView(QWidget *parent, const char *name=0, KNDragHandler *dh=0);
    ~KNListView();

    int sortColumn()                { return s_ortCol; }
    bool ascending()                { return s_ortAsc; }
    void setColAsc(int c, bool a)   { s_ortCol=c; s_ortAsc=a; }

    void setActive(QListViewItem *item, bool activate);
    void clear();

    void triggerDropError(const QString &e);

  public slots:
    void slotSortList(int col);
    void slotSizeChanged(int,int,int);
      
  protected:
    void activeRemoved()            { a_ctiveItem = 0; }
    void contentsMousePressEvent(QMouseEvent *e);
    void contentsMouseMoveEvent(QMouseEvent *e);
    void contentsMouseReleaseEvent(QMouseEvent *e);
    void contentsDragEnterEvent(QDragEnterEvent *e);
    void contentsDragMoveEvent(QDragMoveEvent *e);
    void contentsDragLeaveEvent(QDragLeaveEvent *e);
    void contentsDropEvent(QDropEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);

    bool s_ortAsc;
    int s_ortCol;
    KNLVItemBase *a_ctiveItem;
    KNDragHandler *d_handler;
    QPoint d_ragStartPos;
    bool d_ragMousePressed;
    KNLVItemBase *d_ragHoverItem;
    QString d_ropError;

  protected slots:
    void slotShowDropError();

  signals:
    void itemSelected(QListViewItem*);
    void sortingChanged(int);
    void focusChanged(QFocusEvent*);
    void dropReceived(const char*,QListViewItem*);

};



#endif

