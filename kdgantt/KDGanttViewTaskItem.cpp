/* -*- Mode: C++ -*-
   $Id$
   KDGantt - a multi-platform charting engine
*/

/****************************************************************************
** Copyright (C) 2002 Klarälvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KDGantt library.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid commercial KDGantt licenses may use this file in
** accordance with the KDGantt Commercial License Agreement provided with
** the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.klaralvdalens-datakonsult.se/Public/products/ for
**   information about KDGantt Commercial License Agreements.
**
** Contact info@klaralvdalens-datakonsult.se if any conditions of this
** licensing are not clear to you.
**
** As a special exception, permission is given to link this program
** with any edition of Qt, and distribute the resulting executable,
** without including the source code for Qt in the source distribution.
**
**********************************************************************/

#include "KDGanttViewTaskItem.h"
#include "KDGanttViewSubwidgets.h"

#include "itemAttributeDialog.h"

/*!
  \class KDGanttViewTaskItem KDGanttViewTaskItem.h

  This class represents calendar items in Gantt charts.

  A calendar item in a Gantt chart has no start/end shape,
  , it is displayed as a rectangle.
  You can set the colors as usual, where only the first argument of
  setColors( col, col, col );
  is important.
  If the start time is equal to the end time, the item is displayed as
  ø, determining that there is no time interval set.

  For a KDGanttViewTaskItem, the text, setted by \a setText(),
  is shown in the item itself and the text is truncated automatically
  to fit in the item.
  For all other item types, the text is shown right of the item.

*/


/*!
  Constructs an empty Gantt item of type event.

  \param view the Gantt view to insert this item into
  \param lvtext the text to show in the listview
  \param name the name by which the item can be identified. If no name
  is specified, a unique name will be generated
*/

KDGanttViewTaskItem::KDGanttViewTaskItem( KDGanttView* view,
                                                const QString& lvtext,
                                                const QString& name ) :
    KDGanttViewItem( Task, view, lvtext, name )
{

  initItem();
}


/*!
  Constructs an empty Gantt item of type event.

  \param parent a parent item under which this one goes
  \param lvtext the text to show in the listview
  \param name the name by which the item can be identified. If no name
  is specified, a unique name will be generated
*/

KDGanttViewTaskItem::KDGanttViewTaskItem( KDGanttViewItem* parent,
                                                const QString& lvtext,
                                                const QString& name ) :
    KDGanttViewItem( Task, parent, lvtext, name )
{

  initItem();

}


/*!
  Constructs an empty Gantt item of type event.

  \param view the Gantt view to insert this item into
  \param after another item at the same level behind which this one should go
  \param lvtext the text to show in the listview
  \param name the name by which the item can be identified. If no name
  is specified, a unique name will be generated
*/

KDGanttViewTaskItem::KDGanttViewTaskItem( KDGanttView* view,
                                                KDGanttViewItem* after,
                                                const QString& lvtext,
                                                const QString& name ) :
    KDGanttViewItem( Task, view, after, lvtext, name )
{

  initItem();

}


/*!
  Constructs an empty Gantt item of type event.

  \param parent a parent item under which this one goes
  \param after another item at the same level behind which this one should go
  \param lvtext the text to show in the listview
  \param name the name by which the item can be identified. If no name
  is specified, a unique name will be generated
*/

KDGanttViewTaskItem::KDGanttViewTaskItem( KDGanttViewItem* parent,
                                                KDGanttViewItem* after,
                                                const QString& lvtext,
                                                const QString& name ) :
    KDGanttViewItem( Task, parent, after, lvtext, name )
{

  initItem();

}


/*!
  The destructor.

*/

KDGanttViewTaskItem::~KDGanttViewTaskItem()
{

}


/*!
  Specifies the end time of this item. The parameter must be valid
  and non-null. If the parameter is invalid or null, no value is set.
  If the end time is less the start time,
  the start time is set to this end time automatically.

  \param end the end time
  \sa setStartTime(), startTime(), endTime()
*/

