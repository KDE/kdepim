/* -*- Mode: C++ -*-
   $Id$
   KDGantt - a multi-platform charting engine
*/

/****************************************************************************
** Copyright (C) 2002-2003 Klar�lvdalens Datakonsult AB.  All rights reserved.
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
** See http://www.klaralvdalens-datakonsult.se/?page=products for
**   information about KDGantt Commercial License Agreements.
**
** Contact info@klaralvdalens-datakonsult.se if any conditions of this
** licensing are not clear to you.
**
**********************************************************************/

#include "KDGanttViewEventItem.h"
#include "KDGanttViewSubwidgets.h"
#include "itemAttributeDialog.h"

/*!
  \class KDGanttViewEventItem KDGanttViewEventItem.h
  An event item in a Gantt chart.

  This class represents event items in Gantt charts.
*/

/*!
  Constructs an empty Gantt item of type event.

  \param view the Gantt view to insert this item into
  \param lvtext the text to show in the list view
  \param name the name by which the item can be identified. If no name
  is specified, a unique name will be generated
*/
KDGanttViewEventItem::KDGanttViewEventItem( KDGanttView* view,
                                            const QString& lvtext,
                                            const QString& name ) :
    KDGanttViewItem( Event, view, lvtext, name )
{
  initItem();
}


/*!
  Constructs an empty Gantt item of type event.

  \param parent a parent item under which this one goes
  \param lvtext the text to show in the list view
  \param name the name by which the item can be identified. If no name
  is specified, a unique name will be generated
*/
KDGanttViewEventItem::KDGanttViewEventItem( KDGanttViewItem* parent,
                                            const QString& lvtext,
                                            const QString& name ) :
    KDGanttViewItem( Event, parent, lvtext, name )
{
 initItem();

}


/*!
  Constructs an empty Gantt item of type event.

  \param view the Gantt view to insert this item into
  \param after another item at the same level behind which this one should go
  \param lvtext the text to show in the list view
  \param name the name by which the item can be identified. If no name
  is specified, a unique name will be generated
*/
KDGanttViewEventItem::KDGanttViewEventItem( KDGanttView* view,
                                            KDGanttViewItem* after,
                                            const QString& lvtext,
                                            const QString& name ) :
    KDGanttViewItem( Event, view, after, lvtext, name )
{

  initItem();
}


/*!
  Constructs an empty Gantt item of type event.

  \param parent a parent item under which this one goes
  \param after another item at the same level behind which this one should go
  \param lvtext the text to show in the list view
  \param name the name by which the item can be identified. If no name
  is specified, a unique name will be generated
*/
KDGanttViewEventItem::KDGanttViewEventItem( KDGanttViewItem* parent,
                                            KDGanttViewItem* after,
                                            const QString& lvtext,
                                            const QString& name ) :
    KDGanttViewItem( Event, parent, after, lvtext, name )
{

  initItem();
}


/*!
  The destructor. Nothing done here.
*/
KDGanttViewEventItem::~KDGanttViewEventItem()
{


}
/*!
  Specifies the start time of this item. The parameter must be valid
  and non-null. If the parameter is invalid or null, no value is set.
  If the start time is less than the lead time,
  the lead time is set to this start time automatically.

  \param start the start time
  \sa startTime()
*/
void KDGanttViewEventItem::setStartTime( const QDateTime& start )
{
  if (! start.isValid() ) {
    qDebug("KDGanttViewEventItem::setStartTime():Invalid parameter-no time set");
    return;
  }
    myStartTime = start;
    if ( myStartTime < leadTime() )
      setLeadTime( myStartTime );
    else {
      updateCanvasItems();
    }

}


/*!
  Specifies whether the event item should be shown with a lead time
  line, and if yes, when the lead time starts.
  If the start time is less than the lead time,
  the start time is set to this lead time automatically.

  \param leadTimeStart the start time of the lead time; pass an
  invalid QDateTime object in order to turn the lead time off.
  \sa leadTime()
*/

void KDGanttViewEventItem::setLeadTime( const QDateTime& leadTimeStart )
{
  if (!myLeadTime) myLeadTime = new QDateTime;
  *myLeadTime =  leadTimeStart;
  if ( startTime() < leadTime() )
      setStartTime( leadTimeStart );
  else {
    updateCanvasItems();
  }

}


/*!
  Returns whether the event item is shown with a lead time line and if
  yes, when the lead time starts.

  \return if the event item is shown with a lead time line, returns
  the QDateTime object representing the start of the lead time,
  otherwise returns an invalid QDateTime object
  \sa setLeadTime()
*/
QDateTime KDGanttViewEventItem::leadTime() const
{
  if(myLeadTime)
    return *myLeadTime;
  return myStartTime;
}


void KDGanttViewEventItem::hideMe()
{
    startShape->hide();
    startShapeBack->hide();
    startLine->hide();
    startLineBack->hide();
    textCanvas->hide();
}


void KDGanttViewEventItem::showItem(bool show, int coordY)
{
  isVisibleInGanttView = show;
  invalidateHeight () ;
  if (!show) {
    hideMe();
    return;
  }
  float prio = ((float) ( priority() - 100 )) / 100.0;
  startShape->setZ( prio + 0.0055 );
  startShapeBack->setZ( prio + 0.003 );
  startLine->setZ( prio + 0.0015  );
  textCanvas->setZ( prio + 0.006 );
  startLineBack->setZ( prio );

  if ( displaySubitemsAsGroup() && firstChild() ) {
    myStartTime = myChildStartTime();
    myEndTime = myChildEndTime();
  }
  int startX, endX, allY;
  if ( coordY )
    allY = coordY;
  else
    allY = getCoordY();
  startX = myGanttView->myTimeHeader->getCoordX(myStartTime);
  if (myLeadTime) {
    endX = myGanttView->myTimeHeader->getCoordX(*myLeadTime);
    startLine->setPoints(startX,allY,endX,allY);
    startLine->show();
    startLineBack->setPoints(startX+1,allY,endX-1,allY);
    startLineBack->show();
  }
  else {
    startLine->hide();
    startLineBack->hide();
  }
  startShape->move(startX,allY);
  startShape->show();
  startShapeBack->move(startX,allY);
  startShapeBack->show();
  textCanvas->move(startX+2*myItemSize,allY-myItemSize/2 );
  textCanvas->show();
  if (textCanvas->text().isEmpty())
    textCanvas->hide();
}


void KDGanttViewEventItem::initItem()
{
  isVisibleInGanttView = false;
  myLeadTime = 0;
  showItem(true);
  myGanttView->myTimeTable->updateMyContent();
  setDragEnabled( myGanttView->dragEnabled() );
  setDropEnabled( myGanttView->dropEnabled() );
}



