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

#include "KDGanttViewSubwidgets.h"
#include "KDGanttViewEventItem.h"
#include "KDGanttViewSummaryItem.h"
#include "KDGanttViewTaskItem.h"
#ifndef KDGANTT_MASTER_CVS
#include "KDGanttViewSubwidgets.moc"
#endif

#include <qlabel.h>
#include <qheader.h>
#include <qpainter.h>
#include <qrect.h>
#include <qtooltip.h>
#include <qapplication.h>

#include <kdialog.h>
#include <klocale.h>

KDTimeTableWidget:: KDTimeTableWidget( QWidget* parent,KDGanttView* myGantt):QCanvas (parent)
{
    myGanttView = myGantt;
    taskLinksVisible = true;
    flag_blockUpdating = false;
    int_blockUpdating = 0;
    gridPen.setStyle(Qt::DotLine);
    gridPen.setColor(QColor(100,100,100));
    maximumComputedGridHeight = 0;
    denseLineCount = 0;
    denseLineBrush = QBrush( QColor ( 240,240,240 ));
    noInfoLineBrush = QBrush(  QColor ( 100,100,100 ), Qt::FDiagPattern );
    pendingHeight = 0;
    pendingWidth = 0;
    resize(1,1);
}

QPtrList<KDGanttViewTaskLink> KDTimeTableWidget::taskLinks()
{
   return myTaskLinkList;
}

void KDTimeTableWidget::resetWidth( int wid )
{
  if ( wid == width() ) {
    if (pendingHeight)
      pendingWidth = wid;
    else
      pendingWidth = 0;
    return;
  }
  if ( ! pendingHeight )
    pendingHeight = height();
  pendingWidth = wid;
  updateMyContent();
}

void KDTimeTableWidget::checkHeight( int hei )
{
  if( hei < height() )
    return;
  if ( pendingHeight < hei+100)
    pendingHeight = hei+100;
  if ( !  pendingWidth )
   pendingWidth = width();
  maximumComputedGridHeight = 0; //force recomputing all
  updateMyContent();
}


void KDTimeTableWidget::setNoInformationBrush( const QBrush& brush )
{
  noInfoLineBrush = brush;
  updateMyContent();
}
QBrush KDTimeTableWidget::noInformationBrush() const
{
  return noInfoLineBrush;
}

void KDTimeTableWidget::removeItemFromTasklinks( KDGanttViewItem* item)
{
    QPtrListIterator<KDGanttViewTaskLink> it((myTaskLinkList));
    for ( ; it.current(); ++it ) {
        it.current()->removeItemFromList( item );
    }
}

void KDTimeTableWidget::expandItem( QListViewItem * item)
{
    item->invalidateHeight () ;
    qApp->processEvents();
    updateMyContent();
}
void KDTimeTableWidget::collapseItem( QListViewItem * item)
{
  item->invalidateHeight () ;
  qApp->processEvents();
  updateMyContent();
}

void KDTimeTableWidget::highlightItem( QListViewItem * item )
{
    static bool itemwashighlighted;
    static KDGanttViewItem* highlightedItem = 0;
    if (highlightedItem)
        highlightedItem->setHighlight(itemwashighlighted);
    highlightedItem = ( KDGanttViewItem*)item;
    itemwashighlighted = highlightedItem->highlight();
    highlightedItem->setHighlight(true);
    item->invalidateHeight () ;
    myGanttView->myListView->contentsY();
    updateMyContent();
}
int  KDTimeTableWidget::computeHeight()
{
  // compute height of ListView
  // show only items shown in ListView
  int hei = 0;
  KDGanttViewItem* temp;
  temp = myGanttView->firstChild();
  while (temp) {
    hei += temp->computeHeight();
    temp = temp->nextSibling();
  }
  // set hei  to 1 to avoid canavs to be a null pixmap
  if (hei == 0) {
    hei = 1;
  }
  // qDebug("COMPUTED HEI %d ", hei);
  emit heightComputed( hei );
  return hei;
}
void KDTimeTableWidget::computeVerticalGrid()
{
  // recompute the vertical grid
  // compute the vertical grid
  // if we have too much lines, hide them
  //qDebug("computeVerticalGrid() ");
  int cw =  myGanttView->myTimeHeader->myGridMinorWidth;
  int i = 0;
  int h ;
  if (pendingHeight > height() )
    h = pendingHeight;
  else
    h = height();
  int wid;
  if ( pendingWidth )
    wid = pendingWidth;
  else
    wid = width();
  KDCanvasLine* templine;
  KDCanvasRectangle* temprect;
  QColor colcol;
  QPen colPen;
  bool colorIterator = true;


  if (myGanttView->showMinorTicks()){//minor
            colPen.setWidth(cw);
            QPtrListIterator<KDCanvasRectangle> itcol(columnColorList);
            QPtrListIterator<KDCanvasLine> itgrid(verGridList);
            for ( ; itgrid.current(); ++itgrid ) {
                if (i < wid) {
                    itgrid.current()->setPoints(i,0,i,h);
                    itgrid.current()->show();

                    if (myGanttView->myTimeHeader->getColumnColor(colcol,i,i+cw))
                        {

                            colPen.setColor(colcol);
                            if (colorIterator)
                                colorIterator = itcol.current();
                            if (colorIterator)
                                {/*
                                    itcol.current()->setPen(colPen);
                                    itcol.current()->setPoints(i+(cw/2),0,i+(cw/2),h);
				 */

				  itcol.current()->setPen( QPen::NoPen );
				  itcol.current()->setBrush( QBrush( colcol, SolidPattern) );
				  itcol.current()->setSize(cw ,h );
				  itcol.current()->move( i, 0 );
                                    itcol.current()->show();
                                    ++itcol;
                                } else {

				  /*
                                    templine = new KDCanvasLine(this,0,Type_is_KDGanttGridItem);
                                    templine->setPen(colPen);
                                    templine->setPoints(i+(cw/2),0,i+(cw/2),h);
				  */
				  temprect = new KDCanvasRectangle(this,0,Type_is_KDGanttGridItem);
				  temprect->setPen( QPen::NoPen );
				  temprect->setBrush( QBrush( colcol, SolidPattern) );
				  temprect->setSize(cw ,h );
				  temprect->move( i, 0 );
                                    temprect->setZ(-20);
                                    temprect->show();
                                    columnColorList.append(temprect);
                                }
                        }
                    i += cw;
                } else {
                    itgrid.current()->hide();
                }
            }
            // create additional Lines for vertical grid
            for ( ;i < wid;i += cw) {
                templine = new KDCanvasLine(this,0,Type_is_KDGanttGridItem);
                templine->setPen(gridPen);
                templine->setPoints(i,0,i,h);
                templine->setZ(0);
                templine->show();
                verGridList.append(templine);
                if (myGanttView->myTimeHeader->getColumnColor(colcol,i,i+cw))
                    {
                        colPen.setColor(colcol);
                        if (colorIterator)
                            colorIterator = itcol.current();
                        if (colorIterator)
                            {/*
                                itcol.current()->setPen(colPen);
                                itcol.current()->setPoints(i+(cw/2),0,i+(cw/2),h);
			     */
				  itcol.current()->setPen( QPen::NoPen );
				  itcol.current()->setBrush( QBrush( colcol, SolidPattern) );
				  itcol.current()->setSize(cw ,h );
				  itcol.current()->move( i, 0 );
                                itcol.current()->show();
                                ++itcol;
                            } else {
			      temprect = new KDCanvasRectangle(this,0,Type_is_KDGanttGridItem);
				  temprect->setPen( QPen::NoPen );
				  temprect->setBrush( QBrush( colcol, SolidPattern) );
				  temprect->setSize(cw ,h );
				  temprect->move( i, 0 );
                                    temprect->setZ(-20);
                                    temprect->show();
                                    columnColorList.append(temprect);
				    /*
                                templine = new KDCanvasLine(this,0,Type_is_KDGanttGridItem);
                                templine->setPen(colPen);
                                templine->setPoints(i+(cw/2),0,i+(cw/2),h);
                                templine->setZ(-20);
                                templine->show();
                                columnColorList.append(templine);
				    */
                            }
                    }
            }
            if (colorIterator)
                for ( ; itcol.current(); ++itcol )
                    itcol.current()->hide();
        } else {//major
            if (myGanttView->showMajorTicks()) {
                QValueList<int>::iterator intIt = myGanttView->myTimeHeader->majorTicks.begin();
                QValueList<int>::iterator intItEnd = myGanttView->myTimeHeader->majorTicks.end();
                QPtrListIterator<KDCanvasRectangle> itcol(columnColorList);
                QPtrListIterator<KDCanvasLine> itgrid(verGridList);
                int left = 0;
                for ( ; itgrid.current(); ++itgrid ) {
                    if (intIt != intItEnd) {
                        left = (*intIt);
                        ++intIt;
                        itgrid.current()->setPoints(left,0,left,h);
                        itgrid.current()->show();
                        //int right = (*intIt);
                        if ((*intIt))
                            if (myGanttView->myTimeHeader->getColumnColor(colcol,left,(*intIt) ))
                                {
                                    int mid = (-left+(*intIt));
                                    colPen.setColor(colcol);
                                    colPen.setWidth((*intIt)-left);
                                    if (colorIterator)
                                        colorIterator = itcol.current();
                                    if (colorIterator)
                                        {/*
                                            itcol.current()->setPen(colPen);
                                            itcol.current()->setPoints(i+mid,0,mid,h);
					 */
					  itcol.current()->setPen( QPen::NoPen );
					  itcol.current()->setBrush( QBrush( colcol, SolidPattern) );
					  itcol.current()->setSize(mid ,h );
					  itcol.current()->move( left, 0 );
					  itcol.current()->show();
					  ++itcol;
                                        } else {
					   temprect = new KDCanvasRectangle(this,0,Type_is_KDGanttGridItem);
					   temprect->setPen( QPen::NoPen );
					   temprect->setBrush( QBrush( colcol, SolidPattern) );
					   temprect->setSize(mid,h );
					   temprect->move( left, 0 );
					   temprect->setZ(-20);
					   temprect->show();
					   columnColorList.append(temprect);
					   /*
					     templine = new KDCanvasLine(this,0,Type_is_KDGanttGridItem);
					     templine->setPen(colPen);
					     templine->setPoints(mid,0,i+mid,h);
					     templine->setZ(-20);
					     templine->show();
					     columnColorList.append(templine);
					   */

                                        }
                                }

                    } else {
                        itgrid.current()->hide();
                    }
                }
                KDCanvasLine* templine;
                // create additional Lines for vertical grid
                for ( ;intIt != intItEnd  ;++intIt) {

                    templine = new KDCanvasLine(this,0,Type_is_KDGanttGridItem);
                    templine->setPen(gridPen);
                    templine->setPoints((*intIt),0,(*intIt),h);
                    templine->setZ(0);
                    templine->show();
                    verGridList.append(templine);
                    if ((*intIt))
                        if (myGanttView->myTimeHeader->getColumnColor(colcol,left,(*intIt)))
                            {
                                int mid = (-left+(*intIt));
                                colPen.setColor(colcol);
                                colPen.setWidth((*intIt)-left);
                                if (colorIterator)
                                    colorIterator = itcol.current();
                                if (colorIterator)
                                    {/*
                                        itcol.current()->setPen(colPen);
                                        itcol.current()->setPoints(i+mid,0,mid,h);
				     */
				      itcol.current()->setPen( QPen::NoPen );
				      itcol.current()->setBrush( QBrush( colcol, SolidPattern) );
				      itcol.current()->setSize(mid ,h );
				      itcol.current()->move( left, 0 );
				      itcol.current()->show();
				      ++itcol;
                                    } else {
				      temprect = new KDCanvasRectangle(this,0,Type_is_KDGanttGridItem);
				      temprect->setPen( QPen::NoPen );
				      temprect->setBrush( QBrush( colcol, SolidPattern) );
				      temprect->setSize(mid ,h );
				      temprect->move( left, 0 );
				      temprect->setZ(-20);
				      temprect->show();
				      columnColorList.append(temprect);
				      /*
                                        templine = new KDCanvasLine(this,0,Type_is_KDGanttGridItem);
                                        templine->setPen(colPen);
                                        templine->setPoints(mid,0,i+mid,h);
                                        templine->setZ(-20);
                                        templine->show();
                                        columnColorList.append(templine);
				      */
                                    }
                            }
                    left = (*intIt);
                }
                if (colorIterator)
                    for ( ; itcol.current(); ++itcol ) {
                        itcol.current()->hide();
                    }

            }
            else {
                //hideall
                QPtrListIterator<KDCanvasLine> itgrid(verGridList);
                for ( ; itgrid.current(); ++itgrid ) {
                    itgrid.current()->hide();
                }
                QPtrListIterator<KDCanvasRectangle> itcol(columnColorList);
                for ( ; itcol.current(); ++itcol ) {
                    itcol.current()->hide();
                }
            }
        }
}
void KDTimeTableWidget::computeHorizontalGrid()
{
  // compute  horizontal grid
  //qDebug("computeHorizontalGrid() ");
    KDGanttViewItem* temp = myGanttView->firstChild();
    int wid;
    if ( pendingWidth )
      wid = pendingWidth;
    else
      wid = width();
    KDCanvasLine* templine;
    QPtrListIterator<KDCanvasLine> ithor(horGridList);
    if ( ithor.current() ) {
      templine = ithor.current();
      ++ithor;
    } else {
      templine = new KDCanvasLine(this,0,Type_is_KDGanttGridItem);
      templine->setPen(gridPen);
      templine->setZ(0);
      horGridList.append(templine);
    }
    templine->setPoints(0,0,wid,0);
    templine->show();
    int posY;
    while ( temp ) {
      posY = temp->itemPos() + temp->height();
      if ( ithor.current() ) {
	templine = ithor.current();
	++ithor;
      } else {
	//new vertical grid line
	templine = new KDCanvasLine(this,0,Type_is_KDGanttGridItem);
        templine->setPen(gridPen);
        templine->setZ(0);
        horGridList.append(templine);
      }
      if ( templine->endPoint() != QPoint(wid,posY ))
	templine->setPoints(0,posY,wid,posY );
      if ( !templine->isVisible() )
	templine->show();
      temp = temp->itemBelow ();
    }
    while ( ithor.current() ) {
      if ( ithor.current()->isVisible() )
	ithor.current()->hide();
      ++ithor;
    }
}

