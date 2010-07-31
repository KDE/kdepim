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


#ifndef KDGANTTVIEWSUBWIDGETS_H
#define KDGANTTVIEWSUBWIDGETS_H


#include <tqwidget.h>
#include <tqlistview.h>
#include <tqsplitter.h>
#include <tqevent.h>
#include <tqvaluelist.h>
#include <tqcanvas.h>
#include <tqwhatsthis.h>
#include <tqpopupmenu.h>
#include <tqtooltip.h>
#include <tqtimer.h>
#include <tqgroupbox.h>
#include <tqvgroupbox.h>
#include <tqlayout.h>
#include <tqlabel.h>
#include <tqbrush.h>
#include <tqvbox.h>
#include <tqdockwindow.h>
#include <tqtimer.h>

#include "KDGanttView.h"
#include "KDGanttViewTaskLink.h"
#include "KDGanttViewTaskLinkGroup.h"
#include "KDGanttViewSummaryItem.h"
#include "KDGanttSemiSizingControl.h"
#include "KDGanttViewItemDrag.h"

#define Type_is_KDGanttGridItem 1
#define Type_is_KDGanttViewItem 2
#define Type_is_KDGanttTaskLink 3

class KDIntervalColorRectangle;
class KDCanvasWhatsThis;
class KDToolTip;
class KDCanvasRectangle;
class KDTimeHeaderToolTip;

class KDTimeHeaderWidget : public QWidget
{
   Q_OBJECT

public:
   typedef KDGanttView::Scale Scale;
   typedef KDGanttView::YearFormat YearFormat;
   typedef KDGanttView::HourFormat HourFormat;
   struct DateTimeColor {
     TQDateTime datetime;
     TQDateTime end;
     TQColor color;
     Scale minScaleView;
     Scale maxScaleView;
     //KDCanvasLine* canvasLine;
     KDCanvasRectangle* canvasRect;
   };
   typedef TQValueList<DateTimeColor> ColumnColorList;
   typedef TQValueList<KDIntervalColorRectangle *> IntervalColorList;
   /*
     enum Scale { Minute, Hour, Day, Week, Month, Auto };
     enum YearFormat { FourDigit, TwoDigit, TwoDigitApostrophe };
     enum HourFormat { Hour_24, Hour_12 };
   */

   KDTimeHeaderWidget (TQWidget* parent,KDGanttView* gant);
  ~KDTimeHeaderWidget();

   TQString getToolTipText(TQPoint p);
   void zoomToFit();
   void zoom(double, bool absolute = true);
   void zoomToSelection( TQDateTime startTime, TQDateTime endTime);
   double zoomFactor();
   void setAutoScaleMinorTickCount( int count );
   int autoScaleMinorTickCount();
   void setHorizonStart( const TQDateTime& start );
   TQDateTime horizonStart() const;
   void setHorizonEnd( const TQDateTime& start );
   TQDateTime horizonEnd() const;

