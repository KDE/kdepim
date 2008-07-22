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


#ifndef KDGANTTVIEWSUBWIDGETS_H
#define KDGANTTVIEWSUBWIDGETS_H


#include <QWidget>
#include <QSplitter>
#include <QEvent>
#include <QWhatsThis>
#include <QTimer>
#include <QLayout>
#include <QLabel>
#include <QBrush>

#include "kdgantt_qt3_compat.h"


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
class KDCanvasRectangle;
class KDTimeHeaderToolTip;
class KActionCollection;
class KSelectAction;

class KDTimeHeaderWidget : public QWidget
{
   Q_OBJECT

public:
   typedef KDGanttView::Scale Scale;
   typedef KDGanttView::YearFormat YearFormat;
   typedef KDGanttView::HourFormat HourFormat;
   typedef KDGanttView::ShowTicksType ShowTicksType;
   struct DateTimeColor {
     QDateTime datetime;
     QDateTime end;
     QColor color;
     Scale minScaleView;
     Scale maxScaleView;
     //KDCanvasLine* canvasLine;
     KDCanvasRectangle* canvasRect;
       int priority;
   };
   typedef QValueList<DateTimeColor> ColumnColorList;
   typedef QValueList<KDIntervalColorRectangle *> IntervalColorList;
   /*
     enum Scale { Minute, Hour, Day, Week, Month, Auto };
     enum YearFormat { FourDigit, TwoDigit, TwoDigitApostrophe };
     enum HourFormat { Hour_24, Hour_12 };
   */

   KDTimeHeaderWidget (QWidget* parent,KDGanttView* gant);
  ~KDTimeHeaderWidget();

   QString getToolTipText(QPoint p);
   void zoomToFit();
   void zoom(double, bool absolute = true);
   void zoomToSelection( const QDateTime& startTime, const QDateTime &endTime);
   void performZoomToSelection( const QDateTime& startTime, const QDateTime &endTime);
   void zoomToSelectionAndSetStartEnd( const QDateTime &start, const QDateTime &end);
   double zoomFactor();
   void setAutoScaleMinorTickCount( int count );
   int autoScaleMinorTickCount();
   void setHorizonStart( const QDateTime& start );
   QDateTime horizonStart() const;
   void setHorizonEnd( const QDateTime& start );
   QDateTime horizonEnd() const;

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
   YearFormat yearFormat() const;
   void setHourFormat( HourFormat format );
   HourFormat hourFormat() const;
   void setShowTicks( ShowTicksType show = KDGanttView::ShowMajorTicks );
   ShowTicksType showTicks() const;
   void setScale( Scale unit, bool update = true );
   void setColumnBackgroundColor( const QDateTime& column,
				  const QColor& color,
				  Scale mini =  KDGanttView::Second ,
				  Scale maxi =  KDGanttView::Month);
#if 0
   // This API has been replaced with KDIntervalColorRectangle and addIntervalBackgroundColor
    void setIntervalBackgroundColor( const QDateTime& start,
                                     const QDateTime& end,
                                     const QColor& color,
                                     int priority = 0,
                                     Scale mini =  KDGanttView::Second ,
                                     Scale maxi =  KDGanttView::Month);
   bool changeBackgroundInterval( const QDateTime& oldstart,
				  const QDateTime& oldend,
				  const QDateTime& newstart,
				  const QDateTime& newend );
   bool deleteBackgroundInterval( const QDateTime& start,
				  const QDateTime& end );
#endif
   void addIntervalBackgroundColor( KDIntervalColorRectangle* newItem );
   void clearBackgroundColor();
   QColor columnBackgroundColor( const QDateTime& column ) const;
   void setWeekendBackgroundColor( const QColor& color );
   QColor weekendBackgroundColor() const;
   void setWeekdayBackgroundColor( const QColor& color, int weekday );
   QColor weekdayBackgroundColor(int weekday) const;
   void setWeekendDays( int start, int end );
   void weekendDays( int& start, int& end ) const;
   void computeTicks(bool doNotComputeRealScale = false);
   void paintEvent(QPaintEvent *);
   int getCoordX(QDate);
   int getCoordX(QDateTime);
   QDateTime getDateTimeForIndex(int coordX, bool local = true );
   void setShowPopupMenu( bool show, bool showZoom, bool showScale,bool showTime,
                          bool showYear,bool showGrid, bool showPrint);
   bool registerStartTime();
   bool registerEndTime();
   bool showPopupMenu() const;
   ColumnColorList columnBackgroundColorList() const {
      return ccList;
    }
   IntervalColorList intervalBackgroundColorList() const {
      return icList;
   }
   QColor weekdayColor[8];
   void repaintMe(int left, int wid, QPainter *p = 0);