void KDTimeTableWidget::computeDenseLines()
{
    KDGanttViewItem* temp = myGanttView->firstChild();
    int wid;
    if ( pendingWidth )
      wid = pendingWidth;
    else
      wid = width();
    QPtrListIterator<KDCanvasRectangle> ithordense(horDenseList);
    KDCanvasRectangle* denseLine;
    int tempDenseLineCount = 0;
    while ( temp ) {
      ++tempDenseLineCount;
      if ( tempDenseLineCount == denseLineCount ) {
	tempDenseLineCount = 0;
	if ( ithordense.current() ) {
	  denseLine = ithordense.current();
	  ++ithordense;
	} else {
	  denseLine =new KDCanvasRectangle(this,0,Type_is_KDGanttGridItem);
	  denseLine->setZ(-2);
	  horDenseList.append( denseLine );
	}
	if ( denseLine->rect() != QRect(0, temp->itemPos(),wid, temp->height()) ) {
	  denseLine->move( 0, temp->itemPos() );
	  denseLine->setSize( wid, temp->height());
	}
	if (denseLine->brush() != denseLineBrush ) {
	  denseLine->setPen( QPen(  Qt::NoPen ) );
	  denseLine->setBrush( denseLineBrush);
	}
	if (!denseLine->isVisible() )
	  denseLine->show();

      }
      temp = temp->itemBelow ();
    }
    while ( ithordense.current() ) {
      if ( ithordense.current()->isVisible() )
	ithordense.current()->hide();
      ++ithordense;
    }
}
void KDTimeTableWidget::computeShowNoInformation()
{
    KDGanttViewItem* temp = myGanttView->firstChild();
    int wid;
    if ( pendingWidth )
      wid = pendingWidth;
    else
      wid = width();
    QPtrListIterator<KDCanvasRectangle> itnoinfo(showNoInfoList);
    KDCanvasRectangle* noInfoLine;
    while ( temp ) {
      if ( temp->showNoInformation() ) {
	if ( itnoinfo.current() ) {
	  noInfoLine = itnoinfo.current();
	  ++itnoinfo;
	} else {
	  noInfoLine =new KDCanvasRectangle(this,0,Type_is_KDGanttGridItem);
	  showNoInfoList.append( noInfoLine );
	  noInfoLine->setZ(-1);
	}
	noInfoLine->move( 0, temp->itemPos() );
	noInfoLine->setSize( wid, temp->height());
	noInfoLine->setPen( QPen(  Qt::NoPen ) );
	noInfoLine->setBrush( noInfoLineBrush);
	noInfoLine->show();
      }
      temp = temp->itemBelow ();
    }
    while ( itnoinfo.current() ) {
      itnoinfo.current()->hide();
      ++itnoinfo;
    }

}

void KDTimeTableWidget::computeTaskLinks()
{
  //compute and show tasklinks
    QPtrListIterator<KDGanttViewTaskLink> it((myTaskLinkList));
    for ( ; it.current(); ++it ) {
        if (it.current()->isVisible())
            it.current()->setVisible(true);
        else
            it.current()->setVisible(false);
    }
}

// updateMyContent() can be blocked by blockUpdating( true ) or inc_blockUpdating()
// updateMyContent() is blocked, if the GanttView is hidden after startup

void KDTimeTableWidget::updateMyContent()
{
  if ( flag_blockUpdating || int_blockUpdating ) {
    // qDebug("KDTimeTableWidget::updateMyContent() blocked! ");
    return;
  }
  //qDebug("KDTimeTableWidget::updateMyContent() ********************************* ");
  /*
  // debug output
  KDGanttViewItem* temp =  myGanttView->firstChild();
  while (temp != 0) {
    temp->printinfo("  " );
    temp = temp->nextSibling();
  }
  */
  int hei = computeHeight();
    minimumHeight = hei;
    int viewport_hei = myGanttView->myCanvasView->viewport()->height();
    if ( viewport_hei > hei )
      hei = viewport_hei + 100;
    if ( myGanttView->myTimeHeader->registerStartTime() )
      return; // try again via timeheader computeTicks();
    if ( myGanttView->myTimeHeader->registerEndTime() )
      return; // try again via timeheader computeTicks();
    if ( hei > height() ) {
      if ( !  pendingWidth )
	pendingWidth = width();
      if ( pendingHeight < hei )
      pendingHeight = hei;
    }
    if (pendingHeight > hei )
      hei =  pendingHeight;
    if (hei > maximumComputedGridHeight)
    {
        maximumComputedGridHeight = hei;
	// compute the background interval lines
	myGanttView->myTimeHeader->computeIntervals( hei );
	//compute VerticalGrid and column color
	computeVerticalGrid();
    }
    computeTaskLinks();
    computeHorizontalGrid();
    computeDenseLines();
    computeShowNoInformation();
    //setAllChanged();
    update();
     if (pendingWidth && pendingHeight ) {
      resize( pendingWidth, pendingHeight );
      pendingWidth = 0;
      pendingHeight = 0;
      emit heightComputed( 0 );
    }
    pendingWidth = 0;
    pendingHeight = 0;

}
// used for blocking recursive methods
// e.g. KDGanttViewItem::setHighlight() and  displaySubitemsAsGroup() == true

void KDTimeTableWidget::inc_blockUpdating( )
{
    ++ int_blockUpdating;
}
// used for blocking recursive methods
void KDTimeTableWidget::dec_blockUpdating( )
{
    -- int_blockUpdating;
}
// if false(i.e. unblock), sets int_blockUpdating to 0
void KDTimeTableWidget::setBlockUpdating( bool block )
{
  if ( !block )
    int_blockUpdating = 0;
  flag_blockUpdating = block;
}
bool KDTimeTableWidget::blockUpdating()
{
  return flag_blockUpdating;
}

void KDTimeTableWidget::setShowTaskLinks( bool show )
{
    taskLinksVisible = show;
    updateMyContent();
}
bool KDTimeTableWidget::showTaskLinks()
{
    return taskLinksVisible;
}
void KDTimeTableWidget::setHorBackgroundLines( int count,  QBrush brush )
{
  denseLineBrush = brush;
  denseLineCount = 0;
  if ( count > 1 )
    denseLineCount = count;
}


int KDTimeTableWidget::horBackgroundLines(  QBrush& brush )
{
  brush =  denseLineBrush;
  return denseLineCount;
}



/* ***************************************************************
   KDTimeHeaderWidget:: KDTimeHeaderWidget
   ***************************************************************** */
KDTimeHeaderWidget:: KDTimeHeaderWidget( QWidget* parent,KDGanttView* gant ):QWidget (parent)
{
    new KDTimeHeaderToolTip(this,this);
    mySizeHint = 0;
    myGanttView = gant;
    flagDoNotRecomputeAfterChange = true;
    QDateTime start = (QDateTime::currentDateTime ()).addSecs(-3600);
    setHorizonStart(start);
    setHorizonEnd( start.addSecs(3600*2));
    flagStartTimeSet = false;
    flagEndTimeSet = false;
    myCenterDateTime = QDateTime::currentDateTime ();
    setScale(KDGanttView::Auto);
    //setScale(KDGanttView::Hour);
    myMaxScale = KDGanttView::Month;
    myMinScale = KDGanttView::Minute;
    myAutoScaleMinorTickcount = 100;
    setMajorScaleCount( 1 );
    setMinorScaleCount( 1);
    setMinimumColumnWidth( 5 );
    setYearFormat(KDGanttView::FourDigit );
    setHourFormat( KDGanttView::Hour_12 );
    myZoomFactor = 1.0;
    setWeekendBackgroundColor(QColor(220,220,220) );
    setWeekendDays( 6, 7 );
    myGridMinorWidth = 0;
    myPopupMenu = new QPopupMenu(this);
    QPopupMenu *scalePopupMenu = new QPopupMenu(this);
    myPopupMenu->insertItem (i18n("Scale"),scalePopupMenu);
    scalePopupMenu->insertItem( i18n("Minute"),this, SLOT(setSettings(int)),0 ,1,1 );
    scalePopupMenu->insertItem( i18n("Hour"),this, SLOT(setSettings(int)),0 ,2,2 );
    scalePopupMenu->insertItem( i18n("Day"),this, SLOT(setSettings(int)),0 ,3,3 );
    scalePopupMenu->insertItem( i18n("Week"),this, SLOT(setSettings(int)),0 ,4,4 );
    scalePopupMenu->insertItem( i18n("Month"),this, SLOT(setSettings(int)),0 ,5,5 );
    scalePopupMenu->insertItem( i18n("Auto"),this, SLOT(setSettings(int)),0 ,6,6 );
    QPopupMenu *gridPopupMenu = new QPopupMenu(this);
    myPopupMenu->insertItem (i18n("Grid"),gridPopupMenu);
    gridPopupMenu->insertItem( i18n("Show minor grid"),this, SLOT(setSettings(int)),0 ,10,10 );
    gridPopupMenu->insertItem( i18n("Show major grid"),this, SLOT(setSettings(int)),0 ,11,11 );
    gridPopupMenu->insertItem( i18n("Show no grid"),this, SLOT(setSettings(int)),0 ,12,12 );
    QPopupMenu *zoomPopupMenu = new QPopupMenu(this);
    myPopupMenu->insertItem (i18n("Zoom"),zoomPopupMenu);
    zoomPopupMenu->insertItem( i18n("Zoom to fit"),this, SLOT(setSettings(int)),0 ,20,20 );
    zoomPopupMenu->insertItem( i18n("Zoom to 100%"),this, SLOT(setSettings(int)),0 ,21,21 );
    zoomPopupMenu->insertItem( i18n("Zoom in (x 2)"),this, SLOT(setSettings(int)),0 ,22,22 );
    zoomPopupMenu->insertItem( i18n("Zoom out (x 1/2)"),this, SLOT(setSettings(int)),0 ,23,23 );
    myPopupMenu->insertItem( i18n("Print"),this, SLOT(setSettings(int)),0 ,30,30 );
    flagZoomZoFit = false;
    setShowMinorTicks( true );
    myRealEnd =  myHorizonEnd;
    myRealStart = myHorizonStart;
    flagDoNotRecomputeAfterChange = false;
    flagDoNotRepaintAfterChange = false;
    setShowPopupMenu(false);
    for (int j =1;j<8;++j)
        weekdayColor[j] = Qt::white;
    myMinimumWidth = 0;
}
QString  KDTimeHeaderWidget::getToolTipText(QPoint p)
{
    QString s = getDateTimeForIndex( p.x()).toString();
    return s;
}
void KDTimeHeaderWidget::addTickRight( int num )
{
  int secs = ((num*getTickTime())-30);
  setHorizonEnd(getDateTimeForIndex(width()).addSecs(secs));
  qApp->processEvents();
}

