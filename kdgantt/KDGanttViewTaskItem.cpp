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

#include "KDGanttViewTaskItem.h"
#include <QApplication>
#if QT_VERSION < 0x040000
#include "itemAttributeDialog.h"
#endif
/*!
  \class KDGanttViewTaskItem KDGanttViewTaskItem.h

  This class represents calendar items in Gantt charts.

  A calendar item in a Gantt chart has no start/end shape,
  it is displayed as a rectangle.
  You can set the colors as usual, where only the first argument of
  setColors( col, col, col )
  is important.
  If the start time is equal to the end time, the item is displayed as
  ø, showing that there is no time interval set.

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
  \param lvtext the text to show in the list view
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
  \param lvtext the text to show in the list view
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
  Moves the connector c to point p.

  \param  c the connector to move
  \param  p point for connector where to move to
  \return true if some value of the item was changed
  \sa getConnector()
*/
bool KDGanttViewTaskItem::moveConnector( KDGanttViewItem::Connector c, QPoint p )
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
    case Move:
        {
            int secs = myStartTime.secsTo( myEndTime );
            myStartTime = myGanttView->myTimeHeader->getDateTimeForIndex( p.x() - mCurrentConnectorDiffX );
            setEndTime( myStartTime.addSecs( secs ) );
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

KDGanttViewItem::Connector  KDGanttViewTaskItem::getConnector( QPoint p )
{
    if (! enabled() || displaySubitemsAsGroup() )
        return KDGanttViewItem::NoConnector;
    KDCanvasRectangle* temp = (KDCanvasRectangle*) startShape;
    mCurrentConnectorCoordX =  p.x();
    mCurrentConnectorDiffX =  p.x() - (int)temp->x();
    if (  p.y() < (int)temp->y() ||  
          p.y() > (int)temp->y() +(int)temp->height() ||
          p.x() < (int)temp->x() ||
          p.x() > (int)temp->x() + (int)temp->width() )
        return KDGanttViewItem::NoConnector;
    int miniwid = 4;
    if ( (int)temp->width() < miniwid )
        return KDGanttViewItem::TaskLink;
    int margin = 5;

    if ( (int)temp->width() < 14 ) {
        --margin;
        if ( (int)temp->width() < 10 )
            --margin;
        if ( (int)temp->width() < 8 )
            --margin;
    } else if ( (int)temp->width() > 50 ) {
        margin = 10;
    }
    if ( p.x() < (int)temp->x() + margin )
        return KDGanttViewItem::Start;
    if ( p.x() > (int)temp->x() + (int)temp->width() - margin )
        return KDGanttViewItem::End;
    if ( p.x() < (int)temp->x() + (int)temp->width() - margin - margin )
        return KDGanttViewItem::Move;

    return KDGanttViewItem::TaskLink;
}

/*!
  KDGanttViewTaskItem::getTimeForTimespan

  \param  start
          end
  \return the computed sum of times in seconds 
*/
 
unsigned int KDGanttViewTaskItem::getTimeForTimespan( const QDateTime& start, const QDateTime& end )
{
    unsigned int retval = 0;
    if ( displaySubitemsAsGroup() )
        return retval;
    if ( start.isValid () ) {
        if ( end.isValid() ) {
            if ( myEndTime > start && myStartTime < end ) {
                if ( myStartTime < start ) {
                    if ( myEndTime < end )
                        retval = start.secsTo( myEndTime );
                    else
                       retval = start.secsTo( end ); 
                } else {
                    // myStartTime >= start 
                    if ( myEndTime < end )
                        retval = myStartTime.secsTo( myEndTime );
                    else
                       retval = myStartTime.secsTo( end ); 
                }
            }
        } else {
            // start valid - end invalid
            if ( myStartTime > start )
                retval = myStartTime.secsTo( myEndTime );
            else {
                if ( myEndTime > start )
                    retval = start.secsTo( myEndTime );
            }
        }
    } else {
        // start invalid
        if ( end.isValid() ) {
            if ( myEndTime < end )
                retval = myStartTime.secsTo( myEndTime );
            else {
                if ( myStartTime < end )
                    retval = myStartTime.secsTo( end );
            }
        } else {
            // start invalid - end invalid
            retval = myStartTime.secsTo( myEndTime );
        }
    }
    return retval;
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
    if ( mTextCanvas )
        mTextCanvas->hide();
}


void KDGanttViewTaskItem::showItem(bool show, int coordY)
{

    //qDebug("KDGanttViewTaskItem::showItem() %d %s ", (int) show, listViewText().toLatin1());
    isVisibleInGanttView = show;
    mCurrentCoord_Y = coordY;
    invalidateHeight () ;
    if (!show) {
        hideMe();
        return;
    }
    float prio = ((float) ( priority() - 100 )) / 100.0;
    startShape->setZ( prio );
    if ( mTextCanvas )
        mTextCanvas->setZ( prio + 0.005 );
    if ( displaySubitemsAsGroup() && !parent() && !isOpen() ) {
        hideMe();
        return;
    }
    if ( displaySubitemsAsGroup()  && ( firstChild() || myGanttView->calendarMode() )  ) {
        hideMe();//new
        return;//new
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
    startX = myGanttView->myTimeHeader->getCoordX(myStartTime);
    checkCoord( &startX );
    endX = myGanttView->myTimeHeader->getCoordX(myEndTime);
    checkCoord( &endX );
    midX = endX;
    int hei ;
#if 0
    bool takedefaultHeight = true ; // pending: make configureable
    // commented out until height is made configurable
    hei = height();
    if ( ! isVisible() ) {
        KDGanttViewItem * par = parent();
        while ( par != 0 && !par->isVisible() )
            par = par->parent();
        if ( par )
            hei = par->height();
    }

    if ( takedefaultHeight )
#endif
        hei = 16;
    if ( myStartTime == myEndTime ) {
        if ( mTextCanvas )
            mTextCanvas->hide();
        if ( showNoInformation() ) {
            startShape->hide();
        } else {
            startShape->setZ( 1.01 );
            if (myGanttView->displayEmptyTasksAsLine() ) {
                hei = myGanttView->myTimeTable->height();
                if (hei  < myGanttView->myTimeTable->pendingHeight )
                    hei = myGanttView->myTimeTable->pendingHeight;
                temp->setSize(5,  hei  );
                temp->move(startX, 0);
                temp->show();
            } else {
                temp->setSize( 1,  hei -3 );
                temp->move(startX, allY-hei/2 +2);
                temp->show();
            }
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
    if ( mTextCanvas ) {
        if ( !displaySubitemsAsGroup() && !myGanttView->calendarMode()) {
            mTextCanvas->move(endX+2*myItemSize,allY-myItemSize/2 );
            mTextCanvas->show();
      
        } else {
            if ( textCanvasText.isEmpty()  || wid < 5)
                mTextCanvas->hide();
            else {
                mTextCanvas->move(startX+3, allY-mTextCanvas->boundingRect().height()/2);
                QString temp = textCanvasText;
                mTextCanvas->setText(temp);
                int len =  temp.length();
                while ( mTextCanvas->boundingRect().width() > wid ) {
                    temp.truncate(--len);
                    mTextCanvas->setText(temp);
                }
                if ( temp.isEmpty())
                    mTextCanvas->hide();
                else {
                    mTextCanvas->show();
                }
            }
        }
    }
}


void KDGanttViewTaskItem::initItem()
{
  isVisibleInGanttView = false;

  if ( myGanttView->calendarMode() && parent() ) {
    setVisible( false );
    parent()->setVisible( true );
  } else
    showItem(true);
  //qDebug("initItem  %s %s", listViewText().toLatin1(),startShape->brush().color().name().toLatin1() );
  myGanttView->myTimeTable->updateMyContent();
  setDragEnabled( myGanttView->dragEnabled() );
  setDropEnabled( myGanttView->dropEnabled() );
}