    void centerDateTime( const QDateTime& center, bool changeHorizon = false );

    void setTooltipDateTimeFormat( const QString& fmt );
    const QString tooltipDateTimeFormat();

    const QString dateFormatMonth();
    void setDateFormatMonth( const QString& fmt );
    const QString dateFormatWeek();
    void setDateFormatWeek( const QString& fmt );
    const QString dateFormatDay();
    void setDateFormatDay( const QString& fmt );
    const QString datetimeFormatHour();
    void setDatetimeFormatHour( const QString& fmt );
    const QString datetimeFormatMinute();
    void setDatetimeFormatMinute( const QString& fmt );
    const QString datetimeFormatSecond();
    void setDatetimeFormatSecond( const QString& fmt );

    void setWeekStartsMonday( bool b );
    bool weekStartsMonday();
    void setWeekScaleShowNumber( bool b );
    bool weekScaleShowNumber();

    QDate yesterday() const;
    QDate today() const;
    QDate tomorrow() const;
    QDate currentWeek() const;
    QDate lastWeek() const;
    QDate currentMonth() const;
    QDate lastMonth() const;
    QDate currentYear() const;
    QDate lastYear() const;

    void pendingPaint();

public slots:
    void checkWidth( int );
    void addTickRight( int num = 1 );
    void addTickLeft( int num = 1 );
    void preparePopupMenu();

    void center(const QDate &dt);
    void centerToday() { center(today()); }
    void centerYesterday() { center(yesterday()); }
    void centerCurrentWeek() { center(currentWeek()); }
    void centerLastWeek() { center(lastWeek()); }
    void centerCurrentMonth() { center(currentMonth()); }
    void centerLastMonth() { center(lastMonth()); }
    void centerCurrentYear() { center(currentYear()); }
    void centerLastYear() { center(lastYear()); }

    void zoomTo( Scale unit, const QDate &start, const QDate &end );
    void showToday() { zoomTo( KDGanttView::Hour, today(), tomorrow() ); }
    void showYesterday() { zoomTo( KDGanttView::Hour, yesterday(), today() ); }
    void showCurrentWeek() { zoomTo( KDGanttView::Day, currentWeek(), currentWeek().addDays( 7 ) ); }
    void showLastWeek() { zoomTo( KDGanttView::Day, lastWeek(), currentWeek() ); }
    void showCurrentMonth() { zoomTo( KDGanttView::Day, currentMonth(), currentMonth().addMonths( 1) ); }
    void showLastMonth() { zoomTo( KDGanttView::Day, lastMonth(), currentMonth() ); }
    void showCurrentYear() { zoomTo( KDGanttView::Month, currentYear(), currentYear().addYears( 1) ); }
    void showLastYear() { zoomTo( KDGanttView::Month, lastYear(), currentYear() ); }

    void zoom1() { zoom(1.0); }
    void zoom2() { zoom(2.0,false); }
    void zoomOut2() { zoom(0.5,false); }
    void zoom6() { zoom(6.0,false); }
    void zoomOut6() { zoom(0.16666,false); }
    void zoom12() { zoom(12.0,false); }
    void zoomOut12() { zoom(0.08333,false); }

