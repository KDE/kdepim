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

#include "KDGanttView.h"
#include "KDGanttViewSubwidgets.h"
#include "KDGanttViewItem.h"
#include "KDXMLTools.h"
#include "itemAttributeDialog.h"
#include <qprinter.h>
#include <qpainter.h>
#include <qlayout.h>
#include <qpaintdevicemetrics.h>
#include <qfile.h>
#include <qheader.h>
#include <qscrollview.h>
#include <qapplication.h>

#ifndef KDGANTT_MASTER_CVS
#include "KDGanttView.moc"
#endif

/*!
  \class KDGanttView KDGanttView.h
  This class represents a Gantt view with the Gantt chart, the header,
  an optional listview and an optional legend.
  If there are problem with the repainting of the content of the
  Gantt View after scrolling call \a setRepaintMode().

  In order to set up a Gantt view, create an object of this class, as
  well as a number of \a KDGanttViewItem objects.
*/

/*!
  Constructs an empty KDGanttView.

  \param parent the widget parent
  \param name the internal debugging name
*/

KDGanttView::KDGanttView( QWidget* parent, const char* name  ) : KDMinimizeSplitter( Qt::Vertical, parent, name )
{
    setMinimizeDirection ( KDMinimizeSplitter::Down );
    KDMinimizeSplitter *mySplitter = new KDMinimizeSplitter( this );
    mySplitter->setMinimizeDirection ( KDMinimizeSplitter::Left );
    leftWidget = new QVBox( mySplitter );
    rightWidget = new QVBox( mySplitter );

    myLegend = new KDLegendWidget( leftWidget, this );
    spacerLeft = new QWidget( leftWidget );
    myListView = new KDListView(leftWidget, this);

    connect( myListView, SIGNAL( selectionChanged( QListViewItem* ) ),
             this, SLOT( slotSelectionChanged( QListViewItem* ) ) );
    connect( myListView, SIGNAL( mouseButtonClicked ( int, QListViewItem * , const QPoint &, int ) ), this, SLOT( slotmouseButtonClicked ( int , QListViewItem * , const QPoint &, int ) ) );
    connect( myListView, SIGNAL( contextMenuRequested ( QListViewItem * , const QPoint &, int  ) ), this, SLOT( slotcontextMenuRequested ( QListViewItem * , const QPoint & , int ) ) );
    connect( myListView, SIGNAL(doubleClicked ( QListViewItem *  ) ), this, SLOT(slotdoubleClicked ( QListViewItem * ) ) );

  connect( myListView, SIGNAL(currentChanged( QListViewItem *  ) ), this, SLOT(slotCurrentChanged ( QListViewItem * ) ) );
  connect( myListView, SIGNAL(itemRenamed ( QListViewItem * , int , const QString &  ) ), this, SLOT(slotItemRenamed ( QListViewItem *, int , const QString &  ) ) );
  connect( myListView, SIGNAL(mouseButtonPressed(  int, QListViewItem * , const QPoint &, int ) ), this, SLOT(slotMouseButtonPressed (  int , QListViewItem * , const QPoint & , int ) ) );

    //connect( myListView, SIGNAL( ), this, SLOT( ) );
    myTimeTable = new KDTimeTableWidget (0,this);

    spacerRight = new QWidget(  rightWidget );
    myTimeHeaderScroll = new QScrollView ( rightWidget );
    myTimeHeaderScroll->setHScrollBarMode( QScrollView::AlwaysOff );
    myTimeHeaderScroll->setVScrollBarMode( QScrollView::AlwaysOn );

    //myTimeHeader = new KDTimeHeaderWidget (rightWidget,this);
    myTimeHeader = new KDTimeHeaderWidget (myTimeHeaderScroll->viewport(),this);
    myTimeHeaderScroll->addChild( myTimeHeader );
    myTimeHeaderScroll->viewport()->setBackgroundColor( myTimeHeader->backgroundColor() );
    myCanvasView = new KDGanttCanvasView (this,myTimeTable,rightWidget);
    myTimeHeaderScroll->setFrameStyle( QFrame::NoFrame  );
    myTimeHeaderScroll->setMargin( myCanvasView->frameWidth() );
    setFrameStyle(myListView->frameStyle());
    setLineWidth( 2 );
    QObject::connect(myListView, SIGNAL (  expanded ( QListViewItem * ) ) , myTimeTable , SLOT( expandItem(QListViewItem * ))) ;
    QObject::connect(myListView, SIGNAL (collapsed ( QListViewItem * ) ) , myTimeTable , SLOT(collapseItem(QListViewItem * ))) ;

    listViewIsVisible = true;
    chartIsEditable = true;
    editorIsEnabled = true;
    _displaySubitemsAsGroup = false;
    initDefaults();

    myTextColor = Qt::black;
    myLegendItems = new QPtrList<legendItem>;
    //QObject::connect( this, SIGNAL (itemDoubleClicked( KDGanttViewItem* ) ) , this, SLOT( editItem( KDGanttViewItem*  ))) ;
    myItemAttributeDialog = new itemAttributeDialog();
    setRepaintMode( KDGanttView::Medium );
    //setRepaintMode( KDGanttView::Always );
    setShowLegendButton( true );
    setHeaderVisible( false );

    // now connecting the widgets
    connect(myCanvasView->horizontalScrollBar(), SIGNAL (  valueChanged ( int )) ,myTimeHeaderScroll->horizontalScrollBar(), SLOT( setValue ( int))) ;
    connect(myCanvasView, SIGNAL (  heightResized( int )) ,myTimeTable, SLOT( checkHeight ( int))) ;
    connect(myCanvasView, SIGNAL (  widthResized( int )) ,myTimeHeader, SLOT( checkWidth ( int))) ;

    QObject::connect(myCanvasView->verticalScrollBar(), SIGNAL ( valueChanged ( int ) ) ,myListView->verticalScrollBar(), SLOT( setValue ( int ))) ;
    connect(myTimeHeader, SIGNAL ( sizeChanged( int ) ) ,this, SLOT(slotHeaderSizeChanged()  )) ;
    connect(myTimeHeader, SIGNAL ( sizeChanged( int ) ) ,myTimeTable, SLOT(resetWidth( int ) )) ;
    connect(myListView, SIGNAL ( contentsMoving ( int, int ) ) ,myCanvasView, SLOT(  moveMyContent( int, int ))) ;
   connect(myTimeTable, SIGNAL ( heightComputed ( int ) ) ,myCanvasView, SLOT(  setMyContentsHeight( int ))) ;
   // the next three are for adding new ticks at left/right
    connect( myCanvasView->horizontalScrollBar(), SIGNAL (prevLine () ) ,this, SLOT(addTickLeft()));
    connect( myCanvasView->horizontalScrollBar(), SIGNAL (nextLine () ) ,this, SLOT(addTickRight()));
    connect( myCanvasView->horizontalScrollBar(), SIGNAL (valueChanged ( int ) ) ,this, SLOT( enableAdding( int )));

   // now initing
    fCenterTimeLineAfterShow = false;
   myTimeHeader->computeTicks();
   centerTimelineAfterShow( QDateTime::currentDateTime () );
   myTimeTable->setBlockUpdating();// block updating until this->show() is called
}



KDGanttView::~KDGanttView()
{
  // delete cutted Item, if there is any
  myCanvasView->resetCutPaste( 0 );
}
/*!
  Sets the updating of the content of the gantt view.
  Call setUpdateEnabled( false ) before inserting large amounts of gantt items
  to avoid flickering of the gantt view.
  After inserting, call  setUpdateEnabled( true ) to enable updating.
  With calling setUpdateEnabled( true ),
  all the content is recomputed, resized and updated.

  Before calling show() for the first time, update is disabled.
  With calling show() update is automatically enabled.

  \param enable if true, the content of the gantt view is updated after
  every inserting of a new item.
*/
void KDGanttView::setUpdateEnabled( bool enable )
{
  myTimeTable->setBlockUpdating( !enable );
  if ( enable ) {
    myTimeTable->updateMyContent();
    myCanvasView->setMyContentsHeight( 0 );
  }
}

bool KDGanttView::getUpdateEnabled(  ) const
{
  return !myTimeTable->blockUpdating();
}



/*!
  Updates the  content of the GanttView and shows it.
  Automatically sets setUpdateEnabled( true ).
  \sa setUpdateEnabled()
*/
void KDGanttView::show()
{
  myTimeTable->setBlockUpdating( false );
  if (myCanvasView->horizontalScrollBar()->value() > 0 )
    myCanvasView->horizontalScrollBar()->setValue(myCanvasView->horizontalScrollBar()->value()-1  );
  else
    myCanvasView->horizontalScrollBar()->setValue(1 );
  myTimeTable->updateMyContent();
  QWidget::show();
  myCanvasView->setMyContentsHeight( 0 );
  if ( fCenterTimeLineAfterShow ) {
    fCenterTimeLineAfterShow = false;
    centerTimeline (dtCenterTimeLineAfterShow);
  }
}
/*!
  Returns a useful size.
  Returned width:
  Sizehint.width of ListView + width of TimeTable
  Returned height:
  Height of TimeHeader + height of TimeTable + height of Legend(if shown)
*/

QSize KDGanttView::sizeHint()
{
  bool block = myTimeTable->blockUpdating();
  myTimeTable->setBlockUpdating( false );
  myTimeTable->updateMyContent();
  qApp->processEvents();
  int hintHeight = myTimeHeader->height();
  int legendHeight = 0;
  if ( showLegendButton() )
    legendHeight = myLegend->height();
  int listViewHeaderHeight = 0;
  if ( headerVisible() )
    listViewHeaderHeight = myListView->header()->height();
  if ( hintHeight < legendHeight+listViewHeaderHeight )
    hintHeight = legendHeight + listViewHeaderHeight;
  hintHeight += myListView->horizontalScrollBar()->height();
  if ( myLegend->isShown() )
    hintHeight += myLegend->legendSizeHint().height() +10;
  hintHeight += myTimeTable->minimumHeight+myListView->frameWidth()*2+2;
  int hintWid = myListView->sizeHint().width();
  //hintWid += myTimeHeader->mySizeHint+myCanvasView->verticalScrollBar()->width();
  hintWid += myCanvasView->sizeHint().width();
  // add 10 for the splitter-bars
  // qDebug("sizehint %d %d ",hintWid+10, hintHeight );
  myTimeTable->setBlockUpdating( block );
  return QSize( hintWid+10, hintHeight );
}


/*!
  Specifies whether the legend button should be visible. By default,
  it is visible.

  \param true to show the legend button, false to hide it
*/
void KDGanttView::setShowLegendButton( bool show )
{
  _showLegendButton = show;
     if ( show )
         myLegend->show();
    else
         myLegend->hide();
     slotHeaderSizeChanged();
}


/*!
  Returns whether the legend button is visible.

  \return whether the legend button is visible
*/
bool KDGanttView::showLegendButton() const
{
    return _showLegendButton;
}


/*!
  Specifies whether the listview header should be visible. By default,
  it is not visible.

  \param visible true to make the header visible, false to make it invisible
*/
void KDGanttView::setHeaderVisible( bool visible )
{
    if( visible )
        myListView->header()->show();
    else
        myListView->header()->hide();
    _showHeader = visible;
    slotHeaderSizeChanged();
}


/*!
  Returns whether the listview header is be visible.

  \return whether the header is visible
*/
bool KDGanttView::headerVisible() const
{
  return _showHeader;
}
/*
  Returns the corresponding datetime of the coordinate X in the gantt view.
*/
QDateTime KDGanttView::getDateTimeForCoordX(int coordX)
{
   return myTimeHeader->getDateTimeForIndex(coordX);
}
/*
  Implements a casted pass-through of the selectionChanged() signal.
*/
void KDGanttView::slotSelectionChanged( QListViewItem* item )
{
    KDGanttViewItem* gItem = static_cast<KDGanttViewItem*>( item );
    Q_ASSERT( gItem );
    emit lvSelectionChanged( gItem );
}
/*
  Implements a casted pass-through of the mouseButtonClicked() signal.
  Signals itemLeftClicked() , itemMidClicked() are emitted as well.
*/
void KDGanttView::slotmouseButtonClicked ( int button, QListViewItem * item, const QPoint & pos, int c )
{
  KDGanttViewItem* gItem = static_cast<KDGanttViewItem*>( item );
  emit lvMouseButtonClicked ( button , gItem,  pos,  c );
  emit mouseButtonClicked ( button , gItem,  pos,  c );
   {
    switch ( button ) {
    case  LeftButton:
      emit lvItemLeftClicked( gItem );
      emit itemLeftClicked( gItem );
      break;
    case  MidButton:
      emit lvItemMidClicked( gItem );
      emit itemMidClicked( gItem );
      break;
    }
  }
}
/*
  Implements a casted pass-through of the contextMenuRequested () signal.
  The signal itemRightClicked() is emitted as well
*/
void KDGanttView::slotcontextMenuRequested ( QListViewItem * item, const QPoint & pos, int col )
{
    KDGanttViewItem* gItem = static_cast<KDGanttViewItem*>( item );
    emit lvContextMenuRequested ( gItem,  pos,  col );
     {
      emit lvItemRightClicked( gItem );
      emit itemRightClicked( gItem );
    }
}
/*
  Implements a casted pass-through of the doubleClicked() signal.
*/
void KDGanttView::slotdoubleClicked ( QListViewItem * item )
{
   {
    KDGanttViewItem* gItem = static_cast<KDGanttViewItem*>( item );
    emit lvItemDoubleClicked( gItem );
    emit itemDoubleClicked( gItem );
  }
}
/*
  Implements a casted pass-through of the currentChanged signal.
*/
void KDGanttView::slotCurrentChanged ( QListViewItem * item )
{
    KDGanttViewItem* gItem = static_cast<KDGanttViewItem*>( item );
    emit lvCurrentChanged( gItem );
}
/*
  Implements a casted pass-through of the itemRenamed signal.
*/
void KDGanttView::slotItemRenamed ( QListViewItem * item , int col, const QString & text )
{
    KDGanttViewItem* gItem = static_cast<KDGanttViewItem*>( item );
    emit lvItemRenamed( gItem,  col, text );
}
/*
  Implements a casted pass-through of the mouseButtonPressed signal.
*/
void KDGanttView::slotMouseButtonPressed ( int button, QListViewItem * item, const QPoint & pos, int c )
{
    KDGanttViewItem* gItem = static_cast<KDGanttViewItem*>( item );
    emit lvMouseButtonPressed( button, gItem,  pos,  c  );
}

