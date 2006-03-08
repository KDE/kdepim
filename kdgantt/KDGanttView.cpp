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

// we need the protected QHeader method 
// QHeader::paintSection ( QPainter * p, int index, const QRect & fr )
// in order to print the header of the list view
// because we cannot reimplement QHeader to access this method we
// have to get access to the protected QHeader methods

#include "KDGanttView.h"
#include "KDGanttViewSubwidgets.h"
#include "KDGanttMinimizeSplitter.h"
#include "KDGanttViewItem.h"
#include "KDGanttViewTaskItem.h"
#include "KDGanttViewEventItem.h"
#include "KDGanttViewSummaryItem.h"
#include "KDGanttXMLTools.h"
#if QT_VERSION < 0x040000
#include "itemAttributeDialog.h"
#endif
#include <qprinter.h>
#include <qpainter.h>
#include <qlayout.h>
#include <qfile.h>
#include <qapplication.h>

#include <qmessagebox.h>
#include <qfileinfo.h>
#include <qtextstream.h>



#if defined KDAB_EVAL
#include "../evaldialog/evaldialog.h"
#endif

/*!
  \class KDGanttView KDGanttView.h
  This class represents a Gantt view with the Gantt chart, the header,
  an optional listview and an optional legend.

  In order to set up a Gantt view, create an object of this class, and
  populate it with a number of \a KDGanttViewItem objects.

  If you experience problems with the repainting of the content of the
  Gantt View after scrolling, call \a setRepaintMode().
*/

/*!
  Constructs an empty KDGanttView.

  \param parent the widget parent
  \param name the internal debugging name
*/


KDGanttView::KDGanttView( QWidget* parent, const char* name  )
    : KDGanttMinimizeSplitter( Qt::Vertical, parent, name ),
      myTimeHeaderScroll(0),
      myCanvasView(0)
{
  
#if defined KDAB_EVAL
    EvalDialog::checkEvalLicense( "KD Gantt" );
#endif
  myCurrentItem = 0;
  mUserHorizonChangeEnabled = true;
#if QT_VERSION < 0x040000
    setMinimizeDirection ( KDGanttMinimizeSplitter::Down );
#endif
    mySplitter = new KDGanttMinimizeSplitter( this );
#if QT_VERSION < 0x040000
    mySplitter->setMinimizeDirection ( KDGanttMinimizeSplitter::Left );
#endif
    leftWidget = new QVBox( mySplitter );
    rightWidget = new QVBox( mySplitter );
    //leftWidget->setMinimumSize( 20, 20 );
    //rightWidget->setMinimumSize( 20, 20 );
    myLegend = new KDLegendWidget( leftWidget, this );
    spacerLeft = new QWidget( leftWidget );
    spacerLeftLayout = new QHBoxLayout( spacerLeft );
    spacerLeftLayout->setMargin( 0 );
    spacerLeftLayout->setSpacing( 0 );
    
    myListView = new KDListView(leftWidget, this);
    myListView->setVScrollBarMode (QScrollView::AlwaysOff );
#if QT_VERSION < 0x040000
    connect( myListView, SIGNAL( selectionChanged( QListViewItem* ) ),
             this, SLOT( slotSelectionChanged( QListViewItem* ) ) );
    connect( myListView, SIGNAL( mouseButtonClicked ( int, QListViewItem * , const QPoint &, int ) ), 
             this, SLOT( slotmouseButtonClicked ( int , QListViewItem * , const QPoint &, int ) ) );
    connect( myListView, SIGNAL( contextMenuRequested ( QListViewItem * , const QPoint &, int  ) ), 
             this, SLOT( slotcontextMenuRequested ( QListViewItem * , const QPoint & , int ) ) );
    connect( myListView, SIGNAL(doubleClicked ( QListViewItem *  ) ), 
             this, SLOT(slotdoubleClicked ( QListViewItem * ) ) );
    connect( myListView, SIGNAL(expanded ( QListViewItem *  ) ), 
             this, SLOT(slotItemExpanded ( QListViewItem * ) ) );
    connect( myListView, SIGNAL(collapsed ( QListViewItem *  ) ), 
             this, SLOT(slotItemCollapsed( QListViewItem * ) ) );
    connect( myListView, SIGNAL(currentChanged( QListViewItem *  ) ), 
             this, SLOT(slotCurrentChanged ( QListViewItem * ) ) );
    connect( myListView, SIGNAL(itemRenamed ( QListViewItem * , int , const QString &  ) ), 
             this, SLOT(slotItemRenamed ( QListViewItem *, int , const QString &  ) ) );
    connect( myListView, SIGNAL(mouseButtonPressed(  int, QListViewItem * , const QPoint &, int ) ), 
             this, SLOT(slotMouseButtonPressed (  int , QListViewItem * , const QPoint & , int ) ) );
#else
    connect( myListView, SIGNAL( selectionChanged( Q3ListViewItem* ) ),
             this, SLOT( slotSelectionChanged( Q3ListViewItem* ) ) );
    connect( myListView, SIGNAL( mouseButtonClicked ( int, Q3ListViewItem * , const QPoint &, int ) ), 
             this, SLOT( slotmouseButtonClicked ( int , Q3ListViewItem * , const QPoint &, int ) ) );
    connect( myListView, SIGNAL( contextMenuRequested ( Q3ListViewItem * , const QPoint &, int  ) ), 
             this, SLOT( slotcontextMenuRequested ( Q3ListViewItem * , const QPoint & , int ) ) );
    connect( myListView, SIGNAL(doubleClicked ( Q3ListViewItem *  ) ), 
             this, SLOT(slotdoubleClicked ( Q3ListViewItem * ) ) );
    connect( myListView, SIGNAL(expanded ( Q3ListViewItem *  ) ), 
             this, SLOT(slotItemExpanded ( Q3ListViewItem * ) ) );
    connect( myListView, SIGNAL(collapsed ( Q3ListViewItem *  ) ), 
             this, SLOT(slotItemCollapsed( Q3ListViewItem * ) ) );
    connect( myListView, SIGNAL(currentChanged( Q3ListViewItem *  ) ), 
             this, SLOT(slotCurrentChanged ( Q3ListViewItem * ) ) );
    connect( myListView, SIGNAL(itemRenamed ( Q3ListViewItem * , int , const QString &  ) ), 
             this, SLOT(slotItemRenamed ( Q3ListViewItem *, int , const QString &  ) ) );
    connect( myListView, SIGNAL(mouseButtonPressed(  int, Q3ListViewItem * , const QPoint &, int ) ), 
             this, SLOT(slotMouseButtonPressed (  int , Q3ListViewItem * , const QPoint & , int ) ) );
#endif
    //connect( myListView, SIGNAL( ), this, SLOT( ) );
    myTimeTable = new KDTimeTableWidget (0,this);

    spacerRight = new QWidget(  rightWidget );

    myTimeHeaderContainer = new QHBox( rightWidget );
    myTimeHeaderContainer->setFrameStyle( QFrame::NoFrame  );
    myTimeHeaderContainer->setMargin( 0 );
    myTimeHeaderScroll = new QScrollView ( myTimeHeaderContainer );
    myTimeHeaderScroll->setHScrollBarMode( QScrollView::AlwaysOff );
    myTimeHeaderScroll->setVScrollBarMode( QScrollView::AlwaysOff );
    timeHeaderSpacerWidget = new QWidget( myTimeHeaderContainer );


    /*
    myTimeHeaderScroll = new QScrollView ( rightWidget );
    myTimeHeaderScroll->setHScrollBarMode( QScrollView::AlwaysOff );
    myTimeHeaderScroll->setVScrollBarMode( QScrollView::AlwaysOn );
    */
    //myTimeHeader = new KDTimeHeaderWidget (rightWidget,this);
    myTimeHeader = new KDTimeHeaderWidget (myTimeHeaderScroll->viewport(),this);
    myTimeHeaderScroll->addChild( myTimeHeader );
    myTimeHeaderScroll->viewport()->setBackgroundColor( myTimeHeader->backgroundColor() );
    timeHeaderSpacerWidget->setBackgroundColor( myTimeHeader->backgroundColor() );
    myCanvasView = new KDGanttCanvasView( this, myTimeTable, rightWidget);
    myTimeHeaderScroll->setFrameStyle( QFrame::NoFrame  );
    //
    myCanvasView->setFrameStyle( QFrame::NoFrame  );
    myCanvasView->setMargin( 0 );
    //
    myTimeHeaderScroll->setMargin( 0 );//myCanvasView->frameWidth() );
    setFrameStyle(myListView->frameStyle());
    setLineWidth( 2 );
    myListView->setFrameStyle( QFrame::NoFrame  );
    myListView->setMargin( 0 );
#if QT_VERSION < 0x040000
    QObject::connect(myListView, SIGNAL (  expanded ( QListViewItem * ) ) , 
                     myTimeTable , SLOT( expandItem(QListViewItem * ))) ;
    QObject::connect(myListView, SIGNAL (collapsed ( QListViewItem * ) ) , 
                     myTimeTable , SLOT(collapseItem(QListViewItem * ))) ;
#else
    QObject::connect(myListView, SIGNAL (  expanded ( Q3ListViewItem * ) ) , 
                     myTimeTable , SLOT( expandItem(Q3ListViewItem * ))) ;
    QObject::connect(myListView, SIGNAL (collapsed ( Q3ListViewItem * ) ) , 
                     myTimeTable , SLOT(collapseItem(Q3ListViewItem * ))) ;
#endif
    timeHeaderSpacerWidget->setFixedWidth(myCanvasView->verticalScrollBar()->width() );
    listViewIsVisible = true;
    chartIsEditable = true;
    editorIsEnabled = true;
    _displaySubitemsAsGroup = false;
    initDefaults();
    _showHeader = false;

    myTextColor = Qt::black;
    //QObject::connect( this, SIGNAL (itemDoubleClicked( KDGanttViewItem* ) ) , this, SLOT( editItem( KDGanttViewItem*  ))) ;
#if QT_VERSION < 0x040000
    myItemAttributeDialog = 0;
#endif
    setRepaintMode( KDGanttView::Medium );
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
#if QT_VERSION < 0x040000
    //connect( myCanvasView->horizontalScrollBar(), SIGNAL( prevLine () ) ,this, SLOT( addTickLeft()));
    //connect( myCanvasView->horizontalScrollBar(), SIGNAL( nextLine () ) ,this, SLOT( addTickRight()));
#else
    connect( myCanvasView->horizontalScrollBar(), SIGNAL( actionTriggered ( int ) ) ,this, SLOT( hScrollBarAction( int )));
    connect( myCanvasView->verticalScrollBar(), SIGNAL( actionTriggered ( int ) ) ,this, SLOT( vScrollBarAction( int )));
#endif
   // now initing
    fCenterTimeLineAfterShow = false;
    fDragEnabled = false;
    fDropEnabled = false;
    closingBlocked = false;
   myTimeHeader->computeTicks();
   centerTimelineAfterShow( QDateTime::currentDateTime () );
   setDisplayEmptyTasksAsLine( false );
   //QValueList<int> list;
   //list.append(240);
   //list.append(530);
   //mySplitter->setSizes( list );
   myTimeTable->setBlockUpdating();// block updating until this->show() is called
   mAddTickcountForTimeline = 1;
   myListView->setDefaultRenameAction( QListView::Reject );
}

KDGanttView::~KDGanttView()
{
    clearAll();
    // delete cut item, if there is any
    myCanvasView->resetCutPaste( 0 );
    delete myTimeTable;
}
/*!
  This virtual method returns 0.
  Reimplement this virtual method to create your own subclassed items 
  on a Drag&Drop operation or on save/load of data.
  Please read KDGanttView::createNewItem() for details.

  \param KDGanttViewItemTypeAsString the type of the new item as string. 
         Is not Task, Summary or Event but another name for subclassed items.
         Usually the value is used what KDGanttViewItem::typeString() returns for that subclass.
  \param parent item of the new item. May be 0. If 0 the new item becomes a root item in this KDGantt view.
  \param previous the item behind the new one should appear. May be 0.
  \param lvtext the text to show in the list view
  \param name the name by which the item can be identified. If no name
              is specified, a unique name will be generated
  \return the newly created item
  \sa createNewItem()
*/

void KDGanttView::vScrollBarAction( int action )
{
    //qDebug("vScrollBarAction %d ", action ); 
    switch ( mRepaintMode ) {
    case  No:

        break;
    case Medium:
        if ( action == 2 || action == 1 )
            forceRepaint();
        break;
    case Always:
        if ( action != 0 )
            forceRepaint();
        break;
    }

}
void KDGanttView::hScrollBarAction( int action )
{
    //qDebug("hScrollBarAction %d ", action );
    if (  action == 2 ) { // left
        addTickLeft();
    } else if ( action == 1 ) {
        addTickRight();
    }
    switch ( mRepaintMode ) {
    case  No:

        break;
    case Medium:
        if ( action == 2 || action == 1 )
            forceRepaint();
        break;
    case Always:
        if ( action != 0 )
            forceRepaint();
        break;
    }
}

KDGanttViewItem* KDGanttView::createUserDefinedItem( QString kdGanttViewItemTypeAsString,
                                                     KDGanttViewItem* parent,
                                                     KDGanttViewItem* after,
                                                     const QString& lvtext,
                                                     const QString& name )
{
    Q_UNUSED( kdGanttViewItemTypeAsString )
    Q_UNUSED( parent )
    Q_UNUSED( after )
    Q_UNUSED( lvtext )
    Q_UNUSED( name )
    qDebug("KDGanttView::createUserDefinedItem: Unknown type %s ", kdGanttViewItemTypeAsString.toLatin1() );
    return 0;
}
/*!
  Creates a KDGanttViewItem according to the specification in the parameters.
  Calls KDGanttView::createUserDefinedItem() for a user defined item type.
  Reimplement this virtual method in a subclass of KDGanttView to define specific behaviour when adding new 
  standard items and user defined (subclassed) items.
  In general it is not needed to reimplement this virtual method and it is sufficient to reimplement
  KDGanttView::createUserDefinedItem() only. 
  You can find details about subclassing and needed reimplementation of methods in the manual chapter 
  "Subclassing of KDGanttView and KDGanttViewItem" and there is a detailed real world 
  example program "timerTrackerApp" provided which demonstrates all reimplementations needed for 
  a proper implementation of subclassing.
  We give here a short overview of the problems and solutions.
  Well, there is in general no problem if you subclass KDGanttView.
  But if you subclass a KDGanttViewItem 
  (to be more precise: A KDGanttViewTaskItem,  KDGanttViewSummaryItem or KDGanttViewEventItem)
  and you enable Drag&Drop (DnD) or you want to save/load configuration data via
  KDGanttView::loadProject() or KDGanttView::saveProject() there is a problem:
  When DnD is performed or load/save is called there are items created automatically in KDGantt and added
  to KDGanttView. But if you drag a subclassed item you want to create an instance of that 
  subclass on a drop event, of course. 
  For that reason this method KDGanttView::createUserDefinedItem() is called every time a new item is created
  internally in KDGantt.
  Such that if you subclass a KDGanttViewItem you should subclass KDGanttView as well and reimplement 
  KDGanttView::createUserDefinedItem() and
  KDGanttViewItem::typeString().
  It is a good idea to reimplement 
  KDGanttViewItem::userWriteToElement() and
  KDGanttViewItem::userReadFromElement()
  as well to preserve item subclass specific properties during DnD or save/load.
  If you want some specific behaviour on dropping standard types you have to reimplement this method
  KDGanttView::createNewItem() as well. 

  \param KDGanttViewItemTypeAsString the type of the new item as string. 
         May be Task, Summary or Event for standard KDGanttViewItems. Should be another name for subclassed items.
         Usually the value is used what KDGanttViewItem::typeString() returns for that subclass.
         Calls createUserDefinedItem() if such a non standard type name is passed.
  \param parent item of the new item. May be 0. If 0 the new item becomes a root item in this KDGantt view.
  \param previous the item behind the new one should appear. May be 0.
  \param lvtext the text to show in the list view
  \param name the name by which the item can be identified. If no name
              is specified, a unique name will be generated
  \return the newly created item
  \sa createUserDefinedItem()
*/
KDGanttViewItem* KDGanttView::createNewItem( QString KDGanttViewItemTypeAsString,
                                             KDGanttViewItem* parent,
                                             KDGanttViewItem* after,
                                             const QString& lvtext,
                                             const QString& name )
{
    KDGanttViewItem* retItem = 0;
    KDGanttViewItem::Type type = KDGanttViewItem::stringToType( KDGanttViewItemTypeAsString );
   switch( type ) {
    case KDGanttViewItem::Event:
        if ( parent ) {
            if ( after ) 
                retItem = new KDGanttViewEventItem( parent, after, lvtext, name);
            else
                retItem = new KDGanttViewEventItem( parent, lvtext, name);
        } else {
            if ( after ) 
                retItem = new KDGanttViewEventItem( this, after, lvtext, name);
            else
                retItem = new KDGanttViewEventItem( this, lvtext, name);
        }
        break;
    case KDGanttViewItem::Summary:
        if ( parent ) {
            if ( after ) 
                retItem = new KDGanttViewSummaryItem( parent, after, lvtext, name);
            else
                retItem = new KDGanttViewSummaryItem( parent, lvtext, name);
        } else {
            if ( after ) 
                retItem = new KDGanttViewSummaryItem( this, after, lvtext, name);
            else
                retItem = new KDGanttViewSummaryItem( this, lvtext, name);
        }
        break;
    case KDGanttViewItem::Task:
        if ( parent ) {
            if ( after ) 
                retItem = new KDGanttViewTaskItem( parent, after, lvtext, name);
            else
                retItem = new KDGanttViewTaskItem( parent, lvtext, name);
        } else {
            if ( after ) 
                retItem = new KDGanttViewTaskItem( this, after, lvtext, name);
            else
                retItem = new KDGanttViewTaskItem( this, lvtext, name);
        }
        break;
    case KDGanttViewItem::UnknownType: 
        retItem = createUserDefinedItem( KDGanttViewItemTypeAsString,parent,after,lvtext,name );
        break;
    default:
        qDebug( "Unknown type in KDGanttView::createNewItem()" );
    }
   return retItem;
}
/*!
  Sets the column col of the list view to the width w .
  \param col column number of list view. col number 0 is the first column.
  \param w the width of the column

*/
void KDGanttView::setListViewColumnWidth ( int col, int w )
{
    myListView->setColumnWidth ( col, w );
}

/*!
  Returns a pointer to the header of the list view.
  The header is not visible per default.
  To show/hide the header you have to call setHeaderVisible()
  in order to get the internal layout properly updated.

  \return a pointer to the header of the list view.
  \sa setHeaderVisible(), headerVisible()
*/
QHeader * KDGanttView::listViewHeader () const
{
    return myListView->header();
}

/*!
  Sets all list view items with subitems which may be expanded recursively to expanded state.
  It makes sure that the item which was currently visible 
  at the top of the list view before the operation is visible after the operation.
  \sa setAllClose()
*/
void KDGanttView::setAllOpen()
{
   
    KDGanttViewItem* curItem = (KDGanttViewItem*) myListView->itemAt( QPoint (  myListView->width() - 10 , 5 ));
    bool block = myTimeTable->blockUpdating();
    myTimeTable->setBlockUpdating( true );
    KDGanttViewItem* temp = firstChild();
    while (temp != 0) {
      temp->setAllSubitemsExpanded( true );
      temp = temp->nextSibling();
    }
    myTimeTable->setBlockUpdating( block );
    myTimeTable->updateMyContent();
    if ( curItem && ! block ) {
        myListView->ensureItemVisible( curItem );
    }
        
}
/*!
  Sets all list view items with subitems recursively to closed state.
  It makes sure that the item which was currently visible 
  at the top of the list view before the operation is visible after the operation.
  \sa setAllOpen()
*/
void KDGanttView::setAllClose()
{
    KDGanttViewItem* curItem = (KDGanttViewItem*) myListView->itemAt( QPoint (  myListView->width() - 10 , 5 ));
    bool block = myTimeTable->blockUpdating();
    myTimeTable->setBlockUpdating( true );
    KDGanttViewItem* temp = firstChild();
    while (temp != 0) {
        temp->setAllSubitemsExpanded( false );
        temp = temp->nextSibling();
    }
    myTimeTable->setBlockUpdating( block );
    myTimeTable->updateMyContent();
    if ( curItem && !block ) {
        myListView->ensureItemVisible( curItem );
    }
}