    void gridSettings( int i );

    void setScale( int i );
    void setHourFormat( int i );
    void setYearFormat( int i );

signals:
    void sizeChanged( int );

private:
    friend class KDTimeTableWidget;
    friend class KDGanttViewItem;
    friend class KDGanttView;
    friend class KDGanttCanvasView; // calls computeIntervals
    virtual void mousePressEvent ( QMouseEvent * e );
    virtual void mouseReleaseEvent ( QMouseEvent * e );
    virtual void mouseDoubleClickEvent ( QMouseEvent * e );
    virtual void mouseMoveEvent ( QMouseEvent * e );
    double secsFromTo( QDateTime begin, QDateTime end );
    void updateTimeTable();
    void computeIntervals( int height );
    bool getColumnColor(QColor& col,int coordLow, int coordHigh);
    void moveTimeLineTo(int x);
    //void  mousePressEvent ( QMouseEvent * ) ;
    void resizeEvent ( QResizeEvent * ) ;
    QValueList<int> majorTicks;
    QValueList<QString> minorText;
    QValueList<QString> majorText;
    QDateTime myHorizonStart, myHorizonEnd, myRealEnd,myRealStart;
    QDateTime myCenterDateTime;
    void saveCenterDateTime();
    Scale myScale,myRealScale,myMaxScale,myMinScale;
    YearFormat myYearFormat;
    HourFormat myHourFormat;
    int myMinimumColumWidth;
    ShowTicksType flagShowTicks;
    bool flagShowPopupMenu;
    bool flagShowZoom, flagShowScale ,flagShowTime ,flagShowYear;
    bool flagShowGrid ,flagShowPrint;
    bool flagStartTimeSet,flagEndTimeSet;
    QColor myWeekendBackgroundColor;
    int myWeekendDaysStart, myWeekendDaysEnd;
    ColumnColorList ccList;
    IntervalColorList icList;
    int myMinorScaleCount,myMajorScaleCount;
    int myRealMinorScaleCount,myRealMajorScaleCount;
    bool flagDoNotRecomputeAfterChange,flagDoNotRepaintAfterChange;
    QString getYear(QDate);
    QString getHour(QTime);
    QString getHourMinutes(QTime);
    int  getMaxTextWidth( const QString& format, int mode );
    QFont myFont();
    int getWeekOfYear( const QDate& date );
    QDateTime getEvenTimeDate(QDateTime ,Scale);
    void computeRealScale(QDateTime start);
    int myGridMinorWidth;
    int myMajorGridHeight;
    KDGanttView* myGanttView;
    double myZoomFactor;
    int myAutoScaleMinorTickcount;
    bool flagZoomToFit;
    int mySizeHint;
    int myMinimumWidth;
    int getTickTime();
    QDateTime addMajorTickTime( const QDateTime& dt, int fac );
    KDTimeHeaderToolTip* myToolTip;
    bool mouseDown;
    int beginMouseDown;
    int endMouseDown;
    bool autoComputeTimeLine;
    QPixmap paintPix;
    QString mDateFormatMonth;
    QString mDateFormatWeek;
    QString mDateFormatDay;
    QString mDatetimeFormatHour;
    QString mDatetimeFormatMinute;
    QString mDatetimeFormatSecond;
    int mMaxWidFormatMonth;
    int mMaxWidFormatWeek;
    int mMaxWidFormatDay;
    int mMaxWidtimeFormatHour;
    int mMaxWidtimeFormatMinute;
    int mMaxWidtimeFormatSecond;
    QString mTooltipDateFormat;
    bool mWeekStartsMonday;
    bool mWeekScaleShowNumber;

