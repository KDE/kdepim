/***************************************************************************
                     knlistview.h - description
 copyright            : (C) 1999 by Christian Thurner
 email                : cthurner@freepage.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KNLISTVIEW_H
#define KNLISTVIEW_H

#include <qlistview.h>

class KNListView;


class KNLVItemBase : public QListViewItem  {
  
  public:
    enum pixmapType {   PTgreyBall=0, PTredBall=1, PTgreyBallChkd=2,
                        PTredBallChkd=3, PTnewFups=4, PTeyes=5,
                        PTmail=6, PTposting=7, PTcontrol=8,
                        PTstatusSent=9, PTstatusEdit=10,
                        PTstatusCanceled=11, PTnntp=12,
                        PTgroup=13, PTfolder=14, PTnull=15 };
                        
    KNLVItemBase(KNListView *view);      // restricted to KNListView to prevent that the
    KNLVItemBase(KNLVItemBase *item);    // static_cast in ~KNLVItemBase fails. (single selection in multi-mode hack)
    ~KNLVItemBase();
    
    void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment);
    int width(const QFontMetrics &fm, const QListView *lv, int column);
    void paintFocus(QPainter *, const QColorGroup & cg, const QRect & r);
    void sortChildItems(int column, bool a);
    
    void expandChildren();

    static void updateAppearance();
    static void initIcons();
    static void clearIcons();
    static QPixmap& icon(pixmapType t);
    
  protected:
    virtual bool greyOut()          { return false; }
    virtual bool firstColBold()     { return false; }
    static QPixmap *pms[15];
    static QColor normal, grey;
};


//==================================================================================


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
    void sortingChanged(int);
    void focusChanged(QFocusEvent*);  
    
  private:
    QListViewItem* exclusiveSelectedItem;     // single selection in multi mode hack... 
  
};

#endif