/*!
  Enables or disables updating of the content of the Gantt view.
  To avoid flickering in the Gantt view while inserting large amounts
  of Gantt items, you should call

  bool upd = KDGanttView::getUpdateEnabled();
  KDGanttView::settUpdateEnabled( false );
    ...  insert items here  ...
  KDGanttView::settUpdateEnabled( upd );

  With this code, you avoid unwanted side effects with other parts in
  your code, where you disable (and re-enable) the update.

  When calling setUpdateEnabled( true ),
  all the content is recomputed, resized, and updated.

  Before calling show() for the first time, updating is disabled.
  When calling show(), updating is automatically enabled.

  \param enable if true, the content of the Gantt view is updated after
  every insertion of a new item.
  \sa getUpdateEnabled()
*/
void KDGanttView::setUpdateEnabled( bool enable )
{
  myTimeTable->setBlockUpdating( !enable );
  if ( enable ) {
      QTimer::singleShot( 0, this, SLOT ( updateGanttContent() ) );
      //updateGanttContent();
  }
}
void KDGanttView::updateGanttContent()
{
    myTimeTable->updateMyContent();
    myCanvasView->setMyContentsHeight( 0 );
}

/*!
  Returns whether updating is enabled or not.

  \return true, if updating is enabled
  \sa setUpdateEnabled()
*/

bool KDGanttView::getUpdateEnabled() const
{
  return !myTimeTable->blockUpdating();
}

/*!
  Sets the maximum width of the Gantt view part widget in pixels.
  The largest allowed width is 32767.
  Note that this does not set the maximum width of the Gantt view content,
  it sets the maximum width of the Gantt view widget itself.
  \param w the maximum width
*/

void KDGanttView::setGanttMaximumWidth( int w )
{
  myTimeHeader->setMaximumWidth ( w );
}
/*!
  Returns the maximum width of the Gantt view part widget in pixels.
  The default maximum width is 32767 pixels.

  \return the maximum width of the Gantt view part widget in pixels.
*/

int  KDGanttView::ganttMaximumWidth() const
{
  return myTimeHeader->maximumWidth();
}

/*!
  Updates the content of the GanttView and shows it.
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
  Closes the widget.
  The closing is rejected, if a repainting is currently being done.
  \param alsoDelete if true, the widget is deleted
  \return true, if the widget was closed
*/

bool KDGanttView::close ( bool alsoDelete )
{
  //qDebug("close ");
  if ( closingBlocked )
    return false;
  return QWidget::close ( alsoDelete );
}


/*!
  Returns a useful size for the view.
  Returned width:
  sizeHint().width() of the list view + width of TimeTable
  Returned height:
  height() of TimeHeader + height() of TimeTable + height() of Legend (if shown)
*/

QSize KDGanttView::sizeHint() const
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
  hintHeight += myTimeTable->minimumHeight()+myListView->frameWidth()*2+2;
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

  \param show true to show the legend button, false to hide it
  \sa showLegendButton()
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
  \sa setShowLegendButton()
*/
bool KDGanttView::showLegendButton() const
{
    return _showLegendButton;
}


/*!
  Specifies whether the listview header should be visible. By default,
  it is not visible.

  \param visible true to make the header visible, false to make it invisible
  \sa listViewHeader ()
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
  Returns whether the listview header is visible.

  \return whether the header is visible
  \sa listViewHeader ()
*/
bool KDGanttView::headerVisible() const
{
  return _showHeader;
}


/*!
  Returns the corresponding date and time of the coordinate X in the
  Gantt view.

  \param coordX the coordinate to search for
  \param global true if coordX is a global position, false otherwise
  \return the date and time at coordinate X in the Gantt view.
*/
QDateTime KDGanttView::getDateTimeForCoordX(int coordX, bool global ) const
{
  // default for myTimeHeader->getDateTimeForIndex() is local
   return myTimeHeader->getDateTimeForIndex(coordX, !global );
}


/*!
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
void KDGanttView::slotmouseButtonClicked ( int button, QListViewItem * item,
                                           const QPoint & pos, int c )
{
  KDGanttViewItem* gItem = static_cast<KDGanttViewItem*>( item );
  emit lvMouseButtonClicked ( button , gItem,  pos,  c );
  if (gItem == 0 && myCurrentItem != 0 ) {
    myCurrentItem = 0;
    emit lvCurrentChanged( gItem );
  }
  if (gItem != 0 && myCurrentItem == 0 ) {
    myCurrentItem = gItem;
    emit lvCurrentChanged( gItem );
  }

  // removed - makes no sense!
  //emit mouseButtonClicked ( button , gItem,  pos,  c );
   {
    switch ( button ) {
    case  Qt::LeftButton:
      emit lvItemLeftClicked( gItem );
      emit itemLeftClicked( gItem );
      break;
    case  Qt::MidButton:
      emit lvItemMidClicked( gItem );
      emit itemMidClicked( gItem );
      break;
    }
  }
}


/*
  Implements a casted pass-through of the contextMenuRequested() signal.
  The signal itemRightClicked() is emitted as well;
  the position is the global position.
*/
void KDGanttView::slotcontextMenuRequested ( QListViewItem * item, const QPoint & pos, int col )
{
    KDGanttViewItem* gItem = static_cast<KDGanttViewItem*>( item );
    emit lvContextMenuRequested ( gItem,  pos,  col );
    emit lvItemRightClicked( gItem );
    emit itemRightClicked( gItem );
}

/*
  Implements a casted pass-through of the collapsed() signal.
*/
void KDGanttView::slotItemCollapsed ( QListViewItem * item )
{
   {
    KDGanttViewItem* gItem = static_cast<KDGanttViewItem*>( item );
    emit itemCollapsed ( gItem );
  }
}


