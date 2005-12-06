/* -*- Mode: C++ -*-
   $Id$
   KDGantt - a multi-platform charting engine
*/
/****************************************************************************
 ** Copyright (C)  2002-2004 Klarälvdalens Datakonsult AB.  All rights reserved.
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


#include "KDGanttViewSubwidgets.h"
#include "KDGanttViewSummaryItem.h"

#if QT_VERSION < 0x040000
#include "itemAttributeDialog.h"
#endif

/*!
  \class KDGanttViewSummaryItem KDGanttViewSummaryItem.h
  A summary item in a Gantt chart.

  This class represents summary items in Gantt charts.
*/


/*!
  Constructs an empty Gantt item of type event.

  \param view the Gantt view to insert this item into
  \param lvtext the text to show in the list view
  \param name the name by which the item can be identified. If no name
  is specified, a unique name will be generated
*/
KDGanttViewSummaryItem::KDGanttViewSummaryItem( KDGanttView* view,
                                                const QString& lvtext,
                                                const QString& name ) :
    KDGanttViewItem( Summary, view, lvtext, name )
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
KDGanttViewSummaryItem::KDGanttViewSummaryItem( KDGanttViewItem* parent,
                                                const QString& lvtext,
                                                const QString& name ) :
    KDGanttViewItem( Summary, parent, lvtext, name )
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
KDGanttViewSummaryItem::KDGanttViewSummaryItem( KDGanttView* view,
                                                KDGanttViewItem* after,
                                                const QString& lvtext,
                                                const QString& name ) :
    KDGanttViewItem( Summary, view, after, lvtext, name )
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
KDGanttViewSummaryItem::KDGanttViewSummaryItem( KDGanttViewItem* parent,
                                                KDGanttViewItem* after,
                                                const QString& lvtext,
                                                const QString& name ) :
    KDGanttViewItem( Summary, parent, after, lvtext, name )
{

  initItem();

}


/*!
  The destructor. Delete the datetimes, if created.
*/
KDGanttViewSummaryItem::~KDGanttViewSummaryItem()
{
    if ( myActualEndTime )
        delete myActualEndTime;
    if ( myMiddleTime )
        delete myMiddleTime;
}


/*!
  Moves the connector c to point p.

  \param  c the connector to move
  \param  p point for connector where to move to
  \return true if some value of the item was changed
  \sa getConnector()
*/
bool KDGanttViewSummaryItem::moveConnector( KDGanttViewItem::Connector c, QPoint p )
{

    //qDebug("DIFF %d ",mCurrentConnectorDiffX );
    switch( c ) {
    case Start:
        setStartTime( myGanttView->myTimeHeader->getDateTimeForIndex( p.x() ) );
        return true;
        break;
    case End:
        setEndTime( myGanttView->myTimeHeader->getDateTimeForIndex( p.x() ) );
        return true;
        break;
    case Middle:
        setMiddleTime( myGanttView->myTimeHeader->getDateTimeForIndex( p.x() ) );
        return true;
        break;
    case ActualEnd:
        setActualEndTime( myGanttView->myTimeHeader->getDateTimeForIndex( p.x() ) );
        return true;
        break;
    case Move:
        {
            int secsEnd = myStartTime.secsTo( myEndTime );
            int secsMid = -1;
            if ( myMiddleTime )
                secsMid = myStartTime.secsTo( *myMiddleTime );
            myStartTime = myGanttView->myTimeHeader->getDateTimeForIndex( p.x() - mCurrentConnectorDiffX );
            if ( secsMid >= 0 )
                *myMiddleTime = myStartTime.addSecs( secsMid );
            setEndTime( myStartTime.addSecs( secsEnd ) );
            return true;
        }
        break;
    case TaskLink:
        // handled externally
        break;
    default:
        qDebug( "Unsupported connector type in KDGanttViewTaskItem::moveConnector: %d", c );
    }

    return false;
}



/*!
  Returns the region of the item for the position p.
  A region is a connector and it is used for changing item in the gantt view.

  \param  p point to check for a connector
  \return Returns the connector for the point p
*/