void KDTimeHeaderWidget::addTickLeft( int num )
{
  int secs = ((num*getTickTime())-30);
  setHorizonStart(getDateTimeForIndex(0).addSecs(-secs));
  qApp->processEvents();
}
  // the time in secs of one minor grid tick
int KDTimeHeaderWidget::getTickTime()
{
  return getDateTimeForIndex(0).secsTo(getDateTimeForIndex(myGridMinorWidth));
}


void KDTimeHeaderWidget::checkWidth( int wid )
{
  myMinimumWidth = wid;

  if ( myMinimumWidth  > width() ||
       ( myMinimumWidth > mySizeHint &&
	 myMinimumWidth < (width() - myGridMinorWidth  )) )
    computeTicks();

}

bool KDTimeHeaderWidget::registerStartTime()
{

  QListViewItemIterator it( myGanttView->myListView );
    if (!flagStartTimeSet) {
        QDateTime temp , time;
        KDGanttViewItem* item;
	bool setNewTime = false;
        item = (KDGanttViewItem*)myGanttView->myListView->firstChild();
        if ( item ) {
            temp = item->startTime();
            time = temp;
	    //  while ( item != 0)
	      for ( ; it.current(); ++it ) {
		item = ( KDGanttViewItem* )it.current();
	      if (item->isVisible) {
		if ( !setNewTime )
		  temp = item->startTime();
		switch( item->type() ) {
		case KDGanttViewItem::Event:
		  time = ((KDGanttViewEventItem*)item)->leadTime();
		  setNewTime = true;
		  break;
		case KDGanttViewItem::Summary:
		case KDGanttViewItem::Task:
		  time = item->startTime();
		  setNewTime = true;
		  break;
		default:
		  time = temp;
		}
		if ( time < temp) {
		  temp = time ;
		}
	      }
	    }
	    if ( setNewTime )
	      if ( myHorizonStart != temp) {
                myHorizonStart = temp;
                computeTicks();
		return true;
            }
        }
    }
    return false;
}


bool KDTimeHeaderWidget::registerEndTime()
{
    if (!flagEndTimeSet) {
        QDateTime temp , time;
        KDGanttViewItem* item;
	bool setNewTime = false;
        item = (KDGanttViewItem*)myGanttView->myListView->firstChild();
        if ( item ) {
            temp = item->startTime();
            time = temp;
	    QListViewItemIterator it( myGanttView->myListView );
	    for ( ; it.current(); ++it ) {
		item = ( KDGanttViewItem* )it.current();
	      if (item->isVisible) {
		if ( !setNewTime )
		  temp = item->startTime();
                switch( item->type() ) {
                case KDGanttViewItem::Event:
		  time = ((KDGanttViewEventItem*)item)->startTime();
		  setNewTime = true;
		  break;
                case KDGanttViewItem::Summary:
		  time = item->endTime();
		  if ( time < ((KDGanttViewSummaryItem*)item)->actualEndTime())
		    time = ((KDGanttViewSummaryItem*)item)->actualEndTime();
		  setNewTime = true;
		  break;
		case KDGanttViewItem::Task:
		  time = item->endTime();
		  setNewTime = true;
		  break;
                default:
		  time = temp;
                }
                if ( time > temp)
		  temp = time ;
	      }
	    }

	    if ( setNewTime )
	      if (myHorizonEnd != temp ) {
                myHorizonEnd = temp;
                computeTicks();
		return true;
            }
        }
    }
    return false;
}


void KDTimeHeaderWidget::setShowPopupMenu( bool show )
{
    flagShowPopupMenu = show;
}


bool KDTimeHeaderWidget::showPopupMenu() const
{
    return flagShowPopupMenu;
}

void KDTimeHeaderWidget::setSettings(int i)
{

    switch (i) {
    case 1:
        setScale(KDGanttView::Minute );
        break;
    case 2:
        setScale(KDGanttView::Hour );
        break;
    case 3:
        setScale(KDGanttView::Day );
        break;
    case 4:
        setScale(KDGanttView::Week );
        break;
    case 5:
        setScale(KDGanttView::Month );
        break;
    case 6:
        setScale(KDGanttView::Auto );
        break;
    case 10:
        setShowMinorTicks( true );
        break;
    case 11:
        setShowMajorTicks( true );{

        }
        break;
    case 12:
        setShowMajorTicks( false );
        setShowMinorTicks( false);
        break;
    case 20:
        zoomToFit();
        break;
    case 21:
        zoom(1.0);
        break;
    case 22:
        zoom(2.0,false);
        break;
    case 23:
        zoom(0.5,false);
        break;
    case 30:
        myGanttView->print();
        break;
    }
    // myGanttView->myTimeTable->updateMyContent();
}
void KDTimeHeaderWidget::zoomToFit()
{
    flagZoomZoFit = true;
    computeTicks();
}
double KDTimeHeaderWidget::zoomFactor()
{
    return myZoomFactor;
}
double KDTimeHeaderWidget::secsFromTo( QDateTime begin, QDateTime end )
{
    QDateTime temp;
    double secs, days;
    days = begin.daysTo(end);
    temp = begin.addDays((int) days);
    secs = temp.secsTo(end);
    secs += days * 86400.0;
    return secs;
}


void KDTimeHeaderWidget::zoomToSelection( QDateTime start, QDateTime end)
{
    if (start < myHorizonStart)
        start =  myHorizonStart;
    if (end > myHorizonEnd)
        end = myHorizonEnd;
    flagDoNotRepaintAfterChange = true;//avoid flicker
    zoom(1.0); // set to 100%
    int viewWid = myGanttView->myCanvasView->viewport()->width();
    double fac = viewWid/(((double)width())* (secsFromTo( start, end ) /
                                              secsFromTo( myHorizonStart, myHorizonEnd) ));
    zoom (fac);
    int timeWid =  getCoordX(end)-getCoordX(start);
    while (timeWid >viewWid ) {
        fac = (fac * (double)viewWid)/(double)timeWid;
        zoom (fac);
        timeWid =  getCoordX(end)-getCoordX(start);
    }
    //done
    flagDoNotRepaintAfterChange = false;
    updateTimeTable();
    repaint();
    moveTimeLineTo((getCoordX(start)-(viewWid-timeWid)/2));
}
void KDTimeHeaderWidget::moveTimeLineTo(int X)
{
    int Y = myGanttView->myCanvasView->contentsY ();
    myGanttView->myCanvasView->setContentsPos (X, Y );
}

void KDTimeHeaderWidget::zoom(double factor, bool absolute)
{
    if (absolute)
        myZoomFactor = factor;
    else
        myZoomFactor *= factor;
    computeTicks();
}

/*!
  Sets the start of the horizon of the Gantt chart. If \a start is
  null, the horizon start is computed automatically.

  \param start the start of the horizon
  \sa horizonStart()
*/

void KDTimeHeaderWidget::setHorizonStart( const QDateTime& start )
{
    myHorizonStart = start;
    flagStartTimeSet = true;
    computeTicks();
}


/*!
  Returns the start of the horizon of the Gantt chart.

  \return the start of the horizon of the Gantt chart
  \sa setHorizonStart()
*/

QDateTime KDTimeHeaderWidget::horizonStart() const
{
    return myHorizonStart;
}


/*!
  Sets the end of the horizon of the Gantt chart. If \a end is
  null, the horizon end is computed automatically.

  \param end the end of the horizon
  \sa setHorizonEnd()
*/

void KDTimeHeaderWidget::setHorizonEnd( const QDateTime& start )
{
    myHorizonEnd = start;
    flagEndTimeSet = true;
    computeTicks();

}


/*!
  Returns the end of the horizon of the Gantt chart.

  \return the end of the horizon of the Gantt chart
  \sa setHorizonEnd()

*/

QDateTime KDTimeHeaderWidget::horizonEnd() const
{
    return myHorizonEnd;
}


/*!
  Configures the unit of the lower scale of the header. The higher
  unit is computed automatically.
  Resets the zoomng factor to 1 (i.e. 100%).

  \param unit the unit of the lower scale of the header.
  \sa scale()
*/

void KDTimeHeaderWidget::setScale(Scale unit )
{
    myScale = unit;
    myZoomFactor = 1.0;
    computeTicks();


}


/*!
  Returns the unit of the lower scale of the header.

  \return the unit of the lower scale of the header.
  \sa setScale()
*/

KDTimeHeaderWidget::Scale KDTimeHeaderWidget::scale() const
{
    return myScale;
}
/*!
  Sets the maximal allowed time scale of the lower scale of the header.

  \param unit the unit of the lower scale of the header.
  \sa scale()
*/

void  KDTimeHeaderWidget::setMaximumScale( Scale unit )
{
    myMaxScale = unit;
    computeTicks();
}


/*!
  Returns the maximal allowed time scale of the lower scale of the header.

  \return the unit of the lower scale of the header.
  \sa setScale()
*/

KDTimeHeaderWidget::Scale  KDTimeHeaderWidget::maximumScale() const
{
    return myMaxScale;
}

/*!
  Sets the minimal allowed time scale of the lower scale of the header.

  \param unit the unit of the lower scale of the header.
  \sa scale()
*/

void  KDTimeHeaderWidget::setMinimumScale( Scale unit )
{
    myMinScale = unit;
    computeTicks();
}


/*!
  Returns the minimal allowed time scale of the lower scale of the header.

  \return the unit of the lower scale of the header.
  \sa setScale()
*/

KDTimeHeaderWidget::Scale  KDTimeHeaderWidget::minimumScale() const
{
    return myMinScale;
}


/*!
  Sets the minimum width a column needs to have. If the size of the
  Gantt chart and the scale would make it necessary to go below this
  limit otherwise, the chart will automatically be made less exact.

  \param width the minimum column width
  \sa minimumColumnWidth()
*/

void KDTimeHeaderWidget::setMinimumColumnWidth( int width )
{
    myMinimumColumWidth =  width;
    computeTicks();
}


/*!
  Returns the minimum width a column needs to have.

  \return the column minimum width
  \sa setMinimumColumnWidth()
*/

int KDTimeHeaderWidget::minimumColumnWidth() const
{
    return myMinimumColumWidth;
}


/*!
  Specifies the format in which to display years. If no years are
  shown, this method has no effect.

  \param format the year format
  \sa yearFormat(), setHourFormat(), hourFormat()
*/