/*
  Implements a casted pass-through of the  expanded() signal.
*/
void KDGanttView::slotItemExpanded ( QListViewItem * item )
{
   {
       KDGanttViewItem* gItem = static_cast<KDGanttViewItem*>( item );
       emit itemExpanded( gItem );
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
  Implements a casted pass-through of the currentChanged() signal.
*/
void KDGanttView::slotCurrentChanged ( QListViewItem * item )
{
    KDGanttViewItem* gItem = static_cast<KDGanttViewItem*>( item );
    myCurrentItem = gItem;
    emit lvCurrentChanged( gItem );
}


/*
  Implements a casted pass-through of the itemRenamed() signal.
*/
void KDGanttView::slotItemRenamed ( QListViewItem * item , int col,
                                    const QString & text )
{
    KDGanttViewItem* gItem = static_cast<KDGanttViewItem*>( item );
    emit lvItemRenamed( gItem,  col, text );
}


/*
  Implements a casted pass-through of the mouseButtonPressed() signal.
*/
void KDGanttView::slotMouseButtonPressed ( int button, QListViewItem * item,
                                           const QPoint & pos, int c )
{
    KDGanttViewItem* gItem = static_cast<KDGanttViewItem*>( item );
    emit lvMouseButtonPressed( button, gItem,  pos,  c  );
}


/*!
  Specifies whether the content should be repainted after scrolling or
  not.

  \param mode If No, there is no repainting after scrolling. This is
                 the fastest mode.
              If Medium, there is extra repainting after releasing the
                 scrollbar. This provides fast scrolling with updated content
                 after scrolling. Recommended, when repaint problems occur.
                 This is the default value after startup.
              If Always, there is an extra update after every move of the
                 scrollbar. This entails slow scrolling with updated
                 content at all time.
*/
void KDGanttView::setRepaintMode( RepaintMode mode )
{

  QScrollBar  *cvh, *cvv;
  cvh = myCanvasView->horizontalScrollBar();
  cvv = myCanvasView->verticalScrollBar();
  // first disconnect
  mRepaintMode = mode;
  cvh->disconnect( this );
  cvv->disconnect( this );

#if QT_VERSION < 0x040000
  connect( myCanvasView->horizontalScrollBar(), SIGNAL( prevLine () ) ,this, SLOT( addTickLeft()));
  connect( myCanvasView->horizontalScrollBar(), SIGNAL( nextLine () ) ,this, SLOT( addTickRight()));
#endif
  switch ( mode ) {
  case  No:

    break;
  case Medium:
    connect( cvv, SIGNAL (sliderReleased () ) ,this, SLOT(forceRepaint()));
    connect( cvh, SIGNAL (sliderReleased () ) ,this, SLOT(forceRepaint()));
#if QT_VERSION < 0x040000
    connect( cvv, SIGNAL (nextLine () ) ,this, SLOT(forceRepaint()));
    connect( cvh, SIGNAL (nextLine () ) ,this, SLOT(forceRepaint()));
    connect( cvv, SIGNAL (prevLine () ) ,this, SLOT(forceRepaint()));
    connect( cvh, SIGNAL (prevLine () ) ,this, SLOT(forceRepaint()));
#endif
    break;
  case Always:
#if QT_VERSION < 0x040000
    connect( cvv, SIGNAL (valueChanged ( int ) ) ,this, SLOT(forceRepaint( int )));
    connect( cvh, SIGNAL (valueChanged ( int ) ) ,this, SLOT(forceRepaint( int )));
#endif
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
  int timeHeaderHeight = myTimeHeader->height()+myTimeHeaderScroll->frameWidth()*2;;
  int diffY = timeHeaderHeight-legendHeight-listViewHeaderHeight;
  if ( diffY < 0 ) {
    spacerLeft->setFixedHeight( 0 );
    spacerRight->setFixedHeight(-diffY);
  } else {
    spacerRight->setFixedHeight( 0 );
    spacerLeft->setFixedHeight( diffY );
  }
  myLegend->setFixedHeight( legendHeight );
  myTimeHeaderContainer->setFixedHeight( timeHeaderHeight );
}


/*!
  Specifies whether the legend should be shown as a dock window or not.

  \param show if true, show legend as a dock window
  \sa showLegend(), legendIsDockwindow(), legendDockwindow()
*/void KDGanttView::setLegendIsDockwindow( bool show )
{
  bool isdock = myLegend->asDockwindow();
  if ( show != isdock ) {
    myLegend->setAsDockwindow(show);
    // legend is cleared - reinit legend with list
    legendItem* li;
    for ( li = myLegendItems.first(); li; li = myLegendItems.next() ) {
        if ( li->has2 )
            myLegend->addLegendItem(li->shape, li->color, li->text,li->shape2, li->color2, li->text2 );
        else
            myLegend->addLegendItem(li->shape, li->color, li->text );
    }
  }
}


/*!
  Returns whether the legend is shown as a dock window

  \return true if the legend is shown as a dock window
  \sa setShowLegend(), setLegendIsDockwindow(), legendDockwindow()
*/bool KDGanttView::legendIsDockwindow() const
{
    return myLegend->asDockwindow();
}


/*!
  Returns the pointer to the legend dock window.
  DO NOT DELETE THIS POINTER!
  If the legend is not a dock window, 0 is returned
  To set the legend as a dock window, call
  KDGanttView::setLegendIsDockwindow( true );

  \return the pointer to the legend dock window
          0 is returned, if the legend is no dock window
	  DO NOT DELETE THIS POINTER!
  \sa setShowLegend(), setLegendIsDockwindow(),legendIsDockwindow()
*/
QDockWindow* KDGanttView::legendDockwindow() const
{
  return myLegend->dockwindow();
}


/*!
  Specifies whether the legend should be shown or not. Besides setting
  this programmatically, the user can also show/hide the legend by
  using the button provided for this purpose.

  \param show force legend to be shown
  \sa showLegend()
*/
void KDGanttView::setShowLegend( bool show )
{
    myLegend->showMe(show);
}


/*!
  Returns whether the legend is currently shown. The visibility of the
  legend can be changed both by \a setShowLegend(), and interactively
  by the user.

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
  Saves the state of the Gantt view in an IO device in XML format. The saved
  data can be reloaded with \a loadProject().

  \param device a pointer to the IO device in which to store the Gantt
  view state.
  \return true if the data could be written, false if an error
  occurred
  \sa loadProject()
*/

bool KDGanttView::saveProject( QIODevice* device )
{
    Q_ASSERT( device );

    QDomDocument doc = saveXML();
    if( device->isOpen() )
        device->close();
    if( device->open( IO_WriteOnly ) ) {
        QTextStream ts( device );
        ts << doc.toString();
        return true;
    } else
        return false;
}


/*!
  Loads a previously saved state of the Gantt view. All current
  settings and items are discarded before loading the data.

  \param device a pointer to the IO device from which to load the
  Gantt view state.
  \return true if the file could be read, false if an error
  occurred
  \sa saveProject()
*/

bool KDGanttView::loadProject( QIODevice* device )
{
    Q_ASSERT( device );

    if( device->isOpen() )
        device->close();
    if( device->open( IO_ReadOnly ) ) {
        QDomDocument doc( "GanttView" );
	QString err;
	int errline, errcol;
	if ( !doc.setContent( device, &err, &errline, &errcol ) ) {
	  qDebug("KDGantt::Error parsing XML data at line %d. Message is:", errline );
	  qDebug("%s ", err.toLatin1());
	  device->close();
	  return false;
	}
        device->close();
        clearAll();
        return loadXML( doc );
    } else
        return false;
}


/*!
  Sends a Gantt view to a printer. The printer should already be set
  up for printing (by calling QPrinter::setup()).
  If the printer is not set up, QPrinter::setup() is called before printing

  You can specify, whether the ListView, TimeLine, or Legend will be
  printed. All combinations of these three widgets are allowed.

  \param printer a pointer to the printer to print on. If printer is
  0, the method creates a temporary printer and discards it when it is
  done printing.
  \param printListView if true, the list view is printed
  \param printTimeLine if true, the time line is printed
  \param printLegend if true, the legend is printed

  \sa drawContents()
*/

void KDGanttView::print( QPrinter* printer ,
                         bool printListView, bool printTimeLine,
                         bool printLegend )
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
#if QT_VERSION >= 0x040000
  dx = (float) printer->width()  / (float)size.width();
  dy  = (float)(printer->height() - ( 2 * hei )) / (float)size.height();
#else
  QPaintDeviceMetrics m = QPaintDeviceMetrics ( printer );
  dx = (float) m.width()  / (float)size.width();
  dy  = (float)(m.height() - ( 2 * hei )) / (float)size.height();
#endif
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
  You can specify, whether the list view, the time line, or the legend
  is painted.
  All combinations of these three widgets are allowed.
  Returns the size of the painted area.
  Paints the list view in the top-left corner, the time line to the
  right of the list view, and the legend below the list view.
  If called with \a p = 0, nothing is painted and only
  the size of the painted area is computed.
  This is useful for determining only the painted area and setting
  the scale of the painter, before calling this method with a painter.
  In order to get the output fitted to your paper and your printer,
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

  For a detailed example, please see the commented source code in
  KDGanttView::print(...)

  \param p  a pointer to the painter to paint on. If p is
  0, nothing is painted and only the size of the painted area is computed
  \param drawListView if true, the list view is painted
  \param drawTimeLine if true, the time line is painted
  \param drawLegend if true, the legend is painted
  \return the size of the painted area
  \sa print()
*/
QSize KDGanttView::drawContents( QPainter* p,
		      bool drawListView , bool drawTimeLine, bool drawLegend )
{
    QSize size;
    int lvX, lvY, thX, thY, tlX, tlY, lwX, lwY, allX, allY;
    lvX = myListView->contentsWidth();
    lvY = myTimeTable->minimumHeight() + 2;
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
            if ( headerVisible() ) {
                p->translate( 0,  -myListView->header()->height());
                int cou = myListView->header()->count();
                int iii;
                QRect rect ( 0,0,0, myListView->header()->height());
                for ( iii = 0; iii < cou; ++iii ) {
                    rect.setLeft ( myListView->header()->sectionPos( iii ) ); 
                    rect.setRight ( myListView->header()->sectionPos( iii ) + myListView->header()->sectionSize (iii)); 
                    myListView->header()->paintSection ( p,  myListView->header()->mapToIndex (iii),  rect );
                }
                p->translate( 0,  myListView->header()->height());
            }
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
  Zooms such that the Gantt chart is less than the available space of the widget.

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

  \param start the start datetime of the selected period
  \param end the end datetime of the selected period

  \sa zoomToSelectionAndSetStartEnd()
  \sa setZoomFactor()
  \sa zoomFactor()
  \sa zoomToFit()
*/

void KDGanttView::zoomToSelection( const QDateTime& start,  const QDateTime&  end )
{

  myTimeHeader->zoomToSelection( start, end);

}
/*!
  Sets the horizon start and end to the  selected time period.
  Zooms so that at least the selected time period is visible after the zoom.

  \param start the start datetime of the selected period
  \param end the end datetime of the selected period

  \sa zoomToSelection()
  \sa setZoomFactor()
  \sa zoomFactor()
  \sa zoomToFit()
*/

void KDGanttView::zoomToSelectionAndSetStartEnd( const QDateTime& start,  const QDateTime&  end )
{
    /*
    myTimeHeader->setHorizonStart(start);
    myTimeHeader->setHorizonEnd(start);
    myTimeHeader->zoomToSelection( start, end);
    */ 
    myTimeHeader->zoomToSelectionAndSetStartEnd( start, end);
}


/*!
  Makes sure that the specified Gantt item is visible without
  scrolling.

  \sa center(), centerTimelineAfterShow()
*/
void KDGanttView::ensureVisible( KDGanttViewItem* item )
{
    if ( item == 0 ) return;
    myListView->ensureItemVisible (item);
}


/*!
  Makes sure that the specified QDateTime is in the center of the
  visible Gantt chart (if possible).
  If you want to center the timeline when the KDGanttView is hidden,
  calling centerTimelineAfterShow() is the better alternative.
  There are three possibilities what does happen with the timeline
  if you call this method:
  If the specified QDateTime is within the datetime range of
  horizonStart() and horizonEnd() then the specified QDateTime is made visible
  on the timeline and the specified QDateTime is centered, if possible.
  (It is not possible to center it if the specified QDateTime is too near to the 
  horizonStart() or horizonEnd(), of course.)
  If userHorizonChangeEnabled() is enabled and the specified QDateTime 
  is not within the datetime range of  horizonStart() and horizonEnd()
  then horizonStart() and horizonEnd() are moved such that
  the specified QDateTime is in the center of the new horizonStart() and horizonEnd()
  and the timeline is displayed centered on the specified QDateTime.
  
  \sa center(), centerTimelineAfterShow() userHorizonChangeEnabled()
*/
void KDGanttView::centerTimeline( const QDateTime& center )
{
    myTimeHeader->centerDateTime( center, mUserHorizonChangeEnabled );
}


/*!
  Makes sure that the specified QDateTime is in the center of the
  visible Gantt chart (if possible).  If the KDGanttView is currently
  hidden, this method resets the center once again after the next
  show() call. Use this method if you want to center the timeline when
  the KDGanttView is hidden. After calling KDGanttView::show(), there
  may be computations of the sizes of the widgets and subwidgets and
  of the automatically computed start datetime. This method ensures
  that the center of the timeline is to be properly reset after
  show().

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
  Sets the timeline to the horizon start.
*/

void KDGanttView::setTimelineToStart()
{
  myCanvasView->horizontalScrollBar()->setValue( 0 );
}


/*!
  Sets the timeline to the horizon end.
*/
void KDGanttView::setTimelineToEnd()
{
 myCanvasView->horizontalScrollBar()->setValue(myCanvasView->horizontalScrollBar()->maxValue());
}


/*!
  Add \a num minor ticks of the current scale of the timeline
  to the start of the timeline.
  The timeline is not set automatically at the start.
  Call \a setTimelineToStart() to ensure that the timeline is at the start
  after calling this method.

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
  The timeline is not set automatically at the end.
  Call \a setTimelineToEnd() to ensure that the timeline is at the end
  after calling this method.
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
  Sets the font in the left list view widget and
  in the right time header widget.
  The settings of the fonts in the time table widget are not effected.

  \param font the new font of the widget
*/
void KDGanttView::setFont(const QFont& font)
{
    myListView->setFont(font);
    myListView->repaint();
    myTimeHeader->setFont(font);
    myLegend->setFont( font );
    QWidget::setFont( font );
    setScale(scale());
    QTimer::singleShot( 0, this, SLOT( slotHeaderSizeChanged() ) );
}


/*!
  Specifies whether the configure popup menu should be shown on
  right click on the time header widget.
  This menu lets the user quickly change
  the zoom factor,
  the scale mode (minute, hour, day, week, month, auto) ,
  the time format,
  the year format,
  the grid format,
  and printing.
  The default setting is not to show the popup menu.
  This functionality must be enabled explicitly by the application
  developer.
  You can disable each submenu of the popmenu.

  \param show true in order to show the popup menu, false in order not
  to. The default is true.
  \param showZoom show the zoom submenu, default: true
  \param showScale show the scale submenu, default: true
  \param showTime show the time format submenu, default: true
  \param showYear show the year format submenu, default: true
  \param showGrid show the grid submenu, default: true
  \param showPrint show the print submenu, default: false
*/
void KDGanttView::setShowHeaderPopupMenu( bool show,
                                          bool showZoom,
                                          bool showScale,
                                          bool showTime,
                                          bool showYear,
                                          bool showGrid,
                                          bool showPrint)
{
    if ( show && myTimeHeader->showPopupMenu() ) return;
    myTimeHeader->setShowPopupMenu( show,showZoom,showScale,showTime,
                                    showYear,showGrid,showPrint );
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
  This menu lets the user quickly add new items to the Gantt view
  (as root, as child or after an item).
  It also offers cutting and pasting of items.

  The default setting is that the popup menu is not shown.
  It must be enabled by the program.

  \param show true in order to show popup menu, false in order not to

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

  This setting overrides any shape settings made on individual items.
  These settings will be taken as initial values of any newly created
  item of this certain type.
  See also the documentation of the KDGanttViewItem class.

  \param type the type of Gantt items for which to set the shapes
  \param start the shape to use for the beginning of the item
  \param middle the shape to use for the middle of the item
  \param end the shape to use for the end of the item
  \param overwriteExisting if true, overwrites existing shape settings
  in the individual items
  \sa shapes()
*/
void KDGanttView::setShapes( KDGanttViewItem::Type type,
                             KDGanttViewItem::Shape start,
                             KDGanttViewItem::Shape middle,
                             KDGanttViewItem::Shape end,
                             bool overwriteExisting )
{
    if ( overwriteExisting ) {
      QListViewItemIterator it(myListView);
      for ( ; it.current(); ++it ) {
        if ( ((KDGanttViewItem*)it.current())->type() == type)
	  ((KDGanttViewItem*)it.current())->setShapes(start,middle, end );
      }
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
  These settings will be taken as initial values of any newly created
  item of this certain type.
  See also the description of the KDGanttViewItem class.

  \param type the type of Gantt items for which to set the colors
  \param start the color to use for the beginning of the item
  \param middle the color to use for the middle of the item
  \param end the color to use for the end of the item
  \param overwriteExisting if true, overwrites existing color settings
  on invididual items
  \sa colors(), setDefaultColors(), defaultColors()
*/
void KDGanttView::setColors( KDGanttViewItem::Type type,
                             const QColor& start, const QColor& middle,
                             const QColor& end,
                             bool overwriteExisting )
{
    if ( overwriteExisting ) {
      QListViewItemIterator it(myListView);
      for ( ; it.current(); ++it ) {
        if ( ((KDGanttViewItem*)it.current())->type() == type)
	  ((KDGanttViewItem*)it.current())->setColors(start,middle, end );
      }
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
  individual items.
  These settings will be taken as initial values of any newly created
  item of this certain type.
  See also the description of the KDGanttViewItem class.

  \param type the type of Gantt items for which to set the highlight colors
  \param start the highlight color to use for the beginning of the item
  \param middle the highlight color to use for the middle of the item
  \param end the highlight color to use for the end of the item
  \param overwriteExisting if true, overwrites existing color settings
  in the individual items
  \sa highlightColors(), setDefaultHighlightColor(), defaultHighlightColor()
*/
void KDGanttView::setHighlightColors( KDGanttViewItem::Type type,
                                      const QColor& start,
                                      const QColor& middle,
                                      const QColor& end,
                                      bool overwriteExisting )
{
    if ( overwriteExisting ) {
      QListViewItemIterator it(myListView);
      for ( ; it.current(); ++it ) {
        if ( ((KDGanttViewItem*)it.current())->type() == type)
	  ((KDGanttViewItem*)it.current())->setHighlightColors(start,middle, end );
      }
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
  Overrides all individual settings of the Gantt items.

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
  \sa setTextColor()
*/
QColor KDGanttView::textColor() const
{
    return myTextColor;
}


/*!
  Specifies the brush in which the 'showNoInformation' line of items
  should be drawn.

  \param  brush the brush of the 'showNoInformation' lines
  \sa  KDGanttViewItem::showNoInformation(),
  KDGanttViewItem::setShowNoInformation(),
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
QBrush KDGanttView::noInformationBrush() const
{
  return myTimeTable->noInformationBrush();
}


/*!
  Removes all items from the legend.

  \sa addLegendItem()
*/
void KDGanttView::clearLegend( )
{
    setShowLegend( false );
    myLegend->clearLegend();
    myLegendItems.setAutoDelete( true );
    myLegendItems.clear();
}


/*!
  Adds an item to the legend.

  \param shape the shape to display
  \param shapeColor the color in which to display the shape
  \param text the text to display
  \sa clearLegend()
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
    item->has2 = false;
    myLegendItems.append( item );
}

/*!
  Adds an item to the legend with two shapes and text right of the shapes.
  The text may be empty.

  \param shape the first shape to display
  \param shapeColor the color in which to display the first shape
  \param text the text to display right of the first shape. may be empty.
  \param shape2 the second shape to display
  \param shapeColor2 the color in which to display the second shape
  \param text2 the text to display right of the second shape. may be empty.
  \sa clearLegend()
*/
void KDGanttView::addLegendItem( KDGanttViewItem::Shape shape,
                                 const QColor& shapeColor, 
                                 const QString& text,
                                 KDGanttViewItem::Shape shape2,
                                 const QColor& shapeColor2,
                                 const QString& text2 )
{
    myLegend->addLegendItem( shape,shapeColor,text,shape2,shapeColor2,text2 );
    legendItem* item = new legendItem;
    item->shape = shape;
    item->color = shapeColor;
    item->text = text;
    item->has2 = true;
    item->shape2 = shape2;
    item->color2 = shapeColor2;
    item->text2 = text2;

    myLegendItems.append( item );
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
void KDGanttView::setHorizonEnd( const QDateTime& end )
{
    myTimeHeader->setHorizonEnd(end);
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
  unit is computed automatically. Does not update the header.
  Usually used to avoid flicker if immediately another header changing operation follows.

  \param unit the unit of the lower scale of the header.
  \sa scale() setScale()
*/
void KDGanttView::setScaleSilent( Scale unit )
{
    myTimeHeader->setScale( unit, false );
}

/*!
  Configures the unit of the lower scale of the header. The higher
  unit is computed automatically.

  \param unit the unit of the lower scale of the header.
  \sa scale() setScaleSilent()
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
  Sets the maximum allowed time scale of the lower scale of the header.

  \param unit the unit of the lower scale of the header.
  \sa scale()
*/
void KDGanttView::setMaximumScale( Scale unit )
{
    myTimeHeader->setMaximumScale( unit );
}


/*!
  Returns the maximum allowed time scale of the lower scale of the header.

  \return the unit of the lower scale of the header.
  \sa setScale()
*/
KDGanttView::Scale KDGanttView::maximumScale() const
{
    return myTimeHeader->maximumScale();
}


/*!
  Sets the minimum allowed time scale of the lower scale of the header.

  \param unit the unit of the lower scale of the header.
  \sa scale()
*/
void KDGanttView::setMinimumScale( Scale unit )
{
    myTimeHeader->setMinimumScale( unit );
}


/*!
  Returns the minimum allowed time scale of the lower scale of the header.

  \return the unit of the lower scale of the header.
  \sa setScale()
*/
KDGanttView::Scale KDGanttView::minimumScale() const
{
    return myTimeHeader->minimumScale();
}


/*!
  Sets the absolute number of minor ticks, if scaling is set to Auto.
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
  Returns the absolut number of minor ticks, if scaling is set to Auto

  \return the absolut number of minor ticks
  \sa setAutoScaleMinorTickCount(),setScale(),scale()
*/
int KDGanttView::autoScaleMinorTickCount() const
{
  return myTimeHeader->autoScaleMinorTickCount();
}


/*!
  Sets the minimum width that a column needs to have. If the size of the
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
  Hides/shows the grid for the major ticks of the time header in the gantt view.

  \param show true in order to show ticks, false in order to hide them.
         If show is true, setShowMinorTicks( false ) is performed automatically
         to hide the grid of the minor ticks.
         In order to show now grid, call setShowMinorTicks( false ) and 
         setShowMajorTicks( false ).
  \sa showMajorTicks(), setShowMinorTicks(), showMinorTicks()
*/
void KDGanttView::setShowMajorTicks( bool show )
{
    myTimeHeader->setShowMajorTicks(show );
}


/*!
  Returns whether the grid is shown on the major scale.

  \return true if ticks are shown on the major scale
  \sa setShowMajorTicks(), setShowMinorTicks(), showMinorTicks()
*/
bool KDGanttView::showMajorTicks() const
{
    return myTimeHeader->showMajorTicks();
}


/*!
  Hides/shows the grid for the minor ticks of the time header in the gantt view.

  \param show true in order to show ticks, false in order to hide them.
         If show is true, setShowMajorTicks( false ) is performed automatically
         to hide the grid of the major ticks.
         In order to show now grid, call setShowMinorTicks( false ) and 
         setShowMajorTicks( false ).

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
  It can be specified whether the color should be shown in all scales or
  only in specific scales.
  If you want to define the color only for the daily view, specify
  mini and maxi as Day.
  If there is no value specified for mini and maxi, the color for the column
  is shown on all scales. Note that it is possible that there are two
  values for a column in a scale. In this case, the shown color is unspecified.

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
  Sets the background color for a time interval given by \a start and
  \a end.  \a start may be later than \a end.  If there is already a
  background interval with the same \a start and \a end values
  defined, the values (i.e.  const QColor& color , Scale mini, Scale
  maxi) of this background interval are changed.  Change the times of
  an already defined interval with \a changeBackgroundInterval().
  Delete an already defined interval with \a
  deleteBackgroundInterval().
s
  It can be defined, whether the color should be shown in all scales or
  only in specific scales.
  If you want to define the color only for the daily view, scecify
  mini and maxi as Day.
  If there is no value for mini/maxi specified, the color for the columns
  is shown in all scales.

  \param start start datetime of the time interval
  \param end end datetime of the time interval
  \param priority paint priority. May be 0 to 10. 
                  An interval with higher priority is painted over other intervals.
  \param color the background color
  \param mini show the color only in scales greater than this
  \param maxi show the color only in scales lesser than this
  \sa changeBackgroundInterval(), deleteBackgroundInterval(),
  columnBackgroundColor(), setWeekendBackgroundColor(),
  weekendBackgroundColor()
*/
void KDGanttView::setIntervalBackgroundColor( const QDateTime& start,
                                              const QDateTime& end,
                                              const QColor& color ,
                                              int priority,
                                              Scale mini, Scale maxi )
{
    myTimeHeader->setIntervalBackgroundColor( start, end, color, priority, mini,maxi );
}


/*!
  Changes the times of an already defined background color interval.
  The new values \a startnew and \a endnew should not be datetime
  values of an already defined background color interval.
  If that is the case, nothing is changed and false is returned.

  \param oldstart the start date and time of the interval to change
  \param oldend the end date and time of the interval to change
  \param newstart the new start date and time
  \param newend the new end date and time
  \return true, if there is a backgroundcolor interval with values
  \a start and \a end found  and the new values \a startnew and \a endnew
  are not datetime values of an already defined background color interval.
          Returns false otherwise.
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
  Deletes an already defined background color interval.

  \param start start datetime of time interval
  \param end end datetime of time interval
  \return true, if there is a backgroundcolor interval with values
  \a start and \a end found  (and hence deleted).
  \sa changeBackgroundInterval(),  columnBackgroundColor()
*/
bool KDGanttView::deleteBackgroundInterval( const QDateTime& start,
						   const QDateTime& end)
{
  return myTimeHeader->deleteBackgroundInterval( start, end );
}


/*!
  Removes all background color settings set with setColumnBackgroundColor()
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
  Returns the background color for the column closest to \a column.

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
  effect. The days are specified as an intervals of integer values
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

  \param weekday the day of the week (Monday = 1, Sunday = 7)
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
  \fn void KDGanttView::itemDoubleClicked( KDGanttViewItem* )

  This signal is emitted when the user double-clicks an item.
*/


/*!
  \fn void KDGanttView::itemConfigured( KDGanttViewItem* )

  This signal is emitted when the user has configured an item
  visually. This signal is emitted on every mouse move when the user
  is changing an item.
  That means this signal is emitted every time when the user is about 
  to configure an item.
 /sa itemChanged()  
*/
/*!
  \fn void KDGanttView::itemChanged( KDGanttViewItem* )

  This signal is emitted after the mouse key is released 
  when the user has configured an item visually.
  That means this signal is emitted after the user has finished
  the configured of an item.
  /sa itemConfigured()  
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
  \fn void KDGanttView::taskLinkDoubleClicked( KDGanttViewTaskLink* )

  This signal is emitted when the user double-clicks a task link.
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
  \param overwriteExisting if true, existing settings for individual
  items are overwritten
  \sa defaultColor(), setColors(), colors()
*/
void KDGanttView::setDefaultColor( KDGanttViewItem::Type type,
                                   const QColor& color,
                                   bool overwriteExisting )
{
    if ( overwriteExisting ) {
      QListViewItemIterator it(myListView);
      for ( ; it.current(); ++it ) {
        if ( ((KDGanttViewItem*)it.current())->type() == type)
	  ((KDGanttViewItem*)it.current())->setDefaultColor(color );
      }
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
  \param overwriteExisting if true, existing color settings in
  individual items are overwritten
  \sa defaultHighlightColor(), setHighlightColors(), highlightColors()
*/
void KDGanttView::setDefaultHighlightColor( KDGanttViewItem::Type type,
                                            const QColor& color,
                                            bool overwriteExisting )
{
    if ( overwriteExisting ) {
      QListViewItemIterator it(myListView);
      for ( ; it.current(); ++it ) {
        if ( ((KDGanttViewItem*)it.current())->type() == type)
	  ((KDGanttViewItem*)it.current())->setDefaultHighlightColor(color );
      }
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
  This method turns calendar mode on and off.  In calendar mode, only
  those items can be opened which have subitems which have
  subitems. I.e., if an item contains multiple calendars, it can be
  opened, but not a calendar item itself.  If you want to use this
  GanttView as a calendar view, you have to call
  setDisplaySubitemsAsGroup( true ); to use the root items as calendar
  items.  To create new calendar entries for these root items, create
  a new KDGanttViewTaskItem with this root item as a parent.  If you
  want an item with subitems to behave like a calendar (which is
  possibly empty at startup), please call setIsCalendar( true ); for
  this item.

  \param mode if true, the calendar view mode is turned on
              if false, the calendar view mode is turned off
  \sa setDisplaySubitemsAsGroup(), displaySubitemsAsGroup(), calendarMode()
*/
void KDGanttView::setCalendarMode( bool mode )
{
  myListView->setCalendarMode( mode );
}


/*!
  Returns true, if the Gantt view is in calendar mode. See
  setCalendarMode() for the meaning of calendar mode.

  \return returns true, if the Gantt view is in calendermode
  \sa setCalendarMode()
*/
bool  KDGanttView::calendarMode() const
{
  return  myListView->calendarMode();
}



/*!
  This method specifies whether hidden subitems should be displayed.
  It iterates over all KDGanttViewItems in this Gantt view
  and sets their displaySubitemsAsGroup() property.
  All newly created items will have this setting by default.
  \param show if true, the hidden subitems are displayed in all items of
         this Gantt view.
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
  Returns, whether new items are created with the
  displayHiddenSubitems property.
  \return true, if hidden subitems should be displayed on newly created items.
  \sa setDisplaySubitemsAsGroup(),
  KDGanttViewItem::setDisplaySubitemsAsGroup(),
  KDGanttViewItem::displaySubitemsAsGroup()
*/
bool KDGanttView::displaySubitemsAsGroup() const
{
  return _displaySubitemsAsGroup;
}


/*!
  This method specifies whether tasks where the start time and the end
  time are the same are displayed
  as a line over the full height of the Gantt view.
  \param show if true, tasks with starttime == endtime are displayed
  as a line
*/
void KDGanttView::setDisplayEmptyTasksAsLine( bool show )
{
  _displayEmptyTasksAsLine = show;
}


/*!
  Returns, whether tasks where the start time and the end time are the
  same are displayed
  as a line over the full height of the Gantt view.
  \return true, if empty tasks are displayed as line.
*/
bool KDGanttView::displayEmptyTasksAsLine() const
{
  return _displayEmptyTasksAsLine;
}


/*!
  Defines the horizontal background lines of the Gantt chart.
  Call setHorBackgroundLines()
  (equivalent to setHorBackgroundLines( 2, QBrush( QColor ( 240,240,240 )) ) )
  to draw a light grey horizontal background line for every second Gantt item.
  Call setHorBackgroundLines(0) in order to not show horizontal
  background lines.
  You may specify the number of lines and the brush of the lines.

  \param count for count >=  2, every count line gets a backgroud
               specified by brush
               for count <  2, no background lines are drawn
  \param brush the brush of the lines
*/
void KDGanttView::setHorBackgroundLines( int count, QBrush brush )
{
  myTimeTable->setHorBackgroundLines(  count, brush );
}


/*!
  Returns the definition of the horizontal background lines of the
  Gantt chart.

  \param brush the brush of the lines
  \return every nth line gets a background specified by brush
  if 0 is returned, no backgroud lines are drawn

*/
int KDGanttView::horBackgroundLines( QBrush& brush ) const
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
    QPtrList<KDGanttViewTaskLink> retVal = myTimeTable->taskLinks();
    return retVal;
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
   Reads the parameters of the view from an XML document.
   \param doc the XML document to read from
   \return true if the parameters could be read, false if a file
   format error occurred
   \sa saveXML
*/
bool KDGanttView::loadXML( const QDomDocument& doc )
{
    bool block = getUpdateEnabled();
    setUpdateEnabled( false );
    QDomElement docRoot = doc.documentElement(); // ChartParams element
    QDomNode node = docRoot.firstChild();
    while( !node.isNull() ) {
        QDomElement element = node.toElement();
        if( !element.isNull() ) { // was really an element
            QString tagName = element.tagName();
            if( tagName == "ShowLegend" ) {
                bool value;
                if( KDGanttXML::readBoolNode( element, value ) )
                    setShowLegend( value );
            } else if( tagName == "ShowLegendButton" ) {
                bool value;
                if( KDGanttXML::readBoolNode( element, value ) )
                    setShowLegendButton( value );
            } else if( tagName == "LegendIsDockWindow" ) {
                bool value;
                if( KDGanttXML::readBoolNode( element, value ) )
                    setLegendIsDockwindow( value );
            } else if( tagName == "ShowListView" ) {
                bool value;
                if( KDGanttXML::readBoolNode( element, value ) )
                    setShowListView( value );
            } else if( tagName == "ShowHeader" ) {
                bool value;
                if( KDGanttXML::readBoolNode( element, value ) )
                    setHeaderVisible( value );
            } else if( tagName == "ShowTaskLinks" ) {
                bool value;
                if( KDGanttXML::readBoolNode( element, value ) )
                    setShowTaskLinks( value );
            } else if( tagName == "EditorEnabled" ) {
                bool value;
                if( KDGanttXML::readBoolNode( element, value ) )
                    setEditorEnabled( value );
            } else if( tagName == "DisplayEmptyTasksAsLine" ) {
                bool value;
                if( KDGanttXML::readBoolNode( element, value ) )
                    setDisplayEmptyTasksAsLine( value );
            } else if( tagName == "GlobalFont" ) {
                QFont font;
                if( KDGanttXML::readFontNode( element, font ) )
                    setFont( font );
            } else if( tagName == "HorizonStart" ) {
                QDateTime value;
                if( KDGanttXML::readDateTimeNode( element, value ) )
                    setHorizonStart( value );
            } else if( tagName == "HorizonEnd" ) {
                QDateTime value;
                if( KDGanttXML::readDateTimeNode( element, value ) )
                    setHorizonEnd( value );
            } else if( tagName == "Scale" ) {
                QString value;
                if( KDGanttXML::readStringNode( element, value ) )
                    setScale( stringToScale( value ) );
            } else if( tagName == "MinimumScale" ) {
                QString value;
                if( KDGanttXML::readStringNode( element, value ) )
                    setMinimumScale( stringToScale( value ) );
            } else if( tagName == "MaximumScale" ) {
                QString value;
                if( KDGanttXML::readStringNode( element, value ) )
                    setMaximumScale( stringToScale( value ) );
            } else if( tagName == "YearFormat" ) {
                QString value;
                if( KDGanttXML::readStringNode( element, value ) )
                    setYearFormat( stringToYearFormat( value ) );
            } else if( tagName == "HourFormat" ) {
                QString value;
                if( KDGanttXML::readStringNode( element, value ) )
                    setHourFormat( stringToHourFormat( value ) );
            } else if( tagName == "ShowMinorTicks" ) {
                bool value;
                if( KDGanttXML::readBoolNode( element, value ) )
                    setShowMinorTicks( value );
            } else if( tagName == "ShowMajorTicks" ) {
                bool value;
                if( KDGanttXML::readBoolNode( element, value ) )
                    setShowMajorTicks( value );
            } else if( tagName == "DragEnabled" ) {
                bool value;
                if( KDGanttXML::readBoolNode( element, value ) )
                    setDragEnabled( value );
            } else if( tagName == "DropEnabled" ) {
                bool value;
                if( KDGanttXML::readBoolNode( element, value ) )
                    setDropEnabled( value );
            }  else if( tagName == "TickcountForTimeline" ) {
                int value;
                if( KDGanttXML::readIntNode( element, value ) )
                    setAddTickcountForTimeline( value );
            } else if( tagName == "DisplaySubitemsAsGroup" ) {
                bool value;
                if( KDGanttXML::readBoolNode( element, value ) )
                    _displaySubitemsAsGroup = value;
            } else if( tagName == "WeekScaleShowNumber" ) {
                bool value;
                if( KDGanttXML::readBoolNode( element, value ) )
                    setWeekScaleShowNumber( value );
            } else if( tagName == "WeekStartsMonday" ) {
                bool value;
                if( KDGanttXML::readBoolNode( element, value ) )
                    setWeekStartsMonday( value );
            } else if( tagName == "UserHorizonChangeEnabled" ) {
                bool value;
                if( KDGanttXML::readBoolNode( element, value ) )
                    setUserHorizonChangeEnabled( value );
            } else if( tagName == "TimeHeaderDatetimeFormatHour" ) {
                QString value;
                if( KDGanttXML::readStringNode( element, value ) )
                    setTimeHeaderDatetimeFormatHour( value );
            } else if( tagName == "TimeHeaderDatetimeFormatSecond" ) {
                QString value;
                if( KDGanttXML::readStringNode( element, value ) )
                    setTimeHeaderDatetimeFormatSecond( value );
            } else if( tagName == "TimeHeaderDatetimeFormatMinute" ) {
                QString value;
                if( KDGanttXML::readStringNode( element, value ) )
                    setTimeHeaderDatetimeFormatMinute( value );
            } else if( tagName == "TimeHeaderDateFormatDay" ) {
                QString value;
                if( KDGanttXML::readStringNode( element, value ) )
                    setTimeHeaderDateFormatDay( value );
            } else if( tagName == "TimeHeaderDateFormatWeek" ) {
                QString value;
                if( KDGanttXML::readStringNode( element, value ) )
                    setTimeHeaderDateFormatWeek( value );
            } else if( tagName == "TimeHeaderDateFormatMonth" ) {
                QString value;
                if( KDGanttXML::readStringNode( element, value ) )
                    setTimeHeaderDateFormatMonth( value );
            } else if( tagName == "TimeHeaderTooltipDateTimeFormat" ) {
                QString value;
                if( KDGanttXML::readStringNode( element, value ) )
                    setTimeHeaderTooltipDateTimeFormat( value );
            } else if( tagName == "CalendarMode" ) {
                bool value;
                if( KDGanttXML::readBoolNode( element, value ) )
                    setCalendarMode( value );
            } else if( tagName == "Editable" ) {
                bool value;
                if( KDGanttXML::readBoolNode( element, value ) )
                    setEditable( value );
            } else if( tagName == "TextColor" ) {
                QColor value;
                if( KDGanttXML::readColorNode( element, value ) )
                    setTextColor( value );
            } else if( tagName == "MajorScaleCount" ) {
                int value;
                if( KDGanttXML::readIntNode( element, value ) )
                    setMajorScaleCount( value );
            } else if( tagName == "MinorScaleCount" ) {
                int value;
                if( KDGanttXML::readIntNode( element, value ) )
                    setMinorScaleCount( value );
            } else if( tagName == "AutoScaleMinorTickCount" ) {
                int value;
                if( KDGanttXML::readIntNode( element, value ) )
                    setAutoScaleMinorTickCount( value );
            } else if( tagName == "MinimumColumnWidth" ) {
                int value;
                if( KDGanttXML::readIntNode( element, value ) )
                    setMinimumColumnWidth( value );
            } else if( tagName == "GanttMaximumWidth" ) {
                int value;
                if( KDGanttXML::readIntNode( element, value ) )
                    setGanttMaximumWidth( value );
            } else if( tagName == "Backgroundlines" ) {
                QBrush value;
                int linecount = 0;
                QDomNode node = element.firstChild();
                while( !node.isNull() ) {
                    QDomElement element = node.toElement();
                    if( !element.isNull() ) {
                        QString tagName = element.tagName();
                        if( tagName == "BackGroundBrush" ) 
                            KDGanttXML::readBrushNode( element, value );
                        else if( tagName == "LineCount" ) 
                            KDGanttXML::readIntNode( element, linecount );
                    }
                    node = node.nextSibling();
                }
                if ( linecount )
                    setHorBackgroundLines( linecount, value );
            } else if( tagName == "NoInformationBrush" ) {
                QBrush value;
                if( KDGanttXML::readBrushNode( element, value ) )
                    setNoInformationBrush( value );
            } else if( tagName == "GanttViewBackgroundColor" ) {
                QColor value;
                if( KDGanttXML::readColorNode( element, value ) )
                    setGvBackgroundColor( value );
            } else if( tagName == "ListViewBackgroundColor" ) {
                QColor value;
                if( KDGanttXML::readColorNode( element, value ) )
                    setLvBackgroundColor( value );
            } else if( tagName == "TimeHeaderBackgroundColor" ) {
                QColor value;
                if( KDGanttXML::readColorNode( element, value ) )
                    setTimeHeaderBackgroundColor( value );
            } else if( tagName == "LegendHeaderBackgroundColor" ) {
                QColor value;
                if( KDGanttXML::readColorNode( element, value ) )
                    setLegendHeaderBackgroundColor( value );
            } else if( tagName == "WeekendBackgroundColor" ) {
                QColor value;
                if( KDGanttXML::readColorNode( element, value ) )
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
                            if( KDGanttXML::readIntNode( element, value ) )
                                day = value;
                        } else if( tagName == "Color" ) {
                            QColor value;
                            if( KDGanttXML::readColorNode( element, value ) )
                                color = value;
                        } else {
                            qDebug( "Unrecognized tag name: %s", tagName.toLatin1() );
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
                if( KDGanttXML::readDoubleNode( element, value ) )
                    setZoomFactor( value, true );
            } else if( tagName == "ShowHeaderPopupMenu" ) {
                bool value;
                if( KDGanttXML::readBoolNode( element, value ) )
                    setShowHeaderPopupMenu( value );
            } else if( tagName == "ShowTimeTablePopupMenu" ) {
                bool value;
                if( KDGanttXML::readBoolNode( element, value ) )
                    setShowTimeTablePopupMenu( value );
            } else if( tagName == "Shapes" ) {
                QDomNode node = element.firstChild();
                bool undefinedShape = false;
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
                                        if( KDGanttXML::readStringNode( element, value ) )
                                            startShape = KDGanttViewItem::stringToShape( value );
                                        if ( value == "Undefined" )
                                            undefinedShape = true;
                                    } else if( tagName == "Middle" ) {
                                        QString value;
                                        if( KDGanttXML::readStringNode( element, value ) )
                                            middleShape = KDGanttViewItem::stringToShape( value );
                                        if ( value == "Undefined" )
                                            undefinedShape = true;
                                    } else if( tagName == "End" ) {
                                        QString value;
                                        if( KDGanttXML::readStringNode( element, value ) )
                                            endShape = KDGanttViewItem::stringToShape( value );
                                        if ( value == "Undefined" )
                                            undefinedShape = true;
                                    } else {
                                        qDebug( "Unrecognized tag name: %s", tagName.toLatin1() );
                                        Q_ASSERT( false );
                                    }
                                }
                                node = node.nextSibling();
                            }
                            if ( ! undefinedShape )
                                setShapes( KDGanttViewItem::Event, startShape,
                                           middleShape, endShape, false );
                            undefinedShape = false;
                        } else if( tagName == "Task" ) {
                            KDGanttViewItem::Shape startShape, middleShape, endShape;
                            startShape = KDGanttViewItem::TriangleDown;
                            middleShape = KDGanttViewItem::TriangleDown;
                            endShape = KDGanttViewItem::TriangleDown;
                            QDomNode node = element.firstChild();
                            while( !node.isNull()) {
                                QDomElement element = node.toElement();
                                if( !element.isNull() ) { // was really an elemente
                                    QString tagName = element.tagName();
                                    if( tagName == "Start" ) {
                                        QString value;
                                        if( KDGanttXML::readStringNode( element, value ) )
                                            startShape = KDGanttViewItem::stringToShape( value );
                                        if ( value == "Undefined" )
                                            undefinedShape = true;
                                    } else if( tagName == "Middle" ) {
                                        QString value;
                                        if( KDGanttXML::readStringNode( element, value ) )
                                            middleShape = KDGanttViewItem::stringToShape( value );
                                        if ( value == "Undefined" )
                                            undefinedShape = true;
                                    } else if( tagName == "End" ) {
                                        QString value;
                                        if( KDGanttXML::readStringNode( element, value ) )
                                            endShape = KDGanttViewItem::stringToShape( value );
                                        if ( value == "Undefined" )
                                            undefinedShape = true;
                                    } else {
                                        qDebug( "Unrecognized tag name: %s", tagName.toLatin1() );
                                        Q_ASSERT( false );
                                    }
                                }
                                node = node.nextSibling();
                            }
                            if ( ! undefinedShape )
                                setShapes( KDGanttViewItem::Task, startShape, middleShape, endShape, false );
                            undefinedShape = false;
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
                                        if( KDGanttXML::readStringNode( element, value ) )
                                            startShape = KDGanttViewItem::stringToShape( value );
                                        if ( value == "Undefined" )
                                            undefinedShape = true;
                                    } else if( tagName == "Middle" ) {
                                        QString value;
                                        if( KDGanttXML::readStringNode( element, value ) )
                                            middleShape = KDGanttViewItem::stringToShape( value );
                                        if ( value == "Undefined" )
                                            undefinedShape = true;
                                    } else if( tagName == "End" ) {
                                        QString value;
                                        if( KDGanttXML::readStringNode( element, value ) )
                                            endShape = KDGanttViewItem::stringToShape( value );
                                        if ( value == "Undefined" )
                                            undefinedShape = true;
                                    } else {
                                        qDebug( "Unrecognized tag name: %s", tagName.toLatin1() );
                                        Q_ASSERT( false );
                                    }
                                }
                                node = node.nextSibling();
                            }
                            if ( ! undefinedShape )
                                setShapes( KDGanttViewItem::Summary, startShape,
                                           middleShape, endShape, false );
                            undefinedShape = false;
                        } else {
                            qDebug( "Unrecognized tag name: %s", tagName.toLatin1() );
                            Q_ASSERT( false );
                        }
                    }
                    node = node.nextSibling();
                }
            } else if( tagName == "Colors" ) {
                QDomNode node = element.firstChild();
                while( !node.isNull()) {
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
                                        if( KDGanttXML::readColorNode( element, value ) )
                                            startColor = value;
                                    } else if( tagName == "Middle" ) {
                                        QColor value;
                                        if( KDGanttXML::readColorNode( element, value ) )
                                            middleColor = value;
                                    } else if( tagName == "End" ) {
                                        QColor value;
                                        if( KDGanttXML::readColorNode( element, value ) )
                                            endColor = value;
                                    } else {
                                        qDebug( "Unrecognized tag name: %s", tagName.toLatin1() );
                                        Q_ASSERT( false );
                                    }
                                }
                                node = node.nextSibling();
                            }
                            setColors( KDGanttViewItem::Event, startColor,
                                       middleColor, endColor, false );
                        } else if( tagName == "Task" ) {
                            QColor startColor, middleColor, endColor;
                            QDomNode node = element.firstChild();
                            while( !node.isNull() ) {
                                QDomElement element = node.toElement();
                                if( !element.isNull() ) { // was really an elemente
                                    QString tagName = element.tagName();
                                    if( tagName == "Start" ) {
                                        QColor value;
                                        if( KDGanttXML::readColorNode( element, value ) )
                                            startColor = value;
                                    } else if( tagName == "Middle" ) {
                                        QColor value;
                                        if( KDGanttXML::readColorNode( element, value ) )
                                            middleColor = value;
                                    } else if( tagName == "End" ) {
                                        QColor value;
                                        if( KDGanttXML::readColorNode( element, value ) )
                                            endColor = value;
                                    } else {
                                        qDebug( "Unrecognized tag name: %s", tagName.toLatin1() );
                                        Q_ASSERT( false );
                                    }
                                }
                                node = node.nextSibling();
                            }
                            setColors( KDGanttViewItem::Task, startColor,
                                       middleColor, endColor, false );
                        } else if( tagName == "Summary" ) {
                            QColor startColor, middleColor, endColor;
                            QDomNode node = element.firstChild();
                            while( !node.isNull() ) {
                                QDomElement element = node.toElement();
                                if( !element.isNull() ) { // was really an elemente
                                    QString tagName = element.tagName();
                                    if( tagName == "Start" ) {
                                        QColor value;
                                        if( KDGanttXML::readColorNode( element, value ) )
                                            startColor = value;
                                    } else if( tagName == "Middle" ) {
                                        QColor value;
                                        if( KDGanttXML::readColorNode( element, value ) )
                                            middleColor = value;
                                    } else if( tagName == "End" ) {
                                        QColor value;
                                        if( KDGanttXML::readColorNode( element, value ) )
                                            endColor = value;
                                    } else {
                                        qDebug( "Unrecognized tag name: %s", tagName.toLatin1() );
                                        Q_ASSERT( false );
                                    }
                                }
                                node = node.nextSibling();
                            }
                            setColors( KDGanttViewItem::Summary, startColor,
                                       middleColor, endColor , false);
                        } else {
                            qDebug( "Unrecognized tag name: %s", tagName.toLatin1() );
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
                            if( KDGanttXML::readColorNode( element, value ) )
                                setDefaultColor( KDGanttViewItem::Event,
                                                 value, false );
                        } else if( tagName == "Task" ) {
                            QColor value;
                            if( KDGanttXML::readColorNode( element, value ) )
                                setDefaultColor( KDGanttViewItem::Task,
                                                 value, false );
                        } else if( tagName == "Summary" ) {
                            QColor value;
                            if( KDGanttXML::readColorNode( element, value ) )
                                setDefaultColor( KDGanttViewItem::Summary,
                                                 value , false);
                        } else {
                            qDebug( "Unrecognized tag name: %s", tagName.toLatin1() );
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
                                        if( KDGanttXML::readColorNode( element, value ) )
                                            startColor = value;
                                    } else if( tagName == "Middle" ) {
                                        QColor value;
                                        if( KDGanttXML::readColorNode( element, value ) )
                                            middleColor = value;
                                    } else if( tagName == "End" ) {
                                        QColor value;
                                        if( KDGanttXML::readColorNode( element, value ) )
                                            endColor = value;
                                    } else {
                                        qDebug( "Unrecognized tag name: %s", tagName.toLatin1() );
                                        Q_ASSERT( false );
                                    }
                                }
                                node = node.nextSibling();
                            }
                            setHighlightColors( KDGanttViewItem::Event,
                                                startColor,
                                                middleColor, endColor, false );
                        } else if( tagName == "Task" ) {
                            QColor startColor, middleColor, endColor;
                            QDomNode node = element.firstChild();
                            while( !node.isNull() ) {
                                QDomElement element = node.toElement();
                                if( !element.isNull() ) { // was really an elemente
                                    QString tagName = element.tagName();
                                    if( tagName == "Start" ) {
                                        QColor value;
                                        if( KDGanttXML::readColorNode( element, value ) )
                                            startColor = value;
                                    } else if( tagName == "Middle" ) {
                                        QColor value;
                                        if( KDGanttXML::readColorNode( element, value ) )
                                            middleColor = value;
                                    } else if( tagName == "End" ) {
                                        QColor value;
                                        if( KDGanttXML::readColorNode( element, value ) )
                                            endColor = value;
                                    } else {
                                        qDebug( "Unrecognized tag name: %s", tagName.toLatin1() );
                                        Q_ASSERT( false );
                                    }
                                }
                                node = node.nextSibling();
                            }
                            setHighlightColors( KDGanttViewItem::Task,
                                                startColor,
                                                middleColor, endColor , false);
                        } else if( tagName == "Summary" ) {
                            QColor startColor, middleColor, endColor;
                            QDomNode node = element.firstChild();
                            while( !node.isNull() ) {
                                QDomElement element = node.toElement();
                                if( !element.isNull() ) { // was really an elemente
                                    QString tagName = element.tagName();
                                    if( tagName == "Start" ) {
                                        QColor value;
                                        if( KDGanttXML::readColorNode( element, value ) )
                                            startColor = value;
                                    } else if( tagName == "Middle" ) {
                                        QColor value;
                                        if( KDGanttXML::readColorNode( element, value ) )
                                            middleColor = value;
                                    } else if( tagName == "End" ) {
                                        QColor value;
                                        if( KDGanttXML::readColorNode( element, value ) )
                                            endColor = value;
                                    } else {
                                        qDebug( "Unrecognized tag name: %s", tagName.toLatin1() );
                                        Q_ASSERT( false );
                                    }
                                }
                                node = node.nextSibling();
                            }
                            setHighlightColors( KDGanttViewItem::Summary,
                                                startColor,
                                                middleColor, endColor, false );
                        } else {
                            qDebug( "Unrecognized tag name: %s", tagName.toLatin1() );
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
                            if( KDGanttXML::readColorNode( element, value ) )
                                setDefaultHighlightColor( KDGanttViewItem::Event,
                                                          value , false);
                        } else if( tagName == "Task" ) {
                            QColor value;
                            if( KDGanttXML::readColorNode( element, value ) )
                                setDefaultHighlightColor( KDGanttViewItem::Task,
                                                          value, false );
                        } else if( tagName == "Summary" ) {
                            QColor value;
                            if( KDGanttXML::readColorNode( element, value ) )
                                setDefaultHighlightColor( KDGanttViewItem::Summary,
                                                          value, false );
                        } else {
                            qDebug( "Unrecognized tag name: %s", tagName.toLatin1() );
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
                            if ( newItem )
                                previous = newItem;
                        } else {
                            qDebug( "Unrecognized tag name: %s", tagName.toLatin1() );
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
                            qDebug( "Unrecognized tag name: %s", tagName.toLatin1() );
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
                        qDebug( "Unrecognized tag name: %s", tagName.toLatin1() );
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
                            Scale mini = Minute;
                            Scale maxi = Month;
                            while( !node.isNull() ) {
                                QDomElement element = node.toElement();
                                if( !element.isNull() ) { // was
                                    // really an
                                    // element
                                    QString tagName = element.tagName();
                                    if( tagName == "DateTime" ) {
                                        QDateTime value;
                                        if( KDGanttXML::readDateTimeNode( element, value ) )
                                            dateTime = value;
                                    } else if( tagName == "Color" ) {
                                        QColor value;
                                        if( KDGanttXML::readColorNode( element, value ) )
                                            color = value;
                                    } else if( tagName == "MinScale" ) {
                                        QString value;
                                        if( KDGanttXML::readStringNode( element, value ) )
                                            mini = stringToScale( value );
                                    } else if( tagName == "MaxScale" ) {
                                        QString value;
                                        if( KDGanttXML::readStringNode( element, value ) )
                                            maxi = stringToScale( value );
                                    } else {
                                        qDebug( "Unrecognized tag name: %s", tagName.toLatin1() );
                                        Q_ASSERT( false );
                                    }
                                }

                                node = node.nextSibling();
                            }
                            setColumnBackgroundColor( dateTime, color, mini, maxi );
                        } else {
                            qDebug( "Unrecognized tag name: %s", tagName.toLatin1() );
                            Q_ASSERT( false );
                        }
                    }
                    node = node.nextSibling();
                }
            } else if( tagName == "IntervalBackgroundColors" ) {
                QDomNode node = element.firstChild();
                while( !node.isNull() ) {
                    QDomElement element = node.toElement();
                    if( !element.isNull() ) { // was really an element
                        QString tagName = element.tagName();
                        if( tagName == "IntervalBackgroundColor" ) {
                            QDomNode node = element.firstChild();
                            QDateTime dateTime;
                            QDateTime ente;
                            QColor color;
                            Scale mini = Minute;
                            Scale maxi = Month;
                            int prio = 0;
                            while( !node.isNull() ) {
                                QDomElement element = node.toElement();
                                if( !element.isNull() ) { // was
                                    // really an
                                    // element
                                    QString tagName = element.tagName();
                                    if( tagName == "DateTimeEnd" ) {
                                        QDateTime value;
                                        if( KDGanttXML::readDateTimeNode( element, value ) )
                                            ente = value;
                                    }  else if( tagName == "Priority" ) {
                                        int value;
                                        if( KDGanttXML::readIntNode( element, value ) )
                                            prio = value;
                                    } else if( tagName == "DateTimeStart" ) {
                                        QDateTime value;
                                        if( KDGanttXML::readDateTimeNode( element, value ) )
                                            dateTime = value;
                                    } else if( tagName == "Color" ) {
                                        QColor value;
                                        if( KDGanttXML::readColorNode( element, value ) )
                                            color = value;
                                    } else if( tagName == "MinScale" ) {
                                        QString value;
                                        if( KDGanttXML::readStringNode( element, value ) )
                                            mini = stringToScale( value );
                                    } else if( tagName == "MaxScale" ) {
                                        QString value;
                                        if( KDGanttXML::readStringNode( element, value ) )
                                            maxi = stringToScale( value );
                                    } else {
                                        qDebug( "Unrecognized tag name: %s", tagName.toLatin1() );
                                        Q_ASSERT( false );
                                    }
                                }
                                node = node.nextSibling();
                            }
                            setIntervalBackgroundColor( dateTime, ente, color, prio, mini, maxi );
                        } else {
                            qDebug( "Unrecognized tag name: %s", tagName.toLatin1() );
                            Q_ASSERT( false );
                        }
                    }
                    node = node.nextSibling();
                }
            } else if( tagName == "LegendItems" ) {
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
                            KDGanttViewItem::Shape tempLegendShape2;
                            tempLegendShape2 = KDGanttViewItem::TriangleDown;
                            QColor tempLegendColor2;
                            QString tempLegendString2;
                            bool has2 = false;
                            bool ok = true;
                            QDomNode node = element.firstChild();
                            while( !node.isNull() ) {
                                QDomElement element = node.toElement();
                                if( !element.isNull() ) { // was really an element
                                    QString tagName = element.tagName();
                                    if( tagName == "Shape" ) {
                                        QString value;
                                        if( KDGanttXML::readStringNode( element, value ) )
                                            tempLegendShape = KDGanttViewItem::stringToShape( value );
                                        else
                                            ok = false;
                                    } else if( tagName == "Color" ) {
                                        QColor value;
                                        if( KDGanttXML::readColorNode( element, value ) )
                                            tempLegendColor = value;
                                        else
                                            ok = false;
                                    } else if( tagName == "Text" ) {
                                        QString value;
                                        if( KDGanttXML::readStringNode( element, value ) )
                                            tempLegendString = value;
                                        else
                                            ok = false;
                                    } else if( tagName == "Shape2" ) {
                                        QString value;
                                        has2 = true;
                                        if( KDGanttXML::readStringNode( element, value ) )
                                            tempLegendShape2 = KDGanttViewItem::stringToShape( value );
                                        else
                                            ok = false;
                                    } else if( tagName == "Color2" ) {
                                        QColor value;
                                        has2 = true;
                                        if( KDGanttXML::readColorNode( element, value ) )
                                            tempLegendColor2 = value;
                                        else
                                            ok = false;
                                    } else if( tagName == "Text2" ) {
                                        QString value;
                                        has2 = true;
                                        if( KDGanttXML::readStringNode( element, value ) )
                                            tempLegendString2 = value;
                                        else
                                            ok = false;
                                    } else {
                                        qDebug( "Unrecognized tag name: %s", tagName.toLatin1() );
                                        Q_ASSERT( false );
                                    }
                                }
                                node = node.nextSibling();
                            }
                            if( ok ) {
                                if ( has2 )
                                    addLegendItem( tempLegendShape,
                                                   tempLegendColor,
                                                   tempLegendString,
                                                   tempLegendShape2,
                                                   tempLegendColor2,
                                                   tempLegendString2 );
                                else
                                    addLegendItem( tempLegendShape,
                                                   tempLegendColor,
                                                   tempLegendString );
                                //qDebug( "Adding legend item %s", tempLegendString.toLatin1() );
                            }
                        } else {
                            qDebug( "Unrecognized tag name: %s", tagName.toLatin1() );
                            Q_ASSERT( false );
                        }
                    }
                    node = node.nextSibling();
                }
            } else if( tagName == "UserSavedData" ) {
                userReadFromElement( element );
            } else {
                qDebug( "Unrecognized tag name: %s", tagName.toLatin1() );
                Q_ASSERT( false );
            }
        }

        node = node.nextSibling();
    } // while
    setUpdateEnabled( block );
	return true; /* FIXME: Do real error-reporting. The ASSERT's should be "return false" stmnts */
} // method


