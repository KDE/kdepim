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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

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

#include <kmenu.h>
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



  /*!
   * Returns the splitter for this view.
   */
  QSplitter* splitter() {
    return _splitter;
  }


  
  /**
   * Zooms the view to factor @p factor.
   * @param factor the zoom factor. 1.0 is no zoom; use smaller numbers
   *        to zoom out (making the view smaller). Do not use zoom 
   *        values less than or equal to zero.
   */
  void zoom(double factor) {
    barView()->viewport()->zoom(factor);
  }



  ///  Get popup menu.
  /*!
   *
   */
  KMenu* menu() {
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



  /**
   * Add a holiday indicator on the given year @p y, month @p m
   * and day @p d. It might be a bad idea to use negative values
   * for any of the parameters; how months, days and years are 
   * interpreted (zero-based? one-based? relative to 1970, 1AD,
   * or elsewhere?) is a closely guarded secret. Perhaps they
   * are QDate-related.
   *
   * @param y the year the holiday falls in.
   * @param m the month the holiday falls in.
   * @param d the day the holiday falls on.
   */
  void addHoliday(int y, int m, int d) {
    _ganttbar->viewport()->addHoliday(y,m,d);
  }



  /**
   * Remove a holiday.
   *
   * @param y the year the holiday falls in.
   * @param m the month the holiday falls in.
   * @param d the day the holiday falls on.
   *
   * @note Presumably there must have been a holiday added for there
   *       to be one that can be removed.
   */
  void removeHoliday(int y, int m, int d) {
    _ganttbar->viewport()->addHoliday(y,m,d);
  }



public slots:


  /** Proxy slot to set the selection on the viewport. @internal */
  void setSelect() {
    _ganttbar->viewport()->setSelect();
  }

  /** Proxy slot to set the zoom on the viewport. @internal */
  void setZoom() {
    _ganttbar->viewport()->setZoom();
  }

  /** Proxy slot to set the move (?) on the viewport. @internal */
  void setMove() {
    _ganttbar->viewport()->setMove();
  }


  /** Proxy slot to zoom in on the viewport. @internal */
  void zoomIn() {
    _ganttbar->viewport()->zoomIn();
  }

  /** Proxy slot to zoom out on the viewport. @internal */
  void zoomOut() {
    _ganttbar->viewport()->zoomOut();
  }

  /** Proxy slot to zoom on all in the viewport. @internal */
  void zoomAll() {
    _ganttbar->viewport()->zoomAll();
  }

  /** Proxy slot to select everything in the viewport. @internal */
  void selectAll() {
    _ganttbar->viewport()->selectAll();
  }

  /** Proxy slot to unset the selection on the viewport. @internal */
  void unselectAll() {
    _ganttbar->viewport()->unselectAll();
  }

  /** Proxy slot to delete the selection on the viewport. @internal */
  void deleteSelectedItems() {
    _ganttbar->viewport()->deleteSelectedItems();
  }

  /** Proxy slot to insert into the selection on the viewport. @internal */
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