void KDTimeHeaderWidget::setYearFormat( YearFormat format )
{
    myYearFormat =  format;
    computeTicks();
}


/*!
  Returns the format in which to display years.

  \return the year format
  \sa setYearFormat(), setHourFormat(), hourFormat()
*/

KDTimeHeaderWidget::YearFormat KDTimeHeaderWidget::yearFormat() const
{
    return  myYearFormat;
}


/*!
  Specifies the format in which to display hours. If no hours are
  shown, this method has no effect.

  \param format the hour format
  \sa hourFormat(), setYearFormat(), yearFormat()

*/

void KDTimeHeaderWidget::setHourFormat( HourFormat format )
{
    myHourFormat = format;
    computeTicks();
}


/*!
  Returns the format in which to display hours.

  \return the hour format
  \sa setHourFormat(), setYearFormat(), yearFormat()

*/

KDTimeHeaderWidget::HourFormat KDTimeHeaderWidget::hourFormat() const
{
    return myHourFormat;
}


/*!
  Specifies whether ticks should be shown on the major scale.

  \param show true in order to show ticks, false in order to hide them
  \sa showMajorTicks(), setShowMinorTicks(), showMinorTicks()
*/

void KDTimeHeaderWidget::setShowMajorTicks( bool show )
{
    flagShowMajorTicks = show;
    if (show) {
        setShowMinorTicks(false);
    }
    updateTimeTable();

}


/*!
  Returns whether ticks are shown on the major scale.

  \return true if ticks are shown on the major scale
  \sa setShowMajorTicks(), setShowMinorTicks(), showMinorTicks()
*/

bool KDTimeHeaderWidget::showMajorTicks() const
{
    return flagShowMajorTicks;
}


/*!
  Specifies whether ticks should be shown on the minor scale.

  \param show true in order to show ticks, false in order to hide them
  \sa showMinorTicks(), setShowMajorTicks(), showMajorTicks()

*/

void KDTimeHeaderWidget::setShowMinorTicks( bool show )
{
    flagShowMinorTicks = show;
    if (show)
        setShowMajorTicks(false );
    //repaintMe();
    updateTimeTable();
}


/*!
  Returns whether ticks are shown on the minor scale.

  \return true if ticks are shown on the minor scale
  \sa setShowMinorTicks(), setShowMajorTicks(), showMajorTicks()

*/

bool KDTimeHeaderWidget::showMinorTicks() const
{
    return flagShowMinorTicks;
}


/*!
  Sets the background color for the column closest to \a column.

  \param column the column to set the background color for
  \param color the background color
  \sa columnBackgroundColor(), setWeekendBackgroundColor(),
  weekendBackgroundColor()
*/

void KDTimeHeaderWidget::setColumnBackgroundColor( const QDateTime& column,
						   const QColor& color,
						   Scale mini, Scale maxi )
{
    ColumnColorList::iterator it;
    for ( it = ccList.begin(); it != ccList.end(); ++it ) {
        if ((*it).datetime == column) {
            (*it).color = color;
            (*it).minScaleView = mini;
            (*it).maxScaleView = maxi;
            return;
        }
    }
    DateTimeColor newItem;
    newItem.datetime = column;
    newItem.color = color;
    newItem.minScaleView = mini;
    newItem.maxScaleView = maxi;
    ccList.append(newItem);
    updateTimeTable();
}

void KDTimeHeaderWidget::computeIntervals( int height )
{

  IntervalColorList::iterator it;
  int left, right;
  for ( it = icList.begin(); it != icList.end(); ++it ) {
    if ( (*it).minScaleView <= myRealScale && (*it).maxScaleView >= myRealScale ) {
      left = getCoordX((*it).datetime);
      right = getCoordX((*it).end);
      if ( right == left )
	++right;
      (*it).canvasRect->setPen( QPen::NoPen );
      (*it).canvasRect->setBrush( QBrush( (*it).color, SolidPattern) );
      (*it).canvasRect->setSize( right - left ,height );
      (*it).canvasRect->move( left,0 );
      (*it).canvasRect->show();
    } else {
      (*it).canvasRect->hide();
      /*
      (*it).canvasLine->setPen( QPen( (*it).color, right - left ) );
      (*it).canvasLine->setPoints( mid ,0 ,mid ,height );
      (*it).canvasLine->show();
    } else {
      (*it).canvasLine->hide();
      */
    }
  }
}
bool KDTimeHeaderWidget::changeBackgroundInterval( const QDateTime& oldstart,
				   const QDateTime& oldend,
				   const QDateTime& newstart,
				   const QDateTime& newend )
{
  IntervalColorList::iterator it;
  for ( it = icList.begin(); it != icList.end(); ++it ) {
    if ((*it).datetime == oldstart && (*it).end == oldend ) {
      IntervalColorList::iterator it2;
      for ( it2 = icList.begin(); it2 != icList.end(); ++it2 ) {
	if ((*it2).datetime == newstart && (*it2).end == newend )
	  return false;
      }
      (*it).datetime = newstart;
      (*it).end = newend;
      updateTimeTable();
      return true;
    }
  }
  return false;
}
bool KDTimeHeaderWidget::deleteBackgroundInterval( const QDateTime& start,
						   const QDateTime& end)
{
  IntervalColorList::iterator it;
  for ( it = icList.begin(); it != icList.end(); ++it ) {
    if ((*it).datetime == start && (*it).end == end ) {
      //delete  (*it).canvasLine;
      delete  (*it).canvasRect;
      icList.remove(it);
      updateTimeTable();
      return true;
    }
  }
  return false;
}

void KDTimeHeaderWidget::setIntervalBackgroundColor( const QDateTime& start,
				    const QDateTime& end,
				  const QColor& color,
				  Scale mini ,
				  Scale maxi )
{

    IntervalColorList::iterator it;
    for ( it = icList.begin(); it != icList.end(); ++it ) {
        if ((*it).datetime == start && (*it).end == end ) {
            (*it).color = color;
            (*it).minScaleView = mini;
            (*it).maxScaleView = maxi;
            return;
        }
    }
    DateTimeColor newItem;
    if ( start <= end ) {
      newItem.datetime = start;
      newItem.end = end;
    } else {
      newItem.datetime = end;
      newItem.end = start;
    }
    newItem.color = color;
    newItem.minScaleView = mini;
    newItem.maxScaleView = maxi;
    //newItem.canvasLine = new KDCanvasLine(myGanttView->myTimeTable,0,Type_is_KDGanttGridItem);
    newItem.canvasRect = new KDCanvasRectangle(myGanttView->myTimeTable,0,Type_is_KDGanttGridItem);
    newItem.canvasRect->setZ(-19);
    icList.append(newItem);
    updateTimeTable();

}
void KDTimeHeaderWidget::clearBackgroundColor()
{

  IntervalColorList::iterator itic;
  for ( itic = icList.begin(); itic != icList.end(); ++itic ) {
    delete  (*itic).canvasRect;

  }
  ccList.clear();
  icList.clear();
  updateTimeTable();


}
QDateTime KDTimeHeaderWidget::getDateTimeForIndex(int coordX)
{
    double secs = (secsFromTo( myRealStart, myRealEnd ) * ((double)coordX))/(double)width();
    double days = secs/86400.0;
    secs = secs - ( ((int) days) *86400.0 );
    return (myRealStart.addDays ( (int) days )).addSecs( (int) secs);
}
bool KDTimeHeaderWidget::getColumnColor(QColor& col,int coordLow, int coordHigh)
{
    if (!flagShowMajorTicks && !flagShowMinorTicks)
        return false;
    QDateTime start,end;
    start = getDateTimeForIndex(coordLow);
    end = getDateTimeForIndex(coordHigh).addSecs(-1);
    Scale tempScale = myRealScale;
    if (flagShowMajorTicks)
        switch (myRealScale)
            {
            case KDGanttView::Minute: tempScale = KDGanttView::Hour;  break;
            case KDGanttView::Hour: tempScale = KDGanttView::Day   ;  break;
            case KDGanttView::Day: tempScale = KDGanttView::Week   ;  break;
            case KDGanttView::Week: tempScale = KDGanttView::Month  ;  break;
            case KDGanttView::Month: return false   ;  break;
            case KDGanttView::Auto: return false   ;  break;
            }
    //check defined column color
    ColumnColorList::iterator it;
    for ( it = ccList.begin(); it != ccList.end(); ++it ) {
        if ((*it).datetime  >= start && (*it).datetime  <= end) {
            if (tempScale >= (*it).minScaleView &&   tempScale <= (*it).maxScaleView    ) {
                col = (*it).color;
                return true;
            }
        }
    }

    if (tempScale > KDGanttView::Day) return false;

    start = getDateTimeForIndex((coordLow+coordHigh)/2);
    int day = start.date().dayOfWeek ();
    //checkweekdaycolor
    if (weekdayColor[day] != Qt::white) {
        col = weekdayColor[day];
        return true;
    }
    //checkweekendcolor
    int endday = myWeekendDaysEnd;
    col = myWeekendBackgroundColor;
    if (myWeekendDaysStart > myWeekendDaysEnd)
        endday +=7;
    if (day >= myWeekendDaysStart && day <= endday) {
        return true;
    } else {
        if (day+7 >= myWeekendDaysStart && day+7 <= endday) {
            return true;
        }
    }
    return false;
}

/*!
  Returns the background color for the column closes to \a column.

  \param column the column to query the background color for
  \return the background color of the specified column
  \sa setColumnBackgroundColor(), setWeekendBackgroundColor(),
  weekendBackgroundColor()
*/

QColor KDTimeHeaderWidget::columnBackgroundColor( const QDateTime& column ) const
{
    QColor c;
    c = white;
    ColumnColorList::const_iterator ite;
    for ( ite = ccList.begin(); ite != ccList.end(); ++ite ) {
        if ((*ite).datetime == column) {
            c = (*ite).color;
        }
    }
    return c;
}


/*!
  Specifies the background color for weekend days. If no individual
  days are visible on the Gantt chart, this method has no visible
  effect.

  \param color the background color to use for weekend days.
  \sa weekendBackgroundColor(), setWeekendDays(), weekendDays()
*/

void KDTimeHeaderWidget::setWeekendBackgroundColor( const QColor& color )
{
    myWeekendBackgroundColor = color ;
    updateTimeTable();
}


/*!
  Returns the background color for weekend days.

  \return the background color for weekend days
  \sa setWeekendBackgroundColor(), setWeekendDays(), weekendDays()
*/

QColor KDTimeHeaderWidget::weekendBackgroundColor() const
{
    return myWeekendBackgroundColor;
}

/*!
  Specifies the background color for weekday days. If no individual
  days are visible on the Gantt chart, this method has no visible
  effect.The days are specified as an interval of integer values
  where 1 means Monday and 7 means Sunday.


  \param color the background color to use for weekend days.
  \param weekday the day of the week (Monday = 1, Sunday = 7)
  \sa weekendBackgroundColor(), setWeekendDays(), weekendDays()
*/

void KDTimeHeaderWidget::setWeekdayBackgroundColor( const QColor& color, int  weekday )
{
    weekdayColor[weekday] = color;
    updateTimeTable();
}


/*!
  Returns the background color for weekday days.

  \param the day of the week (Monday = 1, Sunday = 7)
  \return the background color for weekend days
  \sa setWeekendBackgroundColor(), setWeekendDays(), weekendDays()
*/

QColor KDTimeHeaderWidget::weekdayBackgroundColor(int weekday) const
{
    return weekdayColor[weekday];
}

/*!
  Defines which days are considered weekends. The days are specified
  as an interval of integer values where 1 means Monday and 7 means
  Sunday. In order to define a weekend from Sunday to Monday, specify
  (7,1).

  \param start the first day of the weekend
  \param end the last day of the weekend
  \sa weekendDays(), setWeekendBackgroundColor(), weekendBackgroundColor()
*/