/*!
  Specifies whether the content should be repainted after scrolling

  \param mode If No, there is no repainting after scrolling. Fastest mode.
              If Medium, there is extra repainting after releasing the scrollbar.
	      This delivers fast scrolling with updated content
	      after scrolling. Recommended, when repaint problems occour.
	      This is the default value after startup.
	      If Always, there is extra update after every moving of the
	      scrollbar. This delivers slow scrolling with updated
	      content at all time.
*/
void KDGanttView::setRepaintMode( RepaintMode mode )
{

  QScrollBar  *cvh, *cvv;
  cvh = myCanvasView->horizontalScrollBar();
  cvv = myCanvasView->verticalScrollBar();
  // first disconnect
  cvh->disconnect( this );
  cvv->disconnect( this );

  switch ( mode ) {
  case  No:

    break;
  case Medium:
    connect( cvv, SIGNAL (sliderReleased () ) ,this, SLOT(forceRepaint()));
    connect( cvh, SIGNAL (sliderReleased () ) ,this, SLOT(forceRepaint()));
    connect( cvv, SIGNAL (nextLine () ) ,this, SLOT(forceRepaint()));
    connect( cvh, SIGNAL (nextLine () ) ,this, SLOT(forceRepaint()));
    connect( cvv, SIGNAL (prevLine () ) ,this, SLOT(forceRepaint()));
    connect( cvh, SIGNAL (prevLine () ) ,this, SLOT(forceRepaint()));
    break;
  case Always:
    connect( cvv, SIGNAL (valueChanged ( int ) ) ,this, SLOT(forceRepaint( int )));
    connect( cvh, SIGNAL (valueChanged ( int ) ) ,this, SLOT(forceRepaint( int )));
    connect( cvv, SIGNAL (sliderReleased () ) ,this, SLOT(forceRepaint()));
    connect( cvh, SIGNAL (sliderReleased () ) ,this, SLOT(forceRepaint()));
    break;
  }
}
void KDGanttView::forceRepaint( int )
{
  if ( myTimeTable->blockUpdating() )
    return;
  // qDebug("forceRepaint( int ) ");
  myTimeTable->setAllChanged();
  myTimeTable->update();
}

void KDGanttView::slotHeaderSizeChanged()
{
  int legendHeight = 0;
  if ( showLegendButton() )
    legendHeight = 24;
  int listViewHeaderHeight = 0;
  if ( headerVisible() )
    listViewHeaderHeight = myListView->header()->height();
  int timeHeaderHeight = myTimeHeader->height()+myTimeHeaderScroll->frameWidth()*2;
  int diffY = timeHeaderHeight-legendHeight-listViewHeaderHeight;
  if ( diffY < 0 ) {
    spacerLeft->setFixedHeight( 0 );
    spacerRight->setFixedHeight(-diffY);
  } else {
    spacerRight->setFixedHeight( 0 );
    spacerLeft->setFixedHeight( diffY );
  }
  myLegend->setFixedHeight( legendHeight );
  myTimeHeaderScroll->setFixedHeight( timeHeaderHeight );
}


/*!
  Specifies whether the legend should be shown or not. Besides setting
  this programmatically, the user can also show/hide the legend by
  means of the button provided for this purpose.

  \param show force legend to be shown
  \sa showLegend()
*/

void KDGanttView::setShowLegend( bool show )
{
    myLegend->showMe(show);
}


/*!
  Returns whether the legend is currently shown. The visibility of the
  legend can be changed both by \a setShowLegend() as well as by the
  user interactively.

  \return true if the legend is currently visible
  \sa setShowLegend()
*/

bool KDGanttView::showLegend() const
{
    return myLegend->isShown();
}


/*!
  Specifies whether the listview of the Gantt view should be shown or
  not.

  \param show pass true in order to show the listview and false in
  order to hide it.
  \sa showListView()
*/

void KDGanttView::setShowListView( bool show )
{
    if(listViewIsVisible == show) return;
    listViewIsVisible = show;
    if (listViewIsVisible)
        myListView->parentWidget()->show();
    else
        myListView->parentWidget()->hide();
}


/*!
  Returns whether the listview of the Gantt view is shown or not.

  \return true if the listview is shown
  \sa setShowListView()
*/

bool KDGanttView::showListView() const
{
    return listViewIsVisible;
}


/*!
  Specifies whether it should be possible to edit the appearance of a
  Gantt item visually in a dialog by double-clicking the item.

  \param enable pass true in order to enable the visual editor and
  false in order to turn it off
  \sa editorEnabled()
*/

void KDGanttView::setEditorEnabled( bool enable )
{
  editorIsEnabled =  enable;
}


/*!
  Returns whether it is possible to edit the appearance of a Gantt
  item visually in a dialog by double-clicking the item.

  \return true if visual editing is enabled, false otherwise
  \sa setEditorEnabled()
*/

bool KDGanttView::editorEnabled() const
{
    return editorIsEnabled;
}


/*!
  Specifies whether the Gantt chart is user-editable.

  \param editable pass true in order to get a user-editable Gantt
  chart, pass false in order to get a read-only chart
  \sa editable()
*/

void KDGanttView::setEditable( bool editable )
{
  chartIsEditable =  editable;
}


/*!
  Returns whether the Gantt chart is user-editable

  \return true if the Gantt chart is user-editable
  \sa setEditable()
*/

bool KDGanttView::editable() const
{
    return chartIsEditable;
}


/*!
  Saves the state of the Gantt view in an XML file. This file can be
  reloaded with \a loadProject().

  \param file a pointer to the file in which to store the Gantt view
  state.
  \return true if the file could be written, false if an error
  occurred
  \sa loadProject()
*/

bool KDGanttView::saveProject( QFile* file )
{
    QDomDocument doc = saveXML();
    if( file->isOpen() )
        file->close();
    if( file->open( IO_WriteOnly ) ) {
        QTextStream ts( file );
        ts << doc.toString();
        return true;
    } else
        return false;
}


/*!
  Loads a previously saved state of the Gantt view. All current
  settings and items are discarded before loading the file.

  \param file a pointer to the file from which to load the Gantt view
  state.
  \return true if the file could be read, false if an error
  occurred
  \sa saveProject()
*/

bool KDGanttView::loadProject( QFile* file )
{
    if( file->isOpen() )
        file->close();
    if( file->open( IO_ReadOnly ) ) {
        QDomDocument doc( "GanttView" );
        doc.setContent( file );
        file->close();
        return loadXML( doc );
    } else
        return false;
}


/*!
  Prints a Gantt view to a printer. The printer should already be set
  up for printing (by means of calling QPrinter::setup()).
  If the printer is not set up, QPrinter::setup() is called before printing

  It can be specified, wether the ListView, TimeLine or Legend is printed out.
  All combinations of these three widgets are allowed.

  \param printer a pointer to the printer to print on. If printer is
  0, the method creates a temporary printer and discards it when it is
  done printing.
  \param printListView if true, the ListView is printed
  \param printTimeLine if true, the TimeLine is printed
  \param printLegend if true, the Legend is printed

  \sa drawContents()
*/

void KDGanttView::print( QPrinter* printer ,
		bool printListView, bool printTimeLine, bool printLegend )
{
  bool deletePrinter = false;
  if (! printer ) {
    printer = new QPrinter();
    deletePrinter = true;
    if ( !printer->setup()) {
      delete printer;
      return;
    }
  }
  // now we have a printer to print on
  QPainter p( printer );
  // get the paper metrics
  QPaintDeviceMetrics m = QPaintDeviceMetrics ( printer );
  float dx, dy;
  // get the size of the desired output for scaling.
  // here we want to print all: ListView, TimeLine, and Legend
  // for this purpose, we call drawContents() with a 0 pointer as painter
  QSize size = drawContents( 0, printListView, printTimeLine, printLegend );

  // at the top, we want to print current time/date
  QString date = "Printing Time: " + QDateTime::currentDateTime().toString();
  int hei = p.boundingRect(0,0, 5, 5, Qt::AlignLeft, date ).height();
  p.drawText( 0, 0, date );

  // compute the scale
  dx = (float) m.width()  / (float)size.width();
  dy  = (float)(m.height() - ( 2 * hei )) / (float)size.height();
  float scale;
  // scale to fit the width or height of the paper
  if ( dx < dy )
    scale = dx;
  else
    scale = dy;
  // set the scale
  p.scale( scale, scale );
  // now printing with y offset:  2 hei
  p.translate( 0, 2*hei );
  drawContents( &p, printListView, printTimeLine, printLegend );
  // the drawContents() has the side effect, that the painter translation is
  // after drawContents() set to the bottom of the painted stuff
  // for instance a
  // p.drawText(0, 0, "printend");
  // would be painted directly below the paintout of drawContents()
  p.end();
  if ( deletePrinter )
    delete printer;
}
/*!
  Paints a Gantt view on a QPainter.
  It can be specified, wether the ListView, TimeLine or Legend is painted.
  All combinations of these three widgets are allowed.
  Gives back the size of the painted area.
  Paints the ListView at top left, the TimeLine right of the ListView
  and the Legend below the ListView.
  If called with painter pointer p = 0, nothing is painted and only
  the size of the painted area is computed.
  This is useful for determining only the painted area and setting
  the scale of the painter, before calling this method with a painter.
  In order to get the output fitting to your paper of your printer,
  call first
  QSize size = drawContents( 0, printListView, printTimeLine, printLegend );
  //then compute the scale
  dx = paper.width()  / size.width();
  dy  = paper.height() / size.height();
  //then make float scale to fit the width or height of the paper
  if ( dx < dy )
    scale = dx;
  else
    scale = dy;
  // then set the scale
  p.scale( scale, scale );
  // and now call drawContents with painter p
  drawContents( &p, printListView, printTimeLine, printLegend );

  For a detailed example look at the commented source code in
  KDGanttView::print(...)

  \param p  a pointer to the painter to paint on. If p is
  0, nothing is painted and only the size of the painted area is computed
  \param drawListView if true, the ListView is painted
  \param drawTimeLine if true, the TimeLine is painted
  \param drawLegend if true, the Legend is painted
  \return the size of the painted area
  \sa print()
*/

QSize KDGanttView::drawContents( QPainter* p,
		      bool drawListView , bool drawTimeLine, bool drawLegend )
{
  QSize size;
  int lvX, lvY, thX, thY, tlX, tlY, lwX, lwY, allX, allY;
  lvX = myListView->contentsWidth();
  lvY = myCanvasView->canvas()->height() + 20;
  thX = myTimeHeader->width();
  thY = myTimeHeader->height();
  tlX = myCanvasView->canvas()->width();
  tlY = lvY;
  lwX = myLegend->legendSize().width();
  lwY = myLegend->legendSize().height();
  allX = 0;
  allY = 0;
  if ( drawListView ) {
    allX += lvX;
    allY += tlY;
  }
  if ( drawTimeLine ) {
    allX += thX;
    allY += thY;
  }
  if ( drawLegend ) {
    allY += lwY;
    if ( allX < lwX )
      allX = lwX ;
  }
  size = QSize( allX, allY );
  int temp = 0;
  if ( p ) {
    if ( drawListView ) {
      if ( drawTimeLine )
	temp =  thY;
      p->translate( 0, temp );
      myListView->drawToPainter( p );
      p->translate( lvX, -temp);
    }
    if ( drawTimeLine ) {
      p->translate( myCanvasView->frameWidth(), 0);
      myTimeHeader->repaintMe( 0, myTimeHeader->width(), p );
      p->translate( -myCanvasView->frameWidth(), thY);
      myCanvasView->drawToPainter( p );
      if ( drawListView )
	p->translate( -lvX, tlY);
      else
	p->translate( 0, tlY);
    } else {
      if ( drawListView )
	p->translate( -lvX, 0 );
    }
    if ( drawLegend ) {
       myLegend->drawToPainter( p );
       p->translate( 0, lwY );
    }
  }
  return size;
}

/*!
  Zooms into the Gantt chart. Values less than 1 mean zooming in,
  values greater than 1 mean zooming out. A zooming factor of exactly
  1.0 means original size.

  \param factor the zoom factor
  \param absolute if true, factor is interpreted absolutely, if false,
  factor is interpreted relatively to the current zoom factor
  \sa zoomToFit()
  \sa zoomToSelection()
  \sa zoomFactor()
*/

void KDGanttView::setZoomFactor( double factor, bool absolute )
{
    myTimeHeader->zoom(factor,absolute);
}


/*!
  Returns the current zoom factor.

  \return the current zoom factor
  \sa zoomToFit(), zoomToSelection(), setZoomFactor()
*/

double KDGanttView::zoomFactor() const
{
    return myTimeHeader->zoomFactor();
}


/*!
  Zooms so that the Gantt chart is less than the available space of the widget.

  \sa setZoomFactor()
  \sa zoomFactor()
  \sa zoomToSelection()
*/

void KDGanttView::zoomToFit()
{
  myTimeHeader->zoomToFit();
}


/*!
  Zooms so that at least the selected time period is visible after the zoom.

  \param the new font of the widget
  \param the new font of the widget

  \sa setZoomFactor()
  \sa zoomFactor()
  \sa zoomToFit()
*/

void KDGanttView::zoomToSelection( const QDateTime& start,  const QDateTime&  end )
{

  myTimeHeader->zoomToSelection( start, end);

}


/*!
  Makes sure that the specified Gantt item is visible without
  scrolling.

  \sa center(), centerTimelineAfterShow()
*/

void KDGanttView::ensureVisible( KDGanttViewItem* item )
{
    myListView->ensureItemVisible (item);
}
/*!
  Makes sure that the specified QDateTime is in the center of the
  visible Gantt chart (if possible).
  If you want to center the timeline, when the KDGanttView is hidden,
  you should better call centerTimelineAfterShow()

  \sa center(), centerTimelineAfterShow()
*/