/**
   Saves the parameters of the view parameters to an XML document.

   \param withPI pass true to store processing instructions, false to
   leave them out
   \return the XML document that represents the parameters
   \sa loadXML
*/
QDomDocument KDGanttView::saveXML( bool withPI ) const
{
    // Create an inital DOM document
    QString docstart = "<GanttView/>";

    QDomDocument doc( "GanttView" );
    doc.setContent( docstart );
    if( withPI ) {
        QDomProcessingInstruction pin = doc.createProcessingInstruction( "kdgantt", "version=\"1.0\" encoding=\"UTF-8\""  ) ;
        doc.appendChild ( pin );
    }

    QDomElement docRoot = doc.documentElement();
    docRoot.setAttribute( "xmlns", "http://www.klaralvdalens-datakonsult.se/kdgantt" );
    docRoot.setAttribute( "xmlns:xsi", "http://www.w3.org/2000/10/XMLSchema-instance" );
    docRoot.setAttribute( "xsi:schemaLocation", "http://www.klaralvdalens-datakonsult.se/kdgantt" );

    // the ShowLegend element
    KDGanttXML::createBoolNode( doc, docRoot, "ShowLegend", showLegend() );

    // the ShowLegendButton element
    KDGanttXML::createBoolNode( doc, docRoot, "ShowLegendButton",
                                showLegendButton() );

    // the LegendIsDockWindow element
    KDGanttXML::createBoolNode( doc, docRoot, "LegendIsDockWindow",
                                legendIsDockwindow() );

    // the ShowListView element
    KDGanttXML::createBoolNode( doc, docRoot, "ShowListView", showListView() );

    // the ShowHeader element
    KDGanttXML::createBoolNode( doc, docRoot, "ShowHeader", headerVisible() );

    // the ShowTaskLinks element
    KDGanttXML::createBoolNode( doc, docRoot, "ShowTaskLinks", showTaskLinks() );

    // the EditorEnabled element
    KDGanttXML::createBoolNode( doc, docRoot, "EditorEnabled", editorEnabled() );

    // the global font element
    KDGanttXML::createFontNode( doc, docRoot, "GlobalFont", font() );

    // the DisplayEmptyTasksAsLine element
    KDGanttXML::createBoolNode( doc, docRoot, "DisplayEmptyTasksAsLine",
                                displayEmptyTasksAsLine() );

    // the HorizonStart element
    KDGanttXML::createDateTimeNode( doc, docRoot, "HorizonStart", horizonStart() );

    // the HorizonEnd element
    KDGanttXML::createDateTimeNode( doc, docRoot, "HorizonEnd", horizonEnd() );

    // the Scale, MinimumScale, MaximumScale elements
    KDGanttXML::createStringNode( doc, docRoot, "Scale", scaleToString( scale() ) );
    KDGanttXML::createStringNode( doc, docRoot, "MinimumScale",
                                  scaleToString( minimumScale() ) );
    KDGanttXML::createStringNode( doc, docRoot, "MaximumScale",
                                  scaleToString( maximumScale() ) );

    // the YearFormat element
    KDGanttXML::createStringNode( doc, docRoot, "YearFormat",
                                  yearFormatToString( yearFormat() ) );

    // the HourFormat element
    KDGanttXML::createStringNode( doc, docRoot, "HourFormat",
                                  hourFormatToString( hourFormat() ) );

    // the ShowMinorTicks element
    KDGanttXML::createBoolNode( doc, docRoot, "ShowMinorTicks", showMinorTicks() );

    // the ShowMajorTicks element
    KDGanttXML::createBoolNode( doc, docRoot, "ShowMajorTicks", showMajorTicks() );

    // the Editable element
    KDGanttXML::createBoolNode( doc, docRoot, "Editable", editable() );

    // the TextColor element
    KDGanttXML::createColorNode( doc, docRoot, "TextColor", textColor() );

    // the MajorScaleCount element
    KDGanttXML::createIntNode( doc, docRoot, "MajorScaleCount", majorScaleCount() );

    // the MinorScaleCount element
    KDGanttXML::createIntNode( doc, docRoot, "MinorScaleCount", minorScaleCount() );

    // the AutoScaleMinorTickCount element
    KDGanttXML::createIntNode( doc, docRoot, "AutoScaleMinorTickCount",
                               autoScaleMinorTickCount() );

    // the MinimumColumnWidth element
    KDGanttXML::createIntNode( doc, docRoot, "MinimumColumnWidth",
                               minimumColumnWidth() );

    // the GanttMaximumWidth element
    KDGanttXML::createIntNode( doc, docRoot, "GanttMaximumWidth",
                               ganttMaximumWidth() );

    QBrush backBrush;
    int backgroundlines = horBackgroundLines( backBrush );
    if ( backgroundlines ) {
        QDomElement backElement = doc.createElement( "Backgroundlines" );
        docRoot.appendChild( backElement );
        KDGanttXML::createBrushNode( doc, backElement, "BackGroundBrush",backBrush );
        KDGanttXML::createIntNode( doc, backElement, "LineCount",  backgroundlines );
    }
    // the NoInformationBrush element
    KDGanttXML::createBrushNode( doc, docRoot, "NoInformationBrush",
                                 noInformationBrush() );

    // the GanttViewBackgroundColor element
    KDGanttXML::createColorNode( doc, docRoot, "GanttViewBackgroundColor",
                                 gvBackgroundColor() );

    // the ListViewBackgroundColor element
    KDGanttXML::createColorNode( doc, docRoot, "ListViewBackgroundColor",
                                 lvBackgroundColor() );

    // the TimeHeaderBackgroundColor element
    KDGanttXML::createColorNode( doc, docRoot, "TimeHeaderBackgroundColor",
                                 timeHeaderBackgroundColor() );

    // the LegendHeaderBackgroundColor element
    KDGanttXML::createColorNode( doc, docRoot, "LegendHeaderBackgroundColor",
                                 legendHeaderBackgroundColor() );

    // the WeekendBackgroundColor element
    KDGanttXML::createColorNode( doc, docRoot, "WeekendBackgroundColor",
                                 weekendBackgroundColor() );

    // the WeekdayBackgroundColor elements
    for( int weekday = 1; weekday <= 7; weekday++ ) {
        QColor color = weekdayBackgroundColor( weekday );
        if( color.isValid() ) {
            QDomElement weekendBackgroundColorElement = doc.createElement( "WeekdayBackgroundColor" );
            docRoot.appendChild( weekendBackgroundColorElement );
            KDGanttXML::createIntNode( doc, weekendBackgroundColorElement,
                                       "Day", weekday );
            KDGanttXML::createColorNode( doc, weekendBackgroundColorElement,
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
    KDGanttXML::createDoubleNode( doc, docRoot, "ZoomFactor",
                                  zoomFactor() );

    // the ShowHeaderPopupMenu element
    KDGanttXML::createBoolNode( doc, docRoot, "ShowHeaderPopupMenu",
                                showHeaderPopupMenu() );

    // the ShowTimeTablePopupMenu element
    KDGanttXML::createBoolNode( doc, docRoot, "ShowTimeTablePopupMenu",
                                showTimeTablePopupMenu() );

    // the Shapes element
    QDomElement shapesElement = doc.createElement( "Shapes" );
    docRoot.appendChild( shapesElement );
    QDomElement shapesEventElement = doc.createElement( "Event" );
    shapesElement.appendChild( shapesEventElement );
    KDGanttViewItem::Shape start, middle, end;
    if( shapes( KDGanttViewItem::Event, start, middle, end ) ) {
        KDGanttXML::createStringNode( doc, shapesEventElement, "Start",
                                      KDGanttViewItem::shapeToString( start ) );
        KDGanttXML::createStringNode( doc, shapesEventElement, "Middle",
                                      KDGanttViewItem::shapeToString( middle ) );
        KDGanttXML::createStringNode( doc, shapesEventElement, "End",
                                      KDGanttViewItem::shapeToString( end ) );
    } else {
        KDGanttXML::createStringNode( doc, shapesEventElement, "Start",
                                      "Undefined" );
        KDGanttXML::createStringNode( doc, shapesEventElement, "Middle",
                                      "Undefined" );
        KDGanttXML::createStringNode( doc, shapesEventElement, "End",
                                      "Undefined" );
    }
    QDomElement shapesTaskElement = doc.createElement( "Task" );
    shapesElement.appendChild( shapesTaskElement );
    if( shapes( KDGanttViewItem::Task, start, middle, end ) ) {
        KDGanttXML::createStringNode( doc, shapesTaskElement, "Start",
                                      KDGanttViewItem::shapeToString( start ) );
        KDGanttXML::createStringNode( doc, shapesTaskElement, "Middle",
                                      KDGanttViewItem::shapeToString( middle ) );
        KDGanttXML::createStringNode( doc, shapesTaskElement, "End",
                                      KDGanttViewItem::shapeToString( end ) );
    } else {
        KDGanttXML::createStringNode( doc, shapesTaskElement, "Start",
                                      "Undefined" );
        KDGanttXML::createStringNode( doc, shapesTaskElement, "Middle",
                                      "Undefined" );
        KDGanttXML::createStringNode( doc, shapesTaskElement, "End",
                                      "Undefined" );
    }
    QDomElement shapesSummaryElement = doc.createElement( "Summary" );
    shapesElement.appendChild( shapesSummaryElement );
    if( shapes( KDGanttViewItem::Event, start, middle, end ) ) {
        KDGanttXML::createStringNode( doc, shapesSummaryElement, "Start",
                                      KDGanttViewItem::shapeToString( start ) );
        KDGanttXML::createStringNode( doc, shapesSummaryElement, "Middle",
                                      KDGanttViewItem::shapeToString( middle ) );
        KDGanttXML::createStringNode( doc, shapesSummaryElement, "End",
                                      KDGanttViewItem::shapeToString( end ) );
    } else {
        KDGanttXML::createStringNode( doc, shapesSummaryElement, "Start",
                                      "Undefined" );
        KDGanttXML::createStringNode( doc, shapesSummaryElement, "Middle",
                                      "Undefined" );
        KDGanttXML::createStringNode( doc, shapesSummaryElement, "End",
                                      "Undefined" );
    }

    // the Colors element
    QDomElement colorsElement = doc.createElement( "Colors" );
    docRoot.appendChild( colorsElement );
    QDomElement colorsEventElement = doc.createElement( "Event" );
    colorsElement.appendChild( colorsEventElement );
    QColor startColor, middleColor, endColor;
    colors( KDGanttViewItem::Event, startColor, middleColor, endColor );
    KDGanttXML::createColorNode( doc, colorsEventElement, "Start", startColor );
    KDGanttXML::createColorNode( doc, colorsEventElement, "Middle", middleColor );
    KDGanttXML::createColorNode( doc, colorsEventElement, "End", endColor );
    QDomElement colorsTaskElement = doc.createElement( "Task" );
    colorsElement.appendChild( colorsTaskElement );
    colors( KDGanttViewItem::Task, startColor, middleColor, endColor );
    KDGanttXML::createColorNode( doc, colorsTaskElement, "Start", startColor );
    KDGanttXML::createColorNode( doc, colorsTaskElement, "Middle", middleColor );
    KDGanttXML::createColorNode( doc, colorsTaskElement, "End", endColor );
    QDomElement colorsSummaryElement = doc.createElement( "Summary" );
    colorsElement.appendChild( colorsSummaryElement );
    colors( KDGanttViewItem::Event, startColor, middleColor, endColor );
    KDGanttXML::createColorNode( doc, colorsSummaryElement, "Start", startColor );
    KDGanttXML::createColorNode( doc, colorsSummaryElement, "Middle", middleColor );
    KDGanttXML::createColorNode( doc, colorsSummaryElement, "End", endColor );

    // the DefaultColor element
    QDomElement defaultColorsElement = doc.createElement( "DefaultColors" );
    docRoot.appendChild( defaultColorsElement );
    KDGanttXML::createColorNode( doc, defaultColorsElement, "Event",
                                 defaultColor( KDGanttViewItem::Event ) );
    KDGanttXML::createColorNode( doc, defaultColorsElement, "Task",
                                 defaultColor( KDGanttViewItem::Task ) );
    KDGanttXML::createColorNode( doc, defaultColorsElement, "Summary",
                                 defaultColor( KDGanttViewItem::Summary ) );


    // the HighlightColors element
    QDomElement highlightColorsElement = doc.createElement( "HighlightColors" );
    docRoot.appendChild( highlightColorsElement );
    QDomElement highlightColorsEventElement = doc.createElement( "Event" );
    highlightColorsElement.appendChild( highlightColorsEventElement );
    highlightColors( KDGanttViewItem::Event, startColor, middleColor, endColor );
    KDGanttXML::createColorNode( doc, highlightColorsEventElement, "Start", startColor );
    KDGanttXML::createColorNode( doc, highlightColorsEventElement, "Middle", middleColor );
    KDGanttXML::createColorNode( doc, highlightColorsEventElement, "End", endColor );
    QDomElement highlightColorsTaskElement = doc.createElement( "Task" );
    highlightColorsElement.appendChild( highlightColorsTaskElement );
    highlightColors( KDGanttViewItem::Task, startColor, middleColor, endColor );
    KDGanttXML::createColorNode( doc, highlightColorsTaskElement, "Start", startColor );
    KDGanttXML::createColorNode( doc, highlightColorsTaskElement, "Middle", middleColor );
    KDGanttXML::createColorNode( doc, highlightColorsTaskElement, "End", endColor );
    QDomElement highlightColorsSummaryElement = doc.createElement( "Summary" );
    highlightColorsElement.appendChild( highlightColorsSummaryElement );
    highlightColors( KDGanttViewItem::Event, startColor, middleColor, endColor );
    KDGanttXML::createColorNode( doc, highlightColorsSummaryElement, "Start", startColor );
    KDGanttXML::createColorNode( doc, highlightColorsSummaryElement, "Middle", middleColor );
    KDGanttXML::createColorNode( doc, highlightColorsSummaryElement, "End", endColor );

    // the DefaultHighlightColor element
    QDomElement defaultHighlightColorsElement = doc.createElement( "DefaultHighlightColors" );
    docRoot.appendChild( defaultHighlightColorsElement );
    KDGanttXML::createColorNode( doc, defaultHighlightColorsElement, "Event",
                                 defaultHighlightColor( KDGanttViewItem::Event ) );
    KDGanttXML::createColorNode( doc, defaultHighlightColorsElement, "Task",
                                 defaultHighlightColor( KDGanttViewItem::Task ) );
    KDGanttXML::createColorNode( doc, defaultHighlightColorsElement, "Summary",
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
        KDGanttXML::createDateTimeNode( doc, columnBackgroundColorElement,
                                        "DateTime", (*it).datetime );
        KDGanttXML::createColorNode( doc, columnBackgroundColorElement,
                                     "Color", (*it).color );
        KDGanttXML::createStringNode( doc, columnBackgroundColorElement, "MinScale", scaleToString( (*it).minScaleView ) );
        KDGanttXML::createStringNode( doc, columnBackgroundColorElement, "MaxScale", scaleToString( (*it).maxScaleView ) );
    }

    // the IntervalBackgroundColors element
    columnBackgroundColorsElement =
        doc.createElement( "IntervalBackgroundColors" );
    docRoot.appendChild( columnBackgroundColorsElement );
    ccList = myTimeHeader->intervalBackgroundColorList();
    for( KDTimeHeaderWidget::ColumnColorList::iterator it = ccList.begin();
         it != ccList.end(); ++it ) {
        QDomElement columnBackgroundColorElement =
            doc.createElement( "IntervalBackgroundColor" );
        columnBackgroundColorsElement.appendChild( columnBackgroundColorElement );
        KDGanttXML::createDateTimeNode( doc, columnBackgroundColorElement,
                                        "DateTimeStart", (*it).datetime );
        KDGanttXML::createDateTimeNode( doc, columnBackgroundColorElement,
                                        "DateTimeEnd", (*it).end );
        KDGanttXML::createColorNode( doc, columnBackgroundColorElement,
                                     "Color", (*it).color );
        KDGanttXML::createIntNode( doc, columnBackgroundColorElement, "Priority", (*it).priority );
        KDGanttXML::createStringNode( doc, columnBackgroundColorElement, "MinScale", scaleToString( (*it).minScaleView ) );
        KDGanttXML::createStringNode( doc, columnBackgroundColorElement, "MaxScale", scaleToString( (*it).maxScaleView ) );
    }

    // the LegendItems element
    QDomElement legendItemsElement =
        doc.createElement( "LegendItems" );
    docRoot.appendChild( legendItemsElement );
    legendItem* current;
    QPtrListIterator<legendItem> lit( myLegendItems );
    while( ( current = lit.current() ) ) {
        ++lit;
        QDomElement legendItemElement = doc.createElement( "LegendItem" );
        legendItemsElement.appendChild( legendItemElement );
        KDGanttXML::createStringNode( doc, legendItemElement, "Shape",
                                      KDGanttViewItem::shapeToString( current->shape ) );
        KDGanttXML::createColorNode( doc, legendItemElement, "Color",
                                     current->color );
        KDGanttXML::createStringNode( doc, legendItemElement, "Text",
                                      current->text );
        if ( current->has2 ) {
            KDGanttXML::createStringNode( doc, legendItemElement, "Shape2",
                                          KDGanttViewItem::shapeToString( current->shape2 ) );
            KDGanttXML::createColorNode( doc, legendItemElement, "Color2",
                                         current->color2 );
            KDGanttXML::createStringNode( doc, legendItemElement, "Text2",
                                          current->text2 );
        }
    }

    // the DragEnabled element
    KDGanttXML::createBoolNode( doc, docRoot, "DragEnabled", isDragEnabled() );

    // the DropEnabled element
    KDGanttXML::createBoolNode( doc, docRoot, "DropEnabled", isDropEnabled() );

    // the CalendarMode element
    KDGanttXML::createBoolNode( doc, docRoot, "CalendarMode", calendarMode() );
    KDGanttXML::createIntNode( doc, docRoot, "TickcountForTimeline", addTickcountForTimeline()  );
 
    KDGanttXML::createBoolNode( doc, docRoot, "DisplaySubitemsAsGroup", displaySubitemsAsGroup()  );
    KDGanttXML::createBoolNode( doc, docRoot, "WeekScaleShowNumber",weekScaleShowNumber()  );
    KDGanttXML::createBoolNode( doc, docRoot, "WeekStartsMonday", weekStartsMonday() );
    KDGanttXML::createBoolNode( doc, docRoot, "UserHorizonChangeEnabled", userHorizonChangeEnabled() );

    KDGanttXML::createStringNode( doc, docRoot, "TimeHeaderDatetimeFormatHour",timeHeaderDatetimeFormatHour()  );
    KDGanttXML::createStringNode( doc, docRoot, "TimeHeaderDatetimeFormatMinute", timeHeaderDatetimeFormatMinute() );
    KDGanttXML::createStringNode( doc, docRoot, "TimeHeaderDatetimeFormatSecond", timeHeaderDatetimeFormatSecond() );
    KDGanttXML::createStringNode( doc, docRoot, "TimeHeaderDateFormatDay", timeHeaderDateFormatDay() );
    KDGanttXML::createStringNode( doc, docRoot, "TimeHeaderDateFormatWeek", timeHeaderDateFormatWeek() );
    KDGanttXML::createStringNode( doc, docRoot, "TimeHeaderDateFormatMonth", timeHeaderDateFormatMonth() );
    KDGanttXML::createStringNode( doc, docRoot, "TimeHeaderTooltipDateTimeFormat", timeHeaderTooltipDateTimeFormat() );
    QDomElement userElement = doc.createElement( "UserSavedData" );
    docRoot.appendChild( userElement );
    userWriteToElement( doc, userElement );

    return doc;
}
/*!
  This virtual method does nothing. 
  Reimplement it to save your own data to the QDomElement.
  The data can be read with userReadFromElement().
  The body contains a small example to write an int value and a QString, 
  which is commented out.
  This method is automatically called from  KDGanttView::saveXML( bool withPI )
  which is called from KDGanttView::saveProject().

  \param doc the DOM document to which the node belongs
  \param parentElement the element into which to insert user defined data
  \sa userReadFromElement() 
*/
void KDGanttView::userWriteToElement( QDomDocument& doc,
                                      QDomElement& userElement ) const
{
    Q_UNUSED( doc );
    Q_UNUSED( userElement );
    // example for writing user defined data
    /*
    int userNumber = 815;
    QString userData = "this is saved text from the user";
    KDGanttXML::createStringNode( doc, userElement, "ExampleText", userData );
    KDGanttXML::createIntNode( doc, userElement, "ExampleNumber", userNumber );
    */

}
/*!
  This virtual method does nothing. 
  Reimplement it to read your own data from the QDomElement.
  The data was written from userWriteToElement().
  The body contains a small example to read  an int value and a QString, 
  which is commented out.
  This method is automatically called from 
  loadXML() which is called from KDGanttView::loadProject().

  \param doc the DOM document to which the node belongs
  \param parentElement the element which contains user defined data
  \sa userWriteToElement() createFromDomElement() loadFromDomElement()
*/
void KDGanttView::userReadFromElement( QDomElement& element )
{
    Q_UNUSED( element );
    // example for reading user defined data
    /*
    int userNumber = 0;
    QString userData = "";
    QDomNode node = element.firstChild();
    while( !node.isNull() ) {
        QDomElement userElement = node.toElement();
        QString tagName = userElement.tagName();
        if ( tagName == "ExampleNumber" ) {
                int value;
                if( KDGanttXML::readIntNode( userElement, value ) )
                    userNumber = value;
        } else if ( tagName == "ExampleText" ) {
                QString value;
                if( KDGanttXML::readStringNode( userElement, value ) )
                    userData = value;

        } 
        node = node.nextSibling();
    }
    qDebug("User data read: %d  %s ", userNumber,userData.toLatin1() );
    */
}

QString KDGanttView::scaleToString( Scale scale )
{
    switch( scale ) {
    case Second:
        return "Second";
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
    
    if( string == "Second" )
        return Second;
    else if( string == "Minute" )
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
    case NoDate:
        return "NoDate";
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
    else if( string == "NoDate" )
        return NoDate;
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
    case Hour_24_FourDigit:
        return "Hour_24_FourDigit";
    }
    return "";
}


KDGanttView::HourFormat KDGanttView::stringToHourFormat( const QString& string )
{
    if( string == "Hour_12" )
        return Hour_12;
    else if( string == "Hour_24" )
        return Hour_24;
    
    return Hour_24_FourDigit;
}


void KDGanttView::addTaskLinkGroup(KDGanttViewTaskLinkGroup* group)
{
    if ( group == 0 ) return;
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


/*!
  This slot is called when a new item has been added to the Gantt
  view. It will show the item attribute dialog in case the item is
  editable. \a item is a pointer to the item that has been created.
*/
void KDGanttView::editItem( KDGanttViewItem*  item)
{
  if ( ! item )
    return;
  if ( editorEnabled() ) {
    if ( item->editable() ) {
#if QT_VERSION < 0x040000
        if ( !myItemAttributeDialog ) {
            myItemAttributeDialog = new itemAttributeDialog();
            myItemAttributeDialog->resize( myItemAttributeDialog->minimumSizeHint() );
        }
        myItemAttributeDialog->reset( item );
        myItemAttributeDialog->show();
#endif
    }
  }
}


/*!
  This method returns the pixmap used for a certain shape, in the
  selected color and size.

  \param shape the shape to generate
  \param shapeColor the foreground color of the shape
  \param backgroundColor the background color of the shape
  \param itemSize the size of the shape
  \return the generated shape pixmap
*/
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
    default:
        index = -1;
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
  Calls to this method are passed through to the underlying \a QListView.
*/
int KDGanttView::addColumn( const QString& label, int width )
{
    return myListView->addColumn( label, width );
}


/*!
  Calls to this method are passed through to the underlying \a QListView.
*/

int KDGanttView::addColumn( const QIconSet& iconset, const QString& label,
                            int width )
{
    return myListView->addColumn( iconset, label, width );
}


/*!
  Calls to this method are passed through to the underlying \a QListView.
*/
void KDGanttView::removeColumn( int index )
{
    myListView->removeColumn( index );
}


/*!
  Calls to this method are passed through to the underlying \a QListView.
*/
KDGanttViewItem* KDGanttView::selectedItem() const
{
    return static_cast<KDGanttViewItem*>( myListView->selectedItem() );
}


/*!
  Calls to this method are passed through to the underlying \a QListView.
*/
void KDGanttView::setSelected( KDGanttViewItem* item, bool selected )
{
    myListView->setSelected( item, selected );
}


/*!
  Returns the pointer to the Gantt item with the name \a name.
  (i.e., listViewText() == name).
  If no item is found, the return value is 0.
  If there is more than one item with the same name in the Gantt view,
  the first item found will be returned. This may not necessarily be
  the first item in the listview.

  \param name the name of the Gantt item
  \return the pointer to the item with name \a name. O, if there is no item
  in the Gantt view with this name.

*/
KDGanttViewItem* KDGanttView::getItemByName( const QString& name ) const
{
    KDGanttViewItem* temp =  firstChild(),* ret;
    while (temp != 0) {
      if ( (ret = temp->getChildByName( name ) ) )
	return ret;
      temp = temp->nextSibling();
    }
    return 0;
}
/*!
  Returns the pointer to the Gantt item with the uid \a uid.
  If no item is found, the return value is 0.
  The uid of an item is not set automatically,
  it has to be set by the programmer.
  If there is more than one item with the same uid in the Gantt view,
  the first item found will be returned. This may not necessarily be
  the first item in the listview.

  \param uid the uid of the requested KDGanttViewItem item
  \param a pointer to a KDGanttViewItem which children are searched for the
         item with uid uid.
         pass 0 to  search in all KDGanttViewItems in the Gantt view.
  \return the pointer to the item with uid \a uid. O, if there is no item
  in the Gantt view with this name.
  \sa KDGanttViewItem::getChildByUid() KDGanttViewItem::uid() KDGanttViewItem::setUid() 

*/
KDGanttViewItem* KDGanttView::getItemByUid( const QString& uid,  KDGanttViewItem* parentItem ) const
{
    if ( parentItem != 0 )
        return parentItem->getChildByUid( uid );

    KDGanttViewItem* temp =  firstChild(),* ret;
    while (temp != 0) {
      if ( (ret = temp->getChildByUid( uid ) ) )
	return ret;
      temp = temp->nextSibling();
    }
    return 0;
}


/*!
  Returns the pointer to the Gantt item at the position \a pos in the
  list view.
  The position \a pos is a global position.
  If no item is found, 0 is returned.

  \param pos the (global) position of the Gantt item
  \return the pointer to the item with position \a pos. O, if there is
  no item in the list view at this position.

*/
KDGanttViewItem* KDGanttView::getItemByListViewPos( const QPoint& pos ) const
{
    return static_cast<KDGanttViewItem*>( myListView->itemAt(myListView->mapFromGlobal(pos) ));
}


/*!
  Returns the pointer to the Gantt item at the position \a pos in the
  Gantt view.
  The position \a pos is a global position.
  If no items are found, or the item is disabled, 0 is returned.
  If there is more than one item with the same position in the Gantt view,
  the first item found will be returned. This is not necessarily the
  first item in the listview.

  \param pos the (global) position of the Gantt item
  \return the pointer to the item with position \a pos. O, if there is no item
  in the Gantt view at this position.

*/
KDGanttViewItem* KDGanttView::getItemByGanttViewPos( const QPoint& pos ) const
{
  KDGanttViewItem* item;
  QPoint local = myCanvasView->mapFromGlobal(pos);

    QCanvasItemList il = myTimeTable->collisions( myCanvasView->viewportToContents( local ));
    QCanvasItemList::Iterator it;
    for ( it = il.begin(); it != il.end(); ++it ) {
      if ( myCanvasView->getType(*it) == Type_is_KDGanttViewItem) {
	item = myCanvasView->getItem(*it);
	if ( item->enabled() )
	  return item;
      }
    }
    return 0;
}


/*!
  Returns the pointer to the Gantt item at the position \a pos in the
  list view part of the Gantt view.
  The position \a pos is a global position if parameter \a global is true.
  If the vertical part (y coordinate) of \a pos
  (mapped to local coordinates) is less than 0 or
  larger than the height of the listview, 0 is returned.
  The horizontal part (x coordinate) of \a pos is ignored.
  \param pos the position of the Gantt item
  \param global if true, pos is assumed to be global
  \return the pointer to the item with position \a pos. O, if there is no item
  in the Gantt view at this position.

*/
KDGanttViewItem* KDGanttView::getItemAt( const QPoint& pos, bool global ) const
{
  KDGanttViewItem* item;
  KDGanttViewItem* retItem = 0;
  int y;
  if ( global )
      y = myCanvasView->viewport()->mapFromGlobal(pos).y()+myCanvasView->contentsY ();
  else
    y = pos.y();
  item = firstChild();
  while ( item != 0 ) {
    int yc = item->itemPos();
    if ( yc <= y && y < yc + item->height()) {
      retItem = item;
      break;
    }
    item = item->itemBelow();
  }
  return retItem;

}
/*!
  This sets to count of minor ticks that are added to the timeline if the user
  expand the horizon by pressing the arrow buttons of the horizontal scrollbar of the gantt view.
  \param count the tick count
  in the Gantt view at this position.
  \sa addTickcountForTimeline()

*/
void  KDGanttView::setAddTickcountForTimeline( int count )
{
    mAddTickcountForTimeline = count;
}
/*!
  Returns the count for adding ticks to the timeline if the user clicks on the arrow 
  buttons of the horizontal scrollbar of the gantt view.

  \return the tick count
  \sa setAddTickcountForTimeline()
*/
int  KDGanttView::addTickcountForTimeline() const
{
    return mAddTickcountForTimeline;
}

void KDGanttView::addTickRight()
{
  if ( _enableAdding && myCanvasView->horizontalScrollBar()->value() ==  myCanvasView->horizontalScrollBar()->maxValue()) {
    //myCanvasView->horizontalScrollBar()->blockSignals( true );
    myTimeHeader->addTickRight();
    //myCanvasView->horizontalScrollBar()->blockSignals( false );
    myCanvasView->updateHorScrollBar();
    setTimelineToEnd();
  }
}

void KDGanttView::addTickLeft()
{
    if (!mUserHorizonChangeEnabled) {
        emit addOneTickLeft();
        return;
    }
    if (  myCanvasView->horizontalScrollBar()->value() == 0 ) {
        myTimeHeader->addTickLeft( mAddTickcountForTimeline );
    }
}


void KDGanttView::enableAdding( int val )
{
  _enableAdding = ( val == 0 || val == myCanvasView->horizontalScrollBar()->maxValue());
}


/*!
  Returns the number of ( toplevel ) root items in the Gantt view.

  \return the number of ( toplevel ) root items in the Gantt view.
*/
int KDGanttView::childCount() const
{
    return myListView->childCount();
}

/*!
  Clears the complete content of the Gant view.
  That is
  all gantt items, 
  the legend and the legend items, 
  the tasklinks,
  the tasklink groups and
  the background color settings for columns/time interval in the gantt view.
  
*/
void KDGanttView::clearAll()
{
  bool block = myTimeTable->blockUpdating();
  myTimeTable->setBlockUpdating( true );
  clearLegend();
  clearBackgroundColor();
  setHorBackgroundLines( 0 );
  QPtrList<KDGanttViewTaskLink>  tll = taskLinks();
  tll.setAutoDelete( true );
  tll.clear();
  QPtrList<KDGanttViewTaskLinkGroup> tlg = myTaskLinkGroupList;
  tlg.setAutoDelete( true );
  tlg.clear();
  clear();
  myTimeTable->setBlockUpdating( block );
}
/*!
  Removes all items from the Gantt view.
*/
void KDGanttView::clear()
{
    if ( ! childCount() ) return;
  bool block = myTimeTable->blockUpdating();
  myTimeTable->setBlockUpdating( true );
  myListView->clear();
  // in Qt3 wee need the processEvents(); to fix a crash
  // in Qt4 with Qt3 compat it crashes here
  // I had a look at the Q3ListView source code:
  // in theory it may not crash ... in theory 
#if QT_VERSION < 0x040000
  qApp->processEvents();
#endif
  myTimeTable->setBlockUpdating( false );
  myTimeTable->updateMyContent();
#if QT_VERSION < 0x040000
  qApp->processEvents();
#endif
  myTimeTable->setBlockUpdating( block );
}


/*!
  Passes on the signal from the list view.
*/
void KDGanttView::slot_lvDropped(QDropEvent* e, KDGanttViewItem* droppedItem, KDGanttViewItem* itemBelowMouse  )
{
  emit dropped( e, droppedItem, itemBelowMouse);
}

/*!
  Implements a pass-through to the list view.
*/
QDragObject * KDGanttView::dragObject ()
{
  return myListView->dragObject ();
}


/*!
  Implements a pass-through to the list view.
*/
void KDGanttView::startDrag ()
{
  //myListView->pt_startDrag ();
}


/*!
  This method is overridden for internal purposes.
*/
void KDGanttView::setPaletteBackgroundColor( const QColor& col)
{
  QWidget::setPaletteBackgroundColor( col );
  timeHeaderSpacerWidget->setPaletteBackgroundColor( col );
}


/*!
  Sets the background color of the Gantt view.

  \param c the background color of the Gantt view.
  \sa gvBackgroundColor()
*/
void KDGanttView::setGvBackgroundColor ( const QColor & c )
{
  myTimeTable->setBackgroundColor( c );
}


/*!
  Sets the background color of the time header.

  \param c the background color of the time header.
  \sa timeHeaderBackgroundColor()
*/
void KDGanttView::setTimeHeaderBackgroundColor ( const QColor & c )
{
  myTimeHeader->setPaletteBackgroundColor( c );
  //rightWidget->setPaletteBackgroundColor( c );
  timeHeaderSpacerWidget->setPaletteBackgroundColor( c );
}


/*!
  Sets the background color of the legend header.

  \param c the background color of the legend header
  \sa legendHeaderBackgroundColor()
*/
void KDGanttView::setLegendHeaderBackgroundColor ( const QColor & c )
{
  myLegend->setPaletteBackgroundColor( c );
  leftWidget->setPaletteBackgroundColor( c );
}


/*!
  Sets the background color of the list view.

  \param c the background color of the list view
  \sa lvBackgroundColor()
*/
void KDGanttView::setLvBackgroundColor ( const QColor & c )
{
 myListView->viewport()->setPaletteBackgroundColor( c );
}


/*!
  Returns the background color of the list view.

  \return the background color of the list view
  \sa setLvBackgroundColor()
*/
QColor KDGanttView::lvBackgroundColor ( )const
{
 return myListView->viewport()->paletteBackgroundColor( );
}


/*!
  Returns the background color of the Gantt view.

  \return the background color of the Gantt view
  \sa setGvBackgroundColor()
*/
QColor KDGanttView::gvBackgroundColor () const
{
 return myTimeTable->backgroundColor( );
}


/*!
  Returns the background color of the time header.

  \return the background color of the time header
  \sa setTimeHeaderBackgroundColor()
*/
QColor KDGanttView::timeHeaderBackgroundColor () const
{
 return myTimeHeader->paletteBackgroundColor( );
}


/*!
  Returns the background color of the legend header.

  \return the background color of the legend header
  \sa setLegendHeaderBackgroundColor()
*/
QColor KDGanttView::legendHeaderBackgroundColor () const
{
 return myLegend->paletteBackgroundColor( );
}


/*!
  Adds a widget to the spacer widget above the list view part and
  below the ShowLegendButton.  To assign all the space above the
  Listview to the spacer widget, hide the ShowLegendButton by calling
  setShowLegendButton( false ).  The spacer widget is a QHBox.  You
  may add as many widgets as you want. They are ordered horizontally
  from left to right.  To remove a widget from the spacer widget, call
  widget->reparent(newParent,...) or delete the widget.  Since the spacer
  is a QHBox, the layout of the added widgets is managed by this
  QHBox.

  \param w A pointer to the widget to be added.
  \sa setShowLegendButton( )
*/
void KDGanttView::addUserdefinedLegendHeaderWidget( QWidget * w )
{
  if ( w ) {
      w->reparent ( spacerLeft, 0, QPoint(0,0) );
      spacerLeftLayout->addWidget( w );
  }
}


/*!
  Specifies whether drag operations are allowed in the Gantt
  view. Recurses over all items contained in the Gantt view and
  enables or disabled them for dragging.

  \param b true if dragging is enabled, false if dragging is disabled
  \sa isDragEnabled(), setDropEnabled(), isDropEnabled(), setDragDropEnabled()
*/
void KDGanttView::setDragEnabled( bool b )
{
  fDragEnabled = b;
 QListViewItemIterator it( myListView );
 for ( ; it.current(); ++it ) {
   (( KDGanttViewItem* )it.current())->setDragEnabled(b);
 }

}


/*!
  Specifies whether drop operations are allowed in the Gantt
  view. Recurses over all items contained in the Gantt view and
  enables or disabled them for dropping.

  \param b true if dragging is enabled, false if dragging is disabled
  \sa setDropEnabled(), setDragEnabled(), isDragEnabled(), setDragDropEnabled()
*/
void KDGanttView::setDropEnabled( bool b )
{
  fDropEnabled = b;

  //myListView->setAcceptDrops( b );
 QListViewItemIterator it( myListView );
 for ( ; it.current(); ++it ) {
   (( KDGanttViewItem* )it.current())->setDropEnabled(b);
 }
}


/*!
  Combines setDragEnabled() and setDropEnabled() in one convenient
  method.

  \param b true if dragging and dropping are enabled, false if
  dragging and dropping are disabled
  \sa setDragEnabled(), setDropEnabled()
*/
void KDGanttView::setDragDropEnabled( bool b )
{
  setDropEnabled( b );
  setDragEnabled( b );
}


/*!
  Returns whether dragging is enabled for this Gantt view.

  \return true if dragging is enabled
  \sa setDragEnabled(), setDragDropEnabled()
*/
bool KDGanttView::isDragEnabled() const
{
  return fDragEnabled;
}


/*!
  Returns whether dropping is enabled for this Gantt view.

  \return true if dropping is enabled
  \sa setDropEnabled(), setDragDropEnabled()
*/
bool KDGanttView::isDropEnabled() const
{
 return fDropEnabled;
}


/*!
  \deprecated Use isDragEnabled() instead
*/
bool KDGanttView::dragEnabled() const
{
  return isDragEnabled();
}


/*!
  \deprecated Use isDropEnabled() instead
*/
bool KDGanttView::dropEnabled() const
{
 return isDropEnabled();
}


/*!
  This virtual method makes it possible to specify user-defined drop
  handling.  The method is called directly before the internal drop
  handling is executed.  Return false to execute internal drop
  handling.  Return true to not execute internal drop handling.  In
  order to specify user-defined drop handling, subclass
  KDGanttView and reimplement this method.

  \param e The QDropEvent
  Note: e->source() is a pointer to the KDGanttView from which the drag started.
  I.e., if e->source() == this, this drag is an internal drag.
  \param droppedItem 0, if this is a drag operation from another
  KDGanttView instance.
  If this drag is an internal drag (i.e., within the KDGanttView),
  this parameter points to the dropped item.
  \param itemBelowMouse a pointer to the item below the dragged
  item (i.e., below the mouse).
  If you accept, the dragged item may be inserted
  in the KDGanttView as a child of this item.
  The value is 0 if there is no item below the dragged item,
  and the dragged item will be inserted as a root item.

  \return false, when the internal drop handling should be executed
          true, when the internal drop handling should not be executed
  \sa lvDropEvent(), lvStartDrag()
*/
bool  KDGanttView::lvDropEvent ( QDropEvent* e,
				 KDGanttViewItem* droppedItem,
				 KDGanttViewItem* itemBelowMouse )
{
    Q_UNUSED( e );
    Q_UNUSED( droppedItem );
    Q_UNUSED( itemBelowMouse );

  // Example code for user defined behaviour:
  // we want to accept the usual drags and the drags of files, which may be
  // a saved Gantt file.
  // Please uncomment the following lines for this behaviour
  // You have to uncomment lines in lvDragMoveEvent() and llvDragEnterEvent() as well

  // ************** begin example ************
  /*
  if ( QUriDrag::canDecode( e ) ) {
    QStrList lst;
    QUriDrag::decode( e, lst );
    // we try the first file of icon-url-list
    QString str = lst.at ( 0 );
    // remove file: at beginning of string
    str = str.right( str.length() - 5  );
    QFileInfo info;
    info.setFile( str ) ;
    if ( info.isFile() ) {
      if (!QMessageBox::information( this, "KDGantt Drag&Drop test",
				     "Try to insert file: "+ str + " ?",
				     "&Okay", "&Cancel",0,1  ) ) {
	QFile file( str );
	loadProject( &file ) ;
    }
    return true;
  }
  */
  // *********** end example ****************
  return false;
}


/*!
  This virtual method specifies whether a drag enter event may be
  accepted or not.
  To accept a drag enter event, call e->accept( true );
  To not accept a drag enter evente, call e->accept( false );
  This method does nothing but accepting the drag enter event, in case
  decoding is possible.
  In order to define accepting drops for particular items yourself,
  subclass KDGanttView and reimplement this method.

  \param e           The QDragMoveEvent
                     Note: e->source() is a pointer to the KDGanttView, the drag started from.
		     I.e., if e->source() == this, this drag is an internal drag.

  \sa lvDropEvent(), lvStartDrag(), lvDragMoveEvent()
*/
void  KDGanttView::lvDragEnterEvent ( QDragEnterEvent * e)
{
  // the standard behaviour:
  // accept drag enter events, if KDGanttViewItemDrag can decode the event
  // e->accept(KDGanttViewItemDrag::canDecode(e) );

  if ( KDGanttViewItemDrag::canDecode(e) ) {
    e->accept( true);
    return;
  }

  // example code for user defined behaviour:
  // we want to accecpt the usual drags and the drags of files, which may be
  // a saved Gantt file
  // Please uncomment the following lines for this behaviour
  // You have to uncomment lines in lvDragMoveEvent() and lvDropEvent () as well

  //  if ( QUriDrag::canDecode( e ) ) {
  // e->accept(true);
  //  return;
  // }

  e->accept( false );
}


/*!
  This virtual method specifies whether a drop event may be accepted or not.
  To accept a drop event, call e->accept( true );
  To not accept a drop event, call e->accept( false );
  This method does nothing but allowing to execute the internal
  drag move event handling.

  In order to specify user-defined drop acceptance for particular
  items, subclass KDGanttView and reimplement this method.

  \param e           The QDragMoveEvent
                     Note: e->source() is a pointer to the KDGanttView, the drag started from.
		     I.e. if e->source() == this, this drag is an internal drag.
         draggedItem 0, if this is a drag operation from another KDGanttView instance.
         If this drag is an internal drag (i.e., within the KDGanttView),
         this parameter points to the dragged item.
         itemBelowMouse a pointer to the item below the dragged item
         (i.e., below the mouse).
         If you accept the drop, the dragged item will be inserted
         in the KDGanttView as a child of this item.
         The value is 0 if there is no item below the dragged item,
         and the dragged item will be inserted as a root item.
  \return false, when the internal drag move event handling should be executed
          true, when the internal drag move event handling should not
          be executed; usually you should return true,
          if you have called  e->accept( true ) before.
  \sa lvDropEvent(), lvStartDrag()
*/
bool  KDGanttView::lvDragMoveEvent ( QDragMoveEvent* /*e*/,
				     KDGanttViewItem* /* draggedItem*/,
				     KDGanttViewItem* /*itemBelowMouse*/)
{

  // Example code 1:
  // To generally block items to be inserted as root items, subclass KDGanttView
  // and reimplement this method with to following code uncommented:

  // if ( !itemBelowMouse ) {
  //  e->accept( false );
  //  return true;
  //}
  //return false;

  // Example code 2:
  // To allow the drags of files, which may be
  // a saved Gantt file, subclass KDGanttView
  // and reimplement this method with to following code uncommented:

  // if ( QUriDrag::canDecode( e ) ) {
  //   e->accept(true);
  //   return true;
  //  }


 // normal behaviour - the internal drag move event handling should be executed
  return false;
}


/*!
  This virtual method creates a QDragObject and starts a drag for a
  KDGanttViewItem.
  In order to prevent drags of particular items, subclass from
  KDGanttView and reimplement this method.

  \param item the KDGanttViewItem, which should be dragged
  \sa lvDropEvent(), lvDragMoveEvent()
*/
void  KDGanttView::lvStartDrag (KDGanttViewItem* item)
{
  QDragObject* d = new KDGanttViewItemDrag(item, this, "itemdrag" );
  // call d->drag() to start the dragging
  // d->drag() returns true, if a move was requested as a drag
  // if a copy (by pressing the <Ctrl>-key) was performed, d->drag() returns false
  // In order to avoid starting drags for particular items, subclass KDGanttView
  // an reimplement this method.
  // insert here some code like
  // if ( item->parent() )
  // return;
  // This particular code will make it impossible to drag other items but root items.
  if ( d->drag()) {
      delete item;
  }
}


/*!
  Sets the width of the list view. Space will be taken from or given
  to the Gantt view.

  \param w the width of the list view
  \sa listViewWidth()
*/
void  KDGanttView::setListViewWidth( int w )
{
  int sw = mySplitter->width();
  QValueList<int> list;
  list.append(w);
  list.append(sw-w);
  mySplitter->setSizes( list );
}


/*!
  Returns the width of the list view.

  \return the width of the list view
  \sa setListViewWidth()
*/
int  KDGanttView::listViewWidth( )
{
  return leftWidget->width();
}


/*!
  Sets the scrollbar mode of the listview. The default is always off.
  Possible values are always on, always off and auto.
  It only makes sense to set this to always off
  if setGvVScrollBarMode() is set to always on.

  \param m the scrollbar mode.
  \sa setGvVScrollBarMode( )
*/
void  KDGanttView::setLvVScrollBarMode( QScrollView::ScrollBarMode m )
{
  myListView->setVScrollBarMode ( m );
}


/*!
  Sets the scrollbar mode of the time table. The default is auto.
  Possible values are auto, always on and always off.
  It only makes sense to set this to always off
  if setLvVScrollBarMode() is set to always on or auto.

  \param m The scrollbar mode.
  \sa setLvVScrollBarMode( )
*/
void  KDGanttView::setGvVScrollBarMode( QScrollView::ScrollBarMode m )
{
    myCanvasView->setMyVScrollBarMode ( m );
#if 0
  if ( m == QScrollView::Auto )
    qDebug("KDGanttView::setListViewVScrollBarMode: QScrollView::Auto not supported. Nothing changed. ");
  else
    {
      myCanvasView->setMyVScrollBarMode ( m );
      if ( m == QScrollView::AlwaysOn )
	timeHeaderSpacerWidget->setFixedWidth(myCanvasView->verticalScrollBar()->width() );
      else
	timeHeaderSpacerWidget->setFixedWidth( 0 );
    }
#endif
}


void  KDGanttView::itemAboutToBeDeleted( KDGanttViewItem * item)
{
#if QT_VERSION < 0x040000
  if ( myItemAttributeDialog && myItemAttributeDialog->getItem() == item ) {
    myItemAttributeDialog->reset( 0 );
  }
#endif
  emit itemDeleted( item );
}


/*!
  Specifies whether the user can expand the horizon
  by pressing the arrow buttons of the horizontal scrollbar of the gantt view.
  The default value for a newly created gantt chart is true.
  If set to false, the signal addOneTickLeft() or addOneTickRight() is emitted, 
  depending on the arrow button the user pressed.

  \param show if true, the user can expand the horizon.
  \sa userHorizonChangeEnabled() addOneTickLeft() addOneTickRight()
*/

void KDGanttView::setUserHorizonChangeEnabled( bool b )
{
    mUserHorizonChangeEnabled = b;
}


/*!
  Returns whether the user can expand the horizon 
  by clicking the scrollbar buttons.

  \return true if the user can expand the horizon
  \sa setUserHorizonChangeEnabled()
*/bool KDGanttView::userHorizonChangeEnabled() const
{
    return mUserHorizonChangeEnabled;
}

/*!
  This method is provided for convenience and as an example 
  how to set the user defined date time formats.
  It sets the formats to formats which are used in Germany.
  It calls
  setTimeHeaderTooltipDateTimeFormat( "dddd, dd. MMMM yyyy - h:mm:ss" );
  setHourFormat(Hour_24_FourDigit);
  setWeekScaleShowNumber( true );
  setWeekStartsMonday( true );
  setTimeHeaderDateFormatWeek( " d. MMM 'yy" );
  setTimeHeaderDateFormatDay( " d. MMM 'yy" );
  setTimeHeaderDatetimeFormatHour( "dddd, dd. MMM 'yy" );
  setTimeHeaderDatetimeFormatMinute( "ddd, dd. MMM h:mm" );
  
  \sa setTimeHeaderTooltipDateTimeFormat ()  setTimeHeaderDateFormatMonth() setTimeHeaderDateFormatWeek() setTimeHeaderDateFormatDay() setTimeHeaderDatetimeFormatMinute () setTimeHeaderDatetimeFormatHour() setWeekStartsMonday() setWeekScaleShowNumber()
*/

void KDGanttView::setGermanDateTimeFormat()
{
    setTimeHeaderTooltipDateTimeFormat( "dddd, dd. MMMM yyyy - h:mm:ss" );
    setHourFormat(Hour_24_FourDigit);
    setWeekScaleShowNumber( true );
    setWeekStartsMonday( true );
    setTimeHeaderDateFormatWeek( " d. MMM 'yy" );
    setTimeHeaderDateFormatDay( " d. MMM 'yy" );
    setTimeHeaderDatetimeFormatHour( "dddd, dd. MMM 'yy" );
    setTimeHeaderDatetimeFormatMinute( "ddd, dd. MMM h:mm" );
    //setTimeHeaderDatetimeFormatSecond( "dddd, dd. MMM h:mm" );
}

/*!
  Returns the user defined datetime format of the time header tooltip. 
  Returns an empty string as default, i.e. if there is no user defined format.
  You can find details about the possible format itself in the Qt documentation of:
  QDateTime::toString ( const QString & format ) const
  QDate::toString( const QString & format ) 
  QTime::toString( const QString & format )

  \return user defined datetime format of the time header tooltip
  \sa setTimeHeaderTooltipDateTimeFormat ()
*/
QString KDGanttView::timeHeaderTooltipDateTimeFormat() const 
{
    return myTimeHeader->tooltipDateTimeFormat();
}
/*!
  Sets the user defined datetime format of the time header tooltip.
  As default datetime format of the time header tooltip the return value of  QDateTime::toString () is used..
  You can find details about the possible user defined format itself in the Qt documentation of:
  QDateTime::toString ( const QString & format ) const
  QDate::toString( const QString & format ) 
  QTime::toString( const QString & format )
  
  \param the datetime format of time header tooltip. To unset the user defined format pass an empty string.
  \sa timeHeaderTooltipDateTimeFormat()
*/
void KDGanttView::setTimeHeaderTooltipDateTimeFormat( const QString& fmt )
{
    myTimeHeader->setTooltipDateTimeFormat( fmt );
}

/*!
  Returns the user defined date format for the upper date row of the time header if the time header scale is set to month. 
  Returns an empty string as default, i.e. if there is no user defined format.
  You can find details about the possible format itself in the Qt documentation of:
  QDateTime::toString ( const QString & format ) const
  QDate::toString( const QString & format ) 
  QTime::toString( const QString & format )

  \return user defined date format of the time header in monthly scale
  \sa setTimeHeaderDateFormatMonth()
*/
QString KDGanttView::timeHeaderDateFormatMonth() const  
{
    return myTimeHeader->dateFormatMonth();
}

/*!
  Sets the user defined date format for the upper date row of the time header if the time header scale is set to month.
  You can find details about the possible user defined format itself in the Qt documentation of:
  QDateTime::toString ( const QString & format ) const
  QDate::toString( const QString & format ) 
  QTime::toString( const QString & format )
  
  \param the user defined date format of the time header in monthly scale
  \sa timeHeaderDateFormatMonth()
*/
void KDGanttView::setTimeHeaderDateFormatMonth( const QString& fmt )
{
    myTimeHeader->setDateFormatMonth( fmt );
}

/*!
  Returns the user defined date format for the upper date row of the time header if the time header scale is set to week. 
  Returns an empty string as default, i.e. if there is no user defined format.
  You can find details about the possible format itself in the Qt documentation of:
  QDateTime::toString ( const QString & format ) const
  QDate::toString( const QString & format ) 
  QTime::toString( const QString & format )

  \return user defined date format of the time header in weekly scale
  \sa setTimeHeaderDateFormatWeek()
*/
QString KDGanttView::timeHeaderDateFormatWeek() const 
{
    return myTimeHeader->dateFormatWeek();
}
/*!
  Sets the user defined date format for the upper date row of the time header if the time header scale is set to week.
  You can find details about the possible user defined format itself in the Qt documentation of:
  QDateTime::toString ( const QString & format ) const
  QDate::toString( const QString & format ) 
  QTime::toString( const QString & format )
  
  \param the user defined date format of the time header in weekly scale
  \sa timeHeaderDateFormatWeek()
*/
void KDGanttView::setTimeHeaderDateFormatWeek( const QString& fmt )
{
    myTimeHeader->setDateFormatWeek( fmt );
}
/*!
  Returns the user defined date format for the upper date row of the time header if the time header scale is set to day. 
  Returns an empty string as default, i.e. if there is no user defined format.
  You can find details about the possible format itself in the Qt documentation of:
  QDateTime::toString ( const QString & format ) const
  QDate::toString( const QString & format ) 
  QTime::toString( const QString & format )

  \return user defined date format of the time header in daily scale
  \sa setTimeHeaderDateFormatDay()
*/
QString KDGanttView::timeHeaderDateFormatDay() const 
{
    return myTimeHeader->dateFormatDay();
}
/*!
  Sets the user defined date format for the upper date row of the time header if the time header scale is set to day.
  You can find details about the possible user defined format itself in the Qt documentation of:
  QDateTime::toString ( const QString & format ) const
  QDate::toString( const QString & format ) 
  QTime::toString( const QString & format )
  
  \param the user defined date format of the time header in daily scale
  \sa timeHeaderDateFormatDay()
*/
void KDGanttView::setTimeHeaderDateFormatDay( const QString& fmt )
{
    myTimeHeader->setDateFormatDay( fmt );
}



/*!
  Returns the user defined datetime format for the upper date row of the time header if the time header scale is set to second. 
  Returns an empty string as default, i.e. if there is no user defined format.
  You can find details about the possible format itself in the Qt documentation of:
  QDateTime::toString ( const QString & format ) const
  QDate::toString( const QString & format ) 
  QTime::toString( const QString & format )

  \return user defined date format of the time header in secondly scale
  \sa setTimeHeaderDateFormatSecond()
*/
QString KDGanttView::timeHeaderDatetimeFormatSecond() const
{
    return myTimeHeader->datetimeFormatSecond();
}
/*!
  Sets the user defined date format for the upper date row of the time header if the time header scale is set to second.
  You can find details about the possible user defined format itself in the Qt documentation of:
  QDateTime::toString ( const QString & format ) const
  QDate::toString( const QString & format ) 
  QTime::toString( const QString & format )
  
  \param the user defined date format of the time header in secondly scale
  \sa timeHeaderDateFormatMinute()
*/
void KDGanttView::setTimeHeaderDatetimeFormatSecond( const QString& fmt )
{
    myTimeHeader->setDatetimeFormatSecond( fmt );
}

/*!
  Returns the user defined datetime format for the upper date row of the time header if the time header scale is set to minute. 
  Returns an empty string as default, i.e. if there is no user defined format.
  You can find details about the possible format itself in the Qt documentation of:
  QDateTime::toString ( const QString & format ) const
  QDate::toString( const QString & format ) 
  QTime::toString( const QString & format )

  \return user defined date format of the time header in minutely scale
  \sa setTimeHeaderDateFormatMinute()
*/
QString KDGanttView::timeHeaderDatetimeFormatMinute() const
{
    return myTimeHeader->datetimeFormatMinute();
}
/*!
  Sets the user defined date format for the upper date row of the time header if the time header scale is set to minute.
  You can find details about the possible user defined format itself in the Qt documentation of:
  QDateTime::toString ( const QString & format ) const
  QDate::toString( const QString & format ) 
  QTime::toString( const QString & format )
  
  \param the user defined date format of the time header in minutely scale
  \sa timeHeaderDateFormatMinute()
*/
void KDGanttView::setTimeHeaderDatetimeFormatMinute( const QString& fmt )
{
    myTimeHeader->setDatetimeFormatMinute( fmt );
}


/*!
  Returns the user defined datetime format for the upper date row of the time header if the time header scale is set to hour. 
  Returns an empty string as default, i.e. if there is no user defined format.
  You can find details about the possible format itself in the Qt documentation of:
  QDateTime::toString ( const QString & format ) const
  QDate::toString( const QString & format ) 
  QTime::toString( const QString & format )

  \return user defined date format of the time header in hourly scale
  \sa setTimeHeaderDateFormatHour()
*/
QString KDGanttView::timeHeaderDatetimeFormatHour() const
{
    return myTimeHeader->datetimeFormatHour();
}
/*!
  Sets the user defined date format for the upper date row of the time header if the time header scale is set to hour.
  You can find details about the possible user defined format itself in the Qt documentation of:
  QDateTime::toString ( const QString & format ) const
  QDate::toString( const QString & format ) 
  QTime::toString( const QString & format )
  
  \param the user defined date format of the time header in hourly scale
  \sa timeHeaderDateFormatHour()
*/
void KDGanttView::setTimeHeaderDatetimeFormatHour( const QString& fmt )
{
    myTimeHeader->setDatetimeFormatHour( fmt );
}
/*!
  Sets the week start to monday. The default week start is sunday.
  This value is used in the time header to compute week starts for weekly scales.
  If the global scale is set to day the upper row of the time header displays a weekly scale.
  If the global scale is set to week the lower row of the time header displays a weekly scale.

  
  \param pass true to set week start to monday
         pass false to set week start to sunday
  \sa weekStartsMonday()
*/
void KDGanttView::setWeekStartsMonday( bool b )
{
    myTimeHeader->setWeekStartsMonday( b );
}

/*!
  Returns true if the week start is monday, false if the week start is sunday

  \return true if the week start is monday, false if the week start is sunday
  \sa setWeekStartsMonday()
*/
bool KDGanttView::weekStartsMonday() const
{
    return myTimeHeader->weekStartsMonday();
}
/*!
  Sets to display week numbers if the global scale is set to week.
  The ISO week numbering is used, i.e. the first week (week 1) of the year
  is the first week which contains a Thursday.

  
  \param pass true to display numbers of weeks
         pass false display numbers of the day of the date where the week starts
  \sa weekStartsMonday()
*/
void KDGanttView::setWeekScaleShowNumber( bool b )
{
    myTimeHeader->setWeekScaleShowNumber( b );
}
/*!
  Returns true if the week scale displays week number.
  The ISO week numbering is used, i.e. the first week (week 1) of the year
  is the first week which contains a Thursday.

  \return true if the week scale displays week number
  \sa setWeekScaleShowNumber()
*/
bool KDGanttView::weekScaleShowNumber() const
{
    return myTimeHeader->weekScaleShowNumber();
}
/*!
  Returns the week number for a given Date.
  The ISO week numbering is used, i.e. the first week (week 1) of the year
  is the first week which contains a Thursday.

  \param date the date
  \return the week number for the date
  \sa setWeekScaleShowNumber() weekScaleShowNumber()
*/
int KDGanttView::getWeekOfYear( const QDate& date )
{
    return myTimeHeader->getWeekOfYear( date );
}

/*!
  This method is provided for convenience.
  It returns the current day.
  \return the current day
*/

QDate KDGanttView::yesterday() const
{
    return myTimeHeader->yesterday() ;
}
/*!
  This method is provided for convenience.
  It returns the current day.
  \return yesterday
*/
QDate KDGanttView::today() const
{
    return myTimeHeader->today() ;
}
/*!
  This method is provided for convenience.
  It returns the current day.
  \return the next day
*/
QDate KDGanttView::tomorrow() const
{
    return myTimeHeader->tomorrow() ;
}
/*!
  This method is provided for convenience.
  It returns the start of the current week 
  dependend on the value weekStartsOnMonday().
  \return the start of current week
*/
QDate KDGanttView::currentWeek() const
{
    return myTimeHeader->currentWeek() ;
}
/*!
  This method is provided for convenience.
  It returns the start of the last week 
  (the week before current week)
  dependend on the value weekStartsOnMonday().
  \return the start of last week (the week before current week)
*/
QDate KDGanttView::lastWeek() const
{
    return myTimeHeader->lastWeek() ;
}
/*!
  This method is provided for convenience.
  It returns the first day of the current month.
  \return first day of the current month
*/
QDate KDGanttView::currentMonth() const
{
    return myTimeHeader->currentMonth() ;
}
/*!
  This method is provided for convenience.
  It returns the first day of last month.
  \return first day of last month
*/
QDate KDGanttView::lastMonth() const
{
    return myTimeHeader->lastMonth() ;
}
/*!
  This method is provided for convenience.
  It returns first day of the current year.
  \return first day of the current year
*/
QDate KDGanttView::currentYear() const
{
    return myTimeHeader->currentYear() ;
}
/*!
  This method is provided for convenience.
  It returns first day of the last year.
  \return first day of the last year
*/
QDate KDGanttView::lastYear() const
{
    return myTimeHeader->lastYear() ;
}

/*!
  This slot is provided for convenience.
  It centers the timeline on start of today.
*/
void KDGanttView::gotoToday()
{
    myTimeHeader->setTimeline( 0 );
}
/*!
  This slot is provided for convenience.
  It centers the timeline on start of yesterday.
*/
void KDGanttView::gotoYesterday()
{
    myTimeHeader->setTimeline( 1 );
}
/*!
  This slot is provided for convenience.
  It centers the timeline on start of current week.
*/
void KDGanttView::gotoCurrentWeek()
{
    myTimeHeader->setTimeline( 2 );
}
/*!
  This slot is provided for convenience.
  It centers the timeline on start of last week .
*/
void KDGanttView::gotoLastWeek()
{
    myTimeHeader->setTimeline( 3 );
}
/*!
  This slot is provided for convenience.
  It centers the timeline on start of current month.
*/
void KDGanttView::gotoCurrentMonth()
{
    myTimeHeader->setTimeline( 4 );
}
/*!
  This slot is provided for convenience.
  It centers the timeline on start of last month.
*/
void KDGanttView::gotoLastMonth()
{
    myTimeHeader->setTimeline( 5 );
}
/*!
  This slot is provided for convenience.
  It centers the timeline on start of current year.
*/
void KDGanttView::gotoCurrentYear()
{
    myTimeHeader->setTimeline( 6 );
}
/*!
  This slot is provided for convenience.
  It centers the timeline on start of last year.
*/
void KDGanttView::gotoLastYear()
{
    myTimeHeader->setTimeline( 7 );
}
/*!
  This slot is provided for convenience.
  It selects today as displayed timespan
  and makes sure that the selected timespan 
  is visible in the gantt view
  and sets the timeline start to the start
  of the selected timespan.
  The scale is set to KDGanttView::Hour.
*/
void KDGanttView::selectToday()
{
    myTimeHeader->setTimeline( 100 );
}
/*!
  This slot is provided for convenience.
  It selects yesterday as displayed timespan
  and makes sure that the selected timespan 
  is visible in the gantt view
  and sets the timeline start to the start
  of the selected timespan.
  The scale is set to KDGanttView::Hour.
*/
void KDGanttView::selectYesterday()
{
    myTimeHeader->setTimeline( 101 );
}
/*!
  This slot is provided for convenience.
  It selects the current week as displayed timespan
  and makes sure that the selected timespan 
  is visible in the gantt view
  and sets the timeline start to the start
  of the selected timespan.
  The scale is set to KDGanttView::Day.
*/
void KDGanttView::selectCurrentWeek()
{
    myTimeHeader->setTimeline( 102 );
}
/*!
  This slot is provided for convenience.
  It selects the last week as displayed timespan
  and makes sure that the selected timespan 
  is visible in the gantt view
  and sets the timeline start to the start
  of the selected timespan.
  The scale is set to KDGanttView::Day.
*/
void KDGanttView::selectLastWeek()
{
    myTimeHeader->setTimeline( 103);
}
/*!
  This slot is provided for convenience.
  It selects the current month as displayed timespan
  and makes sure that the selected timespan 
  is visible in the gantt view
  and sets the timeline start to the start
  of the selected timespan.
  The scale is set to KDGanttView::Day.
*/
void KDGanttView::selectCurrentMonth()
{
    myTimeHeader->setTimeline( 104 );
}
/*!
  This slot is provided for convenience.
  It selects the last month as displayed timespan
  and makes sure that the selected timespan 
  is visible in the gantt view
  and sets the timeline start to the start
  of the selected timespan.
  The scale is set to KDGanttView::Day.
*/
void KDGanttView::selectLastMonth()
{
    myTimeHeader->setTimeline( 105 );
}
/*!
  This slot is provided for convenience.
  It selects the current year as displayed timespan
  and makes sure that the selected timespan 
  is visible in the gantt view
  and sets the timeline start to the start
  of the selected timespan.
  The scale is set to KDGanttView::Month.
*/
void KDGanttView::selectCurrentYear()
{
    myTimeHeader->setTimeline( 106 );
}
/*!
  This slot is provided for convenience.
  It selects the last year as displayed timespan
  and makes sure that the selected timespan 
  is visible in the gantt view
  and sets the timeline start to the start
  of the selected timespan.
  The scale is set to KDGanttView::Month.
*/
void KDGanttView::selectLastYear()
{
    myTimeHeader->setTimeline( 107 );
}

/*!
  \fn addOneTickLeft();

  This signal is emitted when userHorizonChangeEnabled() is set to false
  and the slider of the vertical scrollbar of the gantt view is on the left side
  and the user clicks the "left" arrow of the scrollbar
  \sa setUserHorizonChangeEnabled() userHorizonChangeEnabled() addOneTickLeft()
*/

/*!
  \fn addOneTickRight();
  
  This signal is emitted when userHorizonChangeEnabled() is set to false
  and the slider of the vertical scrollbar of the gantt view is on the right side
  and the user clicks the "right" arrow of the scrollbar
  \sa setUserHorizonChangeEnabled() userHorizonChangeEnabled() addOneTickRight()

*/
/*!
  \fn void KDGanttView:: itemDeleted( KDGanttViewItem* );

  This signal is emitted when a gantt item is deleted.
  When the signal is emitted the pointer points to an existing KDGanttViewItem.
  This  pointer will become invalid immediately after returning of the signal.
*/


/*!
  \fn void KDGanttView::timeIntervalSelected( const QDateTime& start,  const QDateTime&  end);

  This signal is emitted when the user selects a time
  interval with the mouse on the time header connect this signal to
  the slot void zoomToSelection( const QDateTime& start, const
  QDateTime& end) to obtain automatic zooming.
*/


/*!
  \fn void KDGanttView::timeIntervallSelected( const QDateTime& start,  const QDateTime&  end);

  \deprecated This signal is deprecated, do not use it in new code;
  use timeIntervalSelected() instead. timeIntervallSelected() will be
  removed in future versions.
*/


/*!
    \fn void KDGanttView::rescaling( Scale )

    This signal is emitted if another scale is choosen than the
    specified one: i.e. if the horizon has a very wide range from
    start to end and as scale is choosen minute it may be that the
    size of the Gantt widget would become more than 32000 pixels. In
    this case the scale is automatically changed to Hour and
    rescaling( Hour ) is emitted.  If the widget size would be still
    more than 32000 pixels, the scale is automatically changed to day
    and rescaling( Day ) is emitted.  In the new scale, the
    minortickcount is increased such that the horizon will fit in the
    maximum size of 32000 pixels.
*/


/*!
  \fn void KDGanttView::gvCurrentChanged( KDGanttViewItem* item )

  This signal is emitted whenever the user clicks on the Gantt view
  \a item parameter is 0, if no item was clicked
*/


/*!
  \fn void KDGanttView::gvItemLeftClicked( KDGanttViewItem* )

  This signal is emitted whenever the user clicks into the Gantt view
  with the left mouse button.
*/


/*!
  \fn void KDGanttView::gvItemMidClicked( KDGanttViewItem* )

  This signal is emitted whenever the user clicks into the Gantt view
  with the middle mouse button.
*/

/*!
  \fn void KDGanttView::gvItemRightClicked( KDGanttViewItem* )

  This signal is emitted whenever the user clicks into the Gantt view
  with the right mouse button.
*/

/*!
  \fn void KDGanttView::gvMouseButtonClicked ( int button, KDGanttViewItem* item, const QPoint & pos)

  This signal is emitted when the user clicks into the Gantt view with
  any mouse button. Notice that \a pos is the absolute mouse position.
*/

/*!
  \fn void KDGanttView::gvItemDoubleClicked( KDGanttViewItem* )

  This signal is emitted whenever the user double-clicks into the Gantt view.
*/

/*!
  \fn void KDGanttView::gvContextMenuRequested ( KDGanttViewItem * item, const QPoint & pos )

  This signal is emitted when the user requests a context menu in the
  Gantt view. Notice that \a pos is the absolute mouse position.
*/


/*!
  \fn void KDGanttView::lvCurrentChanged( KDGanttViewItem* item )

  This signal is emitted whenever the user clicks on the list view
  \a item parameter is 0, if no item was clicked
*/


/*!
  \fn void KDGanttView::lvItemLeftClicked( KDGanttViewItem* )

  This signal is emitted whenever the user clicks into the list view
  with the left mouse button.
*/


/*!
  \fn void KDGanttView::lvItemMidClicked( KDGanttViewItem* )

  This signal is emitted whenever the user clicks into the list view
  with the middle mouse button.
*/

/*!
  \fn void KDGanttView::lvItemRightClicked( KDGanttViewItem* )

  This signal is emitted whenever the user clicks into the list view
  with the right mouse button.
*/

/*!
  \fn void KDGanttView::lvMouseButtonPressed ( int button,
  KDGanttViewItem* item, const QPoint & pos, int col)

  This signal is emitted when the user presses any mouse button in the
  list view. Notice that \a pos is the absolute mouse position.
*/

/*!
  \fn void KDGanttView::lvMouseButtonClicked ( int button,
  KDGanttViewItem* item, const QPoint & pos, int col)

  This signal is emitted when the user clicks into the
  list view with any mouse button . Notice that \a pos is the absolute
  mouse position.
*/

/*!
  \fn void KDGanttView::lvItemDoubleClicked( KDGanttViewItem* )

  This signal is emitted whenever the user double-clicks into the list view.
*/

/*!
  \fn void KDGanttView::lvItemRenamed( KDGanttViewItem*, int col,
  const QString& text )

  This signal is emitted whenever the user changes the name of an item
  in the list view using in-place editing. \a text contains the new
  text in the list view.
*/

/*!
  \fn void KDGanttView::lvContextMenuRequested( KDGanttViewItem *
  item, const QPoint & pos, int col )

  This signal is emitted when the user requests a context menu in the
  list view. Notice that \a pos is the absolute mouse position.
*/

/*!
  \fn void KDGanttView::lvSelectionChanged( KDGanttViewItem* )

  This signal is emitted whenever the user changes the selection in
  the list view.
*/


/*!
  \fn void KDGanttView::dropped ( QDropEvent * e, KDGanttViewItem* droppedItem, KDGanttViewItem* itemBelowMouse)

  This signal is emitted whenever a Gantt item is dropped onto the
  Gantt view. \a droppedItem is 0, if this is a drag operation from
  another KDGanttView instance. If this drag is an internal drag
  (i.e. within the KDGanttView), this parameter points to the dropped item.
  \a itemBelowMouse is a pointer to the item below the dragged
  item (i.e., below the mouse). The dragged item may be inserted
  in the KDGanttView as a child of this item.
  If The value is 0, if there is no item below the dragged item,
  and the dragged item will be inserted as a root item.

  In order to get user-defined behavior for drop events, reimplement
  KDGanttView::lvDropEvent()
*/


/*!
  \enum KDGanttView::RepaintMode

  Please see setRepaintMode() for a description of the values of this
  enumeration.
*/

#include "KDGanttView.moc"