   void setMaximumScale( Scale );
   KDTimeHeaderWidget::Scale maximumScale() const;
   void setMinimumScale( Scale );
   KDTimeHeaderWidget::Scale minimumScale() const;
   KDTimeHeaderWidget::Scale scale() const;
   void setMajorScaleCount( int count );
   int majorScaleCount() const;
   void setMinorScaleCount( int count );
   int minorScaleCount() const;
   void setMinimumColumnWidth( int width );
   int minimumColumnWidth() const;
   void setYearFormat( YearFormat format );
   KDTimeHeaderWidget::YearFormat yearFormat() const;
   void setHourFormat( HourFormat format );
   KDTimeHeaderWidget::HourFormat hourFormat() const;
   void setShowMajorTicks( bool );
   bool showMajorTicks() const;
   void setShowMinorTicks( bool );
   void setScale( Scale unit);
   bool showMinorTicks() const;
   void setColumnBackgroundColor( const TQDateTime& column,
				  const TQColor& color,
				  Scale mini =  KDGanttView::Minute ,
				  Scale maxi =  KDGanttView::Month);
#if 0
   // This API has been replaced with KDIntervalColorRectangle and addIntervalBackgroundColor
   void setIntervalBackgroundColor( const TQDateTime& start,
				    const TQDateTime& end,
				  const TQColor& color,
				  Scale mini =  KDGanttView::Minute ,
				  Scale maxi =  KDGanttView::Month);
   bool changeBackgroundInterval( const TQDateTime& oldstart,
				  const TQDateTime& oldend,
				  const TQDateTime& newstart,
				  const TQDateTime& newend );
   bool deleteBackgroundInterval( const TQDateTime& start,
				  const TQDateTime& end );
#endif
   void addIntervalBackgroundColor( KDIntervalColorRectangle* newItem );
   void clearBackgroundColor();
   TQColor columnBackgroundColor( const TQDateTime& column ) const;
   void setWeekendBackgroundColor( const TQColor& color );
   TQColor weekendBackgroundColor() const;
   void setWeekdayBackgroundColor( const TQColor& color, int weekday );
   TQColor weekdayBackgroundColor(int weekday) const;
   void setWeekendDays( int start, int end );
   void weekendDays( int& start, int& end ) const;
   void computeTicks(bool doNotComputeRealScale = false);
   void paintEvent(TQPaintEvent *);
   int getCoordX(TQDate);
   int getCoordX(TQDateTime);
   TQDateTime getDateTimeForIndex(int coordX, bool local = true );
   void setShowPopupMenu( bool show, bool showZoom, bool showScale,bool showTime,
                          bool showYear,bool showGrid, bool showPrint);
   bool registerStartTime();
   bool registerEndTime();
   bool showPopupMenu() const;
   ColumnColorList columnBackgroundColorList() const {
      return ccList;
    }
   TQColor weekdayColor[8];
   void repaintMe(int left, int wid, TQPainter *p = 0);

   void centerDateTime( const TQDateTime& center );

public slots:
    void setSettings(int);
    void checkWidth( int );
    void addTickRight( int num = 1 );
    void addTickLeft( int num = 1 );
    void preparePopupMenu();
signals:
    void sizeChanged( int );

private:
    friend class KDTimeTableWidget;
    friend class KDGanttViewItem;
    friend class KDGanttView;
    friend class KDGanttCanvasView; // calls computeIntervals
    virtual void mousePressEvent ( TQMouseEvent * e );
    virtual void mouseReleaseEvent ( TQMouseEvent * e );
    virtual void mouseDoubleClickEvent ( TQMouseEvent * e );
    virtual void mouseMoveEvent ( TQMouseEvent * e );
    double secsFromTo( TQDateTime begin, TQDateTime end );
    void updateTimeTable();
    void computeIntervals( int height );
    bool getColumnColor(TQColor& col,int coordLow, int coordHigh);
    void moveTimeLineTo(int x);
    //void  mousePressEvent ( TQMouseEvent * ) ;
    void resizeEvent ( TQResizeEvent * ) ;
    TQValueList<int> majorTicks;
    TQValueList<TQString> minorText;
    TQValueList<TQString> majorText;
    TQDateTime myHorizonStart, myHorizonEnd, myRealEnd,myRealStart;
    TQDateTime myCenterDateTime;
    void saveCenterDateTime();
    Scale myScale,myRealScale,myMaxScale,myMinScale;
    YearFormat myYearFormat;
    HourFormat myHourFormat;
    int myMinimumColumWidth;
    bool flagShowMajorTicks, flagShowMinorTicks, flagShowPopupMenu;
    bool flagShowZoom, flagShowScale ,flagShowTime ,flagShowYear;
    bool flagShowGrid ,flagShowPrint;
    bool flagStartTimeSet,flagEndTimeSet;
    TQColor myWeekendBackgroundColor;
    int myWeekendDaysStart, myWeekendDaysEnd;
    ColumnColorList ccList;
    IntervalColorList icList;
    int myMinorScaleCount,myMajorScaleCount;
    int myRealMinorScaleCount,myRealMajorScaleCount;
    bool flagDoNotRecomputeAfterChange,flagDoNotRepaintAfterChange;
    TQString getYear(TQDate);
    TQString getHour(TQTime);
    TQDateTime getEvenTimeDate(TQDateTime ,Scale);
    void computeRealScale(TQDateTime start);
    int myGridMinorWidth;
    int myMajorGridHeight;
    TQPopupMenu * myPopupMenu, *scalePopupMenu, *timePopupMenu;
    TQPopupMenu * yearPopupMenu, *gridPopupMenu;
    KDGanttView* myGanttView;
    double myZoomFactor;
    int myAutoScaleMinorTickcount;
    bool flagZoomToFit;
    int mySizeHint;
    int myMinimumWidth;
    int getTickTime();
    KDTimeHeaderToolTip* myToolTip;
    bool mouseDown;
    int beginMouseDown;
    int endMouseDown;
    bool autoComputeTimeLine;
    TQPixmap paintPix;
};