void KDGanttView::centerTimeline( const QDateTime& center )
{
  myTimeHeader->centerDateTime( center );
}
/*!
  Makes sure that the specified QDateTime is in the center of the
  visible Gantt chart (if possible).
  If the KDGanttView is currently hidden, this method
  resets the center one time again, after the next time  show()
  is called. Use this method, if you want to center the timeline, when
  the KDGanttView is hidden. After calling of KDGanttView::show(), there
  may be computations of the widgets and subwidgets size and of the
  automatically computed startdatetime. This method ensures
  the center of the timeline is to be properly reset after show().

  \sa center(), centerTimeline()
*/

void KDGanttView::centerTimelineAfterShow( const QDateTime& center )
{
  myTimeHeader->centerDateTime( center );
  if ( isVisible() ) return;
  dtCenterTimeLineAfterShow = center;
  fCenterTimeLineAfterShow = true;
}

/*!
  Sets Timeline to the horizon start.
*/

void KDGanttView::setTimelineToStart()
{
  myCanvasView->horizontalScrollBar()->setValue( 0 );
}
/*!
  Sets Timeline to the horizon end.
*/

void KDGanttView::setTimelineToEnd()
{
 myCanvasView->horizontalScrollBar()->setValue(myCanvasView->horizontalScrollBar()->maxValue());
}
/*!
  Add \a num minor ticks of the current scale of the timeline
  to the start of the timeline.
  The timeline is set not automatically at the start.
  Call \a setTimelineToStart() to ensure timeline at start after calling
  this method.

  \param num the number of minor ticks which should be added
  \sa addTicksRight(),setTimelineToStart(), setTimelineToEnd()
*/

void KDGanttView::addTicksLeft( int num )
{
  myTimeHeader->addTickLeft( num  );
}
/*!
  Add \a num minor ticks of the current scale of the timeline
  to the end of the timeline.
  The timeline is set not automatically at the end.
  Call \a setTimelineToEnd() to ensure timeline at end after calling
  this method.
  \param num the number of minor ticks which should be added
  \sa addTicksLeft(),setTimelineToStart(), setTimelineToEnd()
*/

void KDGanttView::addTicksRight( int num )
{
  myTimeHeader->addTickRight( num );
}


/*!
  Makes sure that the specified Gantt item is in the center of the
  visible Gantt chart (if possible).
*/

void KDGanttView::center( KDGanttViewItem* item )
{
    ensureVisible(item);
    int x =  myListView->contentsWidth()/2;
    int y = myListView->itemPos (item );
    myListView->center(x,y);
}


/*!
  Specifies whether task links should be shown.

  \param show true for showing task links, false for not showing them
  \sa showTaskLinks(), KDGanttViewTaskLink
*/

void KDGanttView::setShowTaskLinks( bool show )
{
    myTimeTable->setShowTaskLinks(show);

}


/*!
  Returns whether task links should be shown.

  \return true if task links are shown, false otherwise
  \sa setShowTaskLinks(), KDGanttViewTaskLink
*/

bool KDGanttView::showTaskLinks() const
{
    return  myTimeTable->showTaskLinks();
}
/*!
  Sets the font in the left vistview widget and
  in the right time header widget.
  The settings of the fonts in the time table widget are not effected.

  \param the new font of the widget
*/

void KDGanttView::setFont(const QFont& font)
{
    myListView->setFont(font);
    myListView->repaint();
    myTimeHeader->setFont(font);
    myLegend->setFont( font );
    QWidget::setFont( font );
    setScale(scale());
}

/*!
  Specifies whether the configure popup menu should be shown on
  right click on the time header widget.
  This menu lets the user quickly change the scale mode
  (minute, hour, day, week, month) and specify  whether the grid should be shown
  (major-, minor- or no grid).
  The default setting is, that the popup menu is not shown.
  It must be enabled by the program.

  \param show true in order to show popup menu, false in order to do not

*/

void KDGanttView::setShowHeaderPopupMenu( bool show)
{
    myTimeHeader->setShowPopupMenu( show );
}


/*!
  Returns whether the configure popup menu should be shown on right
  click on the time header widget.

  \return true if the popup menu should be shown
*/
bool KDGanttView::showHeaderPopupMenu() const
{
    return myTimeHeader->showPopupMenu();
}
//****************************************************

/*!
  Specifies whether the add item popup menu should be shown on
  right click on the time table widget.
  This menu lets the user quickly add new items to the gantt view
  ( as root, as child or after an item ).
  It offers cut and paste of items to the user as well.

  The default setting is, that the popup menu is not shown.
  It must be enabled by the program.

  \param show true in order to show popup menu, false in order to do not

*/

void KDGanttView::setShowTimeTablePopupMenu( bool show )
{
    myCanvasView->setShowPopupMenu( show );
}


/*!
  Returns whether the add item popup menu should be shown on right
  click on the time table widget.

  \return true if the popup menu should be shown
*/
bool KDGanttView::showTimeTablePopupMenu() const
{
    return myCanvasView->showPopupMenu();
}

/*!
  Sets the shapes for a certain type of Gantt item. Not all items use
  all three shapes (e.g., only summary items use the middle shape).

  This setting overrides any shape settings made on individual items!
  This settings will be taken as initial values of any newly created
  item of this certain type.
  See also the description of the KDGanttViewItem class.

  \param type the type of Gantt items for which to set the shapes
  \param start the shape to use for the beginning of the item
  \param middle the shape to use for the middle of the item
  \param end the shape to use for the end of the item
  \sa shapes()
*/

void KDGanttView::setShapes( KDGanttViewItem::Type type,
                             KDGanttViewItem::Shape start,
                             KDGanttViewItem::Shape middle,
                             KDGanttViewItem::Shape end )
{
    QListViewItemIterator it(myListView);
    for ( ; it.current(); ++it ) {
        if ( ((KDGanttViewItem*)it.current())->type() == type)
            ((KDGanttViewItem*)it.current())->setShapes(start,middle, end );
    }

    int index = getIndex( type );
    myDefaultShape [index*3] = start;
    myDefaultShape [index*3+1] = middle;
    myDefaultShape [index*3+2] = end;
    undefinedShape[index] = false;
}


/*!
  Queries the shapes for a particular type of Gantt item.

  \param type the type of Gantt items for which to query the shapes
  \param start the start shape is returned in this parameter
  \param middle the middle shape is returned in this parameter
  \param end the end shape is returned in this parameter
  \return true if there was a general shape set for the specified
  type. If the return value is false, the values of the three shape
  parameters are undefined.
  \sa setShapes()
*/

bool KDGanttView::shapes( KDGanttViewItem::Type type,
                          KDGanttViewItem::Shape& start,
                          KDGanttViewItem::Shape& middle,
                          KDGanttViewItem::Shape& end ) const
{
    int index = getIndex( type );
    start = myDefaultShape [index*3];
    middle = myDefaultShape [index*3+1];
    end = myDefaultShape [index*3+2];
    return !undefinedShape[index];
}


/*!
  Sets the colors for a certain type of Gantt item. Not all items use
  all three colors (e.g., only summary items use the middle color).

  This setting overrides any color settings made on individual items.
  This settings will be taken as initial values of any newly created
  item of this certain type.
  See also the description of the KDGanttViewItem class.

  \param type the type of Gantt items for which to set the colors
  \param start the color to use for the beginning of the item
  \param middle the color to use for the middle of the item
  \param end the color to use for the end of the item
  \sa colors(), setDefaultColors(), defaultColors()
*/

void KDGanttView::setColors( KDGanttViewItem::Type type,
                             const QColor& start, const QColor& middle,
                             const QColor& end )
{
    QListViewItemIterator it(myListView);
    for ( ; it.current(); ++it ) {
        if ( ((KDGanttViewItem*)it.current())->type() == type)
            ((KDGanttViewItem*)it.current())->setColors(start,middle, end );
    }
    int index = getIndex( type );
    myColor [index*3] = start;
    myColor [index*3+1] = middle;
    myColor [index*3+2] = end;
    undefinedColor[index] = false;
}


/*!
  Queries the colors for a particular type of Gantt item.

  \param type the type of Gantt items for which to query the colors
  \param start the start color is returned in this parameter
  \param middle the middle color is returned in this parameter
  \param end the end color is returned in this parameter
  \return true if there was a general color set for the specified
  type. If the return value is false, the values of the three color
  parameters are undefined.
  \sa setColors(), setDefaultColor(), defaultColor()
*/

bool KDGanttView::colors( KDGanttViewItem::Type type,
                          QColor& start, QColor& middle, QColor& end ) const
{
  int index = getIndex( type );
  start = myColor [index*3];
    middle = myColor [index*3+1];
    end = myColor [index*3+2];
    return !undefinedColor[index];
}


/*!
  Sets the highlight colors for a certain type of Gantt item. Not
  all items use all three highlight colors (e.g., only summary items
  use the middle highlight color).

  This setting overrides any highlight color settings made on
  individual items!
  This settings will be taken as initial values of any newly created
  item of this certain type.
  See also the description of the KDGanttViewItem class.

  \param type the type of Gantt items for which to set the highlight colors
  \param start the highlight color to use for the beginning of the item
  \param middle the highlight color to use for the middle of the item
  \param end the highlight color to use for the end of the item
  \sa highlightColors(), setDefaultHighlightColor(), defaultHighlightColor()
*/

void KDGanttView::setHighlightColors( KDGanttViewItem::Type type,
                                      const QColor& start,
                                      const QColor& middle,
                                      const QColor& end )
{
    QListViewItemIterator it(myListView);
    for ( ; it.current(); ++it ) {
        if ( ((KDGanttViewItem*)it.current())->type() == type)
            ((KDGanttViewItem*)it.current())->setHighlightColors(start,middle, end );
    }
    int index = getIndex( type );
    myColorHL [index*3] = start;
    myColorHL [index*3+1] = middle;
    myColorHL [index*3+2] = end;
    undefinedColorHL[index] = false;

}


/*!
  Queries the highlight colors for a particular type of Gantt item.

  \param type the type of Gantt items for which to query the highlight
  colors
  \param start the start highlight color is returned in this parameter
  \param middle the middle highlight color is returned in this parameter
  \param end the end highlight color is returned in this parameter
  \return true if there was a general highlight color set for the specified
  type. If the return value is false, the values of the three highlight color
  parameters are undefined.
  \sa setHighlightColors(), setDefaultHighlightColor(),
  defaultHighlightColor()
*/

bool KDGanttView::highlightColors( KDGanttViewItem::Type type,
                                   QColor& start, QColor& middle,
                                   QColor& end ) const
{
  int index = getIndex( type );
  start = myColorHL [index*3];
    middle = myColorHL [index*3+1];
    end = myColorHL [index*3+2];
    return !undefinedColorHL[index];
}


/*!
  Sets the color used for texts in the Gantt chart.
  Overwrites all individual settings of the Gantt items.

  \param color the text color to use
  \sa textColor()
*/

void KDGanttView::setTextColor( const QColor& color )
{
    QListViewItemIterator it(myListView);
    for ( ; it.current(); ++it ) {
	((KDGanttViewItem*)it.current())->setTextColor(color);
    }
    myTextColor = color;
}


/*!
  Returns the color used for texts in the Gantt chart.

  \return the color used for texts in the Gantt chart.
  \sa setTextColor()x
*/

QColor KDGanttView::textColor() const
{
    return myTextColor;
}
/*!
  Specifies the brush in which the 'showNoInformation' line of items should be drawn.

  \param  brush the brush of the 'showNoInformation' lines
  \sa  KDGanttViewItem::showNoInformation(), KDGanttViewItem::setShowNoInformation(),
  KDGanttView::noInformationBrush()

*/
void KDGanttView::setNoInformationBrush( const QBrush& brush )
{
  myTimeTable->setNoInformationBrush( brush );
}
/*!
  Returns the brush of the 'showNoInformation' lines
  \return  the brush of the 'showNoInformation' lines
  \sa  KDGanttViewItem::showNoInformation(), KDGanttViewItem::setShowNoInformation(),
  setNoInformationBrush()
*/
QBrush  KDGanttView::noInformationBrush() const
{
  return myTimeTable->noInformationBrush();
}

/*!
  Removes all items from the legend.

*/

void KDGanttView::clearLegend( )
{
    myLegend->clearLegend();
    myLegendItems->setAutoDelete( true );
    delete myLegendItems;
    myLegendItems = new QPtrList<legendItem>;
}


/*!
  Adds an item to the legend.

  \param the shape to display
  \param the color in which to display the shape
  \param the text to display
*/

void KDGanttView::addLegendItem( KDGanttViewItem::Shape shape,
                                 const QColor& shapeColor,
                                 const QString& text )
{
    myLegend->addLegendItem( shape,shapeColor,text );
    legendItem* item = new legendItem;
    item->shape = shape;
    item->color = shapeColor;
    item->text = text;
    myLegendItems->append( item );
}


/*!
  Sets the start of the horizon of the Gantt chart. If \a start is
  null, the horizon start is computed automatically.

  \param start the start of the horizon
  \sa horizonStart()
*/

void KDGanttView::setHorizonStart( const QDateTime& start )
{
    myTimeHeader->setHorizonStart(start);
}


/*!
  Returns the start of the horizon of the Gantt chart.

  \return the start of the horizon of the Gantt chart
  \sa setHorizonStart()
*/

QDateTime KDGanttView::horizonStart() const
{
    return myTimeHeader->horizonStart();
}


/*!
  Sets the end of the horizon of the Gantt chart. If \a end is
  null, the horizon end is computed automatically.

  \param end the end of the horizon
  \sa setHorizonEnd()
*/

void KDGanttView::setHorizonEnd( const QDateTime& start )
{
    myTimeHeader->setHorizonEnd(start);
}


/*!
  Returns the end of the horizon of the Gantt chart.

  \return the end of the horizon of the Gantt chart
  \sa setHorizonEnd()

*/

QDateTime KDGanttView::horizonEnd() const
{
    return myTimeHeader->horizonEnd();
}


/*!
  Configures the unit of the lower scale of the header. The higher
  unit is computed automatically.

  \param unit the unit of the lower scale of the header.
  \sa scale()
*/

void KDGanttView::setScale( Scale unit )
{
    myTimeHeader->setScale( unit );
}


