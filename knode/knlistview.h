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

#include <qbitarray.h>

#include <klistview.h>

class KNListView;


class KNLVItemBase : public QListViewItem  {
  
  public:
  /** restricted to KNListView to prevent that the
      static_cast in @ref ~KNLVItemBase fails. */
    KNLVItemBase(KNListView *view);
    KNLVItemBase(KNLVItemBase *item);
    ~KNLVItemBase();

    void setActive(bool b)  { a_ctive = b; };
    bool isActive()         { return a_ctive; }

    void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment);
    int width(const QFontMetrics &fm, const QListView *lv, int column);
    void paintFocus(QPainter *, const QColorGroup & cg, const QRect & r);
    void sortChildItems(int column, bool a);
    void expandChildren();

    // DND
    virtual QDragObject *dragObject() const             { return 0; };
    virtual bool acceptDrag(QDropEvent* ) const    { return false; };

  protected:
    virtual bool greyOut()          { return false; }
    virtual bool firstColBold()     { return false; }
    virtual QColor normalColor();
    virtual QColor greyColor();
    virtual QString shortString(QString text, int col, int width, QFontMetrics fm);
    virtual const QFont& fontForColumn(int, const QFont &font)    { return font; }

  private:
    bool a_ctive;

};


class KNListView : public KListView  {

  Q_OBJECT

  friend class KNLVItemBase;

  public:
    KNListView(QWidget *parent, const char *name=0);
    ~KNListView();

    int sortColumn()                { return s_ortCol; }
    bool ascending()                { return s_ortAsc; }
    void setColAsc(int c, bool a)   { s_ortCol=c; s_ortAsc=a; }

    void setActive(QListViewItem *item, bool activate);
    void clear();
    void clearSelection();

    void ensureItemVisibleWithMargin(const QListViewItem *i);

    /** @param outsideOk accept drops of this type even if
	the mouse cursor is not on top of an item */
    void addAcceptableDropMimetype(const char *mimeType, bool outsideOk);

  public slots:
    void slotSortList(int col);
    void slotSizeChanged(int,int,int);

  protected:
    void activeRemoved()            { a_ctiveItem = 0; }
    void contentsMousePressEvent(QMouseEvent *e);
    void contentsMouseDoubleClickEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *e);
    bool eventFilter(QObject *, QEvent *);
    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);
    QDragObject* dragObject() const;
    bool acceptDrag(QDropEvent* event) const;

    bool s_ortAsc;
    int s_ortCol, d_elayedCenter;
    KNLVItemBase *a_ctiveItem;
    QArray<const char*> a_cceptableDropMimetypes;
    QBitArray a_cceptOutside;
    bool k_eepSelection;

  protected slots:
    void slotCenterDelayed();

  signals:
    void itemSelected(QListViewItem*);
    void middleMBClick(QListViewItem*);
    void sortingChanged(int);
    void focusChanged(QFocusEvent*);
    void focusChangeRequest(QWidget*);

    void keyPriorPressed();
    void keyNextPressed();
    void keyLeftPressed();
    void keyRightPressed();
    void keyUpPressed();
    void keyDownPressed();

};

#endif