    QMenu *myPopupMenu;
    KActionCollection *actionCollection;
    KSelectAction *mGridAction, *mScaleAction, *mTimeFormatAction, *mYearFormatAction;
    QAction *mGotoAction, *mTimespanAction, *mZoomAction, *mPrintAction;

};

/* KDTimeTableWidget */
class KDListView ;

class KDTimeTableWidget : public QCanvas
{
   Q_OBJECT

public:
   KDTimeTableWidget (QWidget* parent,KDGanttView* my);
   ~KDTimeTableWidget ();
    void setBlockUpdating( bool block = true );
    bool blockUpdating();
    void inc_blockUpdating();
    void dec_blockUpdating();
    void setShowTaskLinks( bool show );
    bool showTaskLinks();
    QPtrList<KDGanttViewTaskLink>taskLinks();
    void removeItemFromTasklinks( KDGanttViewItem * );
    void setHorBackgroundLines( int count, QBrush brush );
    int horBackgroundLines( QBrush& brush );

    void setNoInformationBrush( const QBrush& brush );
    QBrush noInformationBrush() const;

    int getCoordX( QDateTime dt );

    int minimumHeight();
    void computeTaskLinksForItem( KDGanttViewItem * );

signals:
   void   heightComputed( int );

public slots:
void updateSlot();
    void simpleUpdateSlot();
    void updateMyContent();
    void forceUpdate();
    void simpleUpdate();
#if QT_VERSION < 0x040000
    void expandItem(QListViewItem * );
    void collapseItem(QListViewItem * );
#else
    void expandItem(Q3ListViewItem * );
    void collapseItem(Q3ListViewItem * );
#endif
    void resetWidth( int );
    void checkHeight( int );
private:
    friend class KDGanttViewTaskLink;
    friend class KDTimeHeaderWidget;
    friend class KDGanttView;
    friend class KDGanttViewTaskItem;
    KDGanttView* myGanttView;

    bool taskLinksVisible;

    QPtrList<KDGanttViewTaskLink> myTaskLinkList;

    QPtrList<KDCanvasLine> verGridList;
    QPtrList<KDCanvasLine> horGridList;
    QPtrList<KDCanvasRectangle> horDenseList;
    QPtrList<KDCanvasRectangle> showNoInfoList;
    int denseLineCount;
    QBrush denseLineBrush, noInfoLineBrush;
    QPtrList<KDCanvasRectangle> columnColorList;

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

    QPen gridPen;
    int maximumComputedGridHeight;
    int mMinimumHeight;
    int int_blockUpdating;
    bool flag_blockUpdating;
    int pendingHeight;
    int pendingWidth;
    QTimer *mUpdateTimer;
    QTimer *mSimpleUpdateTimer;

};

class KDLegendWidget : public KDGanttSemiSizingControl
{
   Q_OBJECT

public:
  KDLegendWidget ( QWidget* parent, KDGanttMinimizeSplitter* legendParent );
  void showMe(bool);
  bool isShown();
  void addLegendItem( KDGanttViewItem::Shape shape, const QColor& shapeColor, const QString& text );
    void addLegendItem( KDGanttViewItem::Shape shape, const QColor& shapeColor, const QString& text,
                        KDGanttViewItem::Shape shape2, const QColor& shapeColor2,const QString& text2 );
  void clearLegend();
  void setFont( QFont );
  void drawToPainter( QPainter *p );
  void setAsDockwindow( bool dockwin );
  bool asDockwindow();
  QDockWindow* dockwindow();
  QSize legendSize();
  QSize legendSizeHint();
 private:
  QGroupBox * myLegend;
  QLabel* myLabel;
  QScrollView * scroll;
  QDockWindow* dock;
  KDGanttMinimizeSplitter* myLegendParent;
};