void KDTimeHeaderWidget::setWeekendDays( int start, int end )
{
    myWeekendDaysStart = start;
    myWeekendDaysEnd = end;
    updateTimeTable();
}


/*!
  Returns which days are considered weekends.

  \param start in this parameter, the first day of the weekend is returned
  \param end in this parameter, the end day of the weekend is returned
  \sa setWeekendDays(), setWeekendBackgroundColor(), weekendBackgroundColor()
*/

void KDTimeHeaderWidget::weekendDays( int& start, int& end ) const
{
    start = myWeekendDaysStart;
    end = myWeekendDaysEnd ;
}



/*!
  Sets the number of ticks in the major scale.

  \param count the number of ticks in the major scale
  \sa majorScaleCount(), setMinorScaleCount(), minorScaleCount()
*/

void KDTimeHeaderWidget::setMajorScaleCount( int count )
{
    myMajorScaleCount=count;
    computeTicks();
}


/*!
  Returns the number of ticks per unit in the major scale.

  \return the number of ticks in the major scale
  \sa setMajorScaleCount(), setMinorScaleCount(), minorScaleCount()
*/

int KDTimeHeaderWidget::majorScaleCount() const
{
    return myMajorScaleCount;
}


/*!
  Sets the number of ticks in the minor scale.

  \param count the number of ticks in the minor scale
  \sa minorScaleCount, setMajorScaleCount, majorScaleCount()

*/

void KDTimeHeaderWidget::setMinorScaleCount( int count )
{
    myMinorScaleCount = count;
    computeTicks();
}


/*!
  Returns the number of ticks per unit in the minor scale.

  \return the number of ticks in the minor scale
  \sa setMinorScaleCount(), setMajorScaleCount(), majorScaleCount()

*/

int KDTimeHeaderWidget::minorScaleCount() const
{
    return myMinorScaleCount ;

}


void KDTimeHeaderWidget:: resizeEvent ( QResizeEvent * )
{
    //qDebug("KDTimeHeaderWidget:: resizeEvent ");
}


void KDTimeHeaderWidget::updateTimeTable()
{
  //qDebug("KDTimeHeaderWidget::updateTimeTable() ");
    if (flagDoNotRecomputeAfterChange) return;
    // setting the scrolling steps
    int scrollLineStep = myGridMinorWidth;
    if (showMajorTicks()) {
      QValueList<int>::iterator intIt = majorTicks.begin();
      scrollLineStep = 5 * myGridMinorWidth;
      if (intIt != majorTicks.end()) {
	int left = *intIt;
	++intIt;
	if (intIt != majorTicks.end()) {
	  scrollLineStep = *intIt-left;
	}
      }
    }
    myGanttView->myCanvasView->horizontalScrollBar()->setLineStep(scrollLineStep);
    myGanttView->myTimeTable->maximumComputedGridHeight = 0;
    myGanttView->myTimeTable->updateMyContent();
}


void KDTimeHeaderWidget::setAutoScaleMinorTickCount( int count )
{
    myAutoScaleMinorTickcount = count;
    computeTicks();

}
int KDTimeHeaderWidget::autoScaleMinorTickCount()
{
    return myAutoScaleMinorTickcount;
}


void KDTimeHeaderWidget::repaintMe(int left,int paintwid, QPainter* painter)
{
    if (flagDoNotRecomputeAfterChange) return;
    QPainter* p;
    if ( painter )
        p = painter;
    else
        p = new QPainter( this );
    p->setPen(black);
    int hei1 = myMajorGridHeight,
        hei2 = height(),
        wid1 = myGridMinorWidth;
    p->drawLine(0,hei1,width(),hei1);
    QValueList<QString>::iterator it;
    int i = 0;
    for ( it =  minorText.begin(); it !=  minorText.end(); ++it ) {
        if (i*wid1 >= left-wid1 && i*wid1 <= left+paintwid) {
            p->drawLine(i*wid1,hei1,i*wid1,hei2);
            p->drawText(i*wid1,hei1+1,wid1-1,hei2-hei1,Qt::AlignCenter,(*it));
        }
        ++i;
    }
    int xCoord;
    QValueList<int>::iterator intIt = majorTicks.begin();
    for ( it =  majorText.begin(); it !=  majorText.end(); ++it ) {
        xCoord  = (*intIt++);
        if (((*intIt)>= left && xCoord <= left+paintwid)) {
            p->drawLine(xCoord,hei1,xCoord,0);
            p->drawText(xCoord+4,hei1-4,(*it));
        }
    }
    if ( !painter ) {
        p->end();
        delete p;
    }
}

// cuts the secs in the DateTime if scale is Minute ,
// the minutes and secs if scale is Hour and so on

QDateTime KDTimeHeaderWidget::getEvenTimeDate(QDateTime tempdatetime ,Scale sc)
{
    QDate tempdate;
    int min;
    switch (sc)
        {
        case KDGanttView::Month:
            tempdate = tempdatetime.date();
            while (tempdate.day ()!= 1 )
                tempdate = tempdate.addDays(-1);
            //while (tempdate.month ()!= 1 )
            //tempdate = tempdate.addMonths(-1);
            tempdatetime = QDateTime (tempdate, QTime (0,0));
            break;
        case KDGanttView::Week:
            tempdate = tempdatetime.date();
            while (tempdate.dayOfWeek ()!= 1 )
                tempdate = tempdate.addDays(-1);
            //tempdate = tempdate.addDays(-7);
            tempdatetime = QDateTime (tempdate, QTime (0,0));
            break;
        case KDGanttView::Day:
            tempdatetime = QDateTime (tempdatetime.date(), QTime ( 0,0 ) );
            break;
        case KDGanttView::Hour:
            tempdatetime = QDateTime (tempdatetime.date(), QTime (tempdatetime.time().hour(), 0 ));
	    //tempdatetime.addSecs( -3600 );
            break;
        case KDGanttView::Minute:
	  min = tempdatetime.time().minute();
	  //min = (min /15)*15;
            tempdatetime = QDateTime (tempdatetime.date(), QTime (tempdatetime.time().hour(),min ));

            break;
        case KDGanttView::Auto:
            break;
        }
    return tempdatetime;
}

void KDTimeHeaderWidget::computeRealScale(QDateTime start)
{

    if (myScale ==KDGanttView::Auto) {
        double secsPerMinor = (((double)start.daysTo(myHorizonEnd))* 86400.00)/((double)myAutoScaleMinorTickcount);
        secsPerMinor /= myZoomFactor;
        if (secsPerMinor <= 1800) {
            myRealScale =  KDGanttView::Minute;
            myRealMinorScaleCount = (int) secsPerMinor/60;
        } else {
            if (secsPerMinor <= 12*3600) {
                myRealScale =  KDGanttView::Hour;
                myRealMinorScaleCount = (int)  secsPerMinor/3600;
            } else {
                if (secsPerMinor <= 24*3600*3) {
                    myRealScale =  KDGanttView::Day;
                    myRealMinorScaleCount = (int)  secsPerMinor/(3600*24);
                } else {
                    if (secsPerMinor <= 24*3600*14) {
                        myRealScale =  KDGanttView::Week;
                        myRealMinorScaleCount =  (int) secsPerMinor/(3600*24*7);
                    } else {
                        myRealScale =  KDGanttView::Month;
                        myRealMinorScaleCount =  (int) secsPerMinor/(3600*24*30);

                    }
                }
            }
        }
        if(myRealMinorScaleCount == 0)
            myRealMinorScaleCount = 1;
        myRealMajorScaleCount = 1;
    }
    else {
        myRealScale = myScale;
        if (myRealScale > myMaxScale)
            myRealScale = myMaxScale;
        if (myRealScale < myMinScale)
            myRealScale = myMinScale;
        myRealMinorScaleCount = (int) ( myMinorScaleCount /myZoomFactor );
        double tempZoom = myZoomFactor;
        myRealMajorScaleCount =  myMajorScaleCount;
        while (myRealMinorScaleCount == 0) {
            switch (myRealScale)
                {
                case KDGanttView::Minute:
                    myRealMinorScaleCount = 1;
                    return;
                    break;
                case KDGanttView::Hour:
                    if (myRealScale  == myMinScale)
                        break;
                    myRealScale = KDGanttView::Minute;
                    tempZoom = myZoomFactor/60;
                    break;
                case KDGanttView::Day:
                    if (myRealScale  == myMinScale)
                        break;
                    myRealScale = KDGanttView::Hour;
                    tempZoom = myZoomFactor/24;
                    break;
                case KDGanttView::Week:
                    if (myRealScale  == myMinScale)
                        break;
                    myRealScale = KDGanttView::Day;
                    tempZoom = myZoomFactor/7;
                    break;
                case KDGanttView::Month:
                    if (myRealScale  == myMinScale)
                        break;
                    myRealScale =  KDGanttView::Week ;
                    tempZoom = myZoomFactor*7/30;
                    break;
                case KDGanttView::Auto:
                    break;
                }
            myRealMinorScaleCount =  (int) ( myMinorScaleCount /tempZoom );
        }
    }
}