/*!
  Returns the unit of the lower scale of the header.

  \return the unit of the lower scale of the header.
  \sa setScale()
*/

KDGanttView::Scale KDGanttView::scale() const
{
    return myTimeHeader->scale();
}

/*!
  Sets the maximal allowed time scale of the lower scale of the header.

  \param unit the unit of the lower scale of the header.
  \sa scale()
*/

void KDGanttView::setMaximumScale( Scale unit )
{
    myTimeHeader->setMaximumScale( unit );
}


/*!
  Returns the maximal allowed time scale of the lower scale of the header.

  \return the unit of the lower scale of the header.
  \sa setScale()
*/

KDGanttView::Scale KDGanttView::maximumScale() const
{
    return myTimeHeader->maximumScale();
}

/*!
  Sets the minimal allowed time scale of the lower scale of the header.

  \param unit the unit of the lower scale of the header.
  \sa scale()
*/

void KDGanttView::setMinimumScale( Scale unit )
{
    myTimeHeader->setMinimumScale( unit );
}


/*!
  Returns the minimal allowed time scale of the lower scale of the header.

  \return the unit of the lower scale of the header.
  \sa setScale()
*/

KDGanttView::Scale KDGanttView::minimumScale() const
{
    return myTimeHeader->minimumScale();
}


/*!
  Sets the absolut number of minor ticks, if scale is set to Auto.
  If the scale mode is set to Auto, then the actual scale and
  the minorScaleCount is automatically computed, such that there are
  count minor ticks

  \param count the number of minor ticks
  \sa autoScaleMinorTickCount(),setScale(),scale()
*/

void KDGanttView::setAutoScaleMinorTickCount( int count )
{
  myTimeHeader->setAutoScaleMinorTickCount( count );
}


/*!
  Returns the absolut number of minor ticks, if Auto is set as scale

  \return the absolut number of minor ticks
  \sa setAutoScaleMinorTickCount(),setScale(),scale()
*/

int KDGanttView::autoScaleMinorTickCount() const
{
  return myTimeHeader->autoScaleMinorTickCount();
}

/*!
  Sets the minimum width a column needs to have. If the size of the
  Gantt chart and the scale would make it necessary to go below this
  limit otherwise, the chart will automatically be made less exact.

  \param width the minimum column width
  \sa minimumColumnWidth()
*/

void KDGanttView::setMinimumColumnWidth( int width )
{
    myTimeHeader->setMinimumColumnWidth( width );
}


/*!
  Returns the minimum width a column needs to have.

  \return the column minimum width
  \sa setMinimumColumnWidth()
*/

int KDGanttView::minimumColumnWidth() const
{
    return myTimeHeader->minimumColumnWidth();
}


/*!
  Specifies the format in which to display years. If no years are
  shown, this method has no effect.

  \param format the year format
  \sa yearFormat(), setHourFormat(), hourFormat()
*/

void KDGanttView::setYearFormat( YearFormat format )
{
    myTimeHeader->setYearFormat(format );
}


/*!
  Returns the format in which to display years.

  \return the year format
  \sa setYearFormat(), setHourFormat(), hourFormat()
*/

KDGanttView::YearFormat KDGanttView::yearFormat() const
{
    return myTimeHeader->yearFormat();
}


/*!
  Specifies the format in which to display hours. If no hours are
  shown, this method has no effect.

  \param format the hour format
  \sa hourFormat(), setYearFormat(), yearFormat()

*/

void KDGanttView::setHourFormat( HourFormat format )
{
    myTimeHeader->setHourFormat( format );
}


/*!
  Returns the format in which to display hours.

  \return the hour format
  \sa setHourFormat(), setYearFormat(), yearFormat()

*/

KDGanttView::HourFormat KDGanttView::hourFormat() const
{
    return myTimeHeader->hourFormat();
}


/*!
  Specifies whether ticks should be shown on the major scale.

  \param show true in order to show ticks, false in order to hide them
  \sa showMajorTicks(), setShowMinorTicks(), showMinorTicks()
*/

void KDGanttView::setShowMajorTicks( bool show )
{
    myTimeHeader->setShowMajorTicks(show );
}


/*!
  Returns whether ticks are shown on the major scale.

  \return true if ticks are shown on the major scale
  \sa setShowMajorTicks(), setShowMinorTicks(), showMinorTicks()
*/

bool KDGanttView::showMajorTicks() const
{
    return myTimeHeader->showMajorTicks();
}


/*!
  Specifies whether ticks should be shown on the minor scale.

  \param show true in order to show ticks, false in order to hide them
  \sa showMinorTicks(), setShowMajorTicks(), showMajorTicks()

*/

void KDGanttView::setShowMinorTicks( bool show)
{
    myTimeHeader->setShowMinorTicks( show );
}


/*!
  Returns whether ticks are shown on the minor scale.

  \return true if ticks are shown on the minor scale
  \sa setShowMinorTicks(), setShowMajorTicks(), showMajorTicks()

*/

bool KDGanttView::showMinorTicks() const
{
    return myTimeHeader->showMinorTicks();
}


/*!
  Sets the background color for the column closest to \a column.
  It can be defined, wether the color should be show in all scales or
  only on specific scales.
  If you want to define the color only for the dayly view, scecify
  mini and maxi as Day.
  If there is no  value for mini/maxi specified, the color for the column
  is shown on all scales. Note that it may be that there are for a column
  in a scale two values. Then the shown color is undertermined.

  \param column the column to set the background color for
  \param color the background color
  \param mini show the colour only in scales greater than this
  \param maxi show the colour only in scales lesser than this
  \sa columnBackgroundColor(), setWeekendBackgroundColor(),
  weekendBackgroundColor()
*/

void KDGanttView::setColumnBackgroundColor( const QDateTime& column,
                                            const QColor& color ,
					    Scale mini, Scale maxi )
{
  myTimeHeader->setColumnBackgroundColor( column, color,mini,maxi );
}

/*!
  Sets the background color for a time interval given by \a start and \a end.
  \a start may be later than \a end.
  If there is already a background interval with same \a start and \a end
  values defined, the values (i.e.  const QColor& color , Scale mini, Scale maxi)
  of this background interval are changed.
  Change the times of an already defined interval with \a changeBackgroundInterval().
  Delete an already defined interval with \a deleteBackgroundInterval().

  It can be defined, wether the color should be show in all scales or
  only on specific scales.
  If you want to define the color only for the dayly view, scecify
  mini and maxi as Day.
  If there is no  value for mini/maxi specified, the color for the columns
  is shown on all scales.

  \param start startdatetime of time interval
  \param end enddatetime of time interval
  \param color the background color
  \param mini show the colour only in scales greater than this
  \param maxi show the colour only in scales lesser than this
  \sa changeBackgroundInterval(), deleteBackgroundInterval(),
  columnBackgroundColor(), setWeekendBackgroundColor(),
  weekendBackgroundColor()
*/

void KDGanttView::setIntervalBackgroundColor( const QDateTime& start,
					      const QDateTime& end,
                                            const QColor& color ,
					    Scale mini, Scale maxi )
{
  myTimeHeader->setIntervalBackgroundColor( start, end, color,mini,maxi );
}

/*!
  Change the times of an already defined backgroundcolor interval.
  The new  values \a startnew and \a endnew must not be datetime
  values of an already defined backgroundcolor interval!
  If so, nothing is changed and false is returned.

  \param start startdatetime of time interval
  \param end enddatetime of time interval
  \param newstart the background color
  \param newend show the colour only in scales greater than this
  \return true, if there is a backgroundcolor interval with values
  \a start and \a end found  and the new values \a startnew and \a endnew
  are not datetime values of an already defined backgroundcolor interval!
          false otherwise.
  \sa changeBackgroundInterval(), deleteBackgroundInterval(),
  columnBackgroundColor(), setWeekendBackgroundColor(),
  weekendBackgroundColor()
*/



bool KDGanttView::changeBackgroundInterval( const QDateTime& oldstart,
				   const QDateTime& oldend,
				   const QDateTime& newstart,
				   const QDateTime& newend )
{
  return myTimeHeader->changeBackgroundInterval( oldstart, oldend,
						 newstart, newend );
}

/*!
  Deletes an already defined backgroundcolor interval.

  \param start startdatetime of time interval
  \param end enddatetime of time interval
  \return true, if there is a backgroundcolor interval with values
  \a start and \a end found  ( and hence deleted ).
  \sa changeBackgroundInterval(),  columnBackgroundColor()
*/
bool KDGanttView::deleteBackgroundInterval( const QDateTime& start,
						   const QDateTime& end)
{
  return myTimeHeader->deleteBackgroundInterval( start, end );
}

/*!

  Removes all background color settings made with setColumnBackgroundColor()
  and setIntervalBackgroundColor().
  Does not affect the settings of setWeekendBackgroundColor().

  \sa setColumnBackgroundColor(), setWeekendBackgroundColor(),
  weekendBackgroundColor(), setIntervalBackgroundColor()
*/



void KDGanttView::clearBackgroundColor()
{
  myTimeHeader->clearBackgroundColor();
}




/*!
  Returns the background color for the column closes to \a column.

  \param column the column to query the background color for
  \return the background color of the specified color
  \sa setColumnBackgroundColor(), setWeekendBackgroundColor(),
  weekendBackgroundColor()
*/

QColor KDGanttView::columnBackgroundColor( const QDateTime& column ) const
{
    return myTimeHeader->columnBackgroundColor( column ) ;
}


/*!
  Specifies the background color for weekend days. If no individual
  days are visible on the Gantt chart, this method has no visible
  effect.

  \param color the background color to use for weekend days.
  \sa weekendBackgroundColor(), setWeekendDays(), weekendDays()
*/

void KDGanttView::setWeekendBackgroundColor( const QColor& color )
{
    myTimeHeader->setWeekendBackgroundColor( color );
}


/*!
  Returns the background color for weekend days.

  \return the background color for weekend days
  \sa setWeekendBackgroundColor(), setWeekendDays(), weekendDays()
*/