/* KDTimeTableWidget */
class KDListView ;

class KDTimeTableWidget : public QCanvas
{
   Q_OBJECT

public:
   KDTimeTableWidget (TQWidget* parent,KDGanttView* my);

    void setBlockUpdating( bool block = true );
    bool blockUpdating();
    void inc_blockUpdating();
    void dec_blockUpdating();
    void setShowTaskLinks( bool show );
    bool showTaskLinks();
    TQPtrList<KDGanttViewTaskLink>taskLinks();
    void clearTaskLinks();
    void updateMyContent();
    void removeItemFromTasklinks( KDGanttViewItem * );
    void setHorBackgroundLines( int count, TQBrush brush );
    int horBackgroundLines( TQBrush& brush );

    void setNoInformationBrush( const TQBrush& brush );
    TQBrush noInformationBrush() const;

    int getCoordX( TQDateTime dt );

signals:
   void   heightComputed( int );

public slots:
  void expandItem(TQListViewItem * );
  void collapseItem(TQListViewItem * );
  void highlightItem(TQListViewItem * );
  void resetWidth( int );
  void checkHeight( int );
private:
   friend class KDGanttViewTaskLink;
   friend class KDTimeHeaderWidget;
   friend class KDGanttView;
   friend class KDGanttViewTaskItem;
   KDGanttView* myGanttView;

   bool taskLinksVisible;

   TQPtrList<KDGanttViewTaskLink> myTaskLinkList;

   TQPtrList<KDCanvasLine> verGridList;
   TQPtrList<KDCanvasLine> horGridList;
   TQPtrList<KDCanvasRectangle> horDenseList;
   TQPtrList<KDCanvasRectangle> showNoInfoList;
   int denseLineCount;
   TQBrush denseLineBrush, noInfoLineBrush;
   TQPtrList<KDCanvasRectangle> columnColorList;

  int computeHeight();
  void computeVerticalGrid();
  void computeHorizontalGrid();
  void computeDenseLines();
  void computeShowNoInformation();
  void computeTaskLinks();
  void computeMinorGrid();
  void computeMajorGrid();

   void showMajorGrid();
   void showMinorGrid();
   void hideGrid();

   TQPen gridPen;
   int maximumComputedGridHeight;
  int minimumHeight;
  int int_blockUpdating;
  bool flag_blockUpdating;
  int pendingHeight;
  int pendingWidth;

};

class KDLegendWidget : public KDGanttSemiSizingControl
{
   Q_OBJECT

public:
  KDLegendWidget ( TQWidget* parent, KDGanttMinimizeSplitter* legendParent );
  void showMe(bool);
  bool isShown();
  void addLegendItem( KDGanttViewItem::Shape shape, const TQColor& shapeColor, const TQString& text );
  void clearLegend();
  void setFont( TQFont );
  void drawToPainter( TQPainter *p );
  void setAsDockwindow( bool dockwin );
  bool asDockwindow();
  TQDockWindow* dockwindow();
  TQSize legendSize();
  TQSize legendSizeHint();
 private:
  TQGroupBox * myLegend;
  TQLabel* myLabel;
  TQScrollView * scroll;
  TQDockWindow* dock;
  KDGanttMinimizeSplitter* myLegendParent;
};