void KDGanttViewTaskItem::setEndTime( const QDateTime& end )
{
  myEndTime = end;
  if ( myEndTime < startTime() )
      setStartTime( myEndTime );
  else
    updateCanvasItems();
}
/*!
  Specifies the start time of this item. The parameter must be valid
  and non-null. If the parameter is invalid or null, no value is set.
  If the start time is greater than the end time,
  the end time is set to this start time automatically.

  \param start the start time
  \sa startTime(), setEndTime(), endTime()
*/

void KDGanttViewTaskItem::setStartTime( const QDateTime& start )
{
  if (! start.isValid() ) {
    qDebug("KDGanttViewTaskItem::setStartTime():Invalid parameter-no time set");
    return;
  }
    myStartTime = start;
    if ( myStartTime > endTime() )
      setEndTime( myStartTime );
    else
      updateCanvasItems();
}

/*!
  Hides all canvas items of this Gantt item
  \sa showItem()
*/

void KDGanttViewTaskItem::hideMe()
{
    startShape->hide();
    textCanvas->hide();
}
void KDGanttViewTaskItem::showItem(bool show, int coordY)
{
 
  //qDebug("KDGanttViewTaskItem::showItem() %d %s ", (int) show, listViewText().latin1());
  isVisible = show;
  invalidateHeight () ;
  if (!show) {
    hideMe();
    return;
  }
 
  float prio = ((float) ( _priority - 100 )) / 100.0;
  startShape->setZ( prio );
  textCanvas->setZ( prio + 0.005 );
  if ( displaySubitemsAsGroup() && !parent() && !isOpen() ) {
    hideMe();
    return;
  }
  if ( displaySubitemsAsGroup() && firstChild() ) {
    myStartTime = myChildStartTime();
    myEndTime = myChildEndTime();
  }
  //setExpandable(false);
  KDCanvasRectangle* temp = (KDCanvasRectangle*) startShape;
  int startX, endX, midX = 0,allY;
   if ( coordY )
    allY = coordY;
  else
    allY = getCoordY();
  startX = myGantView->myTimeHeader->getCoordX(myStartTime);
  endX = myGantView->myTimeHeader->getCoordX(myEndTime);
  midX = endX;
  int hei = height();
  if (myGantView->myListView->itemAt( QPoint(2, allY)))
    hei =  myGantView->myListView->itemAt( QPoint(2, allY))->height();
  if ( myStartTime == myEndTime ) {
    textCanvas->hide();
    if ( showNoInformation() ) {
      startShape->hide();
    } else {
      startShape->setZ( 1.01 );
      hei = myGantView->myTimeTable->height();
      if (hei  < myGantView->myTimeTable->pendingHeight )
	hei = myGantView->myTimeTable->pendingHeight;
      temp->setSize( 5,  hei );
      temp->move(startX, 0);
      temp->show();
    }
    return;
  } 
  if ( startX +3 >= endX )
    temp->setSize( 3,  hei-3 );
  else
    temp->setSize(endX-startX,  hei-3 );
  temp->move(startX, allY-hei/2 +2);
  temp->show();
  int wid = endX-startX - 4;
  if ( !displaySubitemsAsGroup() ) {
    textCanvas->move(endX+2*myItemSize,allY-myItemSize/2 );
    textCanvas->show();
  } else {
    if ( textCanvasText.isEmpty() || wid < 5)
      textCanvas->hide();
    else {
      textCanvas->move(startX+3, allY-hei/2 +2 );
      QString temp = textCanvasText;
      textCanvas->setText(temp);
      int len =  temp.length();
      while ( textCanvas->boundingRect().width() > wid ) {
	temp.truncate(--len);
	textCanvas->setText(temp);
      }
      if ( temp.isEmpty() )
	textCanvas->hide();
      else {
	textCanvas->show();
      }
    }
  }
}
void KDGanttViewTaskItem::initItem()
{
  isVisible = false;
 
  //startShape->setZ( -0.5 );
  //_priority = 50;
  showItem(true); 
  //qDebug("initItem  %s %s", listViewText().latin1(),startShape->brush().color().name().latin1() );
  myGantView->myTimeTable->updateMyContent();
  // setDragEnabled( true );
  // setDropEnabled( true );
}