void KDTimeHeaderWidget::computeTicks(bool doNotComputeRealScale)
{
    if (flagDoNotRecomputeAfterChange) return;
    bool block = myGanttView->myTimeTable->blockUpdating();
    myGanttView->myTimeTable->setBlockUpdating( true );
    //qDebug("computeticks ");
    majorTicks.clear();
    minorText.clear();
    majorText.clear();
    if ( !doNotComputeRealScale )
      saveCenterDateTime();
    if (!doNotComputeRealScale)
        computeRealScale(myHorizonStart);
    myRealStart = getEvenTimeDate(myHorizonStart ,myRealScale);
    if (!doNotComputeRealScale)
        computeRealScale(myRealStart);
    int tempMinorScaleCount = myRealMinorScaleCount,
        tempMajorScaleCount = myRealMajorScaleCount;
    int minorItems,minorPerMajor = 1;
    minorItems = (int)  (secsFromTo( myRealStart, myHorizonEnd)/60.0);

    QPainter p(this);
    int Width, Height;
    QString testTextMinor,testTextMajor, tempStr;
    QRect itemRectMinor, itemRectMajor;
    QDate tempDate = myRealStart.date();
    myRealEnd = myRealStart;
    // preparing the testtext for the differennt scales
    switch (myRealScale)
        {
            // the x in testTextMajor is added to reserve a little bit more space
        case KDGanttView::Minute:
            testTextMinor = "60";
            if (myHourFormat == KDGanttView::Hour_12)
                testTextMajor = "Mon Aug 30, 12 AMx";
            else
                testTextMajor = "Mon Aug 30, 24x";
            minorPerMajor = 6000;
            break;
        case KDGanttView::Hour:
            minorItems = minorItems/60;
            if (myHourFormat == KDGanttView::Hour_12)
                testTextMinor = "12 AM";
            else
                testTextMinor = "24";
            testTextMajor = "Mon Aug 30, x"+getYear(QDate::currentDate());
            minorPerMajor = 2400;
            break;
        case KDGanttView::Day:
            minorItems = minorItems/(60*24);
            testTextMinor = "88";
            testTextMajor = "Aug 30, x"+getYear(QDate::currentDate());
            minorPerMajor = 700;
            break;
        case KDGanttView::Week:
            minorItems = minorItems/(60*24*7);
            testTextMinor = "88";
            testTextMajor = "Aug x"+getYear(QDate::currentDate());
            minorPerMajor = 435; // 435 = 365days/12months/7days * 100
            break;
        case KDGanttView::Month:
            minorItems = (minorItems*12)/(60*24*365);
            testTextMinor = "M";
            testTextMajor = "x"+getYear(QDate::currentDate());
            minorPerMajor = 1200;
            break;
        case KDGanttView::Auto:
	  qDebug("KDGanttView::Error in KDTimeHeaderWidget::computeTicks() ");
	  qDebug("RealScale == Auto : This may not be! ");
            break;
        }
    itemRectMinor = p.boundingRect ( 10, 10, 2, 2, Qt::AlignLeft,testTextMinor);
    itemRectMajor = p.boundingRect ( 10, 10, 2, 2, Qt::AlignLeft,testTextMajor);
    p.end();
    Height = itemRectMinor.height()+itemRectMajor.height()+9;
    Width = (itemRectMinor.width()+5);
    if (Width < minimumColumnWidth()) Width = minimumColumnWidth();
    // if the desired width is greater than the maximum width of this widget
    // increase the minorscalecount
    int maxWid = myGanttView->myCanvasView->viewport()->width();
    if (!flagZoomZoFit)
        maxWid = maximumWidth();
    while((minorItems/tempMinorScaleCount+1)*Width > maxWid)
        ++tempMinorScaleCount;
    mySizeHint = (minorItems/tempMinorScaleCount+1)*Width;
    while((minorItems/tempMinorScaleCount+1)*Width < myMinimumWidth ) {
      ++minorItems;
    }
    switch (myRealScale)
        {
        case KDGanttView::Minute:
            while (60%tempMinorScaleCount > 0 && 60%tempMinorScaleCount < 60)
                ++tempMinorScaleCount;
            if (tempMinorScaleCount >= 60) {
                myRealScale = KDGanttView::Hour;
                myRealMinorScaleCount = 1;
                myRealMajorScaleCount = 1;
                qDebug("KDGantt::Overzoom:Rescaling from Minute to Hour");
		myGanttView->myTimeTable->setBlockUpdating( block );
                computeTicks(true);
                return;
            }
            break;
        case KDGanttView::Hour:
            while (24%tempMinorScaleCount > 0 && 24%tempMinorScaleCount < 24)
                ++tempMinorScaleCount;
            if (tempMinorScaleCount >= 24) {
                myRealScale = KDGanttView::Day;
                myRealMinorScaleCount = 1;
                myRealMajorScaleCount = 1;
                qDebug("KDGantt::Overzoom:Rescaling from Hour to Day");
		myGanttView->myTimeTable->setBlockUpdating( block );
                computeTicks(true);
                return;
            }
            break;
        default:
            break;
        }
    flagZoomZoFit = false;
    minorItems = (minorItems/tempMinorScaleCount)+1;
    // if not enough space for the text of the major scale, increase majorscalecount
    minorPerMajor = (minorPerMajor*tempMajorScaleCount)/tempMinorScaleCount;
    // checking, if enough space for majorscale
    // if not, increasing MajorScaleCount

    while ((minorPerMajor*Width)/100 < itemRectMajor.width()) {
        minorPerMajor = minorPerMajor/tempMajorScaleCount;
        ++tempMajorScaleCount;
        minorPerMajor = minorPerMajor*tempMajorScaleCount;

    }
    // now we have the fixed  width of the minorscale computed
    myGridMinorWidth = Width;
    // the width of this widget is the gridwidth * the amount of items
    Width *= minorItems;
    // if size changed, reset geometry
    if (width() != Width   || height() != Height  )
        {
	  resize( Width, Height );
	  emit  sizeChanged( Width );
        }
    myMajorGridHeight = itemRectMajor.height()+5;
    QTime tempTime = myRealStart.time();
    QDateTime tempDateTime;
    int i;
    switch (myRealScale)
        {
        case KDGanttView::Minute:
            myRealEnd = myRealEnd.addSecs((minorItems)*tempMinorScaleCount*60);
            for ( i = 0; i < minorItems;++i) {
                tempStr.setNum(tempTime.minute());
                minorText.append(tempStr);
                tempTime = tempTime.addSecs(60*tempMinorScaleCount);
            }
            tempDateTime = myRealStart;
            while (tempDateTime.time().minute() != 0)
                tempDateTime = tempDateTime.addSecs(60);
            while (tempDateTime < myRealEnd) {
                majorTicks.append( getCoordX(tempDateTime));
                tempStr.setNum(tempDateTime.date().day());
                tempStr = QDate::shortDayName (tempDateTime.date().dayOfWeek() )+" "+
                          QDate::shortMonthName(tempDateTime.date().month())+ " "+
                          tempStr+", "+getHour(tempDateTime.time());
                majorText.append(tempStr);
                tempDateTime = tempDateTime.addSecs(3600*tempMajorScaleCount);
            }
            majorTicks.append( getCoordX(tempDateTime));
            break;

        case KDGanttView::Hour:
            myRealEnd = myRealEnd.addSecs(minorItems*tempMinorScaleCount*60*60);

            for ( i = 0; i < minorItems;++i) {
                tempStr = getHour(tempTime);
                minorText.append(tempStr);
                tempTime = tempTime.addSecs(3600*tempMinorScaleCount);
            }
            tempDateTime = myRealStart;
            while (tempDateTime.time().hour() != 0)
                tempDateTime = tempDateTime.addSecs(3600);
            while (tempDateTime < myRealEnd) {
                majorTicks.append( getCoordX(tempDateTime));
                tempStr.setNum(tempDateTime.date().day());
                tempStr = QDate::shortDayName (tempDateTime.date().dayOfWeek() )+" "+
                          QDate::shortMonthName(tempDateTime.date().month())+ " "+
                          tempStr+", "+getYear(tempDateTime.date());
                majorText.append(tempStr);
                tempDateTime = tempDateTime.addDays(tempMajorScaleCount);
            }
            majorTicks.append( getCoordX(tempDateTime));
            break;
        case KDGanttView::Day:
            myRealEnd = myRealEnd.addDays(minorItems*tempMinorScaleCount);
            for ( i = 0; i < minorItems;++i) {
                if (tempMinorScaleCount == 1)
                    minorText.append((QDate::shortDayName(tempDate.dayOfWeek())).left(1));
                else
                    minorText.append(QString::number(tempDate.day()));
                tempDate = tempDate.addDays(tempMinorScaleCount);
            }
            tempDate = myRealStart.date();
            while (tempDate.dayOfWeek() != 7)
                tempDate = tempDate.addDays(1);
            while (tempDate < myRealEnd.date()) {
                majorTicks.append( getCoordX(tempDate));
                tempStr.setNum(tempDate.day());
                tempStr = QDate::shortMonthName(tempDate.month())+ " "+
                          tempStr+", "+getYear(tempDate);
                majorText.append(tempStr);
                tempDate = tempDate.addDays(7*tempMajorScaleCount);
            }
            majorTicks.append( getCoordX(tempDate));
            break;
        case KDGanttView::Week:
            myRealEnd = myRealEnd.addDays(minorItems*tempMinorScaleCount*7);
            for ( i = 0; i < minorItems;++i) {
                tempStr.setNum(tempDate.day());
                minorText.append(tempStr);
                tempDate = tempDate.addDays(7*tempMinorScaleCount);
            }
            tempDate = myRealStart.date();
            while (tempDate.day() != 1)
                tempDate = tempDate.addDays(1);
            while (tempDate < myRealEnd.date()) {
                majorTicks.append( getCoordX(tempDate));
                tempStr = QDate::shortMonthName(tempDate.month())+ " "+getYear(tempDate);
                majorText.append(tempStr);
                tempDate = tempDate.addMonths(tempMajorScaleCount);
            }
            majorTicks.append( getCoordX(tempDate));
            break;
        case KDGanttView::Month:
            myRealEnd = myRealEnd.addMonths(minorItems*tempMinorScaleCount);
            for ( i = 0; i < minorItems;++i) {
                minorText.append((QDate::shortMonthName(tempDate.month())).left(1));
                tempDate = tempDate.addMonths(tempMinorScaleCount);
            }
            tempDate = myRealStart.date();
            while (tempDate.month() != 1)
                tempDate = tempDate.addMonths(1);
            while (tempDate < myRealEnd.date()) {
                majorTicks.append( getCoordX(tempDate));
                tempStr = getYear(tempDate);
                majorText.append(tempStr);
                tempDate = tempDate.addYears(tempMajorScaleCount);
            }
            majorTicks.append( getCoordX(tempDate));
            break;
        case KDGanttView::Auto:
            break;
        }

    if (flagDoNotRepaintAfterChange) {
      myGanttView->myTimeTable->setBlockUpdating( block );
      return;
    }
    myGanttView->myTimeTable->setBlockUpdating( block );
    updateTimeTable();
    centerDateTime(myCenterDateTime);
    repaint();
}
void KDTimeHeaderWidget::saveCenterDateTime()
{
    double wid = width();
    double allsecs = secsFromTo( myRealStart, myRealEnd );
    double center = myGanttView->myCanvasView->viewport()->width();
    center = center / 2;
    center = center + myGanttView->myCanvasView->contentsX();
    double secs = (allsecs*center)/wid;
    double days = secs/86400.0;
    secs = secs - ( (int) days *86400.0 );
    myCenterDateTime =  (myRealStart.addDays ( (int) days )).addSecs( (int) secs);
}

void KDTimeHeaderWidget::centerDateTime( const QDateTime& center )
{
    moveTimeLineTo(getCoordX( center )-(myGanttView->myCanvasView->viewport()->width() /2));
    //  qDebug("centerDateTime %s %d %d", center.toString().latin1(),getCoordX( center ),(myGanttView->myCanvasView->viewport()->width() /2) );
}



void KDTimeHeaderWidget::paintEvent(QPaintEvent *p)
{
    repaintMe(p->rect().x(),p->rect().width());
}


void KDTimeHeaderWidget::mousePressEvent ( QMouseEvent * e)
{
    switch ( e->button() ) {
    case LeftButton:
        break;
    case RightButton:
        if (flagShowPopupMenu)
            myPopupMenu->popup(e->globalPos());
        break;
    case MidButton:
        break;
    default:
        break;
    }
}


int KDTimeHeaderWidget::getCoordX(QDate date)
{
    int wid = width();
    int daysAll = myRealStart.daysTo(myRealEnd);
    if (daysAll == 0) return 0;
    int days = myRealStart.daysTo(QDateTime(date));
    return (wid *days) /daysAll;
}


int KDTimeHeaderWidget::getCoordX(QDateTime datetime)
{
    double wid = width();
    double secsAll = secsFromTo( myRealStart, myRealEnd );
    if (secsAll == 0.0) return 0;
    double secs = secsFromTo( myRealStart, datetime);
    return ((int)((wid *(secs /secsAll))+0.5));
}


QString KDTimeHeaderWidget::getYear(QDate date)
{
    QString ret;
    ret.setNum(date.year());
    switch (yearFormat()) {
    case KDGanttView::FourDigit:
        // nothing to do
        break;
    case KDGanttView::TwoDigit:
        ret = ret.right(2);
        break;
    case KDGanttView::TwoDigitApostrophe:
        ret = "'"+ret.right(2);

        break;
    }
    return ret;
}


QString KDTimeHeaderWidget::getHour(QTime time)
{
    QString ret;
    int hour = time.hour();
    if (myHourFormat == KDGanttView::Hour_12) {
        if (hour >= 12) {
            if (hour > 12) hour -=12;
            ret.setNum(hour);
            ret = ret +" PM";
        } else {
            if (hour == 0) hour = 12;
            ret.setNum(hour);
            ret = ret +" AM";
        }
    } else
        ret.setNum(hour);
    return ret;
}

/* ***************************************************************
   KDLegendWidget:: KDLegendWidget
   ***************************************************************** */
