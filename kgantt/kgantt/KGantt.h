#ifndef _KGANTT_H_
#define _KGANTT_H_
 
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

    file    : KGantt.h
    date    : 26 oct 2000


    changelog : 23 nov 2000, jh

                24 nov 2000, jh

		10 jan 2001m jh, changed to KDE :
		xQGantt -> KGantt

*/


#include <qwidget.h>
#include <qsplitter.h> 

#include <kpopupmenu.h>
#include <kdepimmacros.h>

#include "KGanttItem.h"
#include "xQGanttListView.h"
#include "xQGanttBarView.h"



/**
 *  \mainpage KGantt Module <br> <IMG SRC="gantt.png">
 *
 *  The kgantt module contains several classes (KGanttItem, KGantt)
 *  for drawing and editing gantt-diagramms.
 *
 *  This example shows how to use the gantt module:
 *  \code
 *  #include "kgantt/KGantt.h"
 *  
 *  int main(int args, char* argv[])
 *  {
       ...

       KGantt* gantt = new KGantt(0, mainwindow);

       KGanttItem* toplevel = gantt->getToplevelItem();

       KGanttItem* t1 = new KGanttItem(toplevel, 
                                       "task 1, no subtasks", 
                	               QDateTime::currentDateTime().addDays(10),
                                       QDateTime::currentDateTime().addDays(20) );

       ...

 *  }
 *  \endcode
 *
 *  You just have to create an object of class KGantt and add several objects of
 *  class KGanttItem.
 *
 */



///  Gantt Widget.
/*!
 *   A gantt widget contains two parts, a list view and a
 *   bar view.
 */
////////////////////////////////
class KDE_EXPORT KGantt : public QWidget
////////////////////////////////
{

  Q_OBJECT


public:  


  ///  Constructor.
  /*!
   *
   */
  KGantt(KGanttItem* toplevelitem = 0,
	 QWidget* parent = 0, const char * name=0, WFlags f=0 );


  ///  Destructor.
  /*!
   *
   */
  ~KGantt();



  ///  Set toplevel item.
  /*!
   *   If no toplevel item was specified at construction of this widget a
   *   toplevel item was created. This will be deleted by setting a new
   *   toplevel item. A toplevel item that was passed to the constructor will
   *   not be deleted.
   */
  void setToplevelItem(KGanttItem* item) {
    if(_deleteItem)
      delete _toplevelitem;
    _toplevelitem = item; 
  }



  ///  Get toplevel item.
  /*!
   *
   */
  KGanttItem* getToplevelItem() { 
    return _toplevelitem; 
  }



  ///  Get bar view of items.
  /*!
   *
   */
  xQGanttBarView* barView() {
    return _ganttbar;
  }



  ///  Get list view of items.
  /*!
   *
   */
  xQGanttListView* listView() {
    return _ganttlist;
  }



  QSplitter* splitter() {
    return _splitter;
  }


  
  ///
  /*!
   *
   */
  void zoom(double factor) {
    barView()->viewport()->zoom(factor);
  }



  ///  Get popup menu.
  /*!
   *
   */
  KPopupMenu* menu() {
    return _ganttbar->viewport()->menu();
  }



  ///  Add gantt toolbar to main window.
  /*!
   *   If you want to embed a toolbar with specific actions
   *   like zooming or configuring the gantt, you can add a toolbar
   *   automatically by invoking this method. You have to pass your
   *   mainwindow as a parameter if you call this method the first
   *   time because teh toolbar will be created then. If you
   *   you want to access the pointer to a already created toolbar you
   *   can invoke this method without any parameter.
   */
  KToolBar* toolbar(QMainWindow* mw = 0) {
    return _ganttbar->viewport()->toolbar(mw);
  }



  ///  Print to stdout.
  /*
   *
   */
  void dumpItems();



  ///  Get all selected items.
  /*!
   *   All selected KGanttItems will be added to the passed list. 
   */
  void getSelectedItems(QPtrList<KGanttItem>& list) {
    _ganttbar->viewport()->getSelectedItems(list);
  }



  void addHoliday(int y, int m, int d) {
    _ganttbar->viewport()->addHoliday(y,m,d);
  }



  void removeHoliday(int y, int m, int d) {
    _ganttbar->viewport()->addHoliday(y,m,d);
  }



public slots:


  void setSelect() {
    _ganttbar->viewport()->setSelect();
  }

  void setZoom() {
    _ganttbar->viewport()->setZoom();
  }

  void setMove() {
    _ganttbar->viewport()->setMove();
  }


  void zoomIn() {
    _ganttbar->viewport()->zoomIn();
  }

  void zoomOut() {
    _ganttbar->viewport()->zoomOut();
  }

  void zoomAll() {
    _ganttbar->viewport()->zoomAll();
  }

  void selectAll() {
    _ganttbar->viewport()->selectAll();
  }

  void unselectAll() {
    _ganttbar->viewport()->unselectAll();
  }

  void deleteSelectedItems() {
    _ganttbar->viewport()->deleteSelectedItems();
  }

  void insertIntoSelectedItem() {
    _ganttbar->viewport()->insertIntoSelectedItem();
  }


  ///  Show list view.
  /*!
   *
   */
  void showList() {
    _ganttlist->show();
  }


  ///  Hide list view.
  /*
   *
   */
  void hideList() {
    _ganttlist->hide();
  }


protected:


  void resizeEvent(QResizeEvent* /*e*/) {
    _splitter->resize(width(),height());
  };


private:

  KGanttItem* _toplevelitem;

  QSplitter *_splitter;

  xQGanttBarView* _ganttbar;
  xQGanttListView* _ganttlist;

  bool _deleteItem;

};


#endif
