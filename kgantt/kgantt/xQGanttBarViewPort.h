#ifndef _XQGANTTBARVIEWPORT_H_
#define _XQGANTTBARVIEWPORT_H_

/*

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    author  : jh, jochen@ifb.bv.tu-berlin.de

    file    : xQGanttBarViewPort.h
    date    : 26 oct 2000


    changelog :

*/



#include "KGanttItem.h"

#include <qcursor.h>
#include <qtimer.h>
#include <qlabel.h>

#include <kpopupmenu.h> 
#include <ktoolbar.h> 

#include <qptrdict.h>
#include <qaction.h> 
#include <qlineedit.h>

class xQGanttBarView;

#define sgn(n) (n < 0 ? -1 : 1)


// This is an internal class.
// helper for drawing items

class xQTaskPosition 
////////////////////
{

public :

  xQTaskPosition(int nr, int x, int y, int w, int h, int hiSub, 
		 int tPx, int tPy, int d) 
    :  _nr(nr), _screenX(x), _screenY(y), _screenW(w), _screenH(h), 
       _screenHS(hiSub), _textPosX(tPx), _textPosY(tPy), _depth(d)
    {
      _screenHandleX = _screenHandleY = _screenHandleW = _screenHandleH = 0;
    }

  int _nr;
  int _screenX, _screenY, _screenW;
  int _screenH; // height without subitems
  int _screenHS; // height including subitems
  int _textPosX, _textPosY;

  int _screenHandleX, _screenHandleY, _screenHandleW, _screenHandleH;

  int _depth;
 
};



///  GanttBarViewPort Widget.
/*!
 *
 */

/////////////////////////////////////////
class KDE_EXPORT xQGanttBarViewPort : public QFrame
////////////////////////////////////////
{

  Q_OBJECT

  friend class xQGanttBarView;
  friend class KGantt;

public:

  enum Mode {Init, Select, Zoom, Move};


  ///  Constructor.
  /*!
   *
   */
  xQGanttBarViewPort(KGanttItem* toplevelitem, xQGanttBarView* parent = 0,
		     const char * name=0, WFlags f=0 );


  ///  Destructor.
  /*!
   *
   */
  ~xQGanttBarViewPort();



  ///  Update widget.
  /*!
   *
   */
  void update(int x1, int y1, int x2, int y2); 
  

 
  QPtrDict<xQTaskPosition> _gItemList;


  ///  Add holiday.
  /*!
   *   
   */
  void addHoliday(int y, int m, int d);



  ///  Remove holiday.
  /*!
   *
   */
  void removeHoliday(int y, int m, int d) {
  }



  KPopupMenu* menu() {
    return _menu;
  }



  ///
  /*!
   *
   */
  KToolBar* toolbar(QMainWindow* mw = 0);


  //  zoom by factor sfactor and move wx,wy to the center
  void zoom(double sfactor, int wx, int wy);

  //  zoom by factor, and leave the center unmoved
  void zoom(double sfactor);


  void getSelectedItems(QPtrList<KGanttItem>& list) {
    getSelectedItems(_toplevelitem, list);
  }


signals:

  void modeChanged(int);
  void scroll(int x, int y);
  void resized();
  void recalculated();
  void message(const QString& msg);


protected slots:

  void setMode(int mode);

  void setSelect();
  void setZoom();
  void setMove();

  void zoomIn();
  void zoomOut();
  void zoomAll(); 

  void popup(int index);

  void selectAll();
  void unselectAll();

  void deleteSelectedItems();
  void insertIntoSelectedItem();



private slots:  

  void toplevelitemChanged(KGanttItem* item, KGanttItem::Change c);
  void textEdited();
  void itemDestroyed(KGanttItem*);



private:

  enum Position { Outside = 0, Handle  = 1,
		  North   = 2, South   = 4,
		  West    = 8, East    = 16,
		  Center  = 32 };

  ///  Transform world coordinate to screen coordinate.
  /*!
   *
   */
  inline int screenX(int wx);


  ///  Transform world coordinate to screen coordinate.
  /*!
   *
   */
  inline int screenY(int wy);

  
  ///  Transform screen coordinate to world coordinate.
  /*!
   *
   */
  inline int worldX(int sx);


  ///  Transform screen coordinate to world coordinate.
  /*!
   *
   */
  inline int worldY(int sy);


  //  this will be set by setParentScrollView()
  xQGanttBarView* _parent;

  int _grid, _snapgrid;
  bool _drawGrid, _drawHeader;

  Mode _mode;

  int _marginX, _marginY; // margin in minutes
  double _scaleX, _scaleY;

  int _margin;


  QCursor* _cursor_lupe;

  QLabel* _itemInfo;
  QLineEdit* _itemTextEdit;


  // all item are stored here
  KGanttItem* _toplevelitem;

  static KGanttItem* _currentItem;

  KPopupMenu* _menu;
  KPopupMenu* _selectMenu;
  
  KIconLoader* _iconloader;

  KToolBar* _toolbar;

  QPoint* _startPoint, *_endPoint;

  QPtrList<QDate> _holidays;

  QPtrList<KGanttItem> *_observedList;


  /// 

  void initMenu(); 

  void drawGrid(QPainter*, int x1, int y1, int x2, int y2);
  void drawHeader(QPainter*, int x1, int y1, int x2, int y2);
  void drawItem(KGanttItem* item, QPainter* p, const QRect& rect );

  void drawRelation(QPainter*, KGanttRelation*);

  void recalc(KGanttItem* item, int xPos, int yPos, int depth, int nr );
  void recalc();

  void selectItem(KGanttItem*,bool);

  void getSelectedItems(KGanttItem*, QPtrList<KGanttItem>&);

  void adjustSize();

  void observeList(QPtrList<KGanttItem>*);

  Position check(KGanttItem** founditem, int x, int y);

  void mousePressEvent(QMouseEvent*);
  void mouseReleaseEvent(QMouseEvent*);

  void wheelEvent ( QWheelEvent * e) {
    printf("wheelEvent\n");
  }

  void mouseMoveEvent(QMouseEvent*);  
  void keyPressEvent(QKeyEvent* e);
  void paintEvent(QPaintEvent * e);
  
  
  QPixmap closedIcon, openedIcon;

};




//   inline


int xQGanttBarViewPort::screenX(int wx)
///////////////////////////////////////
{   
  return (int) (0.5 + (wx + _marginX)  * _scaleX);
}  
int xQGanttBarViewPort::screenY(int wy)
/////////////////////////////////////
{   
  return (int) (0.5 + (wy + _marginY) * _scaleY);
}  
int xQGanttBarViewPort::worldX(int sx)
/////////////////////////////////////
{   
  return (int) (0.5 + (sx/_scaleX - _marginX));
}  
int xQGanttBarViewPort::worldY(int sy)
//////////////////////////////////////
{   
  return (int) (0.5 + (sy/_scaleY - _marginY));
}  


#endif
