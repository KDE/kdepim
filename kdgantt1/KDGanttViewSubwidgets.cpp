/* -*- Mode: C++ -*-
   $Id$
   KDGantt - a multi-platform charting engine
*/

/****************************************************************************
 ** Copyright (C)  2002-2004 Klarälvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KDGantt library.
 **
 ** This file may be used under the terms of the GNU General Public
 ** License versions 2.0 or 3.0 as published by the Free Software
 ** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
 ** included in the packaging of this file.  Alternatively you may (at
 ** your option) use any later version of the GNU General Public
 ** License if such license has been publicly approved by
 ** Klarälvdalens Datakonsult AB (or its successors, if any).
 **
 ** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
 ** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE. Klarälvdalens Datakonsult AB reserves all rights
 ** not expressly granted herein.
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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

#include <QLabel>
#include <QPainter>
#include <QRect>

#include <QApplication>
#include <qdrawutil.h>
#include <QPalette>
#include <QMouseEvent>

#include <klocale.h>
#include <kselectaction.h>
#include <kactioncollection.h>

KDTimeTableWidget:: KDTimeTableWidget( QWidget* parent,KDGanttView* myGantt):QCanvas (parent)
{
    mMinimumHeight = 0;
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
    retune(256);
    resize(1600,1200);
    myTaskLinkList.setAutoDelete( false );
    mUpdateTimer = new QTimer ( this );
    mUpdateTimer->setSingleShot( true );
    connect( mUpdateTimer, SIGNAL (  timeout() ) , this, SLOT( updateSlot() )) ;
    mSimpleUpdateTimer = new QTimer ( this );
    mSimpleUpdateTimer->setSingleShot( true );
    connect( mSimpleUpdateTimer, SIGNAL (  timeout() ) , this, SLOT( simpleUpdateSlot() )) ;
}
KDTimeTableWidget::~KDTimeTableWidget()
{

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
    //qApp->processEvents();
    updateMyContent();
}
void KDTimeTableWidget::collapseItem( QListViewItem * item)
{
    item->invalidateHeight () ;
    //qApp->processEvents();
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
    //qDebug("COMPUTED HEI %d ", hei);
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


    if ( myGanttView->showTicks() == KDGanttView::ShowMinorTicks ){//minor
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

                                itcol.current()->setPen( Qt::NoPen );
                                itcol.current()->setBrush( QBrush( colcol, Qt::SolidPattern) );
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
                                temprect->setPen( Qt::NoPen );
                                temprect->setBrush( QBrush( colcol, Qt::SolidPattern) );
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
                            itcol.current()->setPen( Qt::NoPen );
                            itcol.current()->setBrush( QBrush( colcol, Qt::SolidPattern) );
                            itcol.current()->setSize(cw ,h );
                            itcol.current()->move( i, 0 );
                            itcol.current()->show();
                            ++itcol;
                        } else {
                            temprect = new KDCanvasRectangle(this,0,Type_is_KDGanttGridItem);
                            temprect->setPen( Qt::NoPen );
                            temprect->setBrush( QBrush( colcol, Qt::SolidPattern) );
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
        if ( myGanttView->showTicks() == KDGanttView::ShowMinorTicks ||
             myGanttView->showTicks() == KDGanttView::ShowMajorTicks ) {
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
                                        itcol.current()->setPen( Qt::NoPen );
                                        itcol.current()->setBrush( QBrush( colcol, Qt::SolidPattern) );
                                        itcol.current()->setSize(mid ,h );
                                        itcol.current()->move( left, 0 );
                                        itcol.current()->show();
                                        ++itcol;
                                    } else {
                                        temprect = new KDCanvasRectangle(this,0,Type_is_KDGanttGridItem);
                                        temprect->setPen( Qt::NoPen );
                                        temprect->setBrush( QBrush( colcol, Qt::SolidPattern) );
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
                                    itcol.current()->setPen( Qt::NoPen );
                                    itcol.current()->setBrush( QBrush( colcol, Qt::SolidPattern) );
                                    itcol.current()->setSize(mid ,h );
                                    itcol.current()->move( left, 0 );
                                    itcol.current()->show();
                                    ++itcol;
                                } else {
                                    temprect = new KDCanvasRectangle(this,0,Type_is_KDGanttGridItem);
                                    temprect->setPen( Qt::NoPen );
                                    temprect->setBrush( QBrush( colcol, Qt::SolidPattern) );
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
        //QString ts = "asGroup";
        //if (!temp->displaySubitemsAsGroup() )
        //	ts = " NOT asGroup";
        //qDebug("temp name %s %s", temp->listViewText(0).toLatin1(), ts.toLatin1());

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
        if ( temp->isVisible() ) {
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

            } else {
                ;
            }
        }
        temp = temp->itemBelow ();
    }
    while ( ithordense.current() ) {
        if ( ithordense.current()->isVisible() ) {
            ithordense.current()->hide();
        }
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
        bool noInfoBeforeAndAfter = temp->showNoInformationBeforeAndAfter();
        if ( temp->showNoInformation() || noInfoBeforeAndAfter  ) {
            if ( itnoinfo.current() ) {
                noInfoLine = itnoinfo.current();
                ++itnoinfo;
            } else {
                noInfoLine =new KDCanvasRectangle(this,0,Type_is_KDGanttGridItem);
                showNoInfoList.append( noInfoLine );
                noInfoLine->setZ(-1);
            }
            noInfoLine->move( 0, temp->itemPos() );
            int width = noInfoBeforeAndAfter ?
                myGanttView->myTimeHeader->getCoordX( temp->startTime() ) : wid;
            noInfoLine->setSize( width, temp->height());
            noInfoLine->setPen( QPen(  Qt::NoPen ) );
            noInfoLine->setBrush( noInfoLineBrush);
            noInfoLine->show();
            if ( noInfoBeforeAndAfter ) {
                if ( itnoinfo.current() ) {
                    noInfoLine = itnoinfo.current();
                    ++itnoinfo;
                } else {
                    noInfoLine =new KDCanvasRectangle(this,0,Type_is_KDGanttGridItem);
                    showNoInfoList.append( noInfoLine );
                    noInfoLine->setZ(-1);
                }

                int startX = myGanttView->myTimeHeader->getCoordX( temp->endTime() );
                noInfoLine->move( startX, temp->itemPos() );
                int width = wid - startX;
                noInfoLine->setSize( width, temp->height());
                noInfoLine->setPen( QPen(  Qt::NoPen ) );
                noInfoLine->setBrush( noInfoLineBrush);
                noInfoLine->show();
            }
        }
        temp = temp->itemBelow ();
    }
    while ( itnoinfo.current() ) {
        itnoinfo.current()->hide();
        ++itnoinfo;
    }

}
void KDTimeTableWidget::computeTaskLinksForItem( KDGanttViewItem * item )
{
  QPtrListIterator<KDGanttViewTaskLink> it((myTaskLinkList));
    for ( ; it.current(); ++it ) {
        if ( it.current()->isFromToItem( item ) ) {
            if (it.current()->isVisible())
                it.current()->showMe(true);
            else
                it.current()->showMe(false);
        }
    }
}
void KDTimeTableWidget::computeTaskLinks()
{
    //compute and show tasklinks
    QPtrListIterator<KDGanttViewTaskLink> it((myTaskLinkList));
    for ( ; it.current(); ++it ) {
        if (it.current()->isVisible())
            it.current()->showMe(true);
        else
            it.current()->showMe(false);
    }
}

// updateMyContent() can be blocked by blockUpdating( true ) or inc_blockUpdating()
// updateMyContent() is blocked, if the GanttView is hidden after startup

int KDTimeTableWidget::minimumHeight()
{
    return mMinimumHeight;
}
void KDTimeTableWidget::updateSlot()
{
    //qDebug("PERFORMING UPDATE ");
    computeTaskLinks();
    computeHorizontalGrid();
    computeDenseLines();
    computeShowNoInformation();
    update();
}

void KDTimeTableWidget::simpleUpdateSlot()
{
    update();
}
void KDTimeTableWidget::forceUpdate()
{
    mUpdateTimer->stop();
    mSimpleUpdateTimer->stop();
    updateSlot();
}
void KDTimeTableWidget::simpleUpdate()
{
    mSimpleUpdateTimer->start( 0 );
}
void KDTimeTableWidget::updateMyContent()
{
    if ( flag_blockUpdating || int_blockUpdating ) {
        //qDebug("KDTimeTableWidget::updateMyContent() blocked! ");
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
    mMinimumHeight = hei;
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
    //setAllChanged();
    //qDebug("START TIMER ");
    mUpdateTimer->start( 0 );
    if (pendingWidth && pendingHeight ) {
        resize( pendingWidth, pendingHeight );
        pendingWidth = 0;
        pendingHeight = 0;
        emit heightComputed( 0 );

    }
    pendingWidth = 0;
    pendingHeight = 0;
    //qDebug("KDTimeTableWidget::updateMyContent() -------------------------");
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

int KDTimeTableWidget::getCoordX( QDateTime dt ) {
    return myGanttView->myTimeHeader->getCoordX(dt);
}

/* ***************************************************************
   KDTimeHeaderWidget:: KDTimeHeaderWidget
   ***************************************************************** */
KDTimeHeaderWidget:: KDTimeHeaderWidget( QWidget* parent,KDGanttView* gant ):QWidget ( parent, Qt::WNoAutoErase )
{
    setObjectName( "KDTimeHeaderWidget" );
    paintPix = QPixmap( 1280,100 );
    myToolTip = new KDTimeHeaderToolTip(this,this);
    mWeekStartsMonday = false;
    mWeekScaleShowNumber = false;
    myMajorScaleCount = 0;
    mMaxWidFormatMonth = 0;
    mMaxWidFormatWeek = 0;
    mMaxWidFormatDay = 0;
    mMaxWidtimeFormatHour = 0;
    mMaxWidtimeFormatMinute = 0;
    mMaxWidtimeFormatSecond = 0;
    mySizeHint = 0;
    myGanttView = gant;
    flagDoNotRecomputeAfterChange = true;
    QDateTime start = (QDateTime::currentDateTime ()).addSecs(-3600);
    setHorizonStart(start);
    setHorizonEnd( start.addSecs(3600*2));
    flagStartTimeSet = false;
    flagEndTimeSet = false;
    myCenterDateTime = QDateTime::currentDateTime ();
    //setScale(KDGanttView::Auto);
    setScale(KDGanttView::Hour);
    myMaxScale = KDGanttView::Month;
    myMinScale = KDGanttView::Second;
    myAutoScaleMinorTickcount = 100;
    setMajorScaleCount( 1 );
    setMinorScaleCount( 1);
    setMinimumColumnWidth( 5 );
    setYearFormat( KDGanttView::FourDigit );
    setHourFormat( KDGanttView::Hour_12 );
    myZoomFactor = 1.0;
    setWeekendBackgroundColor(QColor(220,220,220) );
    setWeekendDays( 6, 7 );
    myGridMinorWidth = 0;

    actionCollection = new KActionCollection( this );
    myPopupMenu = new QMenu(this);

    QMenu *gotoAction = myPopupMenu->addMenu( tr("Go to start of") );
    gotoAction->addAction( tr("Today"), this, SLOT(centerToday()) );
    gotoAction->addAction( tr("Yesterday"), this, SLOT(centerYesterday()) );
    gotoAction->addAction( tr("Current Week"), this, SLOT(centerCurrentWeek()) );
    gotoAction->addAction( tr("Last Week"), this, SLOT(centerLastWeek()) );
    gotoAction->addAction( tr("Current Month"), this, SLOT(centerCurrentMonth()) );
    gotoAction->addAction( tr("Last Month"), this, SLOT(centerLastMonth()) );
    gotoAction->addAction( tr("Current Year"), this, SLOT(centerCurrentYear()) );
    gotoAction->addAction( tr("Last Year"), this, SLOT(centerLastYear()) );
    mGotoAction = gotoAction->menuAction();

    QMenu *timespanMenu = myPopupMenu->addMenu( tr("Show timespan of") );
    timespanMenu->addAction( tr("Today"), this, SLOT(showToday()) );
    timespanMenu->addAction( tr("Yesterday"), this, SLOT(showYesterday()) );
    timespanMenu->addAction( tr("Current Week"), this, SLOT(showCurrentWeek()) );
    timespanMenu->addAction( tr("Last Week"), this, SLOT(showLastWeek()) );
    timespanMenu->addAction( tr("Current Month"), this, SLOT(showCurrentMonth()) );
    timespanMenu->addAction( tr("Last Month"), this, SLOT(showLastMonth()) );
    timespanMenu->addAction( tr("Current Year"), this, SLOT(showCurrentYear()) );
    timespanMenu->addAction( tr("Last Year"), this, SLOT(showLastYear()) );
    mTimespanAction = timespanMenu->menuAction();

    QMenu *zoomMenu = myPopupMenu->addMenu( tr("&Zoom") );
    zoomMenu->addAction( tr("Zoom to 100%"),this, SLOT(zoom1()) );
    zoomMenu->addAction( tr("Zoom to fit"),myGanttView, SLOT(zoomToFit()) );
    zoomMenu->addAction( tr("Zoom in (x 2)"),this, SLOT(zoom2()) );
    zoomMenu->addAction( tr("Zoom in (x 6)"),this, SLOT(zoom6()) );
    zoomMenu->addAction( tr("Zoom in (x 12)"),this, SLOT(zoom12()) );
    zoomMenu->addAction( tr("Zoom out (x 1/2)"),this, SLOT(zoomOut2()) );
    zoomMenu->addAction( tr("Zoom out (x 1/6)"),this, SLOT(zoomOut6()) );
    zoomMenu->addAction( tr("Zoom out (x 1/12)"),this, SLOT(zoomOut12()) );
    mZoomAction = zoomMenu->menuAction();

    mScaleAction  = new KSelectAction(tr("Scale"), this);
    actionCollection->addAction("Scale Action", mScaleAction );
    QStringList scaleItems;
    scaleItems << tr("Second") << tr("Minute") << tr("Hour") << tr("Day")
               << tr("Week") << tr("Month") << tr("Auto");
    mScaleAction->setItems( scaleItems );
    connect( mScaleAction, SIGNAL( triggered(int) ), this, SLOT( setScale(int) ) );
    myPopupMenu->addAction( mScaleAction );

    mTimeFormatAction  = new KSelectAction(tr("Time Format"), this);
    actionCollection->addAction("Time Format", mTimeFormatAction );
    QStringList timeFormatItems;
    timeFormatItems << tr("24 Hour") << tr("12 PM Hour") << tr("24:00 Hour");
    mTimeFormatAction->setItems( timeFormatItems );
    connect( mTimeFormatAction, SIGNAL( triggered(int) ), this, SLOT( setHourFormat(int) ) );
    myPopupMenu->addAction( mTimeFormatAction );


    mYearFormatAction  = new KSelectAction(tr("Year Format"), this);
    actionCollection->addAction("Year Format", mYearFormatAction );
    QStringList yearFormatItems;
    yearFormatItems << tr("Four Digit") << tr("Two Digit")
                    << tr("Two Digit Apostrophe") << tr("No Date on Minute/Hour Scale");
    mYearFormatAction->setItems( yearFormatItems );
    connect( mYearFormatAction, SIGNAL( triggered(int) ), this, SLOT( setYearFormat(int) ) );
    myPopupMenu->addAction( mYearFormatAction );


    mGridAction  = new KSelectAction(tr("Grid"), this);
    actionCollection->addAction("Grid", mGridAction );
    QStringList gridItems;
    gridItems << tr("Show minor grid") << tr("Show major grid") << tr("Show no grid");
    mGridAction->setItems( gridItems );
    connect( mGridAction, SIGNAL( triggered(int) ), this, SLOT( gridSettings(int) ) );

    mPrintAction = myPopupMenu->addAction( tr("Print"), myGanttView, SLOT(print()) );
    connect(myPopupMenu, SIGNAL (  aboutToShow () ) , this, SLOT( preparePopupMenu() )) ;
    flagZoomToFit = false;
    setShowTicks( KDGanttView::ShowMinorTicks );
    myRealEnd =  myHorizonEnd;
    myRealStart = myHorizonStart;
    autoComputeTimeLine = true;
    flagDoNotRecomputeAfterChange = false;
    flagDoNotRepaintAfterChange = false;
    setShowPopupMenu(false,false,false,false,false,false,false);
    for (int j =1;j<8;++j)
        weekdayColor[j] = Qt::white;
    myMinimumWidth = 0;
    mouseDown = false;
    beginMouseDown = 0;
    endMouseDown = 0;
}