QColor KDGanttView::weekendBackgroundColor() const
{
    return myTimeHeader->weekendBackgroundColor();
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

void KDGanttView::setWeekdayBackgroundColor( const QColor& color, int  weekday )
{
  myTimeHeader->setWeekdayBackgroundColor( color,  weekday );
}


/*!
  Returns the background color for weekday days.

  \param the day of the week (Monday = 1, Sunday = 7)
  \return the background color for weekend days
  \sa setWeekendBackgroundColor(), setWeekendDays(), weekendDays()
*/

QColor KDGanttView::weekdayBackgroundColor(int weekday) const
{
  return myTimeHeader->weekdayBackgroundColor( weekday);
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

void KDGanttView::setWeekendDays( int start, int end )
{
    myTimeHeader->setWeekendDays( start,  end );
}


/*!
  Returns which days are considered weekends.

  \param start in this parameter, the first day of the weekend is returned
  \param end in this parameter, the end day of the weekend is returned
  \sa setWeekendDays(), setWeekendBackgroundColor(), weekendBackgroundColor()
*/

void KDGanttView::weekendDays( int& start, int& end ) const
{
    myTimeHeader->weekendDays( start,  end );
}


/*!
  \fn void KDGanttView::itemLeftClicked( KDGanttViewItem* )

  This signal is emitted when the user clicks on an item with the left
  mouse button.
*/


/*!
  \fn void KDGanttView::itemMidClicked( KDGanttViewItem* )

  This signal is emitted when the user clicks on an item with the middle
  mouse button.
*/


/*!
  \fn void KDGanttView::itemRightClicked( KDGanttViewItem* )

  This signal is emitted when the user clicks on an item with the right
  mouse button.
*/


/*!
  \fn void KDGanttView::itemConfigured( KDGanttViewItem* )

  This signal is emitted when the user has configured an item
  visually.
*/



/*!
  \fn void KDGanttView::taskLinkLeftClicked( KDGanttViewTaskLink* )

  This signal is emitted when the user clicks on a task link with the
  left mouse button.
*/


/*!
  \fn void KDGanttView::taskLinkMidClicked( KDGanttViewTaskLink* )

  This signal is emitted when the user clicks on a task link with the
  middle mouse button.
*/


/*!
  \fn void KDGanttView::taskLinkRightClicked( KDGanttViewTaskLink* )

  This signal is emitted when the user clicks on a task link with the
  right mouse button.
*/


/*!
  \enum KDGanttView::YearFormat

  This enum is used to specify the year format used in the header.
*/


/*!
  \enum KDGanttView::HourFormat

  This enum is used to specify the hour format used in the header.
*/


/*!
  \enum KDGanttView::Scale

  This enum is used to specify the units of the scales in the header.
*/




/*!
  Sets the number of ticks in the major scale.

  \param count the number of ticks in the major scale
  \sa majorScaleCount(), setMinorScaleCount(), minorScaleCount()
*/

void KDGanttView::setMajorScaleCount( int count )
{
    myTimeHeader->setMajorScaleCount(count );
}


/*!
  Returns the number of ticks per unit in the major scale.

  \return the number of ticks in the major scale
  \sa setMajorScaleCount(), setMinorScaleCount(), minorScaleCount()
*/

int KDGanttView::majorScaleCount() const
{
    return myTimeHeader->majorScaleCount();
}


/*!
  Sets the number of ticks in the minor scale.

  \param count the number of ticks in the minor scale
  \sa minorScaleCount, setMajorScaleCount, majorScaleCount()

*/

void KDGanttView::setMinorScaleCount( int count )
{
    myTimeHeader->setMinorScaleCount(count );
}


/*!
  Returns the number of ticks per unit in the minor scale.

  \return the number of ticks in the minor scale
  \sa setMinorScaleCount(), setMajorScaleCount(), majorScaleCount()

*/

int KDGanttView::minorScaleCount() const
{
    return myTimeHeader->minorScaleCount();

}


/*!
  Sets the default color for a particular type of Gantt item that is
  used for the item if no specific start, middle, or end colors are
  set.

  \param type the type of Gantt items for which to query the highlight
  colors
  \param color the default color to use
  \sa defaultColor(), setColors(), colors()
*/

void KDGanttView::setDefaultColor( KDGanttViewItem::Type type,
                                   const QColor& color )
{
    QListViewItemIterator it(myListView);
    for ( ; it.current(); ++it ) {
        if ( ((KDGanttViewItem*)it.current())->type() == type)
            ((KDGanttViewItem*)it.current())->setDefaultColor(color );
    }
    int index = getIndex( type );
    myDefaultColor [index] = color;
}



/*!
  Returns the default color for a particular type of Gantt item that
  is used for the item if no specific start, middle, or end colors are
  set.

  \param type the type of Gantt items for which to query the highlight
  colors
  \return color the default color used
  \sa setDefaultColor(), setColors(), colors()
*/

QColor KDGanttView::defaultColor( KDGanttViewItem::Type type ) const
{
  int index = getIndex( type );
  return myDefaultColor [index];
}


/*!
  Sets the default highlighting color for a particular type of
  Gantt item that is used for the item if no specific start, middle,
  or end colors are set.

  \param type the type of Gantt items for which to query the highlight
  colors
  \param color the default highlighting color to use
  \sa defaultHighlightColor(), setHighlightColors(), highlightColors()
*/

void KDGanttView::setDefaultHighlightColor( KDGanttViewItem::Type type,
                                            const QColor& color )
{

    QListViewItemIterator it(myListView);
    for ( ; it.current(); ++it ) {
        if ( ((KDGanttViewItem*)it.current())->type() == type)
            ((KDGanttViewItem*)it.current())->setDefaultHighlightColor(color );
    }
    int index = getIndex( type );
    myDefaultColorHL [index] = color;
}



/*!
  Returns the default highlighting color for a particular type of
  Gantt item that is used for the item if no specific start, middle,
  or end colors are set.

  \param type the type of Gantt items for which to query the highlight
  colors
  \return color the default highlighting color used
  \sa setDefaultHighlightColor(), setHighlightColors(), highlightColors()
*/

QColor KDGanttView::defaultHighlightColor( KDGanttViewItem::Type type ) const
{
  int index = getIndex( type );
  return myDefaultColorHL [index];
}

/*!
  Returns the first item in the Gantt view.


  \return the first item in the Gantt view, 0 if there are no items
*/
KDGanttViewItem* KDGanttView::firstChild() const
{
    return (KDGanttViewItem*)myListView->firstChild();
}
/*!
  This method sets the calender mode.
  In calendar mode, the root items of the listview are not decorated with a '+'
  if they have children. And it it not possible to open an item with a
  doubleclick on it.
  If you want use this GanttView as a calendar view, you have to call
  setDisplaySubitemsAsGroup( true );
  to use the root items as calendar items.
  To create new calendarentries for these root items, create a new
  KDGanttViewTaskItem with this root item as a parent.

  \param mode if true, the calendar view will be set,
              if false, the calendar view will be unset
  \sa setDisplaySubitemsAsGroup(), displaySubitemsAsGroup(), calendarMode()
*/
void KDGanttView::setCalendarMode( bool mode )
{
  myListView->setCalendarMode( mode );
}
/*!
  Returns true, if GanttView is in calendarmode. See setCalendarMode() what
  calendermode does mean.

  \return returns true, if GanttView is in calendermode
  \sa setCalendarMode()
*/
bool  KDGanttView::calendarMode()
{
  return  myListView->calendarMode();
}



/*!
  This method specifies wether hidden subitems should be displayed.
  It iterates over all KDGanttViewItems in this GanttView
  and sets their property displaySubitemsAsGroup().
  All newly created items will have this setting as default.
  \param show if true, the hidden subitems are displayed in all items of
         this GanttView.
  \sa KDGanttViewItem::setDisplaySubitemsAsGroup(), KDGanttViewItem::displaySubitemsAsGroup()
*/
void KDGanttView::setDisplaySubitemsAsGroup( bool show )
{
 QListViewItemIterator it( myListView );
 for ( ; it.current(); ++it ) {
   KDGanttViewItem* currentItem = ( KDGanttViewItem* )it.current();
   currentItem->setDisplaySubitemsAsGroup( show );
 }
 _displaySubitemsAsGroup = show;
}
/*!
  Returns, wether new Items are created with the DisplayHiddenSubitems property.
  \return true, if hidden subitems should be displayed on newly created items.
  \sa setDisplaySubitemsAsGroup(), KDGanttViewItem::setDisplaySubitemsAsGroup(), KDGanttViewItem::displaySubitemsAsGroup()
*/
bool KDGanttView::displaySubitemsAsGroup() const
{
  return _displaySubitemsAsGroup;
}
/*!
  Defines the horizontal background lines of the gantt chart.
  Call setHorBackgroundLines()
  ( = setHorBackgroundLines( 2, QBrush( QColor ( 240,240,240 )) ) )
  to draw for every second gantt item  a light grey horizontal background line.
  Call setHorBackgroundLines(0) in order to not show horizontal background lines.
  You may specify the number of lines and the brush of the lines.

  \param count for count >=  2, every count line gets a backgroud specified by brush
               for count <  2, no background lines are drawn
  \param brush the brush of the lines
*/
void KDGanttView::setHorBackgroundLines( int count, QBrush brush )
{
  myTimeTable->setHorBackgroundLines(  count, brush );
}
/*!
  Returns the defines of the horizontal background lines of the gantt chart.

  \param brush the brush of the lines
  \return  every count line gets a backgroud specified by brush
  if 0 is returned, no backgroud lines are drawn

*/

int KDGanttView::horBackgroundLines( QBrush& brush )
{
  return myTimeTable->horBackgroundLines( brush );
}


/*!
  Returns the last item in the Gantt view

  \return the last item in the Gantt view, 0 if there are no items
*/
KDGanttViewItem* KDGanttView::lastItem() const
{
    return (KDGanttViewItem*)myListView->lastItem ();
}

/*!
  Returns the list of task links in the Gantt view.

  \return the list of task links in the Gantt view
*/
QPtrList<KDGanttViewTaskLink> KDGanttView::taskLinks() const
{

    return myTimeTable->taskLinks();
}


/*!
  Returns the list of task link groups in the Gantt view.

  \return the list of task link groups in the Gantt view
*/
QPtrList<KDGanttViewTaskLinkGroup> KDGanttView::taskLinkGroups() const
{
    return myTaskLinkGroupList;
}


/**
   Reads the view's parameters from an XML document.
   \param doc the XML document to read from
   \return true if the parameters could be read, false if a file
   format error occurred
   \sa saveXML
*/
bool KDGanttView::loadXML( const QDomDocument& doc )
{
    QDomElement docRoot = doc.documentElement(); // ChartParams element
    QDomNode node = docRoot.firstChild();
    while( !node.isNull() ) {
        QDomElement element = node.toElement();
        if( !element.isNull() ) { // was really an element
            QString tagName = element.tagName();
            if( tagName == "ShowLegend" ) {
                bool value;
                if( KDXML::readBoolNode( element, value ) )
                    setShowLegend( value );
            } else if( tagName == "ShowListView" ) {
                bool value;
                if( KDXML::readBoolNode( element, value ) )
                    setShowListView( value );
            } else if( tagName == "ShowTaskLinks" ) {
                bool value;
                if( KDXML::readBoolNode( element, value ) )
                    setShowTaskLinks( value );
            } else if( tagName == "EditorEnabled" ) {
                bool value;
                if( KDXML::readBoolNode( element, value ) )
                    setEditorEnabled( value );
            } else if( tagName == "GlobalFont" ) {
                QFont font;
                if( KDXML::readFontNode( element, font ) )
                    setFont( font );
            } else if( tagName == "HorizonStart" ) {
                QDateTime value;
                if( KDXML::readDateTimeNode( element, value ) )
                    setHorizonStart( value );
            } else if( tagName == "HorizonEnd" ) {
                QDateTime value;
                if( KDXML::readDateTimeNode( element, value ) )
                    setHorizonEnd( value );
            } else if( tagName == "Scale" ) {
                QString value;
                if( KDXML::readStringNode( element, value ) )
                    setScale( stringToScale( value ) );
            } else if( tagName == "MinimumScale" ) {
                QString value;
                if( KDXML::readStringNode( element, value ) )
                    setMinimumScale( stringToScale( value ) );
            } else if( tagName == "MaximumScale" ) {
                QString value;
                if( KDXML::readStringNode( element, value ) )
                    setMaximumScale( stringToScale( value ) );
            } else if( tagName == "YearFormat" ) {
                QString value;
                if( KDXML::readStringNode( element, value ) )
                    setYearFormat( stringToYearFormat( value ) );
            } else if( tagName == "HourFormat" ) {
                QString value;
                if( KDXML::readStringNode( element, value ) )
                    setHourFormat( stringToHourFormat( value ) );
            } else if( tagName == "ShowMinorTicks" ) {
                bool value;
                if( KDXML::readBoolNode( element, value ) )
                    setShowMinorTicks( value );
            } else if( tagName == "ShowMajorTicks" ) {
                bool value;
                if( KDXML::readBoolNode( element, value ) )
                    setShowMajorTicks( value );
            } else if( tagName == "Editable" ) {
                bool value;
                if( KDXML::readBoolNode( element, value ) )
                    setEditable( value );
            } else if( tagName == "TextColor" ) {
                QColor value;
                if( KDXML::readColorNode( element, value ) )
                    setTextColor( value );
            } else if( tagName == "MajorScaleCount" ) {
                int value;
                if( KDXML::readIntNode( element, value ) )
                    setMajorScaleCount( value );
            } else if( tagName == "MinorScaleCount" ) {
                int value;
                if( KDXML::readIntNode( element, value ) )
                    setMinorScaleCount( value );
            } else if( tagName == "MinimumColumnWidth" ) {
                int value;
                if( KDXML::readIntNode( element, value ) )
                    setMinimumColumnWidth( value );
            } else if( tagName == "WeekendBackgroundColor" ) {
                QColor value;
                if( KDXML::readColorNode( element, value ) )
                    setWeekendBackgroundColor( value );
            } else if( tagName == "WeekdayBackgroundColor" ) {
		QDomNode node = element.firstChild();
                int day = 0;
                QColor color;
		while( !node.isNull() ) {
		    QDomElement element = node.toElement();
		    if( !element.isNull() ) { // was really an elemente
			QString tagName = element.tagName();
			if( tagName == "Day" ) {
                            int value;
                            if( KDXML::readIntNode( element, value ) )
                                day = value;
                        } else if( tagName == "Color" ) {
                            QColor value;
                            if( KDXML::readColorNode( element, value ) )
                                color = value;
                        } else {
                            qDebug( "Unrecognized tag name: %s", tagName.latin1() );
                            Q_ASSERT( false );
                        }
                    }
                    node = node.nextSibling();
                }

                if( day && color.isValid() )
                    setWeekdayBackgroundColor( color, day );
            } else if( tagName == "WeekendDays" ) {
                QString startString = element.attribute( "Start" );
                QString endString = element.attribute( "End" );
                bool startOk = false, endOk = false;
                int start = startString.toInt( &startOk );
                int end = startString.toInt( &endOk );
                if( startOk && endOk )
                    setWeekendDays( start, end );
            } else if( tagName == "ZoomFactor" ) {
                double value;
                if( KDXML::readDoubleNode( element, value ) )
                    setZoomFactor( value, true );
            } else if( tagName == "ShowHeaderPopupMenu" ) {
                bool value;
                if( KDXML::readBoolNode( element, value ) )
                    setShowHeaderPopupMenu( value );
            } else if( tagName == "Shapes" ) {
		QDomNode node = element.firstChild();
		while( !node.isNull() ) {
		    QDomElement element = node.toElement();
		    if( !element.isNull() ) { // was really an elemente
			QString tagName = element.tagName();
			if( tagName == "Event" ) {
                            KDGanttViewItem::Shape startShape, middleShape, endShape;
			    startShape = KDGanttViewItem::TriangleDown;
			    middleShape = KDGanttViewItem::TriangleDown;
			    endShape = KDGanttViewItem::TriangleDown;
                            QDomNode node = element.firstChild();
                            while( !node.isNull() ) {
                                QDomElement element = node.toElement();
                                if( !element.isNull() ) { // was really an elemente
                                    QString tagName = element.tagName();
                                    if( tagName == "Start" ) {
                                        QString value;
                                        if( KDXML::readStringNode( element, value ) )
                                            startShape = KDGanttViewItem::stringToShape( value );
                                    } else if( tagName == "Middle" ) {
                                        QString value;
                                        if( KDXML::readStringNode( element, value ) )
                                            middleShape = KDGanttViewItem::stringToShape( value );
                                    } else if( tagName == "End" ) {
                                        QString value;
                                        if( KDXML::readStringNode( element, value ) )
                                            endShape = KDGanttViewItem::stringToShape( value );
                                    } else {
                                        qDebug( "Unrecognized tag name: %s", tagName.latin1() );
                                        Q_ASSERT( false );
                                    }
                                }
                                node = node.nextSibling();
                            }
                            setShapes( KDGanttViewItem::Event, startShape,
                                       middleShape, endShape );
                        } else if( tagName == "Task" ) {
                            KDGanttViewItem::Shape startShape, middleShape, endShape;
			    startShape = KDGanttViewItem::TriangleDown;
			    middleShape = KDGanttViewItem::TriangleDown;
			    endShape = KDGanttViewItem::TriangleDown;
                            QDomNode node = element.firstChild();
                            while( !node.isNull() ) {
                                QDomElement element = node.toElement();
                                if( !element.isNull() ) { // was really an elemente
                                    QString tagName = element.tagName();
                                    if( tagName == "Start" ) {
                                        QString value;
                                        if( KDXML::readStringNode( element, value ) )
                                            startShape = KDGanttViewItem::stringToShape( value );
                                    } else if( tagName == "Middle" ) {
                                        QString value;
                                        if( KDXML::readStringNode( element, value ) )
                                            middleShape = KDGanttViewItem::stringToShape( value );
                                    } else if( tagName == "End" ) {
                                        QString value;
                                        if( KDXML::readStringNode( element, value ) )
                                            endShape = KDGanttViewItem::stringToShape( value );
                                    } else {
                                        qDebug( "Unrecognized tag name: %s", tagName.latin1() );
                                        Q_ASSERT( false );
                                    }
                                }
                                node = node.nextSibling();
                            }
                            setShapes( KDGanttViewItem::Task, startShape,
                                       middleShape, endShape );
                        } else if( tagName == "Summary" ) {
			    KDGanttViewItem::Shape startShape, middleShape, endShape;
			    startShape = KDGanttViewItem::TriangleDown;
			    middleShape = KDGanttViewItem::TriangleDown;
			    endShape = KDGanttViewItem::TriangleDown;
                            QDomNode node = element.firstChild();
                            while( !node.isNull() ) {
                                QDomElement element = node.toElement();
                                if( !element.isNull() ) { // was really an elemente
                                    QString tagName = element.tagName();
                                    if( tagName == "Start" ) {
                                        QString value;
                                        if( KDXML::readStringNode( element, value ) )
                                            startShape = KDGanttViewItem::stringToShape( value );
                                    } else if( tagName == "Middle" ) {
                                        QString value;
                                        if( KDXML::readStringNode( element, value ) )
                                            middleShape = KDGanttViewItem::stringToShape( value );
                                    } else if( tagName == "End" ) {
                                        QString value;
                                        if( KDXML::readStringNode( element, value ) )
                                            endShape = KDGanttViewItem::stringToShape( value );
                                    } else {
                                        qDebug( "Unrecognized tag name: %s", tagName.latin1() );
                                        Q_ASSERT( false );
                                    }
                                }
                                node = node.nextSibling();
                            }
                            setShapes( KDGanttViewItem::Summary, startShape,
                                       middleShape, endShape );
                        } else {
                            qDebug( "Unrecognized tag name: %s", tagName.latin1() );
                            Q_ASSERT( false );
                        }
                    }
                    node = node.nextSibling();
                }
            } else if( tagName == "Colors" ) {
		QDomNode node = element.firstChild();
		while( !node.isNull() ) {
		    QDomElement element = node.toElement();
		    if( !element.isNull() ) { // was really an elemente
			QString tagName = element.tagName();
			if( tagName == "Event" ) {
                            QColor startColor, middleColor, endColor;
                            QDomNode node = element.firstChild();
                            while( !node.isNull() ) {
                                QDomElement element = node.toElement();
                                if( !element.isNull() ) { // was really an elemente
                                    QString tagName = element.tagName();
                                    if( tagName == "Start" ) {
                                        QColor value;
                                        if( KDXML::readColorNode( element, value ) )
                                            startColor = value;
                                    } else if( tagName == "Middle" ) {
                                        QColor value;
                                        if( KDXML::readColorNode( element, value ) )
                                            middleColor = value;
                                    } else if( tagName == "End" ) {
                                        QColor value;
                                        if( KDXML::readColorNode( element, value ) )
                                            endColor = value;
                                    } else {
                                        qDebug( "Unrecognized tag name: %s", tagName.latin1() );
                                        Q_ASSERT( false );
                                    }
                                }
                                node = node.nextSibling();
                            }
                            setColors( KDGanttViewItem::Event, startColor,
                                       middleColor, endColor );
                        } else if( tagName == "Task" ) {
                            QColor startColor, middleColor, endColor;
                            QDomNode node = element.firstChild();
                            while( !node.isNull() ) {
                                QDomElement element = node.toElement();
                                if( !element.isNull() ) { // was really an elemente
                                    QString tagName = element.tagName();
                                    if( tagName == "Start" ) {
                                        QColor value;
                                        if( KDXML::readColorNode( element, value ) )
                                            startColor = value;
                                    } else if( tagName == "Middle" ) {
                                        QColor value;
                                        if( KDXML::readColorNode( element, value ) )
                                            middleColor = value;
                                    } else if( tagName == "End" ) {
                                        QColor value;
                                        if( KDXML::readColorNode( element, value ) )
                                            endColor = value;
                                    } else {
                                        qDebug( "Unrecognized tag name: %s", tagName.latin1() );
                                        Q_ASSERT( false );
                                    }
                                }
                                node = node.nextSibling();
                            }
                            setColors( KDGanttViewItem::Task, startColor,
                                       middleColor, endColor );
                        } else if( tagName == "Summary" ) {
                            QColor startColor, middleColor, endColor;
                            QDomNode node = element.firstChild();
                            while( !node.isNull() ) {
                                QDomElement element = node.toElement();
                                if( !element.isNull() ) { // was really an elemente
                                    QString tagName = element.tagName();
                                    if( tagName == "Start" ) {
                                        QColor value;
                                        if( KDXML::readColorNode( element, value ) )
                                            startColor = value;
                                    } else if( tagName == "Middle" ) {
                                        QColor value;
                                        if( KDXML::readColorNode( element, value ) )
                                            middleColor = value;
                                    } else if( tagName == "End" ) {
                                        QColor value;
                                        if( KDXML::readColorNode( element, value ) )
                                            endColor = value;
                                    } else {
                                        qDebug( "Unrecognized tag name: %s", tagName.latin1() );
                                        Q_ASSERT( false );
                                    }
                                }
                                node = node.nextSibling();
                            }
                            setColors( KDGanttViewItem::Summary, startColor,
                                       middleColor, endColor );
                        } else {
                            qDebug( "Unrecognized tag name: %s", tagName.latin1() );
                            Q_ASSERT( false );
                        }
                    }
                    node = node.nextSibling();
                }
            } else if( tagName == "DefaultColors" ) {
                QDomNode node = element.firstChild();
                while( !node.isNull() ) {
                    QDomElement element = node.toElement();
                    if( !element.isNull() ) { // was really an element
                        QString tagName = element.tagName();
                        if( tagName == "Event" ) {
                            QColor value;
                            if( KDXML::readColorNode( element, value ) )
                                setDefaultColor( KDGanttViewItem::Event,
                                                 value );
                        } else if( tagName == "Task" ) {
                            QColor value;
                            if( KDXML::readColorNode( element, value ) )
                                setDefaultColor( KDGanttViewItem::Task,
                                                 value );
                        } else if( tagName == "Summary" ) {
                            QColor value;
                            if( KDXML::readColorNode( element, value ) )
                                setDefaultColor( KDGanttViewItem::Summary,
                                                 value );
                        } else {
                            qDebug( "Unrecognized tag name: %s", tagName.latin1() );
                            Q_ASSERT( false );
                        }
                    }

                    node = node.nextSibling();
                }
            } else if( tagName == "HighlightColors" ) {
		QDomNode node = element.firstChild();
		while( !node.isNull() ) {
		    QDomElement element = node.toElement();
		    if( !element.isNull() ) { // was really an elemente
			QString tagName = element.tagName();
			if( tagName == "Event" ) {
                            QColor startColor, middleColor, endColor;
                            QDomNode node = element.firstChild();
                            while( !node.isNull() ) {
                                QDomElement element = node.toElement();
                                if( !element.isNull() ) { // was really an elemente
                                    QString tagName = element.tagName();
                                    if( tagName == "Start" ) {
                                        QColor value;
                                        if( KDXML::readColorNode( element, value ) )
                                            startColor = value;
                                    } else if( tagName == "Middle" ) {
                                        QColor value;
                                        if( KDXML::readColorNode( element, value ) )
                                            middleColor = value;
                                    } else if( tagName == "End" ) {
                                        QColor value;
                                        if( KDXML::readColorNode( element, value ) )
                                            endColor = value;
                                    } else {
                                        qDebug( "Unrecognized tag name: %s", tagName.latin1() );
                                        Q_ASSERT( false );
                                    }
                                }
                                node = node.nextSibling();
                            }
                            setHighlightColors( KDGanttViewItem::Event,
                                                startColor,
                                                middleColor, endColor );
                        } else if( tagName == "Task" ) {
                            QColor startColor, middleColor, endColor;
                            QDomNode node = element.firstChild();
                            while( !node.isNull() ) {
                                QDomElement element = node.toElement();
                                if( !element.isNull() ) { // was really an elemente
                                    QString tagName = element.tagName();
                                    if( tagName == "Start" ) {
                                        QColor value;
                                        if( KDXML::readColorNode( element, value ) )
                                            startColor = value;
                                    } else if( tagName == "Middle" ) {
                                        QColor value;
                                        if( KDXML::readColorNode( element, value ) )
                                            middleColor = value;
                                    } else if( tagName == "End" ) {
                                        QColor value;
                                        if( KDXML::readColorNode( element, value ) )
                                            endColor = value;
                                    } else {
                                        qDebug( "Unrecognized tag name: %s", tagName.latin1() );
                                        Q_ASSERT( false );
                                    }
                                }
                                node = node.nextSibling();
                            }
                            setHighlightColors( KDGanttViewItem::Task,
                                                startColor,
                                                middleColor, endColor );
                        } else if( tagName == "Summary" ) {
                            QColor startColor, middleColor, endColor;
                            QDomNode node = element.firstChild();
                            while( !node.isNull() ) {
                                QDomElement element = node.toElement();
                                if( !element.isNull() ) { // was really an elemente
                                    QString tagName = element.tagName();
                                    if( tagName == "Start" ) {
                                        QColor value;
                                        if( KDXML::readColorNode( element, value ) )
                                            startColor = value;
                                    } else if( tagName == "Middle" ) {
                                        QColor value;
                                        if( KDXML::readColorNode( element, value ) )
                                            middleColor = value;
                                    } else if( tagName == "End" ) {
                                        QColor value;
                                        if( KDXML::readColorNode( element, value ) )
                                            endColor = value;
                                    } else {
                                        qDebug( "Unrecognized tag name: %s", tagName.latin1() );
                                        Q_ASSERT( false );
                                    }
                                }
                                node = node.nextSibling();
                            }
                            setHighlightColors( KDGanttViewItem::Summary,
                                                startColor,
                                                middleColor, endColor );
                        } else {
                            qDebug( "Unrecognized tag name: %s", tagName.latin1() );
                            Q_ASSERT( false );
                        }
                    }
                    node = node.nextSibling();
                }
            } else if( tagName == "DefaultHighlightColors" ) {
                QDomNode node = element.firstChild();
                while( !node.isNull() ) {
                    QDomElement element = node.toElement();
                    if( !element.isNull() ) { // was really an element
                        QString tagName = element.tagName();
                        if( tagName == "Event" ) {
                            QColor value;
                            if( KDXML::readColorNode( element, value ) )
                                setDefaultColor( KDGanttViewItem::Event,
                                                 value );
                        } else if( tagName == "Task" ) {
                            QColor value;
                            if( KDXML::readColorNode( element, value ) )
                                setDefaultColor( KDGanttViewItem::Task,
                                                 value );
                        } else if( tagName == "Summary" ) {
                            QColor value;
                            if( KDXML::readColorNode( element, value ) )
                                setDefaultColor( KDGanttViewItem::Summary,
                                                 value );
                        } else {
                            qDebug( "Unrecognized tag name: %s", tagName.latin1() );
                            Q_ASSERT( false );
                        }
                    }

                    node = node.nextSibling();
                }
            } else if( tagName == "Items" ) {
                QDomNode node = element.firstChild();
                KDGanttViewItem* previous = 0;
                while( !node.isNull() ) {
                    QDomElement element = node.toElement();
                    if( !element.isNull() ) { // was really an element
                        QString tagName = element.tagName();
                        if( tagName == "Item" ) {
                            KDGanttViewItem* newItem;
                            if( previous )
                                newItem =
                                    KDGanttViewItem::createFromDomElement( this,
                                                                           previous,
                                                                           element );
                            else
                                newItem =
                                    KDGanttViewItem::createFromDomElement( this,
                                                                           element );
                            previous = newItem;
                        } else {
                            qDebug( "Unrecognized tag name: %s", tagName.latin1() );
                            Q_ASSERT( false );
                        }
                    }

                    node = node.nextSibling();
                }
            } else if( tagName == "TaskLinks" ) {
                QDomNode node = element.firstChild();
                while( !node.isNull() ) {
                    QDomElement element = node.toElement();
                    if( !element.isNull() ) { // was really an element
                        QString tagName = element.tagName();
                        if( tagName == "TaskLink" )
                            KDGanttViewTaskLink::createFromDomElement( element );
                        else {
                            qDebug( "Unrecognized tag name: %s", tagName.latin1() );
                            Q_ASSERT( false );
                        }
                    }

                    node = node.nextSibling();
                }
            } else if( tagName == "TaskLinkGroups" ) {
                QDomNode node = element.firstChild();
                while( !node.isNull() ) {
                    QDomElement element = node.toElement();
                    if( !element.isNull() ) { // was really an element
                        QString tagName = element.tagName();
                        if( tagName == "TaskLinkGroup" )
                            KDGanttViewTaskLinkGroup::createFromDomElement( element );
                    } else {
                        qDebug( "Unrecognized tag name: %s", tagName.latin1() );
                        Q_ASSERT( false );
                    }

                    node = node.nextSibling();
                }
            } else if( tagName == "ColumnBackgroundColors" ) {
                QDomNode node = element.firstChild();
                while( !node.isNull() ) {
                    QDomElement element = node.toElement();
                    if( !element.isNull() ) { // was really an element
                        QString tagName = element.tagName();
                        if( tagName == "ColumnBackgroundColor" ) {
                            QDomNode node = element.firstChild();
                            QDateTime dateTime;
                            QColor color;
                            while( !node.isNull() ) {
                                QDomElement element = node.toElement();
                                if( !element.isNull() ) { // was
                                                          // really an
                                                          // element
                                    QString tagName = element.tagName();
                                    if( tagName == "DateTime" ) {
                                        QDateTime value;
                                        if( KDXML::readDateTimeNode( element, value ) )
                                            dateTime = value;
                                    } else if( tagName == "Color" ) {
                                        QColor value;
                                        if( KDXML::readColorNode( element, value ) )
                                            color = value;
                                    } else {
                                        qDebug( "Unrecognized tag name: %s", tagName.latin1() );
                                        Q_ASSERT( false );
                                    }
                                }

                                node = node.nextSibling();
                            }
                            setColumnBackgroundColor( dateTime, color );
                        } else {
                            qDebug( "Unrecognized tag name: %s", tagName.latin1() );
                            Q_ASSERT( false );
                        }
                    }
                    node = node.nextSibling();
                }
            } else if( tagName == "LegendItems" ) {
                clearLegend();
                QDomNode node = element.firstChild();
                while( !node.isNull() ) {
                    QDomElement element = node.toElement();
                    if( !element.isNull() ) { // was really an element
                        QString tagName = element.tagName();
                        if( tagName == "LegendItem" ) {
                            KDGanttViewItem::Shape tempLegendShape;
			    tempLegendShape = KDGanttViewItem::TriangleDown;
                            QColor tempLegendColor;
                            QString tempLegendString;
                            bool ok = true;
                            QDomNode node = element.firstChild();
                            while( !node.isNull() ) {
                                QDomElement element = node.toElement();
                                if( !element.isNull() ) { // was really an element
                                    QString tagName = element.tagName();
                                    if( tagName == "Shape" ) {
                                        QString value;
                                        if( KDXML::readStringNode( element, value ) )
                                            tempLegendShape = KDGanttViewItem::stringToShape( value );
                                        else
                                            ok = false;
                                    } else if( tagName == "Color" ) {
                                        QColor value;
                                        if( KDXML::readColorNode( element, value ) )
                                            tempLegendColor = value;
                                        else
                                            ok = false;
                                    } else if( tagName == "Text" ) {
                                        QString value;
                                        if( KDXML::readStringNode( element, value ) )
                                            tempLegendString = value;
                                        else
                                            ok = false;
                                    } else {
                                        qDebug( "Unrecognized tag name: %s", tagName.latin1() );
                                        Q_ASSERT( false );
                                    }
                                }
                                node = node.nextSibling();
                            }
                            if( ok ) {
                                addLegendItem( tempLegendShape,
                                               tempLegendColor,
                                               tempLegendString );
                                qDebug( "Adding legend item %s", tempLegendString.latin1() );
                            }
                        } else {
                            qDebug( "Unrecognized tag name: %s", tagName.latin1() );
                            Q_ASSERT( false );
                        }
                    }
                    node = node.nextSibling();
                }
            } else {
                qDebug( "Unrecognized tag name: %s", tagName.latin1() );
                Q_ASSERT( false );
            }
        }

        node = node.nextSibling();
    } // while
	return true; /* FIXME: Do real error-reporting. The ASSERT's should be "return false" stmnts */
} // method


/**
   Saves the view's parameters to an XML document.

   \param withPI pass true to store processing instructions, false to
   leave them out
   \return the XML document that represents the parameters
   \sa loadXML
*/
QDomDocument KDGanttView::saveXML( bool withPI ) const
{
    // Create an initial DOM document
    QString docstart = "<GanttView/>";

 QDomDocument doc( "GanttView" );
    doc.setContent( docstart );
    if( withPI )
        doc.appendChild( doc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );

    QDomElement docRoot = doc.documentElement();
    docRoot.setAttribute( "xmlns", "http://www.klaralvdalens-datakonsult.se/kdgantt" );
    docRoot.setAttribute( "xmlns:xsi", "http://www.w3.org/2000/10/XMLSchema-instance" );
    docRoot.setAttribute( "xsi:schemaLocation", "http://www.klaralvdalens-datakonsult.se/kdgantt" );

    // the ShowLegend element
    KDXML::createBoolNode( doc, docRoot, "ShowLegend", showLegend() );

    // the ShowListView element
    KDXML::createBoolNode( doc, docRoot, "ShowListView", showListView() );

    // the ShowTaskLinks element
    KDXML::createBoolNode( doc, docRoot, "ShowTaskLinks", showTaskLinks() );

    // the EditorEnabled element
    KDXML::createBoolNode( doc, docRoot, "EditorEnabled", editorEnabled() );

    // the global font element
    KDXML::createFontNode( doc, docRoot, "GlobalFont", font() );

    // the HorizonStart element
    KDXML::createDateTimeNode( doc, docRoot, "HorizonStart", horizonStart() );

    // the HorizonEnd element
    KDXML::createDateTimeNode( doc, docRoot, "HorizonEnd", horizonEnd() );

    // the Scale, MinimumScale, MaximumScale elements
    KDXML::createStringNode( doc, docRoot, "Scale", scaleToString( scale() ) );
    KDXML::createStringNode( doc, docRoot, "MinimumScale",
                             scaleToString( minimumScale() ) );
    KDXML::createStringNode( doc, docRoot, "MaximumScale",
                             scaleToString( maximumScale() ) );

    // the YearFormat element
    KDXML::createStringNode( doc, docRoot, "YearFormat",
                             yearFormatToString( yearFormat() ) );

    // the HourFormat element
    KDXML::createStringNode( doc, docRoot, "HourFormat",
                             hourFormatToString( hourFormat() ) );

    // the ShowMinorTicks element
    KDXML::createBoolNode( doc, docRoot, "ShowMinorTicks", showMinorTicks() );

    // the ShowMajorTicks element
    KDXML::createBoolNode( doc, docRoot, "ShowMajorTicks", showMajorTicks() );

    // the Editable element
    KDXML::createBoolNode( doc, docRoot, "Editable", editable() );

    // the TextColor element
    KDXML::createColorNode( doc, docRoot, "TextColor", textColor() );

    // the MajorScaleCount element
    KDXML::createIntNode( doc, docRoot, "MajorScaleCount", majorScaleCount() );

    // the MinorScaleCount element
    KDXML::createIntNode( doc, docRoot, "MinorScaleCount", minorScaleCount() );

    // the MinimumColumnWidth element
    KDXML::createIntNode( doc, docRoot, "MinimumColumnWidth",
                          minimumColumnWidth() );

    // the WeekendBackgroundColor element
    KDXML::createColorNode( doc, docRoot, "WeekendBackgroundColor",
                            weekendBackgroundColor() );

    // the WeekdayBackgroundColor elements
    for( int weekday = 1; weekday <= 7; weekday++ ) {
        QColor color = weekdayBackgroundColor( weekday );
        if( color.isValid() ) {
            QDomElement weekendBackgroundColorElement = doc.createElement( "WeekdayBackgroundColor" );
            docRoot.appendChild( weekendBackgroundColorElement );
            KDXML::createIntNode( doc, weekendBackgroundColorElement,
                                  "Day", weekday );
            KDXML::createColorNode( doc, weekendBackgroundColorElement,
                                    "Color", color );
        }
    }

    // the WeekendDays element
    QDomElement weekendDaysElement = doc.createElement( "WeekendDays" );
    docRoot.appendChild( weekendDaysElement );
    int weekendStart, weekendEnd;
    weekendDays( weekendStart, weekendEnd );
    weekendDaysElement.setAttribute( "Start", weekendStart );
    weekendDaysElement.setAttribute( "End", weekendStart );

    // the ZoomFactor element
    KDXML::createDoubleNode( doc, docRoot, "ZoomFactor",
                             zoomFactor() );

    // the ShowHeaderPopupMenu elemenet
    KDXML::createBoolNode( doc, docRoot, "ShowHeaderPopupMenu",
                           showHeaderPopupMenu() );

    // the Shapes element
    QDomElement shapesElement = doc.createElement( "Shapes" );
    docRoot.appendChild( shapesElement );
    QDomElement shapesEventElement = doc.createElement( "Event" );
    shapesElement.appendChild( shapesEventElement );
    KDGanttViewItem::Shape start, middle, end;
    if( shapes( KDGanttViewItem::Event, start, middle, end ) ) {
        KDXML::createStringNode( doc, shapesEventElement, "Start",
                                 KDGanttViewItem::shapeToString( start ) );
        KDXML::createStringNode( doc, shapesEventElement, "Middle",
                                 KDGanttViewItem::shapeToString( middle ) );
        KDXML::createStringNode( doc, shapesEventElement, "End",
                                 KDGanttViewItem::shapeToString( end ) );
    } else {
        KDXML::createStringNode( doc, shapesEventElement, "Start",
                                 "Undefined" );
        KDXML::createStringNode( doc, shapesEventElement, "Middle",
                                 "Undefined" );
        KDXML::createStringNode( doc, shapesEventElement, "End",
                                 "Undefined" );
    }
    QDomElement shapesTaskElement = doc.createElement( "Task" );
    shapesElement.appendChild( shapesTaskElement );
    if( shapes( KDGanttViewItem::Task, start, middle, end ) ) {
        KDXML::createStringNode( doc, shapesTaskElement, "Start",
                                 KDGanttViewItem::shapeToString( start ) );
        KDXML::createStringNode( doc, shapesTaskElement, "Middle",
                                 KDGanttViewItem::shapeToString( middle ) );
        KDXML::createStringNode( doc, shapesTaskElement, "End",
                                 KDGanttViewItem::shapeToString( end ) );
    } else {
        KDXML::createStringNode( doc, shapesTaskElement, "Start",
                                 "Undefined" );
        KDXML::createStringNode( doc, shapesTaskElement, "Middle",
                                 "Undefined" );
        KDXML::createStringNode( doc, shapesTaskElement, "End",
                                 "Undefined" );
    }
    QDomElement shapesSummaryElement = doc.createElement( "Summary" );
    shapesElement.appendChild( shapesSummaryElement );
    if( shapes( KDGanttViewItem::Event, start, middle, end ) ) {
        KDXML::createStringNode( doc, shapesSummaryElement, "Start",
                                 KDGanttViewItem::shapeToString( start ) );
        KDXML::createStringNode( doc, shapesSummaryElement, "Middle",
                                 KDGanttViewItem::shapeToString( middle ) );
        KDXML::createStringNode( doc, shapesSummaryElement, "End",
                                 KDGanttViewItem::shapeToString( end ) );
    } else {
        KDXML::createStringNode( doc, shapesSummaryElement, "Start",
                                 "Undefined" );
        KDXML::createStringNode( doc, shapesSummaryElement, "Middle",
                                 "Undefined" );
        KDXML::createStringNode( doc, shapesSummaryElement, "End",
                                 "Undefined" );
    }

    // the Colors element
    QDomElement colorsElement = doc.createElement( "Colors" );
    docRoot.appendChild( colorsElement );
    QDomElement colorsEventElement = doc.createElement( "Event" );
    colorsElement.appendChild( colorsEventElement );
    QColor startColor, middleColor, endColor;
    if( colors( KDGanttViewItem::Event, startColor, middleColor, endColor ) ) {
        KDXML::createColorNode( doc, colorsEventElement, "Start", startColor );
        KDXML::createColorNode( doc, colorsEventElement, "Middle", middleColor );
        KDXML::createColorNode( doc, colorsEventElement, "End", endColor );
    } else {
        KDXML::createColorNode( doc, colorsEventElement, "Start", QColor() );
        KDXML::createColorNode( doc, colorsEventElement, "Middle", QColor() );
        KDXML::createColorNode( doc, colorsEventElement, "End", QColor() );
    }
    QDomElement colorsTaskElement = doc.createElement( "Task" );
    colorsElement.appendChild( colorsTaskElement );
    if( colors( KDGanttViewItem::Task, startColor, middleColor, endColor ) ) {
        KDXML::createColorNode( doc, colorsTaskElement, "Start", startColor );
        KDXML::createColorNode( doc, colorsTaskElement, "Middle", middleColor );
        KDXML::createColorNode( doc, colorsTaskElement, "End", endColor );
    } else {
        KDXML::createColorNode( doc, colorsTaskElement, "Start", QColor() );
        KDXML::createColorNode( doc, colorsTaskElement, "Middle", QColor() );
        KDXML::createColorNode( doc, colorsTaskElement, "End", QColor() );
    }
    QDomElement colorsSummaryElement = doc.createElement( "Summary" );
    colorsElement.appendChild( colorsSummaryElement );
    if( colors( KDGanttViewItem::Event, startColor, middleColor, endColor ) ) {
        KDXML::createColorNode( doc, colorsSummaryElement, "Start", startColor );
        KDXML::createColorNode( doc, colorsSummaryElement, "Middle", middleColor );
        KDXML::createColorNode( doc, colorsSummaryElement, "End", endColor );
    } else {
        KDXML::createColorNode( doc, colorsSummaryElement, "Start", QColor() );
        KDXML::createColorNode( doc, colorsSummaryElement, "Middle", QColor() );
        KDXML::createColorNode( doc, colorsSummaryElement, "End", QColor() );
    }


    // the DefaultColor element
    QDomElement defaultColorsElement = doc.createElement( "DefaultColors" );
    docRoot.appendChild( defaultColorsElement );
    KDXML::createColorNode( doc, defaultColorsElement, "Event",
                            defaultColor( KDGanttViewItem::Event ) );
    KDXML::createColorNode( doc, defaultColorsElement, "Task",
                            defaultColor( KDGanttViewItem::Task ) );
    KDXML::createColorNode( doc, defaultColorsElement, "Summary",
                            defaultColor( KDGanttViewItem::Summary ) );


    // the HighlightColors element
    QDomElement highlightColorsElement = doc.createElement( "HighlightColors" );
    docRoot.appendChild( highlightColorsElement );
    QDomElement highlightColorsEventElement = doc.createElement( "Event" );
    highlightColorsElement.appendChild( highlightColorsEventElement );
    if( highlightColors( KDGanttViewItem::Event, startColor, middleColor, endColor ) ) {
        KDXML::createColorNode( doc, highlightColorsEventElement, "Start", startColor );
        KDXML::createColorNode( doc, highlightColorsEventElement, "Middle", middleColor );
        KDXML::createColorNode( doc, highlightColorsEventElement, "End", endColor );
    } else {
        KDXML::createColorNode( doc, highlightColorsEventElement, "Start", QColor() );
        KDXML::createColorNode( doc, highlightColorsEventElement, "Middle", QColor() );
        KDXML::createColorNode( doc, highlightColorsEventElement, "End", QColor() );
    }
    QDomElement highlightColorsTaskElement = doc.createElement( "Task" );
    highlightColorsElement.appendChild( highlightColorsTaskElement );
    if( highlightColors( KDGanttViewItem::Task, startColor, middleColor, endColor ) ) {
        KDXML::createColorNode( doc, highlightColorsTaskElement, "Start", startColor );
        KDXML::createColorNode( doc, highlightColorsTaskElement, "Middle", middleColor );
        KDXML::createColorNode( doc, highlightColorsTaskElement, "End", endColor );
    } else {
        KDXML::createColorNode( doc, highlightColorsTaskElement, "Start", QColor() );
        KDXML::createColorNode( doc, highlightColorsTaskElement, "Middle", QColor() );
        KDXML::createColorNode( doc, highlightColorsTaskElement, "End", QColor() );
    }
    QDomElement highlightColorsSummaryElement = doc.createElement( "Summary" );
    highlightColorsElement.appendChild( highlightColorsSummaryElement );
    if( highlightColors( KDGanttViewItem::Event, startColor, middleColor, endColor ) ) {
        KDXML::createColorNode( doc, highlightColorsSummaryElement, "Start", startColor );
        KDXML::createColorNode( doc, highlightColorsSummaryElement, "Middle", middleColor );
        KDXML::createColorNode( doc, highlightColorsSummaryElement, "End", endColor );
    } else {
        KDXML::createColorNode( doc, highlightColorsSummaryElement, "Start", QColor() );
        KDXML::createColorNode( doc, highlightColorsSummaryElement, "Middle", QColor() );
        KDXML::createColorNode( doc, highlightColorsSummaryElement, "End", QColor() );
    }

    // the DefaultHighlightColor element
    QDomElement defaultHighlightColorsElement = doc.createElement( "DefaultHighlightColors" );
    docRoot.appendChild( defaultHighlightColorsElement );
    KDXML::createColorNode( doc, defaultHighlightColorsElement, "Event",
                            defaultHighlightColor( KDGanttViewItem::Event ) );
    KDXML::createColorNode( doc, defaultColorsElement, "Task",
                            defaultHighlightColor( KDGanttViewItem::Task ) );
    KDXML::createColorNode( doc, defaultColorsElement, "Summary",
                            defaultHighlightColor( KDGanttViewItem::Summary ) );


    // the Items element
    QDomElement itemsElement = doc.createElement( "Items" );
    docRoot.appendChild( itemsElement );
    KDGanttViewItem* currentItem = firstChild();
    while( currentItem ) {
        currentItem->createNode( doc, itemsElement );
        currentItem = currentItem->nextSibling();
    }

    // the TaskLinks element
    QDomElement taskLinksElement = doc.createElement( "TaskLinks" );
    docRoot.appendChild( taskLinksElement );
    QPtrList<KDGanttViewTaskLink> taskLinkList = taskLinks();
    KDGanttViewTaskLink* currentTL = 0;
    for( currentTL = taskLinkList.first(); currentTL;
         currentTL = taskLinkList.next() )
        currentTL->createNode( doc, taskLinksElement );

    // the TaskLinkGroups element
    QDomElement taskLinkGroupsElement = doc.createElement( "TaskLinkGroups" );
    docRoot.appendChild( taskLinkGroupsElement );
    QPtrList<KDGanttViewTaskLinkGroup> taskLinkGroupList = taskLinkGroups();
    KDGanttViewTaskLinkGroup* currentTLG = 0;
    for( currentTLG = taskLinkGroupList.first(); currentTLG;
         currentTLG = taskLinkGroupList.next() )
        currentTLG->createNode( doc, taskLinkGroupsElement );

    // the ColumnBackgroundColors element
    QDomElement columnBackgroundColorsElement =
        doc.createElement( "ColumnBackgroundColors" );
    docRoot.appendChild( columnBackgroundColorsElement );
    KDTimeHeaderWidget::ColumnColorList ccList =
        myTimeHeader->columnBackgroundColorList();
    for( KDTimeHeaderWidget::ColumnColorList::iterator it = ccList.begin();
         it != ccList.end(); ++it ) {
        QDomElement columnBackgroundColorElement =
            doc.createElement( "ColumnBackgroundColor" );
        columnBackgroundColorsElement.appendChild( columnBackgroundColorElement );
        KDXML::createDateTimeNode( doc, columnBackgroundColorElement,
                                   "DateTime", (*it).datetime );
        KDXML::createColorNode( doc, columnBackgroundColorElement,
                                "Color", (*it).color );
    }

    // the LegendItems element
    QDomElement legendItemsElement =
        doc.createElement( "LegendItems" );
    docRoot.appendChild( legendItemsElement );
    legendItem* current;
    QPtrListIterator<legendItem> lit( *myLegendItems );
    while( ( current = lit.current() ) ) {
        ++lit;
        QDomElement legendItemElement = doc.createElement( "LegendItem" );
        legendItemsElement.appendChild( legendItemElement );
        KDXML::createStringNode( doc, legendItemElement, "Shape",
                                 KDGanttViewItem::shapeToString( current->shape ) );
        KDXML::createColorNode( doc, legendItemElement, "Color",
                                current->color );
        KDXML::createStringNode( doc, legendItemElement, "Text",
                                 current->text );
    }

    return doc;
}



QString KDGanttView::scaleToString( Scale scale )
{
    switch( scale ) {
    case Minute:
        return "Minute";
    case Hour:
        return "Hour";
    case Day:
        return "Day";
    case Week:
        return "Week";
    case Month:
        return "Month";
    case Auto:
        return "Auto";
    }
    return "";
}


KDGanttView::Scale KDGanttView::stringToScale( const QString& string )
{
    if( string == "Minute" )
        return Minute;
    else if( string == "Hour" )
        return Hour;
    else if( string == "Day" )
        return Day;
    else if( string == "Week" )
        return Week;
    else if( string == "Month" )
        return Month;
    else if( string == "Auto" )
        return Auto;

    return Auto;
}


QString KDGanttView::yearFormatToString( YearFormat format )
{
    switch( format ) {
    case FourDigit:
        return "FourDigit";
    case TwoDigit:
        return "TwoDigit";
    case TwoDigitApostrophe:
        return "TwoDigitApostrophe";
    }
    return "";
}


KDGanttView::YearFormat KDGanttView::stringToYearFormat( const QString& string )
{
    if( string == "FourDigit" )
        return FourDigit;
    else if( string == "TwoDigit" )
        return TwoDigit;
    else if( string == "TwoDigitApostrophe" )
        return TwoDigitApostrophe;
    else
        return FourDigit;
}


QString KDGanttView::hourFormatToString( HourFormat format )
{
    switch( format ) {
    case Hour_12:
        return "Hour_12";
    case Hour_24:
        return "Hour_24";
    }
    return "";
}


KDGanttView::HourFormat KDGanttView::stringToHourFormat( const QString& string )
{
    if( string == "Hour_12" )
        return Hour_12;
    else if( string == "Hour_24" )
        return Hour_24;
    else
        return Hour_24;
}


void KDGanttView::addTaskLinkGroup(KDGanttViewTaskLinkGroup* group)
{

  if (myTaskLinkGroupList.isEmpty()) {
    myTaskLinkGroupList.append(group);
    return;
  }
    if (myTaskLinkGroupList.find(group) == -1)
        myTaskLinkGroupList.append(group);
}


void KDGanttView::removeTaskLinkGroup(KDGanttViewTaskLinkGroup* group)
{
    myTaskLinkGroupList.remove(group);
}


void KDGanttView::editItem( KDGanttViewItem*  item)
{
  if ( ! item )
    return;
  if ( editorEnabled() ) {
    if ( item->editable() ) {
      myItemAttributeDialog->reset( item );
      myItemAttributeDialog->show();
    }
  }
}


QPixmap KDGanttView::getPixmap( KDGanttViewItem::Shape shape,
                                const QColor& shapeColor,
                                const QColor& backgroundColor, int itemSize)
{
  // 10 is a good value as size
  int size = itemSize+2;
  int hei = ( itemSize/3 ) / 2;
  QPixmap p = QPixmap( size+4, size+4 );
  p.fill( backgroundColor );
  QPainter paint (&p);
  QBrush b = QBrush ( Qt::SolidPattern );
  b.setColor( shapeColor );
  paint.setBrush( b );
  QPen pen( Qt::black, 1 ) ;
  paint.setPen( pen );
  switch (shape) {
  case KDGanttViewItem::TriangleDown:{
    QPointArray arr = QPointArray(3);
    arr.setPoint(0,-size/2,-hei);
    arr.setPoint(1,size/2,-hei);
    arr.setPoint(2,0,((size/2)-hei));
    arr.translate( ( size/2 ) +2 , ( size/2 ) +2);
    paint.drawPolygon( arr );
    break;
  }
  case KDGanttViewItem::TriangleUp :{
    QPointArray arr = QPointArray(3);
    arr.setPoint(0,-size/2,hei);
    arr.setPoint(1,size/2,hei);
    arr.setPoint(2,0,(-size/2)+hei);
    arr.translate( ( size/2 ) +2 , ( size/2 ) +2);
    paint.drawPolygon( arr );
    break;
  }
  case  KDGanttViewItem::Diamond :{
    QPointArray arr = QPointArray(4);
    arr.setPoint(0,0,-size/2);
    arr.setPoint(1,size/2,0);
    arr.setPoint(2,0,size/2);
    arr.setPoint(3,-size/2,0);
    arr.translate( ( size/2 ) +2 , ( size/2 ) +2);
    paint.drawPolygon( arr );
    break;
  }
  case KDGanttViewItem::Square :{
    QPointArray arr = QPointArray(4);
    arr.setPoint(0,-size/2,-size/2);
    arr.setPoint(1,size/2,-size/2);
    arr.setPoint(2,size/2,size/2);
    arr.setPoint(3,-size/2,size/2);
    arr.translate( ( size/2 ) +2 , ( size/2 ) +2);
    paint.drawPolygon( arr );
    break;
  }
  case  KDGanttViewItem::Circle  :{
    paint.drawEllipse( 2, 2, size, size);
    break;
  }
  }
  paint.end();
  return p;
}

int KDGanttView::getIndex( KDGanttViewItem::Type type) const
{
    int index = -1;
    switch (type) {
    case (KDGanttViewItem::Event):
        index = 0;
        break;
    case (KDGanttViewItem::Task):
        index = 1;
        break;
    case (KDGanttViewItem::Summary):
        index = 2;
        break;
    }
    return index;
}


void KDGanttView::initDefaults()
{
  int i;
  // We have 3 item types. Set all undefined to true.
    for (i = 0;i<3;++i) {
        undefinedShape[i] = true;
        undefinedColor[i] = true;
        undefinedColorHL[i] = true;
    }
    // setting the default colors
    myDefaultColor [ getIndex( KDGanttViewItem::Event ) ] =  Qt::blue; //event
    myDefaultColorHL [ getIndex( KDGanttViewItem::Event ) ] =  Qt::red;
    myDefaultColor [ getIndex( KDGanttViewItem::Task ) ] =  Qt::green;//task
    myDefaultColorHL [ getIndex( KDGanttViewItem::Task ) ] =  Qt::red;
    myDefaultColor [ getIndex( KDGanttViewItem::Summary ) ] =  Qt::cyan;//summary
    myDefaultColorHL [ getIndex( KDGanttViewItem::Summary ) ] =  Qt::red;

    // setting the default shape types
    // currently, we take for each item for all three shapes (start, middle, end) the same default shape
    for (i = 0;i<3;++i) {
      myDefaultShape [3*getIndex( KDGanttViewItem::Event )+ i] =  KDGanttViewItem::Diamond; //event
      myDefaultShape [3*getIndex( KDGanttViewItem::Task ) +i] =  KDGanttViewItem::Square; //task
      myDefaultShape [3*getIndex( KDGanttViewItem::Summary ) +i] =  KDGanttViewItem::TriangleDown; //summary

    }
}



/*!
  A call to this method is passed through to the underlying \a QListView.
*/
int KDGanttView::addColumn( const QString& label, int width )
{
    return myListView->addColumn( label, width );
}


/*!
  A call to this method is passed through to the underlying \a QListView.
*/

int KDGanttView::addColumn( const QIconSet& iconset, const QString& label,
                            int width )
{
    return myListView->addColumn( iconset, label, width );
}


/*!
  A call to this method is passed through to the underlying \a QListView.
*/
void KDGanttView::removeColumn( int index )
{
    myListView->removeColumn( index );
}


/*!
  A call to this method is passed through to the underlying \a QListView.
*/
KDGanttViewItem* KDGanttView::selectedItem() const
{
    return static_cast<KDGanttViewItem*>( myListView->selectedItem() );
}


/*!
  A call to this method is passed through to the underlying \a QListView.
*/
void KDGanttView::setSelected( KDGanttViewItem* item, bool selected )
{
    myListView->setSelected( item, selected );
}
/*!
  Returns the pointer to the gantt item with name \a name.
  If no item found, 0 is returned.
  If there are more than one item with the same name in the gantt view,
  the first item found will be returned. This may not be the first item in
  the listview.

  \param the name of the gantt item
  \return the pointer to the item with name \a name. O, if there is no item
  in the gantt view with this name.

*/
KDGanttViewItem* KDGanttView::getItemByName( const QString& name )
{
    KDGanttViewItem* temp =  firstChild(),* ret;
    while (temp != 0) {
      if ( (ret = temp->getChildByName( name ) ) )
	return ret;
      temp = temp->nextSibling();
    }
    return 0;
}
void KDGanttView::addTickRight()
{
  if ( _enableAdding && myCanvasView->horizontalScrollBar()->value() ==  myCanvasView->horizontalScrollBar()->maxValue()) {
    myCanvasView->horizontalScrollBar()->blockSignals( true );
    myTimeHeader->addTickRight();
    myCanvasView->horizontalScrollBar()->blockSignals( false );
    setTimelineToEnd();
  }
}
void KDGanttView::addTickLeft()
{
  if ( _enableAdding && myCanvasView->horizontalScrollBar()->value() == 0 ) {
    myCanvasView->horizontalScrollBar()->blockSignals( true );
    myTimeHeader->addTickLeft();
    myCanvasView->horizontalScrollBar()->blockSignals( false );
    setTimelineToStart();
  }
}
void KDGanttView::enableAdding( int val )
{
  _enableAdding = ( val == 0 || val == myCanvasView->horizontalScrollBar()->maxValue());
}


/*!
  Returns the number of items in the Gantt view.

  \param the number of items in the Gantt view.
*/
int KDGanttView::childCount() const
{
    return myListView->childCount();
}


/*!
  Removes all items from the Gantt view.
*/
void KDGanttView::clear()
{
  bool block = myTimeTable->blockUpdating();
  myTimeTable->setBlockUpdating( true );
  myListView->clear();
  myTimeTable->setBlockUpdating( block );
  myTimeTable->updateMyContent();
}