KDGanttViewItem::Connector  KDGanttViewSummaryItem::getConnector( QPoint p )
{

    if (! enabled() || displaySubitemsAsGroup() )
        return KDGanttViewItem::NoConnector;

    mCurrentConnectorCoordX =  p.x();
    mCurrentConnectorDiffX =  p.x() - startShape->x();

    if ( startShape->boundingRect().contains( p ) )
        return KDGanttViewItem::Start;
    if ( endShape->boundingRect().contains( p ) )
        return KDGanttViewItem::End;
    if ( myMiddleTime )
        if ( midShape->boundingRect().contains( p ) )
            return KDGanttViewItem::Middle;
    if ( actualEnd && actualEnd->isVisible () ) {
        if ( actualEnd->boundingRect().contains( p ) )
            return KDGanttViewItem::ActualEnd;
    }
    QRect boundingRect = QRect(startShape->boundingRect().topLeft (), 
                               endShape->boundingRect().bottomRight() );
    if ( boundingRect.contains( p ) )
        return KDGanttViewItem::Move;
   
    return KDGanttViewItem::NoConnector;
}


/*!
  Specifies the middle time of this summary item. The parameter must be valid
  and non-null. If the parameter is invalid or null, no value is set.

  \param dateTime the middle time
  \sa middleTime()
*/
void KDGanttViewSummaryItem::setMiddleTime( const QDateTime& dateTime )
{
 if (! dateTime.isValid() ) {
    qDebug("KDGanttViewSummaryItem::setMiddleTime():Invalid parameter-no time set");
    return;
  }
  if (!myMiddleTime) myMiddleTime = new QDateTime;
  *myMiddleTime = dateTime;
  if ( myEndTime < middleTime() )
      setEndTime( middleTime() );
  if ( myStartTime > middleTime() )
      setStartTime( middleTime() );
  updateCanvasItems();
}


/*!
  Returns the middle time of this summary item. If there is no middle
  time defined, the start time is returned.

  \return the middle time of this summary item.
  If there is no middle time defined, the start time is returned.
*/
QDateTime KDGanttViewSummaryItem::middleTime() const
{
  if(myMiddleTime)
    return *myMiddleTime;
  return myStartTime;
}


/*!
  Specifies the end time of this item. The parameter must be valid
  and non-null. If the parameter is invalid or null, no value is set.
  If the end time is less the mid time,
  the mid time is set to this end time automatically.
  \param end the end time
  \sa endTime(), setStartTime(), startTime()
*/
void KDGanttViewSummaryItem::setEndTime( const QDateTime& end )
{
  if (! end.isValid() ) {
    qDebug("KDGanttViewSummaryItem::setEndTime():Invalid parameter-no time set");
    return;
  }
  myEndTime = end;
  if ( myEndTime < middleTime() )
      setMiddleTime( myEndTime );
  else
    updateCanvasItems();
}


/*!
  Specifies the start time of this item. The parameter must be valid
  and non-null. If the parameter is invalid or null, no value is set.
  If the start time is less the mid time,
  the mid time is set to this start time automatically.

  \param start the start time
  \sa startTime(), setEndTime(), endTime()
*/
void KDGanttViewSummaryItem::setStartTime( const QDateTime& start )
{
  if (! start.isValid() ) {
    qDebug("KDGanttViewSummaryItem::setStartTime():Invalid parameter-no time set");
    return;
  }
    myStartTime = start;
 if ( myStartTime > middleTime() ) {
      setMiddleTime( myStartTime );
 }
    else
      updateCanvasItems();
}


/*!
  Specifies the actual end time of this item. The parameter must be valid
  and non-null. Items with undefined start or end times lead to
  undefined visual results.

  \param end the actual end time
  \sa actualEndTime()
  startTime()
*/
void KDGanttViewSummaryItem::setActualEndTime( const QDateTime& end )
{
  if (!myActualEndTime) myActualEndTime = new QDateTime;
  *myActualEndTime =  end;

  updateCanvasItems();

}


/*!
  Returns the actual end time of this item.

  \return the actual end time of this item
  \sa setActualEndTime()

*/
QDateTime KDGanttViewSummaryItem::actualEndTime() const
{
  if(myActualEndTime)
    return *myActualEndTime;
  return myEndTime;
}


void KDGanttViewSummaryItem::hideMe()
{
    startShape->hide();
    midShape->hide();
    endShape->hide();
    startShapeBack->hide();
    midShapeBack->hide();
    endShapeBack->hide();
    startLine->hide();
    endLine->hide();
    if ( mTextCanvas )
        mTextCanvas->hide();
    startLineBack->hide();
    endLineBack->hide();
    actualEnd->hide();
}