KDLegendWidget:: KDLegendWidget( QWidget* parent, KDMinimizeSplitter* legendParent ):KDSemiSizingControl ( KDSemiSizingControl::Before, Vertical, parent)
{
    scroll = new QScrollView( legendParent );
    setMaximizedWidget( scroll );
    setMinimizedWidget( myLabel = new QLabel(" Legend is hidden",this)  );
    setGeometry( 0, 0, 50, 50 );
    myLegend = 0;
    clearLegend();
    showMe ( false );
}
void KDLegendWidget::setFont( QFont font)
{
    myLegend->setFont( font);
    myLabel->setFont( font);
    QWidget::setFont( font );
}
void KDLegendWidget::drawToPainter( QPainter *p )
{
    p->drawPixmap( 0, 0, QPixmap::grabWidget( myLegend ) );
}
QSize KDLegendWidget::legendSize()
{
    return myLegend->size();
}
QSize KDLegendWidget::legendSizeHint()
{
    return QSize( myLegend->sizeHint().width(), myLegend->sizeHint().height()+scroll->horizontalScrollBar()->height());
}
void KDLegendWidget::showMe ( bool show )
{
    minimize( !show );
}
void KDLegendWidget::clearLegend ( )
{
    if ( myLegend ) delete myLegend;
    myLegend = new QGroupBox( 1, Qt::Horizontal, "Legend:", scroll->viewport() );
    myLegend->setBackgroundColor( Qt::white );
    myLegend->setFont( font() );
    scroll->addChild(  myLegend );
    scroll->setResizePolicy( QScrollView::AutoOneFit );
    myLegend->layout()->setMargin( KDialog::marginHint() );
    myLegend->setFrameStyle( QFrame::NoFrame );
    scroll->setMaximumHeight( legendSizeHint().height() );
}

void KDLegendWidget::addLegendItem( KDGanttViewItem::Shape shape, const QColor& shapeColor, const QString& text )
{
    QLabel * temp;
    QPixmap p = KDGanttView::getPixmap( shape,  shapeColor, Qt::white, 10);
    QWidget *w = new QWidget( myLegend );
    w->setBackgroundColor( Qt::white );
    QHBoxLayout *lay = new QHBoxLayout( w ,0, 6);
    temp = new QLabel ( w );
    lay->addWidget( temp, 0, Qt:: AlignRight);
    temp->setPixmap(p);
    temp = new QLabel ( text, w );
    temp->setBackgroundColor( Qt::white );
    lay->addWidget( temp, 0, Qt:: AlignLeft);
    lay->addStretch();
    scroll->setMaximumHeight(legendSizeHint().height());
}


bool KDLegendWidget::isShown ( )
{
    return !isMinimized();
}


KDListView::KDListView(QWidget* parent, KDGanttView* gantView):QListView (parent)
{
    myGantView = gantView;
    new KDListViewWhatsThis(viewport(),this);
    setRootIsDecorated( true );
    setAllColumnsShowFocus( true );
    addColumn( i18n( "Task Name" ) );
    setSorting( -1 );
    setVScrollBarMode (QScrollView::AlwaysOff );
    setHScrollBarMode (QScrollView::AlwaysOn );
    setColumnWidthMode ( 0,Maximum  );
    _calendarMode = false;
    // QObject::connect(this, SIGNAL (  pressed ( QListViewItem * )) , this, SLOT( dragItem( QListViewItem *))) ;
}


void  KDListView::dragItem( QListViewItem *  )
{
    // qDebug("drag ");
    // startDrag();
}
QString KDListView::getWhatsThisText(QPoint p)
{
  KDGanttViewItem* item = ( KDGanttViewItem* ) itemAt( p );
  if ( item )
    return item->whatsThisText();
  return "No item Found";

}

void  KDListView::setCalendarMode( bool mode )
{
  _calendarMode = mode;
  setRootIsDecorated ( ! mode );
}


void  KDListView::contentsMouseDoubleClickEvent ( QMouseEvent * e )
{
  if ( ! _calendarMode )
    QListView::contentsMouseDoubleClickEvent ( e );
  else {

      emit myGantView->lvItemDoubleClicked ( (KDGanttViewItem*) itemAt(e->pos() ) );
      emit myGantView->itemDoubleClicked ( (KDGanttViewItem*) itemAt(e->pos() ) );
    }

}


void  KDListView::drawToPainter ( QPainter * p )
{
    drawContentsOffset ( p, 0, 0, 0, 0, contentsWidth(), contentsHeight() );
}


void KDListView::resizeEvent(QResizeEvent *)
{
    triggerUpdate ();
}


KDCanvasText::KDCanvasText( KDTimeTableWidget* canvas,
                            void* parentItem,
                            int type ) :
    QCanvasText(canvas)
{
    myParentType = type;
    myParentItem = parentItem;
}


KDCanvasLine::KDCanvasLine( KDTimeTableWidget* canvas,
                            void* parentItem,
                            int type ) :
    QCanvasLine(canvas)
{
    myParentType = type;
    myParentItem = parentItem;
}


KDCanvasPolygonItem::KDCanvasPolygonItem( KDTimeTableWidget* canvas,
                                          void* parentItem,
                                          int type ) :
    QCanvasPolygonalItem( canvas )
{
    myParentType = type;
    myParentItem = parentItem;
}


KDCanvasPolygon::KDCanvasPolygon( KDTimeTableWidget* canvas,
                                  void* parentItem,
                                  int type ) :
    QCanvasPolygon( canvas )
{
    myParentType = type;
    myParentItem = parentItem;
}


KDCanvasEllipse::KDCanvasEllipse( KDTimeTableWidget* canvas,
                                  void* parentItem,
                                  int type ) :
    QCanvasEllipse( canvas )
{
    myParentType = type;
    myParentItem = parentItem;
}


KDCanvasRectangle::KDCanvasRectangle( KDTimeTableWidget* canvas,
                                      void* parentItem,
                                      int type ) :
    QCanvasRectangle( canvas )
{
    myParentType = type;
    myParentItem = parentItem;
}




KDGanttCanvasView::KDGanttCanvasView( KDGanttView* sender,QCanvas* canvas, QWidget* parent,  const char* name ) : QCanvasView ( canvas, parent, name )
{
    setHScrollBarMode (QScrollView::AlwaysOn );
    setVScrollBarMode( QScrollView::AlwaysOn );
    new KDCanvasToolTip(viewport(),this);
    mySignalSender =  sender;
    currentItem = 0;
    currentLink = 0;
    cuttedItem = 0;
    //set_Mouse_Tracking(true);
    new KDCanvasWhatsThis(viewport(),this);
    onItem = new QPopupMenu( this );
    QPopupMenu * newMenu = new QPopupMenu( this );
    QPopupMenu * onView = new QPopupMenu( this );
    onView->insertItem( i18n( "Summary" ), this,
                        SLOT ( newRootItem( int ) ), 0, 0 );
    onView->insertItem( i18n( "Event" ), this,
                        SLOT ( newRootItem( int ) ), 0, 1);
    onView->insertItem( i18n( "Task" ), this,
                        SLOT ( newRootItem( int ) ), 0, 2 );

    onItem->insertItem( i18n( "New Root" ), onView );
    newMenu->insertItem( i18n( "Summary" ),
                         this, SLOT ( newChildItem(  int) ), 0, 0 );
    newMenu->insertItem( i18n( "Event" ),
                         this, SLOT ( newChildItem( int ) ), 0, 1 );
    newMenu->insertItem( i18n( "Task" ),
                         this, SLOT ( newChildItem( int ) ), 0, 2 );

    onItem->insertItem( i18n( "New Child" ), newMenu );
    QPopupMenu * afterMenu = new QPopupMenu( this );
    afterMenu->insertItem( i18n( "Summary" ),
                           this, SLOT ( newChildItem(  int) ), 0, 0+4 );
    afterMenu->insertItem( i18n( "Event" ),
                           this, SLOT ( newChildItem( int ) ), 0, 1+4 );
    afterMenu->insertItem( i18n( "Task" ),
                           this, SLOT ( newChildItem( int ) ), 0, 2+4 );
    onItem->insertItem( i18n( "New After" ), afterMenu );
    QPopupMenu *pasteMenu = new QPopupMenu( this );
    pasteMenu->insertItem( i18n( "As Root" ),
                           this, SLOT ( pasteItem( int ) ), 0, 0 );
    pasteMenu->insertItem( i18n( "As Child" ),
                           this, SLOT ( pasteItem( int ) ), 0, 1 );
    pasteMenu->insertItem( i18n( "After" ),
                           this, SLOT ( pasteItem( int ) ), 0, 2 );
    onItem->insertItem( i18n( "Paste" ), pasteMenu, 3 );
    onItem->insertItem( i18n( "Cut Item" ), this, SLOT ( cutItem() ) );
    onItem->setItemEnabled( 3, false );
    myMyContentsHeight = 0;
    _showItemAddPopupMenu = false;

}
KDGanttCanvasView::~KDGanttCanvasView()
{

}
void KDGanttCanvasView::setShowPopupMenu( bool show )
{
  _showItemAddPopupMenu = show;
}
bool KDGanttCanvasView::showPopupMenu()
{
  return _showItemAddPopupMenu;
}


void KDGanttCanvasView::moveMyContent( int, int y)
{
    setContentsPos(contentsX(), y);
}

void KDGanttCanvasView::resizeEvent ( QResizeEvent * e )
{
  int ho = e->oldSize().height();
  int wo = e->oldSize().width();
  int hi = height();
  int wi = width();
  QScrollView::resizeEvent ( e ) ;
  //verticalScrollBar()->setUpdatesEnabled( true);
  if ( ho != hi )
    emit heightResized( viewport()->height());
  if ( wo != wi )
    emit widthResized( viewport()->width() );
  setMyContentsHeight( 0 );
}
void KDGanttCanvasView::setMyContentsHeight( int hei )
{
  qApp->processEvents();
  //qDebug("setMyContentsHeight %d %d ", hei,  myMyContentsHeight);
  if ( hei > 0 )
    myMyContentsHeight = hei;
  if ( viewport()->height() <= myMyContentsHeight )
    verticalScrollBar()->setRange( 0, myMyContentsHeight- viewport()->height()+1);
  else
    verticalScrollBar()->setRange( 0,0 );
  // testing for unmatching ScrollBar values of timeheader and timetable
  // may happen after external resizing
  if ( horizontalScrollBar()->value() != mySignalSender->myTimeHeaderScroll->horizontalScrollBar()->value() ) {
    // I am the Boss!
    mySignalSender->myTimeHeaderScroll->horizontalScrollBar()->setValue(horizontalScrollBar()->value()  );

  }

}

void  KDGanttCanvasView::cutItem()
{
    lastClickedItem->hideSubtree();
    if ( lastClickedItem->parent() )
        lastClickedItem->parent()->takeItem(lastClickedItem);
    else
        mySignalSender->myListView->takeItem( lastClickedItem );
    mySignalSender->myTimeTable->updateMyContent();
    if ( cuttedItem )
        delete cuttedItem;
    cuttedItem = lastClickedItem;
    onItem->setItemEnabled( 3, true );

}
// called from the destructor in KDGanttViewItem or KDGanttView

void  KDGanttCanvasView::resetCutPaste( KDGanttViewItem* item )
{
    if ( item == 0 && cuttedItem ) {
        delete cuttedItem;
        cuttedItem = 0;
    }
    if (item == cuttedItem) {
        onItem->setItemEnabled( 3, false );
        cuttedItem = 0;
    }
}

void  KDGanttCanvasView::pasteItem( int type )
{

    switch( type ) {
    case 0://root
        mySignalSender->myListView->insertItem( cuttedItem );
        break;
    case 1://child
        lastClickedItem->insertItem( cuttedItem );
        break;
    case 2://after
        if ( lastClickedItem->parent() ) {
            lastClickedItem->parent()->insertItem( cuttedItem );
        }
        else
            mySignalSender->myListView->insertItem( cuttedItem );
        cuttedItem->moveItem( lastClickedItem );
        break;
    default:
        ;
    }
    cuttedItem = 0;
    onItem->setItemEnabled( 3, false );
    mySignalSender->myTimeTable->updateMyContent();
}
void  KDGanttCanvasView::newRootItem(int type)
{
    KDGanttViewItem* temp = 0;
    switch( type ) {
    case 1:
        temp = new KDGanttViewEventItem( mySignalSender, "new Event" );
        break;
    case 0:
        temp = new KDGanttViewSummaryItem( mySignalSender, "new Summary" );
        break;
    case 2:
        temp = new KDGanttViewTaskItem( mySignalSender, "new Task" );
        break;
    default:
        ;
    }
    if ( temp )
        mySignalSender->editItem( temp );
}