KDTimeHeaderWidget::~KDTimeHeaderWidget()
{
    delete myToolTip;
}
// FIXME_RK
void  KDTimeHeaderWidget::preparePopupMenu()
{
  mZoomAction->setVisible( flagShowZoom );

  mZoomAction->setText( tr("Zoom  ") + "(" +QString::number( zoomFactor(), 'f',3) +")" );

  mScaleAction->setVisible( flagShowScale );
  mScaleAction->setCurrentItem( (int)( scale() ) );

  mTimeFormatAction->setVisible( flagShowTime );
  mTimeFormatAction->setCurrentItem( (int)( hourFormat() ) );

  mYearFormatAction->setVisible( flagShowYear );
  mYearFormatAction->setCurrentItem( (int)( yearFormat() ) );

  mPrintAction->setVisible( flagShowPrint );

  mGridAction->setVisible( flagShowGrid );
  mGridAction->setCurrentItem( (KDGanttView::ShowTicksType)showTicks() );

}

void KDTimeHeaderWidget::setDateFormatMonth( const QString& fmt )
{
    mDateFormatMonth = fmt;
    mMaxWidFormatMonth = getMaxTextWidth( mDateFormatMonth , 3 );
    computeTicks();
}
const QString KDTimeHeaderWidget::dateFormatMonth()
{
    return mDateFormatMonth;
}


void KDTimeHeaderWidget::setDateFormatWeek( const QString& fmt )
{
    mDateFormatWeek = fmt;
    mMaxWidFormatWeek = getMaxTextWidth( mDateFormatWeek, 2 );
    computeTicks();

}
const QString KDTimeHeaderWidget::dateFormatWeek()
{
    return mDateFormatWeek;
}


void KDTimeHeaderWidget::setDateFormatDay( const QString& fmt )
{
    mDateFormatDay = fmt;
    mMaxWidFormatDay = getMaxTextWidth( mDateFormatDay, 1 );
    computeTicks();
}
const QString KDTimeHeaderWidget::dateFormatDay()
{
    return mDateFormatDay;
}


void KDTimeHeaderWidget::setDatetimeFormatHour( const QString& fmt )
{
    mDatetimeFormatHour = fmt;
    mMaxWidtimeFormatHour = getMaxTextWidth( mDatetimeFormatHour, 0 );
    computeTicks();

}
const QString KDTimeHeaderWidget::datetimeFormatHour()
{
    return mDatetimeFormatHour;
}
void KDTimeHeaderWidget::setDatetimeFormatSecond( const QString& fmt )
{
    mDatetimeFormatSecond = fmt;
    mMaxWidtimeFormatSecond = getMaxTextWidth( mDatetimeFormatSecond, 0 );
    computeTicks();
}
const QString KDTimeHeaderWidget::datetimeFormatSecond()
{
    return mDatetimeFormatSecond;
}
void KDTimeHeaderWidget::setDatetimeFormatMinute( const QString& fmt )
{
    mDatetimeFormatMinute = fmt;
    mMaxWidtimeFormatMinute = getMaxTextWidth( mDatetimeFormatMinute , 0 );
    computeTicks();
}
const QString KDTimeHeaderWidget::datetimeFormatMinute()
{
    return mDatetimeFormatMinute;
}
void KDTimeHeaderWidget::setWeekStartsMonday( bool b )
{
    mWeekStartsMonday = b;
    computeTicks();
}
bool KDTimeHeaderWidget::weekStartsMonday()
{
    return mWeekStartsMonday;
}
void KDTimeHeaderWidget::setWeekScaleShowNumber( bool b )
{
    mWeekScaleShowNumber = b;
    computeTicks();
}
bool KDTimeHeaderWidget::weekScaleShowNumber()
{
    return mWeekScaleShowNumber;
}

void KDTimeHeaderWidget::setTooltipDateTimeFormat( const QString& fmt )
{
    mTooltipDateFormat = fmt;
}
const QString KDTimeHeaderWidget::tooltipDateTimeFormat()
{
    return mTooltipDateFormat;
}

QString  KDTimeHeaderWidget::getToolTipText(QPoint p)
{
    if ( mTooltipDateFormat.isEmpty() )
        return getDateTimeForIndex( p.x()).toString();
    return getDateTimeForIndex( p.x()).toString( mTooltipDateFormat );
}
void KDTimeHeaderWidget::addTickRight( int num )
{
    bool block = myGanttView->myTimeTable->blockUpdating();
    myGanttView->myTimeTable->setBlockUpdating( true );
    setHorizonEnd( addMajorTickTime( getDateTimeForIndex(myGanttView->myCanvasView->contentsWidth() - 1), num ) );
    myGanttView->myTimeTable->setBlockUpdating( block );
    //myGanttView->myCanvasView->updateScrollBars();
    myGanttView->myTimeTable->updateMyContent();
    myGanttView->myCanvasView->horizontalScrollBar()->setValue( myGanttView->myCanvasView->horizontalScrollBar()->maximum() );
    moveTimeLineTo( myGanttView->myCanvasView->contentsWidth() - myGanttView->myCanvasView->viewport()->width()  );
    myGanttView->myTimeTable->forceUpdate();
}

void KDTimeHeaderWidget::addTickLeft( int num )
{
    bool block = myGanttView->myTimeTable->blockUpdating();
    myGanttView->myTimeTable->setBlockUpdating( true );
    setHorizonStart( addMajorTickTime( getDateTimeForIndex(0), -num ) );
    //myGanttView->myCanvasView->updateScrollBars();
    myGanttView->myCanvasView->horizontalScrollBar()->setValue( 0 );
    moveTimeLineTo( 0 );
    myGanttView->myTimeTable->setBlockUpdating( block );
    myGanttView->myTimeTable->updateMyContent();
    myGanttView->myTimeTable->forceUpdate();
}
// the time in secs of one Major grid tick
QDateTime KDTimeHeaderWidget::addMajorTickTime( const QDateTime& dt, int fac )
{
    // qDebug("KDTimeHeaderWidget::addMajorTickTime %d %d - %s  ", fac,myMajorScaleCount,  dt.toString().toLatin1() );
    fac *= myMajorScaleCount;
    switch (myRealScale)
        {
        case KDGanttView::Minute:
            fac *= 60;
            // fall through
        case KDGanttView::Second:
            return dt.addSecs( fac * 60 );
            break;
        case KDGanttView::Day:
            fac *= 7;
            // fall through
        case KDGanttView::Hour:
            return dt.addDays( fac );
            break;
        case KDGanttView::Week:
            return dt.addMonths( fac );
            break;
        case KDGanttView::Month:
            return dt.addYears( fac );
            break;
        case KDGanttView::Auto:
            break;
        }
    return dt;
}
// the time in secs of one minor grid tick
int KDTimeHeaderWidget::getTickTime()
{
    int ret = getDateTimeForIndex(0).secsTo(getDateTimeForIndex(myGridMinorWidth));
    //qDebug("TickTime %d", ret);
    return ret;
}