class KDGanttView;
class KDListView : public QListView
{
   Q_OBJECT

public:
   KDListView (QWidget* parent,KDGanttView* gv );
   KDGanttView* myGanttView;
    QSize minimumSizeHint () const;
   void drawToPainter( QPainter *p );
   void setCalendarMode( bool mode );
  bool calendarMode() { return _calendarMode; }
  QString getWhatsThisText(QPoint p);
  void setOpen ( QListViewItem * item, bool open );
  void dragEnterEvent ( QDragEnterEvent * );
  void dragMoveEvent ( QDragMoveEvent * );
  void dragLeaveEvent ( QDragLeaveEvent * );
  void dropEvent ( QDropEvent * );
  QDragObject * dragObject ();
  void startDrag ();
  void paintemptyarea ( QPainter * p, const QRect & rect ){ QListView::paintEmptyArea( p, rect );}
private slots:
  void dragItem( QListViewItem * );
 private:
   void resizeEvent ( QResizeEvent * ) ;
  void contentsMouseDoubleClickEvent ( QMouseEvent * e );
  bool _calendarMode;



};


class KDCanvasText : public QCanvasText
{
public:
    KDCanvasText( KDTimeTableWidget* canvas, void* parentItem, int type );
    int myParentType;
    void* myParentItem;
    void updateItem() { update(); }
};


class KDCanvasLine : public QCanvasLine
{
public:
    KDCanvasLine( KDTimeTableWidget* canvas, void* parentItem, int type );
    int myParentType;
    void* myParentItem;
    void updateItem() { update(); }
};


class KDCanvasPolygonItem: public QCanvasPolygonalItem
{
public:
    KDCanvasPolygonItem( KDTimeTableWidget* canvas, void* parentItem,
                         int type );
    int myParentType;
    void* myParentItem;
    void updateItem() { update(); }
};


class KDCanvasPolygon: public QCanvasPolygon
{
public:
    KDCanvasPolygon( KDTimeTableWidget* canvas, void* parentItem, int type );
    int myParentType;
    void* myParentItem;
    void updateItem() { update(); }
};


class KDCanvasEllipse: public QCanvasEllipse
{
public:
    KDCanvasEllipse( KDTimeTableWidget* canvas, void* parentItem, int type );
    int myParentType;
    void* myParentItem;
    void updateItem() { update(); }
};


class KDCanvasRectangle: public QCanvasRectangle
{
public:
    KDCanvasRectangle( KDTimeTableWidget* canvas, void* parentItem, int type );
    int myParentType;
    void* myParentItem;
    void updateItem() { update(); }
};


// Interval-color-rectangle, such as the one used in the freebusy view for the current event
class KDIntervalColorRectangle: public KDCanvasRectangle
{
public:
  KDIntervalColorRectangle( KDGanttView* view );

  void setDateTimes( const QDateTime& start,
                     const QDateTime& end );
  QDateTime start() const { return mStart; }
  QDateTime end() const { return mEnd; }

  void setColor( const QColor& color );
  QColor color() const { return mColor; }

  enum HitTest { Start, Middle, End };
  HitTest hitTest( KDTimeHeaderWidget* timeHeader, const QPoint& pos ) const;

  void layout( KDTimeHeaderWidget* timeHeader, int height );

  static const int RTTI = 0x0c58;
  /*reimp*/ int rtti() const { return RTTI; }

private:
  QColor mColor;
  QDateTime mStart;
  QDateTime mEnd;
};

class KDCanvasToolTip;

class KDGanttCanvasView : public QCanvasView
{
    Q_OBJECT

public:
    KDGanttCanvasView(KDGanttView* sender, QCanvas* canvas = 0, QWidget* parent = 0, const char* name = 0 );
    ~KDGanttCanvasView();
    QString getToolTipText(QPoint p);
    QString getWhatsThisText(QPoint p);
    void drawToPainter ( QPainter * p );
    void resetCutPaste( KDGanttViewItem* );
    void setShowPopupMenu( bool show );
    bool showPopupMenu();
    void cutItem (  KDGanttViewItem* );
    void insertItemAsRoot( KDGanttViewItem* );
    void insertItemAsChild( KDGanttViewItem* , KDGanttViewItem* );
    void insertItemAfter( KDGanttViewItem* , KDGanttViewItem* );
    void setMyVScrollBarMode ( ScrollBarMode );
    ScrollBarMode myVScrollBarMode () const;