// shows the item
// if coordY >0, this is taken as the middle y-coordinate
void KDGanttViewSummaryItem::showItem( bool show, int coordY )
{
    mCurrentCoord_Y = coordY;
  isVisibleInGanttView = show;
  invalidateHeight () ;
  if (!show) {
    hideMe();
    return;
  }
 if ( displaySubitemsAsGroup() && !parent() && !isOpen() ) {
    hideMe();
    return;
  }
  float prio = ((float) ( priority() - 100 )) / 100.0;
  startShape->setZ( prio + 0.0055 );
  midShape->setZ( prio + 0.004 );
  endShape->setZ( prio + 0.005 );
  startShapeBack->setZ( prio + 0.003 );
  midShapeBack->setZ( prio + 0.003 );
  endShapeBack->setZ( prio + 0.003 );
  startLine->setZ( prio + 0.0015  );
  endLine->setZ( prio + 0.001 );
  if ( mTextCanvas )
      mTextCanvas->setZ( prio + 0.006 );
  startLineBack->setZ( prio );
  endLineBack->setZ( prio );
  actualEnd->setZ( prio  + 0.007 );
  if ( displaySubitemsAsGroup() ) {
      myStartTime = myChildStartTime();
      myEndTime = myChildEndTime();
  }
  if ( !myStartTime.isValid() || !myEndTime.isValid() ) {
      hideMe();
      return;
  }
  int startX, endX, midX = 0,allY;
  if ( coordY )
    allY = coordY;
  else
    allY = getCoordY();
  startX = myGanttView->myTimeHeader->getCoordX(myStartTime);
  checkCoord( &startX );
  endX = myGanttView->myTimeHeader->getCoordX(myEndTime);
  if (myMiddleTime)
    midX = myGanttView->myTimeHeader->getCoordX(*myMiddleTime);
  else
    midX = endX;
  checkCoord( &midX );
  //qDebug("START %d   END %d",startX ,midX);
  startLine->setPoints(startX,allY,midX,allY);
  startLine->show();
  startLineBack->setPoints(startX-1,allY,midX+1,allY);
  startLineBack->show();
  startShape->move(startX,allY);
  startShapeBack->move(startX,allY);

  endShape->move(endX,allY);
  endShapeBack->move(endX,allY);
  if ( mTextCanvas )
      mTextCanvas->move(endX+2*myItemSize,allY-myItemSize/2 );
  startShape->show();
  startShapeBack->show();
  endShape->show();
  endShapeBack->show();
  if ( mTextCanvas )
      mTextCanvas->show();
  if (myMiddleTime) {
      checkCoord( &endX );
    endLine->setPoints(midX,allY,endX,allY);
    endLine->show();
    endLineBack->setPoints(midX,allY,endX+1,allY);
    endLineBack->show();
    midShape->move(midX,allY);
    midShape->show();
    midShapeBack->move(midX,allY);
    midShapeBack->show();
  }
  else {
    endLine->hide();
    endLineBack->hide();
    midShape->hide();
    midShapeBack->hide();
  }
  if (myActualEndTime) {
    if ( *myActualEndTime == myEndTime ) {
      actualEnd->hide();
    }
    else {
      int actendX = myGanttView->myTimeHeader->getCoordX(*myActualEndTime);
      actualEnd->setPoints(actendX,allY-5,actendX,allY+5);
      actualEnd->show();
    }
  }
  else {
    actualEnd->hide();
  }
  if(myStartTime == myEndTime)
    {
      endShape->moveBy(myItemSize+4,0);
      endShapeBack->moveBy(myItemSize+4,0);
      if ( mTextCanvas )
          mTextCanvas->moveBy(myItemSize+4,0);
      midShape->hide();
      midShapeBack->hide();
      startLine->hide();
      endLine->hide();
      startLineBack->hide();
      endLineBack->hide();
    }
  if ( mTextCanvas )
      if (mTextCanvas->text().isEmpty())
          mTextCanvas->hide();
}
void KDGanttViewSummaryItem::initItem()
{
  isVisibleInGanttView = false;
  myActualEndTime = 0;
  myMiddleTime = 0;
  showItem(true);
  myGanttView->myTimeTable->updateMyContent();
  setDragEnabled( myGanttView->dragEnabled() );
  setDropEnabled( myGanttView->dropEnabled() );
}

