/*
    knlistview.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2000 the KNode authors.
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


class KNLVItemBase : public QListViewItem  {
  
  public:
    KNLVItemBase(KNListView *view);      // restricted to KNListView to prevent that the
    KNLVItemBase(KNLVItemBase *item);    // static_cast in ~KNLVItemBase fails. (single selection in multi-mode hack)
    ~KNLVItemBase();
    
    void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment);
    int width(const QFontMetrics &fm, const QListView *lv, int column);
    void paintFocus(QPainter *, const QColorGroup & cg, const QRect & r);
    void sortChildItems(int column, bool a);
    void expandChildren();

  protected:
    virtual bool greyOut()          { return false; }
    virtual bool firstColBold()     { return false; }
};


class KNListView : public QListView  {

  Q_OBJECT

  public:
    KNListView(QWidget *parent, const char *name=0);
    ~KNListView();
    
    int sortColumn()                { return sCol; }
    bool ascending()                { return sAsc; }
    void setColAsc(int c, bool a)   { sCol=c; sAsc=a; }
    
    virtual void setSelected(QListViewItem *item, bool select);
    void selectedRemoved()          { exclusiveSelectedItem = 0; }
    void clear();
    
  public slots:
    void slotSortList(int col);     
      
  protected:
    void keyPressEvent(QKeyEvent *e);
    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);
    bool sAsc;
    int sCol; 
      
  signals:
    void exSelectionChanged(QListViewItem *);
    void sortingChanged(int);
    void focusChanged(QFocusEvent*);  
    
  private:
    QListViewItem* exclusiveSelectedItem;     // single selection in multi mode hack... 
  
};



#endif