    void setConnectorEnabled(int connector, bool on);
    bool isConnectorEnabled(int connector) const;
    void setAllConnectorsEnabled(bool on);

protected:
    friend class KDGanttView;
    friend class KDListView;
    virtual void contentsMousePressEvent ( QMouseEvent * ) ;
    virtual void contentsMouseReleaseEvent ( QMouseEvent * );
    virtual void contentsMouseDoubleClickEvent ( QMouseEvent * );
    virtual void contentsMouseMoveEvent ( QMouseEvent * ) ;
    virtual void viewportPaintEvent ( QPaintEvent * pe );
    void resizeEvent ( QResizeEvent * ) ;
    void set_MouseTracking(bool on);
    int getType(QCanvasItem*);
    KDGanttViewItem* getItem(QCanvasItem*);
    KDGanttViewTaskLink* getLink(QCanvasItem*);
    KDGanttView* mySignalSender;
    KDGanttViewItem* currentItem, *lastClickedItem, *cuttedItem;
    QCanvasRectangle* movingItem;
    KDGanttViewTaskItem* movingGVItem;
    QPoint movingStart;
    QDateTime movingStartDate;
    enum MovingOperation { Moving, ResizingLeft, ResizingRight };
    MovingOperation movingOperation;
    KDGanttViewTaskLink* currentLink;
    KDGanttViewItem::Connector currentConnector;
    KDCanvasWhatsThis* myWhatsThis;
    QMenu* onItem;
    bool _showItemAddPopupMenu;
    int myMyContentsHeight;
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
  void updateMyScrollBars();
  void updateMyScrollBarsLater();
  void resetScrollBars();
  void cutItem();
  void pasteItem( int );
  void pasteItemRoot() { pasteItem(0); }
  void pasteItemChild() { pasteItem(1); }
  void pasteItemAfter() { pasteItem(2); }

  void newRootItem( int );
  void newRootEvent() { newRootItem( KDGanttViewItem::Event ); }
  void newRootTask() { newRootItem( KDGanttViewItem::Task ); }
  void newRootSummary() { newRootItem( KDGanttViewItem::Summary ); }

  void newChildItem( int );
  void newChildEvent() { newChildItem( KDGanttViewItem::Event ); }
  void newChildTask() { newChildItem( KDGanttViewItem::Task ); }
  void newChildSummary() { newChildItem( KDGanttViewItem::Summary ); }

  void newSiblingItem( int );
  void newSiblingEvent() { newSiblingItem( KDGanttViewItem::Event ); }
  void newSiblingTask() { newSiblingItem( KDGanttViewItem::Task ); }
  void newSiblingSummary() { newSiblingItem( KDGanttViewItem::Summary ); }

  void slotScrollTimer();
  void myUpdateScrollBars();

private:
    QTime mButtonDownTime;
    bool currentItemChanged;
    int mScrollBarCheckCounter;
    ScrollBarMode myScrollBarMode;
    QTimer * mScrollbarTimer;
    QTimer * myScrollTimer;
    KDCanvasToolTip* myToolTip;
    QTimer scrollBarTimer;
    QPoint mousePos;
    QAction *mPasteAction;

    bool mConnectorStartEnabled;
    bool mConnectorMiddleEnabled;
    bool mConnectorEndEnabled;
    bool mConnectorMoveEnabled;
    bool mConnectorActualEndEnabled;
    bool mConnectorLeadEnabled;
    bool mConnectorTaskLinkStartEnabled;
    bool mConnectorTaskLinkEndEnabled;