void KDTimeHeaderWidget::checkWidth( int wid )
{
    // we have to set the minimum width one pixel higher than the
    // viewport width of the canvas view in  order to
    // avoid that the horiz. scrollbar of the canvasview is hidden
    myMinimumWidth = wid + 1;
    if ( myMinimumWidth  > width() ||
         ( myMinimumWidth > mySizeHint &&
           myMinimumWidth < (width() - myGridMinorWidth  )) )
        computeTicks();

    // Update (horizontal) scrollbar,
    // We probably come from an external resize and then we must
    // calculate on basis of myCanvasView.
    // (NOTE: we have disconnected the auto QScrollView scrollbar update)
    if (myGanttView && myGanttView->myCanvasView)
        myGanttView->myCanvasView->updateScrollBars();

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
                if (item->isVisibleInGanttView) {
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
                if (item->isVisibleInGanttView) {
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


void KDTimeHeaderWidget::setShowPopupMenu( bool show,
                                           bool showZoom,
                                           bool showScale,
                                           bool showTime,
                                           bool showYear,
                                           bool showGrid,
                                           bool showPrint)
{    flagShowPopupMenu = show;
    flagShowZoom = showZoom;
    flagShowScale  = showScale;
    flagShowTime  = showTime;
    flagShowYear = showYear;
    flagShowGrid  = showGrid;
    flagShowPrint = showPrint;
}


bool KDTimeHeaderWidget::showPopupMenu() const
{
    return flagShowPopupMenu;
}
void KDTimeHeaderWidget::center(const QDate &dt)
{
  bool allowChange = myGanttView->userHorizonChangeEnabled();
  centerDateTime( QDateTime( dt ), allowChange );
}

void KDTimeHeaderWidget::zoomTo( Scale unit, const QDate &start, const QDate &end )
{
  bool allowChange = myGanttView->userHorizonChangeEnabled();
  setScale( unit, allowChange );
  zoomToSelectionAndSetStartEnd( QDateTime( start ), QDateTime( end ) );
}

void KDTimeHeaderWidget::gridSettings( int i )
{
  switch (i) {
    case 0:
        setShowTicks( KDGanttView::ShowMajorTicks );
        break;
    case 1:
        setShowTicks( KDGanttView::ShowMinorTicks );
        break;
    case 2:
        setShowTicks( KDGanttView::ShowNoTicks );
        break;
  }

}
void KDTimeHeaderWidget::setScale( int i )
{
  setScale( (KDGanttView::Scale)i );
}
void KDTimeHeaderWidget::setHourFormat( int i )
{
  setHourFormat( (KDGanttView::HourFormat)i );
}
void KDTimeHeaderWidget::setYearFormat( int i )
{
  setYearFormat( (KDGanttView::YearFormat)i );
}

void KDTimeHeaderWidget::zoomToFit()
{
    flagZoomToFit = true;
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

void KDTimeHeaderWidget::zoomToSelectionAndSetStartEnd( const QDateTime& start, const QDateTime &end)
{
    //qDebug("start %s end %s ",start.toString().toLatin1(), end.toString().toLatin1() );
    myHorizonStart = start;
    flagStartTimeSet = true;
    myHorizonEnd = end;
    flagEndTimeSet = true;
    performZoomToSelection( start, end);
    moveTimeLineTo((getCoordX(start)));
    updateTimeTable();
    pendingPaint();
}
void KDTimeHeaderWidget::performZoomToSelection( const QDateTime &start, const QDateTime &end)
{
    if (start < myHorizonStart) {
        myHorizonStart = start;
        flagStartTimeSet = true;
        //qDebug("myHorizonStart reset");
    }
    if (end > myHorizonEnd) {
        myHorizonEnd = end;
        flagEndTimeSet = true;
        //qDebug("myHorizonEnd reset ");
    }
    flagDoNotRepaintAfterChange = true;//avoid flicker
    double startFac = 1.0;
    int viewWid = myGanttView->myCanvasView->viewport()->width();
    int timeWid = 0;
    while ( timeWid == 0 && startFac < 100000.0) {
        zoom(startFac);
        timeWid =  getCoordX(end)-getCoordX(start);
        startFac *= 10.0;
    }
    if ( timeWid == 0 )
        timeWid = 1;
    double fac;
    fac  = ( (double)viewWid)/((double) timeWid  );
    fac *= ( startFac/10.0 );
    //qDebug("zoooom %d %d %f %f", viewWid, timeWid,fac, startFac / 10.0 );
    zoom (fac);
    timeWid = getCoordX(end)-getCoordX(start);
    int count = 0;
    int lastScaleCount = 0;
    int incCounter = 2;
    while (timeWid >viewWid || ( ( myRealMinorScaleCount != lastScaleCount)  && timeWid*incCounter < viewWid ) ) {
        ++incCounter;
        //qDebug("******************While****************** ");
        //qDebug("timeWid  %d    viewWid %d ", timeWid ,viewWid );
        //qDebug("myRealMinorScaleCount %d lastScaleCount %d",  myRealMinorScaleCount , lastScaleCount    );
        lastScaleCount = myRealMinorScaleCount;
        fac = (fac * (double)viewWid)/(double)timeWid;
        zoom (fac);
        timeWid =  getCoordX(end)-getCoordX(start);
        if ( count++ > 10 ) {
            //qDebug("Exiting while loop in zoomToSelection ");
            break;
        }
    }
    flagDoNotRepaintAfterChange = false;
}
void KDTimeHeaderWidget::zoomToSelection( const QDateTime &start, const QDateTime &end)
{
    performZoomToSelection( start, end );
    moveTimeLineTo((getCoordX(start)));
    updateTimeTable();
    pendingPaint();
}
void KDTimeHeaderWidget::moveTimeLineTo(int X)
{
    int Y = myGanttView->myCanvasView->contentsY ();
    myGanttView->myCanvasView->setContentsPos (X, Y );
}

void KDTimeHeaderWidget::zoom(double factor, bool absolute)
{
    if ( factor < 0.000001 ) {
        qDebug("KDGanttView::zoom() : Zoom factor too low. Nothing zoomed. ");
        return;
    }
    double newZoom = factor;
    if (!absolute)
        newZoom *= myZoomFactor;

    double relativeZoom = newZoom / myZoomFactor;

    //qDebug("zooming relative %f ", relativeZoom);
    //qDebug("zooming absolute %f ", newZoom);
    int viewWid = myGanttView->myCanvasView->viewport()->width();
    if ( width() * relativeZoom < viewWid && ( newZoom > 1.01 || newZoom < 0.99 ) ) {
        qDebug("KDGanttView::zoom() : Zoom factor too low for current horizon. ");
        // qDebug("zooming relative %f, zooming absolute %f, viewWidth %d width %d ", relativeZoom,  newZoom, viewWid, width() );
        return;
    }
    myZoomFactor = newZoom;
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
void KDTimeHeaderWidget::setScale(Scale unit, bool update  )
{
    myScale = unit;
    myZoomFactor = 1.0;
    if ( update )
        computeTicks();
    // Since we have disconnected autoupdate of scrollbars, we must do it ourselves
    if (myGanttView && myGanttView->myCanvasView)
        myGanttView->myCanvasView->updateHorScrollBar();
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
void KDTimeHeaderWidget::setMaximumScale( Scale unit )
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
  Specifies if and which ticks should be shown on the scale.

  \param ticks ShowTicksType parameter with values of ShowMajorTicks, ShowMinorTicks or ShowNoTicks
  \sa showTicks()
*/
void KDTimeHeaderWidget::setShowTicks( KDGanttView::ShowTicksType ticks )
{
  flagShowTicks = ticks;
  updateTimeTable();
}


/*!
  Returns whether ticks are shown on the major scale.

  \return the type of ticks to show (wither ShowMajorTicks, ShowMinorTicks or ShowNoTicks
  \sa setShowTicks()
*/
KDGanttView::ShowTicksType KDTimeHeaderWidget::showTicks() const
{
    return flagShowTicks;
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
    IntervalColorList::const_iterator it;
    for ( it = icList.begin(); it != icList.end(); ++it ) {
       (*it)->layout( this, height );
    }
}

void KDTimeHeaderWidget::addIntervalBackgroundColor( KDIntervalColorRectangle* newItem )
{
    icList.append(newItem);
    updateTimeTable();
}

#if 0
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
                                                     int priority,
                                                     Scale mini ,
                                                     Scale maxi )
{

    if ( priority < 0 )
        priority = 0;
    if ( priority > 10 )
        priority = 10;

    IntervalColorList::iterator it;
    for ( it = icList.begin(); it != icList.end(); ++it ) {
        if ((*it).datetime == start && (*it).end == end ) {
            (*it).color = color;
            (*it).minScaleView = mini;
            (*it).maxScaleView = maxi;
            (*it).canvasRect->setZ(-19 + priority );
            (*it).priority =  priority;
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
    newItem.priority =  priority;
    newItem.color = color;
    newItem.minScaleView = mini;
    newItem.maxScaleView = maxi;
    //newItem.canvasLine = new KDCanvasLine(myGanttView->myTimeTable,0,Type_is_KDGanttGridItem);
    newItem.canvasRect = new KDCanvasRectangle(myGanttView->myTimeTable,0,Type_is_KDGanttGridItem);
    newItem.canvasRect->setZ(-19 + priority );
    icList.append(newItem);
    updateTimeTable();
}
#endif

void KDTimeHeaderWidget::clearBackgroundColor()
{

    IntervalColorList::iterator itic;
    for ( itic = icList.begin(); itic != icList.end(); ++itic ) {
        delete  (*itic);
    }
    ccList.clear();
    icList.clear();
    updateTimeTable();
}
QDateTime KDTimeHeaderWidget::getDateTimeForIndex(int X, bool local )
{
    int coordX = X;
    if ( !local ) {
        QPoint p = QPoint ( X, 1 );
        coordX = myGanttView->myTimeHeaderScroll->viewportToContents(myGanttView->myTimeHeaderScroll->mapFromGlobal( p )).x();

    }
    double secs = (secsFromTo( myRealStart, myRealEnd ) * ((double)coordX))/(double)width();
    double days = secs/86400.0;
    secs = secs - ( ((int) days) *86400.0 );
    return (myRealStart.addDays ( (int) days )).addSecs( (int) secs);
}
bool KDTimeHeaderWidget::getColumnColor(QColor& col,int coordLow, int coordHigh)
{
    if ( flagShowTicks == KDGanttView::ShowNoTicks )
        return false;
    QDateTime start,end;
    start = getDateTimeForIndex(coordLow);
    end = getDateTimeForIndex(coordHigh).addSecs(-1);
    Scale tempScale = myRealScale;
    if ( myGanttView->showTicks() == KDGanttView::ShowMinorTicks ||
         myGanttView->showTicks() == KDGanttView::ShowMajorTicks )
        switch (myRealScale)
            {
            case KDGanttView::Second: tempScale = KDGanttView::Minute;  break;
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
    QColor c = Qt::white;
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
  Specifies the background color for week days. If no individual
  days are visible on the Gantt chart, this method has no visible
  effect. The days are specified as an interval of integer values
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


void KDTimeHeaderWidget::resizeEvent ( QResizeEvent * )
{
    // qDebug("KDTimeHeaderWidget:: resizeEvent ");
    paintPix = QPixmap( 1280, height () );
}


void KDTimeHeaderWidget::updateTimeTable()
{
    //qDebug("KDTimeHeaderWidget::updateTimeTable() ");
    if (flagDoNotRecomputeAfterChange) return;
    // setting the scrolling steps
    int scrollLineStep = myGridMinorWidth;
    if ( myGanttView->showTicks() == KDGanttView::ShowMinorTicks ||
         myGanttView->showTicks() == KDGanttView::ShowMajorTicks ) {
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
    myGanttView->myCanvasView->horizontalScrollBar()->setSingleStep(scrollLineStep);
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
    //qDebug("REPAINTME %d %d", left, paintwid );
    QColorGroup qcg =QColorGroup( Qt::white, Qt::black,Qt::white, Qt::darkGray,Qt::black,Qt::gray,Qt::gray) ;
    QPainter* p;
    int offsetLeft = 0;
    if ( paintwid > paintPix.width()-100 )
        paintPix = QPixmap( paintwid+100, height () );
    if ( painter )
        p = painter;
    else {
        p = new QPainter( &paintPix );
        offsetLeft = left-50;
    }
    if ( mouseDown ) {
        p->fillRect( left-offsetLeft, 0, paintwid, height(), QBrush( palette().color(backgroundRole() ) ) );
        int start ;
        int wid;
        if ( beginMouseDown < endMouseDown ) {
            start = beginMouseDown ;
            wid = endMouseDown - beginMouseDown ;
        } else {
            start = endMouseDown ;
            wid = -endMouseDown + beginMouseDown ;
        }
        p->fillRect( start-offsetLeft, 0, wid, height(), QBrush( palette().color(backgroundRole()).dark()) );
    } else {
        if (! painter )
            p->fillRect( left-offsetLeft, 0, paintwid, height(), QBrush( palette().color(backgroundRole()) ) );
    }
    p->setPen(QColor(40,40,40));
    p->setFont(myFont());
    int hei1 = myMajorGridHeight,
        hei2 = height(),
        wid1 = myGridMinorWidth;
    int xCoord;
    int lwid = 1;

    QValueList<QString>::iterator it;
    QValueList<int>::iterator intIt = majorTicks.begin();
    for ( it =  majorText.begin(); it !=  majorText.end(); ++it ) {
        xCoord  = (*intIt++);
        if (((*intIt)>= left && xCoord <= left+paintwid)) {
            qDrawShadeLine ( p,xCoord-offsetLeft ,hei1+1, xCoord-offsetLeft, -2, qcg, true, lwid, 1 );
            p->drawText(xCoord+4-offsetLeft,hei1-4,(*it));
        }
    }
    qDrawShadeLine ( p,left-offsetLeft  ,hei1, left+paintwid-offsetLeft, hei1, qcg, true, lwid, 1 );
    int i = 0;
    for ( it =  minorText.begin(); it !=  minorText.end(); ++it ) {
        if (i*wid1 >= left-wid1 && i*wid1 <= left+paintwid) {
            qDrawShadeLine ( p,i*wid1-offsetLeft ,hei1-1, i*wid1-offsetLeft, hei2, qcg, true, lwid, 1 );
            p->drawText(i*wid1+1-offsetLeft,hei1+1,wid1-1,hei2-hei1,Qt::AlignCenter,(*it));
        }
        ++i;
    }
    p->setPen(Qt::black);
    p->drawLine(left-offsetLeft,hei1,left+paintwid-offsetLeft,hei1);
    qDrawShadeLine ( p,left-offsetLeft  ,hei2-1, left+paintwid-offsetLeft, hei2-1, qcg, true, lwid, 1 );
    p->drawLine(left-offsetLeft,hei2-1,left+paintwid-offsetLeft,hei2-1);
    if ( !painter ) {
        p->end();
        delete p;
        bitBlt ( this, left, 0, &paintPix, 50, 0, paintwid, height() );
        bitBlt ( this, left, 0, &paintPix, 50, 0, paintwid, height() );
    }
}

// cuts the secs in the DateTime if scale is Minute ,
// the minutes and secs if scale is Hour and so on

QDateTime KDTimeHeaderWidget::getEvenTimeDate(QDateTime tempdatetime ,Scale sc)
{
    QDate tempdate;
    int min, hour, month;
    int tempMinorScaleCount = myRealMinorScaleCount;
    switch (sc)
        {
        case KDGanttView::Month:
            month = tempdatetime.date().month();
            while ( month > 1 && (month-1) % tempMinorScaleCount > 0 )
                --month;
            tempdatetime = QDateTime (QDate( tempdatetime.date().year(), month,1));
            break;
        case KDGanttView::Week:
            {
                int sub = ( tempdatetime.date().dayOfWeek() - (mWeekStartsMonday?1:0) ) % 7;
                if ( sub )
                    tempdatetime = QDateTime (tempdatetime.date().addDays( - sub ) );
                else
                    tempdatetime = QDateTime (tempdatetime.date());
            }
            break;
        case KDGanttView::Day:
            tempdatetime = QDateTime (tempdatetime.date() );
            break;
        case KDGanttView::Hour:
            hour = tempdatetime.time().hour();
            while (24%tempMinorScaleCount > 0 && 24%tempMinorScaleCount < 24)
                ++tempMinorScaleCount;
            hour = ( hour /tempMinorScaleCount)*tempMinorScaleCount;
            //tempdatetime = QDateTime (tempdatetime.date(), QTime (hour, 0 ));
            tempdatetime = QDateTime (tempdatetime.date());
            break;
        case KDGanttView::Minute:
            min = tempdatetime.time().minute();
            while (60%tempMinorScaleCount > 0 && 60%tempMinorScaleCount < 60)
                ++tempMinorScaleCount;
            // qDebug("myMinorScaleCount %d %d %d",myMinorScaleCount, myRealMinorScaleCount, tempMinorScaleCount);
            min = (min /tempMinorScaleCount)*tempMinorScaleCount;
            //tempdatetime = QDateTime (tempdatetime.date(), QTime (tempdatetime.time().hour(),min ));
            tempdatetime = QDateTime (tempdatetime.date(), QTime (tempdatetime.time().hour(),0 ));

            break;
        case KDGanttView::Second:
            min = tempdatetime.time().minute();
            while (60%tempMinorScaleCount > 0 && 60%tempMinorScaleCount < 60)
                ++tempMinorScaleCount;
            // qDebug("myMinorScaleCount %d %d %d",myMinorScaleCount, myRealMinorScaleCount, tempMinorScaleCount);
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
        //qDebug("Autoscale ");
        //double secsPerMinor = (((double)start.daysTo(myHorizonEnd))* 86400.00)/((double)myAutoScaleMinorTickcount);
        double secsPerMinor = (((double)start.secsTo(myHorizonEnd)))/((double)myAutoScaleMinorTickcount);
        secsPerMinor /= myZoomFactor;
        if (secsPerMinor <= 30 ) {
            myRealScale =  KDGanttView::Second;
            myRealMinorScaleCount = (int) secsPerMinor;
        } else {
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
        }
        if(myRealMinorScaleCount == 0)
            myRealMinorScaleCount = 1;
        myRealMajorScaleCount = 1;
    }
    else {
        //qDebug("Fixed scale ");
        myRealScale = myScale;
        if (myRealScale > myMaxScale)
            myRealScale = myMaxScale;
        if (myRealScale < myMinScale)
            myRealScale = myMinScale;
        myRealMinorScaleCount = (int) ( ((double)myMinorScaleCount) /myZoomFactor );
        double tempZoom = myZoomFactor;
        myRealMajorScaleCount =  myMajorScaleCount;
        while (myRealMinorScaleCount == 0) {
            if (myRealScale  == myMinScale) {
                myRealMinorScaleCount = 1;
                break;
            }
            switch (myRealScale)
                {
                case KDGanttView::Second:
                    myRealMinorScaleCount = 1;
                    return;
                    break;
                case KDGanttView::Minute:
                    myRealScale = KDGanttView::Second;
                    tempZoom = tempZoom/60;
                    break;
                case KDGanttView::Hour:
                    myRealScale = KDGanttView::Minute;
                    tempZoom = tempZoom/60;
                    break;
                case KDGanttView::Day:
                    myRealScale = KDGanttView::Hour;
                    tempZoom = tempZoom/24;
                    break;
                case KDGanttView::Week:
                    myRealScale = KDGanttView::Day;
                    tempZoom = tempZoom/7;
                    break;
                case KDGanttView::Month:
                    myRealScale =  KDGanttView::Week ;
                    tempZoom = tempZoom*7/30;
                    break;
                case KDGanttView::Auto:
                    break;
                }
            myRealMinorScaleCount =  (int) ( myMinorScaleCount /tempZoom );
        }
    }
}
int  KDTimeHeaderWidget::getMaxTextWidth( const QString& format, int mode )
{
    if ( format.isEmpty() )
        return 0;
    int WidthMajor = 0;
    QFontMetrics fm ( myFont() );
    if ( mode == 0 ) { // with time - minute and hour mode
        QDateTime testDT = QDateTime( QDate(2004,1,31 ), QTime (0,0,0 ) );
        QDateTime tempDT = testDT ;
        WidthMajor = fm.width( testDT.toString( format ) );
        int i;
        // testing max month wid
        for ( i = 2; i <= 12; ++i ) {
            testDT = testDT.addDays( testDT.addDays(1).date().daysInMonth() );
            int wid = fm.width( testDT.toString( format ) );
            if ( wid >  WidthMajor ) {
                WidthMajor = wid;
                tempDT = testDT;
            }
        }
        // testing max weekday wid for max month
        testDT = tempDT.addDays( -7 );
        for ( i = 0; i < 6; ++i ) {
            testDT = testDT.addDays( 1 );
            int wid = fm.width( testDT.toString( format ) );
            if ( wid >  WidthMajor ) {
                WidthMajor = wid;
                tempDT = testDT;
            }
        }
    } else if ( mode == 1 ) { // day mode
        QDate testDT;
        if ( mWeekStartsMonday )
            testDT = QDate ( 2004,1,31 ); // it's a monday
        else
            testDT = QDate ( 2004,1,30 ); // it's a sunday
        WidthMajor = fm.width( testDT.toString( mDateFormatDay ) );
        int i;
        for ( i = 2; i <= 12; ++i ) {
            testDT = testDT.addDays( 7*5 );
            if ( testDT.month() != i )
                testDT = testDT.addDays( -7 );
            int wid = fm.width( testDT.toString( format ) );
            if ( wid >  WidthMajor )
                WidthMajor = wid;
        }
    } else if ( mode == 2 ) { // week mode
        QDate testDT;
        testDT = QDate ( 2004,1,1 );
        WidthMajor = fm.width( testDT.toString( mDateFormatWeek ) );
        int i;
        for ( i = 2; i <= 12; ++i ) {
            testDT = testDT.addDays( testDT.daysInMonth() );
            int wid = fm.width( testDT.toString( mDateFormatWeek ) );
            if ( wid >  WidthMajor )
                WidthMajor = wid;
        }
    } else if ( mode == 3 ) { // month mode
        QDate testDT;
        testDT = QDate ( 2004,1,1 );
        WidthMajor = fm.width( testDT.toString( format ) );
    }
    return WidthMajor;
}
int KDTimeHeaderWidget::getWeekOfYear( const QDate& date )
{
    int ret;
    QDate d = date.addDays( mWeekStartsMonday ? 3 : 4 );
    int dayOfYear = d.dayOfYear();
    if (dayOfYear % 7 != 0)
        ret = dayOfYear / 7 + 1;
    else
        ret =dayOfYear / 7;
    return ret;
}
QFont KDTimeHeaderWidget::myFont()
{
    QFont tempFont = font();
    tempFont.setWeight(63);
    return tempFont;
}
void KDTimeHeaderWidget::computeTicks(bool doNotComputeRealScale) // default false
{
    if (flagDoNotRecomputeAfterChange) return;
    bool block = myGanttView->myTimeTable->blockUpdating();
    myGanttView->myTimeTable->setBlockUpdating( true );
    //qDebug("++++++++++++++++++++++computeticks ");
    majorTicks.clear();
    minorText.clear();
    majorText.clear();
    if (!doNotComputeRealScale) {
        saveCenterDateTime();
        computeRealScale(myHorizonStart);
    }
    myRealStart = getEvenTimeDate(myHorizonStart ,myRealScale);
    if (!doNotComputeRealScale)
        computeRealScale(myRealStart);
    int tempMinorScaleCount = myRealMinorScaleCount,
        tempMajorScaleCount = myRealMajorScaleCount;
    int minorItems,minorPerMajor = 1;
    minorItems = (int)  (secsFromTo( myRealStart, myHorizonEnd));
    //qDebug("tempMinorScaleCount %d scale: %d  - real scale: %d", tempMinorScaleCount, myScale, myRealScale);
    //qDebug("Zoom factor %f ",myZoomFactor );
    //qDebug("sta %s - %s end %s ",myRealStart.toString().toLatin1(), myHorizonStart.toString().toLatin1(),myHorizonEnd.toString().toLatin1());
    QString testTextMinor, testTextMajor, tempStr;
    QRect itemRectMinor, itemRectMajor;
    QDate tempDate = myRealStart.date();
    myRealEnd = myRealStart;
    // preparing the testtext for the differennt scales
    QFontMetrics fm ( myFont() );
    int WidthMajor = 0;//fm.width( testTextMajor );
    switch (myRealScale)
        {
            // the x in testTextMajor is added to reserve a little bit more space
        case KDGanttView::Second:
            testTextMinor = "60";
            if ( mMaxWidtimeFormatSecond ) {
                WidthMajor = mMaxWidtimeFormatSecond;
            } else {
                if (myHourFormat == KDGanttView::Hour_12)
                    testTextMajor = "Mon Aug 30, 12:00 AMx";
                else
                    testTextMajor = "Mon Aug 30, 24:00x";
            }
            minorPerMajor = 6000;
            break;
        case KDGanttView::Minute:
            minorItems = minorItems/60;
            testTextMinor = "60";
            if ( mMaxWidtimeFormatMinute ) {
                WidthMajor = mMaxWidtimeFormatMinute;
            } else {
                if (myHourFormat == KDGanttView::Hour_12)
                    testTextMajor = "Mon Aug 30, 12 AMx";
                else
                    testTextMajor = "Mon Aug 30, 24:00x";
            }
            minorPerMajor = 6000;
            break;
        case KDGanttView::Hour:
            minorItems = minorItems/(60*60);
            if (myHourFormat == KDGanttView::Hour_24)
                testTextMinor = "24x";
            else
                testTextMinor = "12 AM";
            if ( mMaxWidtimeFormatHour ) {
                WidthMajor = mMaxWidtimeFormatHour;
            } else {
                testTextMajor = "Mon Aug 30, x";
                if ( yearFormat() != KDGanttView::NoDate )
                    testTextMajor += getYear(QDate::currentDate());
            }
            minorPerMajor = 2400;
            break;
        case KDGanttView::Day:
            minorItems = minorItems/(60*24*60);
            testTextMinor = "88";
            if ( mMaxWidFormatDay ) {
                WidthMajor = mMaxWidFormatDay;
            } else {
                testTextMajor = "Aug 30, x"+getYear(QDate::currentDate());
            }
            minorPerMajor = 700;
            break;
        case KDGanttView::Week:
            minorItems = minorItems/(60*24*7*60);
            testTextMinor = "88";
            if ( mMaxWidFormatWeek ) {
                WidthMajor = mMaxWidFormatWeek;
            } else {
                testTextMajor = "Aug x"+getYear(QDate::currentDate());
            }
            minorPerMajor = 435; // 435 = 365days/12months/7days * 100
            break;
        case KDGanttView::Month:
            minorItems = (minorItems*12)/(60*24*365*60);
            testTextMinor = "M";
            if ( mMaxWidFormatMonth ) {
                WidthMajor = mMaxWidFormatMonth;
            } else {
                testTextMajor = "x"+getYear(QDate::currentDate());
            }
            minorPerMajor = 1200;
            break;
        case KDGanttView::Auto:
            qDebug("KDGanttView::Internal Error in KDTimeHeaderWidget::computeTicks() ");
            qDebug("             RealScale == Auto : This may not be! ");
            break;
        }
    //qDebug("     tempMinorScaleCount %d ", tempMinorScaleCount);
    if ( !WidthMajor )
        WidthMajor = fm.width( testTextMajor );
    else
        WidthMajor += 5; // we need to add space for the borders
    int HeightMajor = fm.height();
    int Height = fm.height() + HeightMajor+11;
    int Width = fm.width( testTextMinor ) + 9;
    if (Width < minimumColumnWidth()) Width = minimumColumnWidth();
    // if the desired width is greater than the maximum width of this widget
    // increase the minorscalecount
    int maxWid = myGanttView->myCanvasView->viewport()->width();
    if (!flagZoomToFit)
        maxWid = maximumWidth();
    while((minorItems/tempMinorScaleCount+1)*Width > maxWid)
        ++tempMinorScaleCount;
    //qDebug("             tempMinorScaleCount %d ", tempMinorScaleCount);
    mySizeHint = (minorItems/tempMinorScaleCount+1)*Width;
    switch (myRealScale)
        {
        case KDGanttView::Second:
            if (tempMinorScaleCount < 60)
                while (60%tempMinorScaleCount > 0 && 60%tempMinorScaleCount < 60)
                    ++tempMinorScaleCount;
            if (tempMinorScaleCount >= 60) {
                myRealScale = KDGanttView::Minute;
                myRealMinorScaleCount = tempMinorScaleCount/ 60;
                // myRealMinorScaleCount = 1;
                myRealMajorScaleCount = 1;
                qDebug("KDGantt::Overzoom:Rescaling from Second to Minute");
                myGanttView->myTimeTable->setBlockUpdating( block );
                emit myGanttView->rescaling( KDGanttView::Minute );
                computeTicks(true);
                return;
            }
            break;
        case KDGanttView::Minute:
            if (tempMinorScaleCount < 60)
                while (60%tempMinorScaleCount > 0 && 60%tempMinorScaleCount < 60)
                    ++tempMinorScaleCount;
            if (tempMinorScaleCount >= 60) {
                myRealScale = KDGanttView::Hour;
                myRealMinorScaleCount = tempMinorScaleCount/ 60;
                // myRealMinorScaleCount = 1;
                myRealMajorScaleCount = 1;
                qDebug("KDGantt::Overzoom:Rescaling from Minute to Hour");
                myGanttView->myTimeTable->setBlockUpdating( block );
                emit myGanttView->rescaling( KDGanttView::Hour );
                computeTicks(true);
                return;
            }
            break;
        case KDGanttView::Hour:
            while (24%tempMinorScaleCount > 0 && 24%tempMinorScaleCount < 24)
                ++tempMinorScaleCount;
            if (tempMinorScaleCount >= 24) {
                myRealScale = KDGanttView::Day;
                myRealMinorScaleCount = tempMinorScaleCount/ 24;
                //myRealMinorScaleCount = 1;
                myRealMajorScaleCount = 1;
                qDebug("KDGantt::Overzoom:Rescaling from Hour to Day");
                myGanttView->myTimeTable->setBlockUpdating( block );
                emit myGanttView->rescaling( KDGanttView::Day );
                computeTicks(true);
                return;
            }
            break;
        case KDGanttView::Day:
            if ( tempMinorScaleCount >= 100 ) {
                myRealScale = KDGanttView::Week;
                myRealMinorScaleCount = tempMinorScaleCount / 7;
                //myRealMinorScaleCount = 1;
                myRealMajorScaleCount = 1;
                qDebug("KDGantt::Overzoom:Rescaling from Day to Week");
                myGanttView->myTimeTable->setBlockUpdating( block );
                emit myGanttView->rescaling( KDGanttView::Week );
                computeTicks(true);
                return;
            }
            break;
        case KDGanttView::Week:
            if ( tempMinorScaleCount >= 54 ) {
                myRealScale = KDGanttView::Month;
                myRealMinorScaleCount = tempMinorScaleCount / 4;
                //myRealMinorScaleCount = 1;
                myRealMajorScaleCount = 1;
                qDebug("KDGantt::Overzoom:Rescaling from Week to Month");
                myGanttView->myTimeTable->setBlockUpdating( block );
                emit myGanttView->rescaling( KDGanttView::Month );
                computeTicks(true);
                return;
            }
            break;
        case KDGanttView::Month:
            while (12%tempMinorScaleCount > 0 && 12%tempMinorScaleCount < 12)
                ++tempMinorScaleCount;
            break;
        default:
            break;
        }
    flagZoomToFit = false;
    while((minorItems/tempMinorScaleCount+1)*Width < myMinimumWidth ) {
        ++minorItems;
    }
    minorItems = (minorItems/tempMinorScaleCount)+1;
    // if not enough space for the text of the major scale, increase majorscalecount
    minorPerMajor = (minorPerMajor*tempMajorScaleCount)/tempMinorScaleCount;
    // checking, if enough space for majorscale
    // if not, increasing MajorScaleCount
    if ( minorPerMajor == 0 )
        minorPerMajor = 1;

    while (minorPerMajor && ( (minorPerMajor*Width)/100 < WidthMajor) ) {
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
    myMajorGridHeight = HeightMajor + 5;
    QTime tempTime = myRealStart.time();
    QDateTime tempDateTime;
    //myMajorScaleCount = tempMajorScaleCount;
    int i;
    switch (myRealScale)
        {
        case KDGanttView::Second:
            myRealEnd = myRealEnd.addSecs((minorItems)*tempMinorScaleCount);
            for ( i = 0; i < minorItems;++i) {
                tempStr.setNum(tempTime.second());
                minorText.append(tempStr);
                tempTime = tempTime.addSecs(tempMinorScaleCount);
            }
            tempDateTime = myRealStart;
            if (tempDateTime.time().second() != 0) {
                tempDateTime = tempDateTime.addSecs( 60 - tempDateTime.time().second() );
            }
            while (tempDateTime < myRealEnd) {
                majorTicks.append( getCoordX(tempDateTime));
                if ( !mDatetimeFormatSecond.isEmpty() ) {
                    tempStr = tempDateTime.toString ( mDatetimeFormatSecond );
                } else {
                    tempStr.setNum(tempDateTime.date().day());
                    if ( yearFormat() == KDGanttView::NoDate ) {
                        tempStr = QDate::longDayName (tempDateTime.date().dayOfWeek() )+", "
                            +getHourMinutes(tempDateTime.time());
                    } else {
                        tempStr = QDate::shortDayName (tempDateTime.date().dayOfWeek() )+" "+
                            QDate::shortMonthName(tempDateTime.date().month())+ " "+
                            tempStr+", "+getHourMinutes(tempDateTime.time());
                    }
                }
                majorText.append(tempStr);
                tempDateTime = tempDateTime.addSecs(60*tempMajorScaleCount);
            }
            majorTicks.append( getCoordX(tempDateTime));
            break;

        case KDGanttView::Minute:
            myRealEnd = myRealEnd.addSecs((minorItems)*tempMinorScaleCount*60);
            for ( i = 0; i < minorItems;++i) {
                tempStr.setNum(tempTime.minute());
                minorText.append(tempStr);
                tempTime = tempTime.addSecs(60*tempMinorScaleCount);
            }
            tempDateTime = myRealStart;
            if (tempDateTime.time().minute() != 0) {
                tempDateTime = tempDateTime.addSecs( ( 60 - tempDateTime.time().minute()) * 60 );
            }
            while (tempDateTime < myRealEnd) {
                majorTicks.append( getCoordX(tempDateTime));
                if ( !mDatetimeFormatMinute.isEmpty() ) {
                    tempStr = tempDateTime.toString ( mDatetimeFormatMinute );
                } else {
                    tempStr.setNum(tempDateTime.date().day());
                    if ( yearFormat() == KDGanttView::NoDate ) {
                        tempStr = QDate::longDayName (tempDateTime.date().dayOfWeek() )+", "
                            +getHour(tempDateTime.time());
                    } else {
                        tempStr = QDate::shortDayName (tempDateTime.date().dayOfWeek() )+" "+
                            QDate::shortMonthName(tempDateTime.date().month())+ " "+
                            tempStr+", "+getHour(tempDateTime.time());
                    }
                }
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
                if ( !mDatetimeFormatHour.isEmpty() ) {
                    tempStr = tempDateTime.toString ( mDatetimeFormatHour );
                } else {
                    tempStr.setNum(tempDateTime.date().day());
                    if ( yearFormat() == KDGanttView::NoDate ) {
                        tempStr = QDate::longDayName (tempDateTime.date().dayOfWeek() );
                    } else {
                        tempStr = QDate::shortDayName (tempDateTime.date().dayOfWeek() )+" "+
                            QDate::shortMonthName(tempDateTime.date().month())+ " "+
                            tempStr+", "+getYear(tempDateTime.date());
                    }
                }
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
            {
                int start = 7;
                int sub = 7;
                if ( mWeekStartsMonday ) {
                    start = 1;
                    ++sub;
                }
                int dof = tempDate.dayOfWeek();
                if ( dof != start )
                        tempDate = tempDate.addDays( sub - dof );
            }
            while (tempDate < myRealEnd.date()) {
                majorTicks.append( getCoordX(tempDate));
                if ( !mDateFormatDay.isEmpty() ) {
                    tempStr = tempDate.toString ( mDateFormatDay );
                } else {
                    tempStr.setNum(tempDate.day());
                    tempStr = QDate::shortMonthName(tempDate.month())+ " "+
                        tempStr+", "+getYear(tempDate);
                }
                majorText.append(tempStr);
                tempDate = tempDate.addDays(7*tempMajorScaleCount);
            }
            majorTicks.append( getCoordX(tempDate));
            break;
        case KDGanttView::Week:
            myRealEnd = myRealEnd.addDays(minorItems*tempMinorScaleCount*7);
            for ( i = 0; i < minorItems;++i) {
                if ( mWeekScaleShowNumber )
                    tempStr.setNum(getWeekOfYear(tempDate));
                else
                    tempStr.setNum(tempDate.day());
                minorText.append(tempStr);
                tempDate = tempDate.addDays(7*tempMinorScaleCount);
            }
            tempDate = myRealStart.date();
            while (tempDate.day() != 1)
                tempDate = tempDate.addDays(1);
            while (tempDate < myRealEnd.date()) {
                majorTicks.append( getCoordX(tempDate));
                if ( !mDateFormatWeek.isEmpty() ) {
                    tempStr = tempDate.toString ( mDateFormatWeek );
                } else {
                    tempStr = QDate::shortMonthName(tempDate.month())+ " "+getYear(tempDate);
                }
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
            if (tempDate.month() != 1)
                tempDate = tempDate.addMonths( 12 - tempDate.month() );
            while (tempDate < myRealEnd.date()) {
                int coord = getCoordX(tempDate) + myGridMinorWidth / 2;
                int diff = coord % myGridMinorWidth;
                coord = coord - diff;
                majorTicks.append( coord );
                if ( !mDateFormatMonth.isEmpty() ) {
                    tempStr = tempDate.toString ( mDateFormatMonth );
                } else {
                    tempStr = getYear(tempDate);
                }
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
    //qDebug("KDTimeHeaderWidget width %d, viewport width %d  ",width (), myGanttView->myCanvasView->viewport()->width());
    myGanttView->myTimeTable->setBlockUpdating( block );
    updateTimeTable();
    if ( ! block )
        centerDateTime(myCenterDateTime);
    pendingPaint();
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


void KDTimeHeaderWidget::centerDateTime( const QDateTime& center,  bool changeHorizon  )
{
    int coordX = getCoordX( center );
    bool outOfRange = ( coordX < 0 || coordX > myGanttView->myCanvasView->contentsWidth() );
    if (  outOfRange ) {
        if ( changeHorizon ) {
            double secs = secsFromTo( myHorizonStart, myHorizonEnd );
            int secdiff = (int) ( secs/2.0 );
            myHorizonStart = center.addSecs( -secdiff );
            myHorizonEnd = center.addSecs( secdiff );
            computeTicks();
            coordX = getCoordX( center );
        } else {
            return;
        }
    }
    moveTimeLineTo(getCoordX( center )-(myGanttView->myCanvasView->viewport()->width() /2));
    //  qDebug("centerDateTime %s %d %d", center.toString().toLatin1(),getCoordX( center ),(myGanttView->myCanvasView->viewport()->width() /2) );
}

void KDTimeHeaderWidget::pendingPaint()
{
    repaint();
}

void KDTimeHeaderWidget::paintEvent(QPaintEvent *p)
{

    repaintMe(p->rect().x(),p->rect().width());
}

int KDTimeHeaderWidget::getCoordX(QDate date)
{
    int wid = width();
    int daysAll = myRealStart.daysTo(myRealEnd);
    if (daysAll == 0) return 0;
    int days = myRealStart.daysTo(QDateTime(date));
    int retVal = (wid *days) /daysAll;
    return retVal;
}


int KDTimeHeaderWidget::getCoordX(QDateTime datetime)
{
    double wid = width();
    double secsAll = secsFromTo( myRealStart, myRealEnd );
    if (secsAll == 0.0) return 0;
    double secs = secsFromTo( myRealStart, datetime);
    int retVal = ((int)((wid *(secs /secsAll))+0.5));
    return retVal;
}


QDate KDTimeHeaderWidget::yesterday() const
{
    return QDate::currentDate().addDays( -1 ) ;
}
QDate KDTimeHeaderWidget::today() const
{
    return QDate::currentDate() ;
}
QDate KDTimeHeaderWidget::tomorrow() const
{
    return QDate::currentDate().addDays( 1 ) ;
}
QDate KDTimeHeaderWidget::currentWeek() const
{
    int sub = ( QDate::currentDate().dayOfWeek() - (mWeekStartsMonday?1:0) ) % 7;
    if ( sub )
        return QDate::currentDate().addDays( -sub );
    return QDate::currentDate();
}
QDate KDTimeHeaderWidget::lastWeek() const
{
    return currentWeek().addDays( -7 ) ;
}
QDate KDTimeHeaderWidget::currentMonth() const
{
    return QDate ( QDate::currentDate().year(),QDate::currentDate().month(),1) ;
}
QDate KDTimeHeaderWidget::lastMonth() const
{
    return QDate ( QDate::currentDate().year(),QDate::currentDate().month(),1).addMonths( -1 ) ;
}
QDate KDTimeHeaderWidget::currentYear() const
{
    return QDate ( QDate::currentDate().year(), 1, 1 ) ;
}
QDate KDTimeHeaderWidget::lastYear() const
{
    return QDate ( QDate::currentDate().year() - 1, 1, 1) ;
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
    case KDGanttView::NoDate:
        // nothing to do
        break;
    }
    return ret;
}
QString KDTimeHeaderWidget::getHourMinutes(QTime time)
{
    QString ret;
    int hour = time.hour();
    QString min = QString::number( time.minute() );
    if ( min.length() == 1 )
        min = "0" + min;
    if (myHourFormat == KDGanttView::Hour_12) {
        if (hour >= 12) {
            if (hour > 12) hour -=12;
            ret.setNum(hour);
            ret += "." + min;
            ret = ret +" PM";
        } else {
            if (hour == 0) hour = 12;
            ret.setNum(hour);
            ret += "." + min;
            ret = ret +" AM";
        }
    } else {
        ret.setNum(hour);
        ret += ":" + min;
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
    } else {
        if (myHourFormat == KDGanttView::Hour_24)
            ret.setNum(hour);
        else {
            ret.setNum(hour);
            ret += ":00";
        }
    }
    return ret;
}


void KDTimeHeaderWidget::mousePressEvent ( QMouseEvent * e )
{
    mouseDown = false;
    switch ( e->button() ) {
    case Qt::LeftButton:
        mouseDown = true;
        beginMouseDown = e->pos().x();
        endMouseDown = e->pos().x();
        break;
    case Qt::RightButton:
        if (flagShowPopupMenu)
            myPopupMenu->popup(e->globalPos());
        break;
    case Qt::MidButton:
        break;
    default:
        break;
    }

}


void KDTimeHeaderWidget::mouseReleaseEvent ( QMouseEvent *  )
{
    if ( mouseDown ) {
        mouseDown = false;
        // zoom to selection getDateTimeForIndex(
        int start, end;
        if ( beginMouseDown < endMouseDown ) {
            start = beginMouseDown;
            end = endMouseDown;
        } else {
            start = endMouseDown;
            end = beginMouseDown;
        }
        if (start < 0 )
            start = 0;
        if ( end > width() )
            end = width();
        //qDebug("start %s ",getDateTimeForIndex(start).toString().toLatin1() );
        //qDebug("end %s ",getDateTimeForIndex(end).toString().toLatin1() );
        emit myGanttView->timeIntervalSelected( getDateTimeForIndex(start),getDateTimeForIndex(end) );
        emit myGanttView->timeIntervallSelected( getDateTimeForIndex(start),getDateTimeForIndex(end) );
    }
    mouseDown = false;
    pendingPaint();
}


void KDTimeHeaderWidget::mouseDoubleClickEvent ( QMouseEvent *  )
{

}


void KDTimeHeaderWidget::mouseMoveEvent ( QMouseEvent * e )
{
    if ( mouseDown ) {
        if ( e->pos().y() < -height() || e->pos().y() > 2* height() ) {
            mouseDown = false;
            pendingPaint();
            return;
        }
        endMouseDown = e->pos().x();
        //repaint;
        int val = -1;
        if (endMouseDown <  -x() ) {
            val = myGanttView->myCanvasView->horizontalScrollBar()->value() -
                myGanttView->myCanvasView->horizontalScrollBar()->singleStep();
            if ( val < 0 ) {
                val = 0;
            }
        }
        if (endMouseDown >  -x() +parentWidget()->width() ) {
            val = myGanttView->myCanvasView->horizontalScrollBar()->value() +
                myGanttView->myCanvasView->horizontalScrollBar()->singleStep();

        }
        pendingPaint();
        //epaintMe(-x(),parentWidget()->width());
        if ( val > -1 ) {
            if ( val > myGanttView->myCanvasView->horizontalScrollBar()->maximum() ) {
                val = myGanttView->myCanvasView->horizontalScrollBar()->maximum();
            }
            myGanttView->myCanvasView->horizontalScrollBar()->setValue( val );
        }
        //qDebug("mousemove %d %d %d %d",endMouseDown, -x(),parentWidget()->width() , e->pos().y());
    }
}


/* ***************************************************************
   KDLegendWidget:: KDLegendWidget
   ***************************************************************** */
KDLegendWidget:: KDLegendWidget( QWidget* parent,
                                 KDGanttMinimizeSplitter* legendParent ) :
    KDGanttSemiSizingControl ( KDGanttSemiSizingControl::Before, Qt::Vertical,
                               parent)
{
    myLegendParent = legendParent;
    dock = 0;
    scroll = new QScrollView( legendParent );
    setMaximizedWidget( scroll );

    setMinimizedWidget( myLabel = new QLabel(tr(" Legend is hidden"),this)  );
    setGeometry( 0, 0, 50, 50 );
    myLegend = 0;
    clearLegend();
    showMe ( false );
}
void KDLegendWidget::setAsDockwindow( bool dockwin )
{
    if ( (dock == 0 && !dockwin) || ( dock && dockwin  ) )
        return;
    if ( dockwin )
        {
            setMaximizedWidget( 0 );
            showMe ( false );
            if ( dock ) delete dock;
            dock = new QDockWindow(QDockWindow:: OutsideDock,0 );
            dock->resize( 200, 100 );
            dock->setHorizontallyStretchable( true );
            dock->setVerticallyStretchable( true );
            dock->setWindowTitle(tr("Legend: ") );
            dock->setResizeEnabled (true );
            delete myLegend;
            myLegend = 0;
            delete scroll;
            scroll = new QScrollView( dock );
            clearLegend();
            dock->setWidget(scroll);
            setMaximizedWidget( dock );
            showMe ( false );

        } else {
            setMaximizedWidget( 0 );
            showMe ( false );
            delete myLegend;
            myLegend = 0;
            delete scroll;
            delete dock;
            dock = 0;
            scroll = new QScrollView( myLegendParent );
            clearLegend();
            setMaximizedWidget( scroll );
            showMe ( false );
        }

}


bool KDLegendWidget::asDockwindow( )
{
    if ( dock )
        return true;
    return false;
}


QDockWindow* KDLegendWidget::dockwindow( )
{
    return dock;
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
    QApplication::sendPostedEvents( 0, QEvent::LayoutHint );
    return QSize( myLegend->sizeHint().width(), myLegend->sizeHint().height()+scroll->horizontalScrollBar()->height());
}


void KDLegendWidget::showMe ( bool show )
{
    minimize( !show );
}


void KDLegendWidget::clearLegend ( )
{
    if ( myLegend ) delete myLegend;
    if ( dock )
        myLegend = new QGroupBox( 1, Qt::Horizontal, scroll->viewport() );
    else
        myLegend = new QGroupBox( 1, Qt::Horizontal, tr("Legend:"), scroll->viewport() );
    QPalette pal = palette();
    pal.setColor( backgroundRole(), Qt::white );
    myLegend->setPalette( pal );
    myLegend->setFont( font() );
    scroll->addChild(  myLegend );
    scroll->setResizePolicy( QScrollView::AutoOneFit );
    myLegend->layout()->setMargin( 11 );
#if QT_VERSION < 0x040000
    myLegend->setFrameStyle( QFrame::NoFrame );
#endif
    if ( dock )
        scroll->setMaximumHeight( 32000 );
    else
        scroll->setMaximumHeight( legendSizeHint().height() );
}


void KDLegendWidget::addLegendItem( KDGanttViewItem::Shape shape, const QColor& shapeColor, const QString& text )
{
    QLabel * temp;
    QWidget *w = new QWidget( myLegend );
    QPalette pal = w->palette();
    pal.setColor( w->backgroundRole(), Qt::white );
    w->setPalette( pal );
    QHBoxLayout *lay = new QHBoxLayout( w  );
    lay->setSpacing( 6 );
    lay->setMargin( 0 );
    temp = new QLabel ( w );
    lay->addWidget( temp, 0, Qt:: AlignRight);
    temp->setPixmap(KDGanttView::getPixmap( shape,  shapeColor, Qt::white, 10));
    temp->setPalette( pal );
    temp = new QLabel ( text, w );
    temp->setPalette( pal );
    lay->addWidget( temp, 0, Qt:: AlignLeft);
    lay->addStretch();
    if ( dock )
        scroll->setMaximumHeight( 32000 );
    else
        scroll->setMaximumHeight( legendSizeHint().height() );
}

void KDLegendWidget::addLegendItem( KDGanttViewItem::Shape shape, const QColor& shapeColor, const QString& text,
                                    KDGanttViewItem::Shape shape2, const QColor& shapeColor2, const QString& text2 )
{
    QLabel * temp;
    QPixmap p = KDGanttView::getPixmap( shape,  shapeColor, Qt::white, 10);
    QWidget *w = new QWidget( myLegend );
    QPalette pal;
    pal.setColor( w->backgroundRole(), Qt::white );
    w->setPalette( pal );
    QHBoxLayout *lay = new QHBoxLayout( w  );
    lay->setSpacing( 6 );
    lay->setMargin( 0 );
    temp = new QLabel ( w );
    lay->addWidget( temp, 0, Qt:: AlignRight);
    temp->setPixmap(p);
    QPalette pal1;
    pal1.setColor( temp->backgroundRole(), Qt::white );
    temp->setPalette( pal1 );
    if( !text.isEmpty() ){
        temp = new QLabel ( text, w );
        QPalette pal2;
        pal2.setColor( temp->backgroundRole(), Qt::white );
        temp->setPalette( pal2 );
        lay->addWidget( temp, 0, Qt::AlignCenter);
    }
    temp = new QLabel ( w );
    lay->addWidget( temp, 0, Qt:: AlignRight);
    temp->setPixmap( KDGanttView::getPixmap( shape2,  shapeColor2, Qt::white, 10));
    QPalette pal3;
    pal3.setColor( temp->backgroundRole(), Qt::white );
    temp->setPalette( pal3 );
    if( !text2.isEmpty() ){
        temp = new QLabel ( text2, w );
        QPalette pal4;
        pal4.setColor( temp->backgroundRole(), Qt::white );
        temp->setPalette( pal4 );
        lay->addWidget( temp, 0, Qt::AlignLeft);
    }
    lay->addStretch();
    if ( dock )
        scroll->setMaximumHeight( 32000 );
    else
        scroll->setMaximumHeight( legendSizeHint().height() );
}


bool KDLegendWidget::isShown ( )
{
    return !isMinimized();
}


KDListView::KDListView(QWidget* parent, KDGanttView* gantView):QListView (parent)
{
    myGanttView = gantView;
    setAcceptDrops(true);
    new KDListViewWhatsThis(viewport(),this);
    setRootIsDecorated( true );
    setAllColumnsShowFocus( true );
    addColumn( tr( "Task Name" ) );
    setSorting( -1 );
    //setVScrollBarMode (QScrollView::AlwaysOn );
    setHScrollBarMode (QScrollView::AlwaysOn );
    setDefaultRenameAction(QListView::Accept);
    setColumnWidthMode ( 0,Maximum  );
    _calendarMode = false;
    // QObject::connect(this, SIGNAL (  pressed ( QListViewItem * )) , this, SLOT( dragItem( QListViewItem *))) ;
}


QSize KDListView::minimumSizeHint () const
{
    return QSize( 10, 10 );
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
    // setRootIsDecorated ( ! mode );
}

void  KDListView::setOpen(QListViewItem * item, bool open )
{
    if (! _calendarMode || ! open ) {
        (( KDGanttViewItem*)item)->setCallListViewOnSetOpen( false );
        QListView::setOpen ( item, open );
        (( KDGanttViewItem*)item)->setCallListViewOnSetOpen( true );
        return;
    }
    // we are in calendarmode
    // in calendarmode only items can be opened which have subitems which have subitems

    QListViewItem* temp;
    temp = item->firstChild();
    bool openItem = false;
    while (temp) {
        if ( (( KDGanttViewItem*)temp)->displaySubitemsAsGroup() ) {
            temp->setVisible( true );
            openItem = true;
        }
        else {
            temp->setVisible( false );
            //qDebug(" temp->setVisible( false );");
        }
        temp = temp->nextSibling();
    }
    if ( openItem ) {
        (( KDGanttViewItem*)item)->setCallListViewOnSetOpen( false );
        QListView::setOpen ( item, open );
        (( KDGanttViewItem*)item)->setCallListViewOnSetOpen( true );
    }
}


void  KDListView::contentsMouseDoubleClickEvent ( QMouseEvent * e )
{
    //QListView::contentsMouseDoubleClickEvent ( e );
    //if ( ! _calendarMode )
    // QListView::contentsMouseDoubleClickEvent ( e );
    // else
    {

        emit myGanttView->lvItemDoubleClicked ( (KDGanttViewItem*) itemAt(e->pos() ) );
        emit myGanttView->itemDoubleClicked ( (KDGanttViewItem*) itemAt(e->pos() ) );
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
void KDListView::dragEnterEvent ( QDragEnterEvent * e)
{
    if ( !myGanttView->dropEnabled() ) {
        e->setAccepted( false );
        return;
    }
    myGanttView->lvDragEnterEvent(e);
    //e->setAccepted(KDGanttViewItemDrag::canDecode(e) );
}

void KDListView::dragMoveEvent ( QDragMoveEvent * e)
{
    if ( !myGanttView->dropEnabled() ) {
        e->setAccepted( false );
        return;
    }
    KDGanttViewItem* draggedItem = 0;
    QPoint pos = e->pos();
    if ( myGanttView->headerVisible() )
        pos.setY( pos.y() - header()->height()  );
    KDGanttViewItem* gItem = (KDGanttViewItem*)itemAt( pos ) ;
    setCurrentItem( gItem );
    if (  e->source() == myGanttView )
        draggedItem = myGanttView->myCanvasView->lastClickedItem;
    // execute user defined dragMoveEvent handling
    if (myGanttView->lvDragMoveEvent ( e , draggedItem, gItem ) )
        return;
    if ( !KDGanttViewItemDrag::canDecode(e) ) {
        e->setAccepted( false );
        return;
    }
    if ( e->source() == myGanttView && gItem ){
        // internal drag - do not allow to drag the item to a subitem of itself
        KDGanttViewItem* pItem = gItem->parent();
        while ( pItem ) {
            if ( pItem == myGanttView->myCanvasView->lastClickedItem ) {
                e->setAccepted( false );
                return;
            }
            pItem = pItem->parent();
        }
        if ( gItem == myGanttView->myCanvasView->lastClickedItem ) {
            e->setAccepted( false );
            return;
        }
    }
    e->setAccepted( true );
}

void KDListView::dragLeaveEvent ( QDragLeaveEvent * )
{
    //qDebug("contentsDragLeaveEvent ");
}
void KDListView::dropEvent ( QDropEvent *e )
{
    if ( !myGanttView->dropEnabled() ) {
        e->setAccepted( false );
        return;
    }
    QPoint pos = e->pos();
    if ( myGanttView->headerVisible() )
        pos.setY( pos.y() - header()->height()  );
    KDGanttViewItem* gItem = (KDGanttViewItem*)itemAt( pos ) ;
    KDGanttViewItem* draggedItem = 0;
    if (  e->source() == myGanttView ) {
        //qDebug("internal drop ");
        draggedItem = myGanttView->myCanvasView->lastClickedItem;
    }
    if (myGanttView->lvDropEvent ( e, draggedItem, gItem ))
        return;
    QString string;
    KDGanttViewItemDrag::decode( e, string );
    KDGanttViewItem* newItem = 0;

    if ( gItem == myGanttView->myCanvasView->lastClickedItem && gItem != 0 ) {
        qDebug("KDGanttView::Possible bug in drag&drop code ");
        return;
    }

    QDomDocument doc( "GanttView" );
    doc.setContent( string );
    QDomElement docRoot = doc.documentElement(); // ChartParams element
    QDomNode node = docRoot.firstChild();
    bool enable = myGanttView->myTimeTable->blockUpdating( );
    myGanttView->myTimeTable->setBlockUpdating( true );
    while( !node.isNull() ) {
        QDomElement element = node.toElement();
        if( !element.isNull() ) { // was really an element
            QString tagName = element.tagName();
            if( tagName == "Items" ) {
                QDomNode node = element.firstChild();
                while( !node.isNull() ) {
                    QDomElement element = node.toElement();
                    if( !element.isNull() ) { // was really an element
                        QString tagName = element.tagName();
                        if( tagName == "Item" ) {
                            if (  gItem )
                                newItem = KDGanttViewItem::createFromDomElement( gItem,
                                                                                 element );
                            else
                                newItem = KDGanttViewItem::createFromDomElement( myGanttView,
                                                                                 element );
                        } else {
                            qDebug( "Unrecognized tag name: %s", tagName.toLatin1().constData() );
                            Q_ASSERT( false );
                        }
                    }
                    //qDebug("next node1 ");
                    node = node.nextSibling();
                }
            }
        }
        //qDebug("next node2 ");
        node = node.nextSibling();
    }
    if ( newItem ) {
        //newItem->setDisplaySubitemsAsGroup(myGanttView->displaySubitemsAsGroup());
        //newItem->resetSubitemVisibility();
        myGanttView->slot_lvDropped(e, draggedItem, gItem);
        myGanttView->myTimeTable->setBlockUpdating( enable );
        myGanttView->myTimeTable->updateMyContent();
    }
    return;
}

QDragObject * KDListView::dragObject ()
{
    return QListView::dragObject ();
}

void KDListView::startDrag ()
{
    if ( ! myGanttView->dragEnabled() )
        return;
    KDGanttViewItem* cItem = (KDGanttViewItem*) currentItem ();
    myGanttView->myCanvasView->lastClickedItem = cItem;
    myGanttView->lvStartDrag (cItem);
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


KDGanttCanvasView::KDGanttCanvasView( KDGanttView* sender,QCanvas* canvas, QWidget* parent,  const
    char* name ) : QCanvasView ( canvas, parent, name ),
    movingGVItem( 0 ),
    scrollBarTimer( 0 )
{
    setHScrollBarMode (QScrollView::AlwaysOn );
    setVScrollBarMode( QScrollView::AlwaysOn );
    myToolTip = new KDCanvasToolTip(viewport(),this);
    mySignalSender =  sender;
    currentItem = 0;
    currentLink = 0;
    cuttedItem = 0;
    mouseDown = false;
    currentConnector =  KDGanttViewItem::NoConnector;
    mConnectorStartEnabled = true;
    mConnectorMiddleEnabled = true;
    mConnectorEndEnabled = true;
    mConnectorMoveEnabled = true;
    mConnectorActualEndEnabled = true;
    mConnectorLeadEnabled = true;
    mConnectorTaskLinkStartEnabled = false;
    mConnectorTaskLinkEndEnabled = false;

    mTaskLinkFromItem = 0;
    mLinkLine = new QCanvasLine(canvas);
    mLinkLine->hide();
    mLinkLine->setZ(1000); // must be on top

    set_Mouse_Tracking(true); // mouse cursor changes over KDIntervalColorRectangle borders
    new KDCanvasWhatsThis(viewport(),this);
    onItem = new QMenu( this );
    QMenu *onView = onItem->addMenu( tr( "New Root" ) );
    onView->addAction( tr( "Event" ), this, SLOT( newRootEvent() ) );
    onView->addAction( tr( "Task" ), this, SLOT( newRootTask() ) );
    onView->addAction( tr( "Summary" ), this, SLOT( newRootSummary() ) );

    QMenu *newMenu = onItem->addMenu( tr( "New Child" ) );
    newMenu->addAction( tr( "Event" ), this, SLOT( newChildEvent() ) );
    newMenu->addAction( tr( "Task" ), this, SLOT( newChildTask() ) );
    newMenu->addAction( tr( "Summary" ), this, SLOT( newChildSummary() ) );

    QMenu *afterMenu = onItem->addMenu( tr( "New After" ) );
    afterMenu->addAction( tr( "Event" ), this, SLOT( newSiblingEvent() ) );
    afterMenu->addAction( tr( "Task" ), this, SLOT( newSiblingTask() ) );
    afterMenu->addAction( tr( "Summary" ), this, SLOT( newChildSummary() ) );


    QMenu *pasteMenu = onItem->addMenu( tr( "Paste" ) );
    pasteMenu->addAction( tr( "As Root" ), this, SLOT ( pasteItemRoot() ) );
    pasteMenu->addAction( tr( "As Child" ), this, SLOT ( pasteItemChild() ) );
    pasteMenu->addAction( tr( "After" ), this, SLOT ( pasteItemAfter() ) );
    mPasteAction = pasteMenu->menuAction();
    mPasteAction->setEnabled( false );

    onItem->addAction( tr( "Cut Item" ), this, SLOT ( cutItem() ) );
    myMyContentsHeight = 0;
    _showItemAddPopupMenu = false;

     QObject *scrollViewTimer = child( "scrollview scrollbar timer", "QTimer", false );
    Q_ASSERT( scrollViewTimer );
    if ( scrollViewTimer ) {
        disconnect( scrollViewTimer, SIGNAL(timeout()), this, SLOT(updateScrollBars() ) );
    }
    // If they needed a scrollbar timer in scrollview...
    connect( &scrollBarTimer, SIGNAL(timeout()), this, SLOT(myUpdateScrollBars() ) );
    scrollBarTimer.setSingleShot( true );

    mScrollbarTimer = new QTimer( this );
    connect( mScrollbarTimer, SIGNAL( timeout() ),
             this, SLOT( updateMyScrollBars() ) );
    myScrollBarMode = Auto;
    mScrollBarCheckCounter = 20;

    myScrollTimer = new QTimer( this );
    connect( myScrollTimer, SIGNAL( timeout() ), SLOT( slotScrollTimer() ) );
    autoScrollEnabled = false;
}


KDGanttCanvasView::~KDGanttCanvasView()
{
    delete myToolTip;
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
    //QScrollView::blockSignals( true );

    verticalScrollBar()->setUpdatesEnabled( false );
    // QScrollView::resizeEvent starts an update time for the scrollbars
    // we have to avoid this because we use our own timer
    // we do not call QScrollView::resizeEvent ( e ) ;
    QFrame::resizeEvent( e );
    // as in QScrollView::resizeEvent we call now QScrollView::updateScrollBars()
    // one time and later again via
    // setMyContentsHeight() - updateMyScrollBars()
    updateScrollBars();
    emit heightResized( viewport()->height());
    emit widthResized( viewport()->width() + verticalScrollBar()->width() );
    //setMyContentsHeight( 0 ); //via timer
    //QScrollView::blockSignals( false );
    scrollBarTimer.start(0);
}
void KDGanttCanvasView::setMyVScrollBarMode ( QScrollView::ScrollBarMode m )
{
    myScrollBarMode = m;
    resetScrollBars();
    mScrollBarCheckCounter = 0;
    QTimer::singleShot( 0, this, SLOT ( updateMyScrollBarsLater() ) );
}
QScrollView::ScrollBarMode KDGanttCanvasView::myVScrollBarMode () const
{
    return myScrollBarMode;
}
// we are watching the main event loop 20 times if someone else is resetting the
// max value of the verticalScrollBar()
void KDGanttCanvasView::updateMyScrollBarsLater()
{
    if ( mScrollBarCheckCounter < 20 ) {
        ++mScrollBarCheckCounter;
        QTimer::singleShot( 0, this, SLOT ( updateMyScrollBarsLater() ) );
    }
    if ( verticalScrollBar()->maximum () > myMyContentsHeight- viewport()->height()+1  ) {
        //qDebug("found!!!!!! %d ",mScrollBarCheckCounter );
        if ( mScrollBarCheckCounter < 20 ) {
            resetScrollBars();
            mScrollBarCheckCounter = 20;
        }
    }
}
void KDGanttCanvasView::updateMyScrollBars()
{
    // we call now QScrollView::updateScrollBars();
    updateScrollBars();
    // and now we reset the vertical scrollbar to the right range
    // and enable updating for the scrollbar again
    resetScrollBars();
}
void KDGanttCanvasView::resetScrollBars()
{
    verticalScrollBar()->setUpdatesEnabled( true );
    if ( viewport()->height() <= myMyContentsHeight ) {
        if ( myScrollBarMode == QScrollView::Auto ) {
            setVScrollBarMode( QScrollView::AlwaysOn );
            mySignalSender->timeHeaderSpacerWidget->setFixedWidth( verticalScrollBar()->width() );
        }
        //qDebug("set range %d %d %d  ",myMyContentsHeight, viewport()->height(),myMyContentsHeight- viewport()->height()+1 );
        verticalScrollBar()->setRange( 0, myMyContentsHeight- viewport()->height()+1);
    }
    else {
        if ( myScrollBarMode == QScrollView::Auto ) {
            mySignalSender->timeHeaderSpacerWidget->setFixedWidth( 0 );
            setVScrollBarMode( QScrollView::AlwaysOff );
        }
        verticalScrollBar()->setRange( 0,0 );
    }
    if (  myScrollBarMode == QScrollView::AlwaysOn ) {
        mySignalSender->timeHeaderSpacerWidget->setFixedWidth( verticalScrollBar()->width() );
        setVScrollBarMode( QScrollView::AlwaysOn );
        if ( viewport()->height() <= myMyContentsHeight )
            verticalScrollBar()->setRange( 0, myMyContentsHeight- viewport()->height()+1);
        else
            verticalScrollBar()->setRange( 0,0 );

    } else if (  myScrollBarMode == QScrollView::AlwaysOff ) {
        mySignalSender->timeHeaderSpacerWidget->setFixedWidth( 0 );
        setVScrollBarMode( QScrollView::AlwaysOff );
        if ( viewport()->height() <= myMyContentsHeight )
            verticalScrollBar()->setRange( 0, myMyContentsHeight- viewport()->height()+1);
        else
            verticalScrollBar()->setRange( 0,0 );
    }
    // testing for unmatching ScrollBar values of timeheader and timetable
    // may happen after external resizing
    if ( horizontalScrollBar()->value() != mySignalSender->myTimeHeaderScroll->horizontalScrollBar()->value() ) {
        // I am the Boss!
        mySignalSender->myTimeHeaderScroll->horizontalScrollBar()->setValue(horizontalScrollBar()->value()  );

    }
    if ( mScrollBarCheckCounter >= 20 ) {
        mScrollBarCheckCounter = 0;
        QTimer::singleShot( 0, this, SLOT ( updateMyScrollBarsLater() ) );
    }
    mySignalSender->closingBlocked = false;
}
void KDGanttCanvasView::myUpdateScrollBars()
{
    setMyContentsHeight( 0 );
}
void KDGanttCanvasView::setMyContentsHeight( int hei )
{
    //qDebug("setMyContentsHeight %d %d ", hei,  myMyContentsHeight);
    if ( hei > 0 )
        myMyContentsHeight = hei;
    verticalScrollBar()->setUpdatesEnabled( true ); // set false in resizeEvent()
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

// Call after *internal* resizing (like addTickRight())
// Then the new scrollbar maxValue is in myTimeHeader.
void KDGanttCanvasView::updateHorScrollBar() {
    //qDebug("horizontalScrollBar max=%d, myTimeHeaderScroll=%d", horizontalScrollBar()->maxValue(), mySignalSender->myTimeHeaderScroll->horizontalScrollBar()->value());

    horizontalScrollBar()->setRange(mySignalSender->myTimeHeaderScroll->horizontalScrollBar()->minimum(), mySignalSender->myTimeHeaderScroll->horizontalScrollBar()->maximum());

}

void  KDGanttCanvasView::cutItem( KDGanttViewItem* item )
{
    lastClickedItem = item;
    cutItem();
}
void  KDGanttCanvasView::insertItemAsRoot( KDGanttViewItem* item )
{
    mySignalSender->myListView->insertItem( item  );
    if ( item == cuttedItem )
        cuttedItem = 0;
}
void  KDGanttCanvasView::insertItemAsChild( KDGanttViewItem* parent, KDGanttViewItem* item )
{
    parent->insertItem( cuttedItem );
    if ( item == cuttedItem )
        cuttedItem = 0;
}
void  KDGanttCanvasView::insertItemAfter( KDGanttViewItem* parent , KDGanttViewItem* item )
{
    if ( parent->parent() ) {
        parent->parent()->insertItem( item );
    }
    else
        mySignalSender->myListView->insertItem( item );
    item->moveItem( parent );
    if ( item == cuttedItem )
        cuttedItem = 0;
}

void  KDGanttCanvasView::cutItem()
{
    lastClickedItem->hideSubtree();
    //qDebug("last clicked %d parent %d ", lastClickedItem  , lastClickedItem->parent());
    if ( lastClickedItem->parent() )
        lastClickedItem->parent()->takeItem(lastClickedItem);
    else
        mySignalSender->myListView->takeItem( lastClickedItem );
    mySignalSender->myTimeTable->updateMyContent();
    if ( cuttedItem )
        delete cuttedItem;
    cuttedItem = lastClickedItem;
    mPasteAction->setEnabled( true );

}
// called from the destructor in KDGanttViewItem or KDGanttView

void  KDGanttCanvasView::resetCutPaste( KDGanttViewItem* item )
{
    if ( item == 0 && cuttedItem ) {
        delete cuttedItem;
        cuttedItem = 0;
    }
    if (item == cuttedItem) {
        mPasteAction->setEnabled( false );
        cuttedItem = 0;
    }
}

void  KDGanttCanvasView::pasteItem( int type )
{
    if ( !cuttedItem )
        return;
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
    mPasteAction->setEnabled( false );
    mySignalSender->myTimeTable->updateMyContent();
}
void  KDGanttCanvasView::newRootItem(int type)
{
    //KDGanttViewItem::typeToString( (KDGanttViewItem::Type)type );
    KDGanttViewItem* temp = mySignalSender->createNewItem( KDGanttViewItem::typeToString( (KDGanttViewItem::Type)type ), 0, 0, "new " + KDGanttViewItem::typeToString( (KDGanttViewItem::Type)type ) );
    if ( temp )
        mySignalSender->editItem( temp );
}

void  KDGanttCanvasView::newChildItem( int type )
{
    int newType = type;
    QString newText = "new " + KDGanttViewItem::typeToString( (KDGanttViewItem::Type) newType );
    KDGanttViewItem* par = lastClickedItem;
    KDGanttViewItem* after = 0;
    KDGanttViewItem* temp = mySignalSender->createNewItem( KDGanttViewItem::typeToString( (KDGanttViewItem::Type)type ),  par, after, newText );
    if ( temp )
        mySignalSender->editItem( temp );
}

void  KDGanttCanvasView::newSiblingItem( int type )
{
    int newType = type;
    QString newText = "new " + KDGanttViewItem::typeToString( (KDGanttViewItem::Type) newType );
    KDGanttViewItem* par = lastClickedItem->parent();
    KDGanttViewItem* after = lastClickedItem;
    KDGanttViewItem* temp = mySignalSender->createNewItem( KDGanttViewItem::typeToString( (KDGanttViewItem::Type)type ),  par, after, newText );
    if ( temp )
        mySignalSender->editItem( temp );
}

void  KDGanttCanvasView::drawToPainter ( QPainter * p )
{
    drawContents ( p, 0, 0, canvas()->width(), mySignalSender->myTimeTable->minimumHeight() + 2 );
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


KDGanttCanvasView::MovingOperation KDGanttCanvasView::gvItemHitTest( KDGanttViewItem *item, KDTimeHeaderWidget* timeHeader, const QPoint &pos )
{
  const int left = timeHeader->getCoordX( item->startTime() );
  const int right = timeHeader->getCoordX( item->endTime() );
  const int width = right - left + 1;
  const int x = pos.x();
  if ( x < left + width / 10 )
    return KDGanttCanvasView::ResizingLeft;
  if ( x > right - width / 10 )
    return KDGanttCanvasView::ResizingRight;
  return KDGanttCanvasView::Moving;
}

/**
   Handles the mouseevent if a mousekey is pressed

   \param e the mouseevent

*/

void KDGanttCanvasView::contentsMousePressEvent ( QMouseEvent * e )
{
    //qDebug("mousepress! %d ", this);
    //qDebug("focus %d ",qApp->focusWidget());
    setFocus();
    currentLink = 0;
    currentItem = 0;
    movingItem = 0;
    mouseDown = true;
    currentItemChanged = false;
    currentConnector =  KDGanttViewItem::NoConnector;
    if (e->button() == Qt::RightButton && mySignalSender->editable()) {
        lastClickedItem = (KDGanttViewItem*) mySignalSender->getItemAt( QPoint(2,e->globalPos().y()), true );
        if ( lastClickedItem ) {
            if ( lastClickedItem->displaySubitemsAsGroup() && ! lastClickedItem->isOpen() ) {
                // find subitem
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
    KDGanttViewItem* testItem;
    QCanvasItemList il = canvas() ->collisions ( e->pos() );
    QCanvasItemList::Iterator it;
    for ( it = il.begin(); it != il.end(); ++it ) {
        switch ( e->button() ) {
        case Qt::LeftButton:
            switch (getType(*it)) {
            case Type_is_KDGanttViewItem:
                testItem = getItem(*it);
                if ( testItem->enabled() ) {
                    if ( !currentItem ) {
                        currentItem = testItem;
                        if ( mySignalSender->editable() )
                            currentConnector = currentItem->getConnector( e->pos() );
                    } else {
                        if ( testItem->priority() > currentItem->priority() )
                            currentItem = testItem;
                        if ( mySignalSender->editable() )
                            currentConnector = currentItem->getConnector( e->pos() );
                    }
                }
                {
                  KDCanvasRectangle *rect = dynamic_cast<KDCanvasRectangle*>( *it );
                  if ( rect ) {
                    movingGVItem = dynamic_cast<KDGanttViewTaskItem*>( getItem( rect ) );
                    if ( movingGVItem ) {
                      movingStart = e->pos();
                      movingStartDate = movingGVItem->startTime();
                      movingOperation = gvItemHitTest( movingGVItem, mySignalSender->myTimeHeader, e->pos() );
                      if ( movingOperation == Moving && !movingGVItem->isMoveable() )
                        movingGVItem = 0;
                      else if ( movingOperation != Moving && !movingGVItem->isResizeable() )
                        movingOperation = Moving;
                    } else {
                      movingGVItem = 0;
                    }
                  }
                }
                break;
            case Type_is_KDGanttTaskLink:
                currentLink = getLink(*it);
                break;
            case Type_is_KDGanttGridItem:
              if ( (*it)->rtti() == KDIntervalColorRectangle::RTTI ) {
                // Cleaner would be isMovable()/isResizeable() in an interface
                // implemented by all movable objects...
                movingItem = static_cast<QCanvasRectangle *>(*it);
                movingStart = e->pos();
                KDIntervalColorRectangle* icr = static_cast<KDIntervalColorRectangle *>( movingItem );
                KDIntervalColorRectangle::HitTest hitTest = icr->hitTest( mySignalSender->myTimeHeader, movingStart );
                movingOperation = hitTest == KDIntervalColorRectangle::Start ? ResizingLeft :
                                  hitTest == KDIntervalColorRectangle::End ? ResizingRight :
                                  Moving;
              }
              break;
            default:
                break;
            }
            break;
        case Qt::RightButton:
            switch (getType(*it)) {
            case Type_is_KDGanttViewItem:
                testItem = getItem(*it);
                if ( testItem->enabled() ) {
                    if ( !currentItem ) {
                        currentItem = testItem;
                    } else {
                        if ( testItem->priority() > currentItem->priority() )
                            currentItem = testItem;
                    }
                }
                break;
            case Type_is_KDGanttTaskLink:
                currentLink = getLink(*it);
                break;
            }
            break;
        case Qt::MidButton:
            switch (getType(*it)) {
            case Type_is_KDGanttViewItem:
                testItem = getItem(*it);
                if ( testItem->enabled() ) {
                    if ( !currentItem ) {
                        currentItem = testItem;
                    } else {
                        if ( testItem->priority() > currentItem->priority() )
                            currentItem = testItem;
                    }
                }
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
    if (e->button() == Qt::RightButton ) {
        mySignalSender->gvContextMenuRequested( currentItem, e->globalPos() );
    }
    //qDebug("Connector %d ", currentConnector);

    switch( currentConnector ) {
    case KDGanttViewItem::Start:
    case KDGanttViewItem::End:
    case KDGanttViewItem::Middle:
    case KDGanttViewItem::ActualEnd:
    case KDGanttViewItem::Lead:
        viewport()->setCursor(Qt::sizeHorCursor);
        break;
    case KDGanttViewItem::Move:
        viewport()->setCursor(Qt::sizeAllCursor);
        break;
    case KDGanttViewItem::TaskLinkStart:
    case KDGanttViewItem::TaskLinkEnd:
        viewport()->setCursor(Qt::sizeVerCursor);
        switch ( e->button() ) {
            case Qt::LeftButton:
                if ( currentItem ) {
                    mTaskLinkFromItem = currentItem;
                    mLinkLine->setPoints(e->pos().x(), e->pos().y(), e->pos().x(), e->pos().y());
                    mLinkLine->show();
                }
                break;
            case Qt::MidButton:
                break;
            case Qt::RightButton:
                break;
            default:
                break;
        }
        break;
    default:
        break;
    }
    mySignalSender->gvMouseButtonPressed( e->button(), currentItem ,  e->globalPos() );
    mButtonDownTime.start();

    if (autoScrollEnabled && e->button() == Qt::LeftButton) {
        myScrollTimer->start(50);
    }

}
/**
   Handles the mouseevent if a mousekey is released

   \param e the mouseevent

*/

void KDGanttCanvasView::contentsMouseReleaseEvent ( QMouseEvent * e )
{
    mouseDown = false;
    viewport()->unsetCursor();
    static KDGanttViewItem* lastClicked = 0;
    mySignalSender->gvMouseButtonClicked( e->button(), currentItem ,  e->globalPos() );
    mySignalSender->gvMouseButtonReleased( e->button(), currentItem ,  e->globalPos() );
    //qDebug("datetime %s ",mySignalSender->getDateTimeForCoordX(e->globalPos().x(), true ).toString().toLatin1() );
    //qDebug("mousepos %d %d ",e->pos().x(),e->pos().y() );
    //qDebug("mouseup ");
    // if ( currentLink || currentItem )
    {
        switch ( e->button() ) {
        case Qt::LeftButton:
            {
                mySignalSender->itemLeftClicked( currentItem );
                mySignalSender->gvItemLeftClicked( currentItem );
            }
            myScrollTimer->stop();
            if ( currentLink )
                mySignalSender->taskLinkLeftClicked( currentLink );
            if (mTaskLinkFromItem) {
                mLinkLine->hide();
                canvas()->update();
                KDGanttViewItem *testItem = 0, *toItem = 0;
                QCanvasItemList il = canvas() ->collisions ( e->pos() );
                QCanvasItemList::Iterator it;
                for ( it = il.begin(); it != il.end(); ++it ) {
                    if ( getType(*it) == Type_is_KDGanttViewItem ) {
                        testItem = getItem(*it);
                        if ( toItem == 0) {
                            toItem = testItem;
                        } else if ( testItem->priority() > toItem->priority() ) {
                            toItem = testItem;
                        }
                    }
                }
                qDebug("toItem=%p",toItem);
                if ( toItem && toItem != mTaskLinkFromItem ) {
                    mySignalSender->gvCreateTaskLink( mTaskLinkFromItem, currentConnector, toItem, toItem->getConnector( e->pos(), true ) );
                }
            }
            if ( movingGVItem ) {
              mySignalSender->gvItemMoved( movingGVItem );
              movingGVItem = 0;
            }
            break;
        case Qt::RightButton:
            {
                mySignalSender->itemRightClicked( currentItem );
                mySignalSender->gvItemRightClicked( currentItem );

            }
            if ( currentLink )
                mySignalSender->taskLinkRightClicked( currentLink );
            break;
        case Qt::MidButton:
            {
                mySignalSender->itemMidClicked( currentItem );
                mySignalSender->gvItemMidClicked( currentItem );
            }
            if ( currentLink )
                mySignalSender->taskLinkMidClicked( currentLink );
            break;
        default:
            break;
        }
    }
    if ( lastClicked != currentItem )
        mySignalSender->gvCurrentChanged( currentItem );
    lastClicked = currentItem;
    if ( currentItem && currentItemChanged ) {
        emit mySignalSender->itemChanged( currentItem );
    }
    currentLink = 0;
    currentItem = 0;
    currentConnector =  KDGanttViewItem::NoConnector;
    mTaskLinkFromItem = 0;
}
/**
   Handles the mouseevent if a mousekey is doubleclicked

   \param e the mouseevent

*/

void KDGanttCanvasView::contentsMouseDoubleClickEvent ( QMouseEvent * e )
{
    QCanvasItemList il = canvas() ->collisions ( e->pos() );

    if ( il.isEmpty() && e->button() == Qt::LeftButton ) {
        //not directly sending a signal here (encapsulation and whatnot)
        mySignalSender->emptySpaceDoubleClicked( e );
        return;
    }

    QCanvasItemList::Iterator it;
    for ( it = il.begin(); it != il.end(); ++it ) {
        switch ( e->button() ) {
        case Qt::LeftButton:
            switch (getType(*it)) {
            case Type_is_KDGanttViewItem:
                {
                    KDGanttViewItem *clickItem = getItem(*it);
                    if ( clickItem->enabled() )
                        mySignalSender->itemDoubleClicked( clickItem );
                    mySignalSender->gvItemDoubleClicked( clickItem );
                    return;
                }
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
              case Qt::RightButton:
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
              case Qt::MidButton:
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
   Handles the mouseevent if a mouse button is pressed an the mouse is moved

   \param e the mouseevent

*/

void KDGanttCanvasView::contentsMouseMoveEvent ( QMouseEvent * e )
{
    if ( !mouseDown ) {
      // Update cursor
      bool found = false;
      QCanvasItemList il = canvas() ->collisions ( e->pos() );
      QCanvasItemList::Iterator it;
      for ( it = il.begin(); it != il.end(); ++it ) {
        if ( (*it)->rtti() == KDIntervalColorRectangle::RTTI ) {
          found = true;
          KDIntervalColorRectangle* icr = static_cast<KDIntervalColorRectangle *>( *it );
          KDIntervalColorRectangle::HitTest hitTest = icr->hitTest( mySignalSender->myTimeHeader, e->pos() );
          switch ( hitTest ) {
          case KDIntervalColorRectangle::Start:
          case KDIntervalColorRectangle::End:
            setCursor( Qt::SplitHCursor );
            break;
          default:
            unsetCursor();
          }
        }
        KDGanttViewItem *gvItem = getItem( *it );
        if ( dynamic_cast<KDGanttViewTaskItem*>( gvItem ) ) {
          found = true;
          MovingOperation op = gvItemHitTest( gvItem, mySignalSender->myTimeHeader, e->pos() );
          switch ( op ) {
            case ResizingLeft:
            case ResizingRight:
              if ( gvItem->isResizeable() )
                setCursor( Qt::SplitHCursor );
              break;
            default:
              unsetCursor();
          }
        }
      }
      if ( !found )
        unsetCursor();
      return;
    }

    const QPoint p = e->pos();
    if ( movingItem ) {
      int x = qRound( movingItem->x() );
      int width = movingItem->width();
      switch( movingOperation ) {
      case Moving:
        x += p.x() - movingStart.x();
        break;
      case ResizingLeft: {
        width = qRound( movingItem->x() + movingItem->width() - p.x() );
        x = p.x();
        break;
      }
      case ResizingRight:
        width = p.x() - x;
        break;
      }
      movingStart = p;
      if ( movingItem->rtti() == KDIntervalColorRectangle::RTTI ) {
        KDIntervalColorRectangle* icr = static_cast<KDIntervalColorRectangle *>(movingItem);
        const QDateTime newStart = mySignalSender->myTimeHeader->getDateTimeForIndex(x);
        const QDateTime newEnd = mySignalSender->myTimeHeader->getDateTimeForIndex(x + width);
        icr->setDateTimes( newStart, newEnd );
        emit mySignalSender->intervalColorRectangleMoved( newStart, newEnd );
        mySignalSender->myTimeHeader->computeIntervals( movingItem->height() );
      }
      canvas()->update();
    }

    //qDebug("mousemove! ");
    mySignalSender->gvMouseMove( e->button(), currentItem ,  e->globalPos() );
    if ( movingGVItem ) {
      int dx = movingStart.x() - e->pos().x();
      int x = movingGVItem->middleLeft().x() - dx;
      QDateTime dt = mySignalSender->getDateTimeForCoordX( x, false );
      int duration = movingGVItem->startTime().secsTo( movingGVItem->endTime() );
      if ( movingOperation == Moving ) {
        movingGVItem->setStartTime( dt );
        movingGVItem->setEndTime( dt.addSecs( duration ) );
      } else if ( movingOperation == ResizingLeft ) {
        movingGVItem->setStartTime( dt );
      } else if ( movingOperation == ResizingRight ) {
        movingGVItem->setEndTime( dt.addSecs( duration ) );
      }
      movingStart = e->pos();
    }

    static int moves = 0;
    if ( (currentLink || currentItem) && (moves < 3) ) {
        ++moves;
    } else {
        moves = 0;
        currentLink = 0;
        currentItem = 0;
    }
    if (autoScrollEnabled)
        mousePos = e->pos()- QPoint(contentsX(),contentsY()); // make mousePos relative 0
    if ( !currentItem && mySignalSender->editable() ) {
        KDGanttViewItem* testItem, *foundItem = 0;
        KDIntervalColorRectangle* foundIcr = 0;
        QCanvasItemList il = canvas() ->collisions ( e->pos() );
        QCanvasItemList::Iterator it;
        for ( it = il.begin(); it != il.end(); ++it ) {
            if ( getType(*it) == Type_is_KDGanttViewItem ) {
                testItem = getItem(*it);
                if ( testItem->enabled() ) {
                    if ( !foundItem ) {
                        foundItem = testItem;
                    } else {
                        if ( testItem->priority() > foundItem->priority() )
                            foundItem = testItem;
                    }
                }
            } else if ( (*it)->rtti() == KDIntervalColorRectangle::RTTI ) {
              foundIcr = static_cast<KDIntervalColorRectangle *>(*it);
            }
        }
        if ( foundItem ) {
            KDGanttViewItem::Connector connector = foundItem->getConnector( e->pos() );
            //qDebug("FOUNDITEM connector %d",connector );
            switch( connector ) {
            case KDGanttViewItem::Start:
            case KDGanttViewItem::End:
            case KDGanttViewItem::Middle:
            case KDGanttViewItem::ActualEnd:
            case KDGanttViewItem::Lead:
                viewport()->setCursor(Qt::sizeHorCursor);
                break;
            case KDGanttViewItem::Move:
                viewport()->setCursor(Qt::sizeAllCursor);
                break;
                case KDGanttViewItem::TaskLinkStart:
                case KDGanttViewItem::TaskLinkEnd:
                    viewport()->setCursor(Qt::sizeVerCursor);
                break;
            default:
                viewport()->unsetCursor();
                break;
            }
        } else if ( foundIcr ) {
          KDIntervalColorRectangle::HitTest hitTest = foundIcr->hitTest( mySignalSender->myTimeHeader, e->pos() );
          switch ( hitTest ) {
          case KDIntervalColorRectangle::Start:
          case KDIntervalColorRectangle::End:
            viewport()->setCursor( Qt::sizeHorCursor );
            break;
          default:
            viewport()->unsetCursor();
          }
        } else {
            viewport()->unsetCursor();
        }
        return;
    }
    if ( !currentItem )
        return;
    if ( currentConnector == KDGanttViewItem::TaskLinkStart || currentConnector == KDGanttViewItem::TaskLinkEnd) {
        if (mTaskLinkFromItem) {
            mLinkLine->setPoints(mLinkLine->startPoint().x(), mLinkLine->startPoint().y(), e->pos().x(), e->pos().y());
            canvas()->update();
        }
        return;
    }
    if ( mButtonDownTime.elapsed() > 200 )
        {
        if ( currentConnector != KDGanttViewItem::NoConnector && currentItem ) {
            bool result = currentItem->moveConnector( currentConnector, e->pos() );
            if ( result ) {
                emit  mySignalSender->itemConfigured( currentItem );
                currentItemChanged = true;
            }
        }
    }
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
    case QCanvasItem::Rtti_Rectangle:
    case KDIntervalColorRectangle::RTTI:
      return ((KDCanvasRectangle *)it)->myParentType;
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

/*
  Enable or disable a connector.
 */
void KDGanttCanvasView::setConnectorEnabled(int connector, bool on)
{
    switch (connector) {
        case KDGanttViewItem::Start:
            mConnectorStartEnabled = on;
            break;
        case KDGanttViewItem::Middle:
            mConnectorMiddleEnabled = on;
            break;
        case KDGanttViewItem::End:
            mConnectorEndEnabled = on;
            break;
        case KDGanttViewItem::Move:
            mConnectorMoveEnabled = on;
            break;
        case KDGanttViewItem::ActualEnd:
            mConnectorActualEndEnabled = on;
            break;
        case KDGanttViewItem::Lead:
            mConnectorLeadEnabled = on;
            break;
        case KDGanttViewItem::TaskLinkStart:
            mConnectorTaskLinkStartEnabled = on;
            break;
        case KDGanttViewItem::TaskLinkEnd:
            mConnectorTaskLinkEndEnabled = on;
            break;
        default:
            qDebug("setConnectorEnabled: Unknown connector type");
    }
}

/*
  See if a connector is enabled or disabled.
 */
bool KDGanttCanvasView::isConnectorEnabled(int connector) const
{
    switch (connector) {
        case KDGanttViewItem::Start:
            return mConnectorStartEnabled;
            break;
        case KDGanttViewItem::Middle:
            return mConnectorMiddleEnabled;
            break;
        case KDGanttViewItem::End:
            return mConnectorEndEnabled;
            break;
        case KDGanttViewItem::Move:
            return mConnectorMoveEnabled;
            break;
        case KDGanttViewItem::ActualEnd:
            return mConnectorActualEndEnabled;
            break;
        case KDGanttViewItem::Lead:
            return mConnectorLeadEnabled;
            break;
        case KDGanttViewItem::TaskLinkStart:
            return mConnectorTaskLinkStartEnabled;
            break;
        case KDGanttViewItem::TaskLinkEnd:
            return mConnectorTaskLinkEndEnabled;
            break;
        default:
            qDebug("isConnectorEnabled: Unknown connector type");
    }
    return false;
}

/*
  Enable or disable all connectors.
 */
void KDGanttCanvasView::setAllConnectorsEnabled(bool on)
{
    mConnectorStartEnabled = on;
    mConnectorMiddleEnabled = on;
    mConnectorEndEnabled = on;
    mConnectorMoveEnabled = on;
    mConnectorActualEndEnabled = on;
    mConnectorLeadEnabled = on;
    mConnectorTaskLinkStartEnabled = on;
    mConnectorTaskLinkEndEnabled = on;
}

void KDGanttCanvasView::slotScrollTimer() {
    int mx = mousePos.x();
    int my = mousePos.y();
    int dx = 0;
    int dy = 0;
    if (mx < 0)
        dx = -5;
    else if (mx > visibleWidth())
        dx = 5;
    if (my < 0)
        dy = -5;
    else if (my > visibleHeight())
        dy = qMin(5, verticalScrollBar()->maximum() - verticalScrollBar()->value()
);

    if (dx != 0 || dy != 0)
        scrollBy(dx, dy);
}

/*!
  Represents the background color for a given interval of time (across all tasks).
  \sa KDGanttView::addIntervalBackgroundColor
  \param view parent view
 */
KDIntervalColorRectangle::KDIntervalColorRectangle( KDGanttView* view )
  : KDCanvasRectangle( view->timeTableWidget(), 0, Type_is_KDGanttGridItem ),
  mStart(), mEnd()
{
  setZ( -19 );
}

/*!
  \param start start datetime of the time interval
  \param end end datetime of the time interval
 */
void KDIntervalColorRectangle::setDateTimes( const QDateTime& start,
                                             const QDateTime& end )
{
  mStart = start;
  mEnd = end;
  if ( mEnd < mStart )
    qSwap( mStart, mEnd );
}

/*!
  Sets the background color
  \param color the background color
*/
void KDIntervalColorRectangle::setColor( const QColor& color )
{
  mColor = color;
}

/*!
  \internal
*/
void KDIntervalColorRectangle::layout( KDTimeHeaderWidget* timeHeader, int height )
{
  int left = timeHeader->getCoordX(mStart);
  int right = timeHeader->getCoordX(mEnd);
  if ( right == left )
    ++right;
  setPen( Qt::NoPen );
  setBrush( QBrush(mColor) );
  setSize( right - left, height );
  move( left, 0 );
  show();
}

/*!
  \internal
*/
KDIntervalColorRectangle::HitTest KDIntervalColorRectangle::hitTest( KDTimeHeaderWidget* timeHeader, const QPoint& pos ) const
{
  const int left = timeHeader->getCoordX(mStart);
  const int right = timeHeader->getCoordX(mEnd);
  const int width = right - left + 1;
  const int x = pos.x();
  if ( x < left + width / 10 )
    return Start;
  if ( x > right - width / 10 )
    return End;
  return Middle;
}
