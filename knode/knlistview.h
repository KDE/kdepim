/*
    knlistview.h

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

#ifndef KNLISTVIEW_H
#define KNLISTVIEW_H

#include <klistview.h>

class KNListView;
class QPainter;

class KNLVItemBase : public KListViewItem  {
  public:
  /** restricted to KNListView to prevent that the
      static_cast in @ref ~KNLVItemBase fails. */
    KNLVItemBase(KNListView *view);
    KNLVItemBase(KNLVItemBase *item);
    ~KNLVItemBase();

    void setActive(bool b)  { a_ctive = b; }
    bool isActive()const         { return a_ctive; }

    void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment);
    int width(const QFontMetrics &fm, const QListView *lv, int column);
    void sortChildItems(int column, bool a);
    void expandChildren();

    // DND
    virtual QDragObject *dragObject()              { return 0; }

    virtual int countUnreadInThread()   { return 0; }

  protected:
    virtual bool greyOut()          { return false; }
    virtual bool firstColBold()     { return false; }
    virtual QColor normalColor();
    virtual QColor greyColor();

  private:
    bool a_ctive;

};


class KNListView : public KListView  {

  Q_OBJECT

  friend class KNLVItemBase;

  public:
    KNListView(QWidget *parent, const char *name=0);
    ~KNListView();

    int sortColumn() const               { return s_ortCol; }
    bool ascending() const               { return s_ortAsc; }
    void setColAsc(int c, bool a)   { s_ortCol=c; s_ortAsc=a; }
    bool sortByThreadChangeDate() const    { return s_ortByThreadChangeDate; }
    void setSortByThreadChangeDate(bool b) { s_ortByThreadChangeDate = b; }

    void setActive(QListViewItem *item, bool activate);
    void clear();
    void clearSelection();

    void ensureItemVisibleWithMargin(const QListViewItem *i);

    virtual void reparent(QWidget *parent, WFlags f, const QPoint &p, bool showIt=false);
  public slots:
    void slotSortList(int col);
    void slotSizeChanged(int,int,int);

  protected:
    void activeRemoved()            { a_ctiveItem = 0; }
    /**
     * Reimplemented to avoid that KListview reloads the alternate
     * background on palette changes. ;-)
     */
    virtual bool event(QEvent *e);
    void contentsMousePressEvent(QMouseEvent *e);
    void contentsMouseDoubleClickEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *e);
    bool eventFilter(QObject *, QEvent *);
    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);
    virtual QDragObject* dragObject();

    bool s_ortAsc, s_ortByThreadChangeDate;
    int s_ortCol, d_elayedCenter;
    KNLVItemBase *a_ctiveItem;
    bool k_eepSelection;

  protected slots:
    void slotCenterDelayed();

  signals:
    void itemSelected(QListViewItem*);
    void doubleClick( QListViewItem* );
    void sortingChanged(int);
    void focusChanged(QFocusEvent*);
    void focusChangeRequest(QWidget*);

    void reparented();
};

#endif