    KDGanttViewItem *mTaskLinkFromItem;
    QCanvasLine *mLinkLine;
  MovingOperation gvItemHitTest( KDGanttViewItem *item, KDTimeHeaderWidget* timeHeader, const QPoint &pos );
};

#if QT_VERSION >= 0x040000
class KDTimeHeaderToolTip :public QObject
{

public:
  KDTimeHeaderToolTip( QWidget *wid, KDTimeHeaderWidget* header ) : QObject( wid ), _wid(wid),_header (header) {

}

protected:

private:
  QWidget* _wid;
  KDTimeHeaderWidget * _header;
};

class KDCanvasToolTip :public QObject
{

public:
  KDCanvasToolTip( QWidget *wid, KDGanttCanvasView* canview ) :  QObject( wid ), _wid(wid),_canview (canview) {

}

protected:

private:
  QWidget* _wid;
  KDGanttCanvasView * _canview;
};
class KDCanvasWhatsThis :public  QObject
{
public:
  KDCanvasWhatsThis( QWidget *wid, KDGanttCanvasView* canview ) :  QObject( wid ), _wid(wid),_canview (canview) { }

protected:
  virtual QString text( const QPoint& p)
  {
    return _canview->getWhatsThisText(p) ;
  }
private:
  QWidget* _wid;
  KDGanttCanvasView * _canview;
};

class KDListViewWhatsThis :public  QObject
{
public:
  KDListViewWhatsThis( QWidget *wid, KDListView* view ) :  QObject( wid ), _wid(wid),_view (view) { }

protected:
  virtual QString text( const QPoint& p)
  {
    return _view->getWhatsThisText(p) ;
  }
private:
  QWidget* _wid;
  KDListView * _view;
};

#else
class KDTimeHeaderToolTip :public QToolTip
{

public:
  KDTimeHeaderToolTip( QWidget *wid, KDTimeHeaderWidget* header ) : QToolTip( wid ), _wid(wid),_header (header) {

};

protected:
  virtual void maybeTip( const QPoint& p)
    {
      static bool ishidden = true;
      if (QToolTip::isGloballyEnabled () ) {
	if (ishidden) {
	  tip( QRect( p.x(),p.y(),5,5), _header->getToolTipText(p));
	}
	else
	  hide();
	ishidden = !ishidden;
      }
  }
private:
  QWidget* _wid;
  KDTimeHeaderWidget * _header;
};

class KDCanvasToolTip :public QToolTip
{

public:
  KDCanvasToolTip( QWidget *wid, KDGanttCanvasView* canview ) : QToolTip( wid ), _wid(wid),_canview (canview) {

};

protected:
  virtual void maybeTip( const QPoint& p)
    {
      static bool ishidden = true;
      if (QToolTip::isGloballyEnabled () ) {
	if (ishidden) {
	  tip( QRect( p.x()-2,p.y()-2,5,5), _canview->getToolTipText(p));
	}
	else
	  hide();
	ishidden = !ishidden;
      }
  }
private:
  QWidget* _wid;
  KDGanttCanvasView * _canview;
};
class KDCanvasWhatsThis :public QWhatsThis
{
public:
  KDCanvasWhatsThis( QWidget *wid, KDGanttCanvasView* canview ) : QWhatsThis( wid ), _wid(wid),_canview (canview) { };

protected:
  virtual QString text( const QPoint& p)
  {
    return _canview->getWhatsThisText(p) ;
  }
private:
  QWidget* _wid;
  KDGanttCanvasView * _canview;
};

class KDListViewWhatsThis :public QWhatsThis
{
public:
  KDListViewWhatsThis( QWidget *wid, KDListView* view ) : QWhatsThis( wid ), _wid(wid),_view (view) { };

protected:
  virtual QString text( const QPoint& p)
  {
    return _view->getWhatsThisText(p) ;
  }
private:
  QWidget* _wid;
  KDListView * _view;
};

#endif



#endif