class KDGanttView;
class KDListView : public QListView
{
   Q_OBJECT

public:
   KDListView (TQWidget* parent,KDGanttView* gv );
   KDGanttView* myGanttView;
   void drawToPainter( TQPainter *p, bool drawHeader=false );
   void setCalendarMode( bool mode );
  bool calendarMode() { return _calendarMode; };
  TQString getWhatsThisText(TQPoint p);
  void setOpen ( TQListViewItem * item, bool open );
  void dragEnterEvent ( TQDragEnterEvent * );
  void dragMoveEvent ( TQDragMoveEvent * );
  void dragLeaveEvent ( TQDragLeaveEvent * );
  void dropEvent ( TQDropEvent * );
  TQDragObject * dragObject ();
  void startDrag ();
  void paintemptyarea ( TQPainter * p, const TQRect & rect ){ TQListView::paintEmptyArea( p, rect );};

public:
    class DrawableItem {
    public:
        DrawableItem(int level, int ypos, TQListViewItem *item ) { y = ypos; l = level; i = item; };
        int y;
        int l;
        TQListViewItem * i;
    };
protected:
    void drawAllContents(TQPainter * p, int cx, int cy, int cw, int ch);
    int buildDrawables(TQPtrList<KDListView::DrawableItem> &lst, int level, int ypos, TQListViewItem *item, int ymin, int ymax) const;

private slots:
  void dragItem( TQListViewItem * );
 private:
   void resizeEvent ( TQResizeEvent * ) ;
  void contentsMouseDoubleClickEvent ( TQMouseEvent * e );
  bool _calendarMode;



};


class KDCanvasText : public QCanvasText
{
public:
    KDCanvasText( KDTimeTableWidget* canvas, void* parentItem, int type );
    int myParentType;
    void* myParentItem;
};


class KDCanvasLine : public QCanvasLine
{
public:
    KDCanvasLine( KDTimeTableWidget* canvas, void* parentItem, int type );
    int myParentType;
    void* myParentItem;
};


class KDCanvasPolygonItem: public QCanvasPolygonalItem
{
public:
    KDCanvasPolygonItem( KDTimeTableWidget* canvas, void* parentItem,
                         int type );
    int myParentType;
    void* myParentItem;
};


class KDCanvasPolygon: public QCanvasPolygon
{
public:
    KDCanvasPolygon( KDTimeTableWidget* canvas, void* parentItem, int type );
    int myParentType;
    void* myParentItem;
};


class KDCanvasEllipse: public QCanvasEllipse
{
public:
    KDCanvasEllipse( KDTimeTableWidget* canvas, void* parentItem, int type );
    int myParentType;
    void* myParentItem;
};


class KDCanvasRectangle: public QCanvasRectangle
{
public:
    KDCanvasRectangle( KDTimeTableWidget* canvas, void* parentItem, int type );
    int myParentType;
    void* myParentItem;
};


// Interval-color-rectangle, such as the one used in the freebusy view for the current event
class KDIntervalColorRectangle: public KDCanvasRectangle
{
public:
  KDIntervalColorRectangle( KDGanttView* view );

  void setDateTimes( const TQDateTime& start,
                     const TQDateTime& end );
  TQDateTime start() const { return mStart; }
  TQDateTime end() const { return mEnd; }

  void setColor( const TQColor& color );

  enum HitTest { Start, Middle, End };
  HitTest hitTest( KDTimeHeaderWidget* timeHeader, const TQPoint& pos ) const;

  void layout( KDTimeHeaderWidget* timeHeader, int height );

  static const int RTTI = 0x0c58;
  /*reimp*/ int rtti() const { return RTTI; }

private:
  TQColor mColor;
  TQDateTime mStart;
  TQDateTime mEnd;
};

class KDCanvasToolTip;

class KDGanttCanvasView : public QCanvasView
{
    Q_OBJECT

public:
    KDGanttCanvasView(KDGanttView* sender, TQCanvas* canvas = 0, TQWidget* parent = 0, const char* name = 0 );
    ~KDGanttCanvasView();
    TQString getToolTipText(TQPoint p);
    TQString getWhatsThisText(TQPoint p);
    void drawToPainter ( TQPainter * p );
    void resetCutPaste( KDGanttViewItem* );
    void setShowPopupMenu( bool show );
    bool showPopupMenu();
    void cutItem (  KDGanttViewItem* );
    void insertItemAsRoot( KDGanttViewItem* );
    void insertItemAsChild( KDGanttViewItem* , KDGanttViewItem* );
    void insertItemAfter( KDGanttViewItem* , KDGanttViewItem* );
protected:
    friend class KDGanttView;
    friend class KDListView;
    virtual void contentsMousePressEvent ( TQMouseEvent * ) ;
    virtual void contentsMouseReleaseEvent ( TQMouseEvent * );
    virtual void contentsMouseDoubleClickEvent ( TQMouseEvent * );
    virtual void contentsMouseMoveEvent ( TQMouseEvent * ) ;
    virtual void viewportPaintEvent ( TQPaintEvent * pe );
    void resizeEvent ( TQResizeEvent * ) ;
    void set_MouseTracking(bool on);
    int getType(TQCanvasItem*);
    KDGanttViewItem* getItem(TQCanvasItem*);
    KDGanttViewTaskLink* getLink(TQCanvasItem*);
    int getItemArea(KDGanttViewItem *item, int x);
    int getLinkType(int from, int to);