void  KDGanttCanvasView::newChildItem( int type )
{
    KDGanttViewItem* temp = 0;
    switch( type ) {
    case 1:
        temp = new KDGanttViewEventItem( lastClickedItem, "new Event" );
        break;
    case 0:
        temp = new KDGanttViewSummaryItem( lastClickedItem, "new Summary" );
        break;
    case 2:
        temp = new KDGanttViewTaskItem( lastClickedItem, "new Task" );
        break;
    case 5:
        if ( lastClickedItem->parent() )
            temp = new KDGanttViewEventItem( lastClickedItem->parent(), lastClickedItem, "new Event" );
        else
            temp = new KDGanttViewEventItem( mySignalSender, lastClickedItem, "new Event" );
        break;
    case 4:
        if ( lastClickedItem->parent() )
            temp = new KDGanttViewSummaryItem( lastClickedItem->parent(), lastClickedItem, "new Summary" );
        else
            temp = new KDGanttViewSummaryItem( mySignalSender, lastClickedItem, "new Summary" );
        break;
    case 6:
        if ( lastClickedItem->parent() )
            temp = new KDGanttViewTaskItem( lastClickedItem->parent(), lastClickedItem, "new Task" );
        else
            temp = new KDGanttViewTaskItem( mySignalSender, lastClickedItem, "new Task" );
        break;


    default:
        ;
    }
    if ( temp )
        mySignalSender->editItem( temp );
}

void  KDGanttCanvasView::drawToPainter ( QPainter * p )
{
    drawContents ( p, 0, 0, canvas()->width(), canvas()->height() );
}
QString  KDGanttCanvasView::getToolTipText(QPoint p)
{
    QCanvasItemList il = canvas()->collisions ( viewportToContents( p ));
    QCanvasItemList::Iterator it;
    for ( it = il.begin(); it != il.end(); ++it ) {
        switch (getType(*it)) {
        case Type_is_KDGanttViewItem:
            return (getItem(*it))->tooltipText();
            break;
        case Type_is_KDGanttTaskLink:
            return (getLink(*it))->tooltipText();
            break;
        default:
            break;
        }
    }
    return "";
}

QString  KDGanttCanvasView::getWhatsThisText(QPoint p)
{
    QCanvasItemList il = canvas() ->collisions (viewportToContents( p ));
    QCanvasItemList::Iterator it;
    for ( it = il.begin(); it != il.end(); ++it ) {
        switch (getType(*it)) {
        case Type_is_KDGanttViewItem:
            return (getItem(*it))->whatsThisText();
            break;
        case Type_is_KDGanttTaskLink:
            return (getLink(*it))->whatsThisText();
            break;
        default:
            break;
        }
    }
    return "";
}


/**
   Handles the mouseevent if a mousekey is pressed

   \param e the mouseevent

*/

void KDGanttCanvasView::contentsMousePressEvent ( QMouseEvent * e )
{
    currentLink = 0;
    currentItem = 0;
    if (e->button() == RightButton && mySignalSender->editable()) {
        lastClickedItem = (KDGanttViewItem*) mySignalSender->myListView->itemAt( QPoint(2,e->pos().y()));
        if ( lastClickedItem ) {
	  if ( lastClickedItem->displaySubitemsAsGroup() && ! lastClickedItem->isOpen() ) {
	    // findSub subitem
	    QCanvasItemList il = canvas() ->collisions ( e->pos() );
	    QCanvasItemList::Iterator it;
	    for ( it = il.begin(); it != il.end(); ++it ) {
	      if ( getType(*it) == Type_is_KDGanttViewItem ) {
		lastClickedItem = getItem(*it);
	      }
	    }
	  }
	  if ( _showItemAddPopupMenu  )
	  onItem->popup(e->globalPos());
	}
    }
    QCanvasItemList il = canvas() ->collisions ( e->pos() );
    QCanvasItemList::Iterator it;
    for ( it = il.begin(); it != il.end(); ++it ) {
        switch ( e->button() ) {
        case LeftButton:
            switch (getType(*it)) {
            case Type_is_KDGanttViewItem:
                currentItem = getItem(*it);
		if (! currentItem->enabled() )
		  currentItem = 0;
                break;
            case Type_is_KDGanttTaskLink:
                currentLink = getLink(*it);
                break;
            default:
                break;
            }
            break;
        case RightButton:
            switch (getType(*it)) {
            case Type_is_KDGanttViewItem:
                currentItem = getItem(*it);
		if (! currentItem->enabled() )
		  currentItem = 0;
                break;
            case Type_is_KDGanttTaskLink:
                currentLink = getLink(*it);
                break;
            }
            break;
        case MidButton:
            switch (getType(*it)) {
            case Type_is_KDGanttViewItem:
                currentItem = getItem(*it);
		if (! currentItem->enabled() )
		  currentItem = 0;
                break;
            case Type_is_KDGanttTaskLink:
                currentLink = getLink(*it);
                break;
            }
            break;
	default:
            break;
        }
    }
    mySignalSender->gvContextMenuRequested( currentItem, e->globalPos() );
}
/**
   Handles the mouseevent if a mousekey is released

   \param e the mouseevent

*/

void KDGanttCanvasView::contentsMouseReleaseEvent ( QMouseEvent * e )
{
  static KDGanttViewItem* lastClicked = 0;
  mySignalSender->gvMouseButtonClicked( e->button(), currentItem ,  e->pos() );

  //qDebug("mouseup ");
  // if ( currentLink || currentItem )
      {
        switch ( e->button() ) {
        case LeftButton:
	   {
	    mySignalSender->itemLeftClicked( currentItem );
	    mySignalSender->gvItemLeftClicked( currentItem );
	  }
	  if ( currentLink )
	    mySignalSender->taskLinkLeftClicked( currentLink );
	  break;
        case RightButton:
	   {
	    mySignalSender->itemRightClicked( currentItem );
	    mySignalSender->gvItemRightClicked( currentItem );

	  }
	  if ( currentLink )
	    mySignalSender->taskLinkRightClicked( currentLink );
	  break;
        case MidButton:
	   {
	    mySignalSender->itemMidClicked( currentItem );
	    mySignalSender->gvItemMidClicked( currentItem );
	  }
	  if ( currentLink )
	    mySignalSender->taskLinkRightClicked( currentLink );
	  break;
        default:
            break;
        }
      }
      if ( lastClicked != currentItem )
	mySignalSender->gvCurrentChanged( currentItem );
      lastClicked = currentItem;
    currentLink = 0;
    currentItem = 0;
}
/**
   Handles the mouseevent if a mousekey is doubleclicked

   \param e the mouseevent

*/

void KDGanttCanvasView::contentsMouseDoubleClickEvent ( QMouseEvent * e )
{
    QCanvasItemList il = canvas() ->collisions ( e->pos() );
    QCanvasItemList::Iterator it;
    for ( it = il.begin(); it != il.end(); ++it ) {
        switch ( e->button() ) {
        case LeftButton:
            switch (getType(*it)) {
            case Type_is_KDGanttViewItem:
	      if ( getItem(*it)->enabled() )
                mySignalSender->itemDoubleClicked(getItem(*it));
                mySignalSender->gvItemDoubleClicked(getItem(*it));
                return;
                break;
            case Type_is_KDGanttTaskLink:
                mySignalSender->taskLinkDoubleClicked(getLink(*it));
                return;
                break;
            default:
                break;
            }
            break;
	    /*
              case RightButton:
              switch (getType(*it)) {
              case Type_is_KDGanttViewItem:
              mySignalSender->itemRightClicked(getItem(*it));
              return;
              break;
              case Type_is_KDGanttTaskLink:
              mySignalSender->taskLinkRightClicked(getLink(*it));
              return;
              break;
              }
              break;
              case MidButton:
              switch (getType(*it)) {
              case Type_is_KDGanttViewItem:
              mySignalSender->itemMidClicked(getItem(*it));
              return;
              break;
              case Type_is_KDGanttTaskLink:
              mySignalSender->taskLinkMidClicked(getLink(*it));
              return;
              break;
              }
              break;
	    */
	default:
            break;
        }
    }
}
/**
   Handles the mouseevent if a mousekey is pressed an the mouse is moved

   \param e the mouseevent

*/

void KDGanttCanvasView::contentsMouseMoveEvent ( QMouseEvent *  )
{
  //qDebug("mousemove! ");
    static int moves = 0;
    if ( (currentLink || currentItem) && (moves < 3) ) {
        ++moves;

    } else {
        moves = 0;
        currentLink = 0;
        currentItem = 0;
    }
    // no action implemented
    //qDebug("mousemove ");
    //QToolTip::setGloballyEnabled (false);
    //QToolTip::remove(viewport());
    // QToolTip::add(viewport(), "hello");
    // QToolTip::setGloballyEnabled (true);
    /*
      QCanvasItemList il = canvas() ->collisions ( e->pos() );

      QCanvasItemList::Iterator it;
      KDGanttItem* mouseover = 0;
      for ( it = il.begin(); it != il.end(); ++it ) {

      }

    */
}
void KDGanttCanvasView::viewportPaintEvent ( QPaintEvent * pe )
{
 QCanvasView::viewportPaintEvent ( pe );
}
void KDGanttCanvasView::set_Mouse_Tracking(bool on)
{
    viewport()->setMouseTracking(on);
}
int  KDGanttCanvasView::getType(QCanvasItem* it)
{
    switch (it->rtti()) {
    case QCanvasItem::Rtti_Line: return ((KDCanvasLine*)it)->myParentType;
    case QCanvasItem::Rtti_Ellipse: return ((KDCanvasEllipse *)it)->myParentType;
    case QCanvasItem::Rtti_Text: return ((KDCanvasText *)it)->myParentType;
    case QCanvasItem::Rtti_Polygon: return ((KDCanvasPolygon *)it)->myParentType;
    case QCanvasItem::Rtti_Rectangle: return ((KDCanvasRectangle *)it)->myParentType;
    }
    return -1;
}
KDGanttViewItem*  KDGanttCanvasView::getItem(QCanvasItem* it)
{
    switch (it->rtti()) {
    case QCanvasItem::Rtti_Line: return (KDGanttViewItem*)  ((KDCanvasLine*)it)->myParentItem;
    case QCanvasItem::Rtti_Ellipse: return (KDGanttViewItem*)  ((KDCanvasEllipse *)it)->myParentItem;
    case QCanvasItem::Rtti_Text: return (KDGanttViewItem*) ((KDCanvasText *)it)->myParentItem;
    case QCanvasItem::Rtti_Polygon: return (KDGanttViewItem*) ((KDCanvasPolygon *)it)->myParentItem;
    case QCanvasItem::Rtti_Rectangle: return (KDGanttViewItem*) ((KDCanvasRectangle *)it)->myParentItem;

    }
    return 0;
}
KDGanttViewTaskLink*  KDGanttCanvasView::getLink(QCanvasItem* it)
{
    switch (it->rtti()) {
    case QCanvasItem::Rtti_Line: return (KDGanttViewTaskLink*)  ((KDCanvasLine*)it)->myParentItem;
    case QCanvasItem::Rtti_Ellipse: return (KDGanttViewTaskLink*)  ((KDCanvasEllipse *)it)->myParentItem;
    case QCanvasItem::Rtti_Text: return (KDGanttViewTaskLink*) ((KDCanvasText *)it)->myParentItem;
    case QCanvasItem::Rtti_Polygon: return (KDGanttViewTaskLink*) ((KDCanvasPolygon *)it)->myParentItem;
    }
    return 0;
}