    KDGanttView* mySignalSender;
    KDGanttViewItem* currentItem, *lastClickedItem, *cuttedItem;
    TQCanvasRectangle* movingItem;
    KDGanttViewTaskItem* movingGVItem;
    TQPoint movingStart;
    TQDateTime movingStartDate;
    enum MovingOperation { Moving, ResizingLeft, ResizingRight };
    MovingOperation movingOperation;
    KDGanttViewTaskLink* currentLink;
    KDCanvasWhatsThis* myWhatsThis;
    TQPopupMenu* onItem;
    bool _showItemAddPopupMenu;
    int myMyContentsHeight;
    KDGanttViewItem *fromItem;
    bool linkItemsEnabled;
    TQCanvasLine *linkLine;
    int fromArea;
    bool autoScrollEnabled;
    bool mouseDown;

signals:
  void heightResized( int );
  void widthResized( int );

public slots:
  void set_Mouse_Tracking(bool on);
  void moveMyContent( int, int );
  void setMyContentsHeight( int );
  void updateHorScrollBar();
private slots:
  void cutItem();
  void pasteItem( int );
  void newRootItem( int );
  void newChildItem( int );
  void slotScrollTimer();
  void myUpdateScrollBars();

private:
  MovingOperation gvItemHitTest( KDGanttViewItem *item, KDTimeHeaderWidget* timeHeader, const TQPoint &pos );
private:
  KDCanvasToolTip* myToolTip;
  TQTimer *myScrollTimer;
  TQPoint mousePos;
  TQTimer scrollBarTimer;
};

class KDTimeHeaderToolTip :public QToolTip
{

public:
  KDTimeHeaderToolTip( TQWidget *wid, KDTimeHeaderWidget* header ) : TQToolTip( wid ), _wid(wid),_header (header) {

};

protected:
  virtual void maybeTip( const TQPoint& p)
    {
      static bool ishidden = true;
      if (TQToolTip::isGloballyEnabled () ) {
	if (ishidden) {
	  tip( TQRect( p.x(),p.y(),5,5), _header->getToolTipText(p));
	}
	else
	  hide();
	ishidden = !ishidden;
      }
  }
private:
  TQWidget* _wid;
  KDTimeHeaderWidget * _header;
};

class KDCanvasToolTip :public QToolTip
{

public:
  KDCanvasToolTip( TQWidget *wid, KDGanttCanvasView* canview ) : TQToolTip( wid ), _wid(wid),_canview (canview) {

};

protected:
  virtual void maybeTip( const TQPoint& p)
    {
      static bool ishidden = true;
      if (TQToolTip::isGloballyEnabled () ) {
	if (ishidden) {
	  tip( TQRect( p.x()-2,p.y()-2,5,5), _canview->getToolTipText(p));
	}
	else
	  hide();
	ishidden = !ishidden;
      }
  }
private:
  TQWidget* _wid;
  KDGanttCanvasView * _canview;
};

class KDCanvasWhatsThis :public QWhatsThis
{
public:
  KDCanvasWhatsThis( TQWidget *wid, KDGanttCanvasView* canview ) : TQWhatsThis( wid ), _wid(wid),_canview (canview) { };

protected:
  virtual TQString text( const TQPoint& p)
  {
    return _canview->getWhatsThisText(p) ;
  }
private:
  TQWidget* _wid;
  KDGanttCanvasView * _canview;
};

class KDListViewWhatsThis :public QWhatsThis
{
public:
  KDListViewWhatsThis( TQWidget *wid, KDListView* view ) : TQWhatsThis( wid ), _wid(wid),_view (view) { };

protected:
  virtual TQString text( const TQPoint& p)
  {
    return _view->getWhatsThisText(p) ;
  }
private:
  TQWidget* _wid;
  KDListView * _view;
};



#endif
