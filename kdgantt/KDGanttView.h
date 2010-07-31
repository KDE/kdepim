/* -*- Mode: C++ -*-
   $Id$
   KDChart - a multi-platform charting engine
*/

/****************************************************************************
 ** Copyright (C)  2001-2004 Klarälvdalens Datakonsult AB.  All rights reserved.
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

#ifndef KDGANTTVIEW_H
#define KDGANTTVIEW_H

#include <tqptrlist.h>
#include <tqwidget.h>
#include <tqlistview.h>
#include <tqsplitter.h>
#include <tqlayout.h>
#include <tqfont.h>
#include <tqdom.h>
#include <tqvbox.h>


#include "KDGanttViewItem.h"
#include "KDGanttViewTaskLinkGroup.h"
#include "KDGanttMinimizeSplitter.h"
#include "KDGanttViewItemDrag.h"

class KDIntervalColorRectangle;
class KDGanttViewTaskLink;
class QPrinter;
class QIODevice;
class itemAttributeDialog;
class KDListView;
class KDGanttViewItem;
class KDGanttViewEventItem;
class KDGanttViewTaskItem;
class KDGanttViewSummaryItem;
class KDTimeTableWidget;
class KDTimeHeaderWidget;
class KDLegendWidget;
class KDGanttCanvasView;
class KDGanttViewTaskLink;
class KDGanttMinimizeSplitter;

class KDGanttView : public KDGanttMinimizeSplitter
{
    Q_OBJECT

    Q_PROPERTY( bool showLegend READ showLegend WRITE setShowLegend )
    Q_PROPERTY( bool showListView READ showListView WRITE setShowListView )
    Q_PROPERTY( bool showTaskLinks READ showTaskLinks WRITE setShowTaskLinks )
    Q_PROPERTY( bool editorEnabled READ editorEnabled WRITE setEditorEnabled )
    Q_PROPERTY( TQDateTime horizonStart READ horizonStart WRITE setHorizonStart )
    Q_PROPERTY( TQDateTime horizonEnd READ horizonEnd WRITE setHorizonEnd )
    Q_PROPERTY( Scale scale READ scale WRITE setScale )
    Q_PROPERTY( YearFormat yearFormat READ yearFormat WRITE setYearFormat )
    Q_PROPERTY( HourFormat hourFormat READ hourFormat WRITE setHourFormat )
    Q_PROPERTY( bool showMinorTicks READ showMinorTicks WRITE setShowMinorTicks )
    Q_PROPERTY( bool showMajorTicks READ showMajorTicks WRITE setShowMajorTicks )
    Q_PROPERTY( bool editable READ editable WRITE setEditable )
    Q_PROPERTY( TQColor textColor READ textColor WRITE setTextColor )
    Q_PROPERTY( int majorScaleCount READ majorScaleCount WRITE setMajorScaleCount )
    Q_PROPERTY( int minorScaleCount READ minorScaleCount WRITE setMinorScaleCount )
    Q_PROPERTY( int autoScaleMinorTickCount READ autoScaleMinorTickCount WRITE setAutoScaleMinorTickCount )
    Q_PROPERTY( Scale maximumScale READ maximumScale WRITE setMaximumScale )
    Q_PROPERTY( Scale minimumScale READ minimumScale WRITE setMinimumScale )
    Q_PROPERTY( int minimumColumnWidth READ minimumColumnWidth WRITE setMinimumColumnWidth )
    Q_PROPERTY( int ganttMaximumWidth READ ganttMaximumWidth WRITE setGanttMaximumWidth )
    Q_PROPERTY( TQColor weekendBackgroundColor READ weekendBackgroundColor WRITE setWeekendBackgroundColor )
    Q_PROPERTY( TQColor ganttViewBackgroundColor READ gvBackgroundColor WRITE setGvBackgroundColor )
    Q_PROPERTY( TQColor listViewBackgroundColor READ lvBackgroundColor WRITE setLvBackgroundColor )
    Q_PROPERTY( TQColor timeHeaderBackgroundColor READ timeHeaderBackgroundColor WRITE setTimeHeaderBackgroundColor )
    Q_PROPERTY( TQColor legendHeaderBackgroundColor READ legendHeaderBackgroundColor WRITE setLegendHeaderBackgroundColor )
    Q_PROPERTY( double zoomFactor READ zoomFactor )
    Q_PROPERTY( bool showHeaderPopupMenu READ showHeaderPopupMenu WRITE setShowHeaderPopupMenu )
    Q_PROPERTY( bool showTimeTablePopupMenu READ showTimeTablePopupMenu WRITE setShowTimeTablePopupMenu )
    Q_PROPERTY( bool headerVisible READ headerVisible WRITE setHeaderVisible )
    Q_PROPERTY( bool showLegendButton READ showLegendButton	WRITE setShowLegendButton )
    Q_PROPERTY( bool legendIsDockwindow READ legendIsDockwindow WRITE setLegendIsDockwindow )
    Q_PROPERTY( bool displayEmptyTasksAsLine READ displayEmptyTasksAsLine WRITE setDisplayEmptyTasksAsLine )
    Q_PROPERTY( TQBrush noInformationBrush READ noInformationBrush WRITE setNoInformationBrush )
    Q_PROPERTY( bool dragEnabled READ dragEnabled WRITE setDragEnabled )
    Q_PROPERTY( bool dropEnabled READ dropEnabled WRITE setDropEnabled )
    Q_PROPERTY( bool calendarMode READ calendarMode WRITE setCalendarMode )

    Q_ENUMS( Scale )
    Q_ENUMS( YearFormat )
    Q_ENUMS( HourFormat )

public:
    enum Scale { Minute, Hour, Day, Week, Month, Auto };
    enum YearFormat { FourDigit, TwoDigit, TwoDigitApostrophe, NoDate };
    enum HourFormat { Hour_24, Hour_12, Hour_24_FourDigit };
    enum RepaintMode { No, Medium, Always };

    KDGanttView( TQWidget* parent = 0, const char* name = 0 );
    ~KDGanttView();

    virtual void show();
    virtual bool close ( bool alsoDelete );
    void setRepaintMode( RepaintMode mode );
    void setUpdateEnabled( bool enable);
    bool getUpdateEnabled( )const;

    void setGanttMaximumWidth( int w );
    int ganttMaximumWidth() const;
    void setShowLegend( bool show );
    bool showLegend() const;
    void setLegendIsDockwindow( bool dock );
    bool legendIsDockwindow( ) const;
    TQDockWindow* legendDockwindow( ) const;
    void setShowListView( bool show );
    bool showListView() const;
    void setEditorEnabled( bool enable );
    bool editorEnabled() const;
    void setListViewWidth( int );
    int listViewWidth();
    void setEditable( bool editable );
    bool editable() const;
    void setCalendarMode( bool mode );
    bool calendarMode() const;
    void setDisplaySubitemsAsGroup( bool show );
    bool displaySubitemsAsGroup() const;
    void setDisplayEmptyTasksAsLine( bool show );
    bool displayEmptyTasksAsLine() const;

    void setHorBackgroundLines( int count = 2,
				TQBrush brush =
				TQBrush( TQColor ( 200,200,200 ),
					Qt::Dense6Pattern ));
    int horBackgroundLines( TQBrush& brush );
    bool saveProject( TQIODevice* );
    bool loadProject( TQIODevice* );
    void print( TQPrinter* printer = 0 ,
		bool printListView = true, bool printTimeLine = true,
		bool printLegend = false );
    TQSize drawContents( TQPainter* p = 0,
		      bool drawListView = true, bool drawTimeLine = true,
		      bool drawLegend = false );
    void setZoomFactor( double factor, bool absolute );
    double zoomFactor() const;
    void zoomToFit();
    void ensureVisible( KDGanttViewItem* );
    void center( KDGanttViewItem* );
    void centerTimeline( const TQDateTime& center );
    void centerTimelineAfterShow( const TQDateTime& center );
    void setTimelineToStart();
    void setTimelineToEnd();
    void addTicksLeft( int num = 1 );
    void addTicksRight( int num = 1 );
    void setShowTaskLinks( bool show );
    bool showTaskLinks() const;

    void setFont(const TQFont& f);
    void setShowHeaderPopupMenu( bool show = true,
                                 bool showZoom = true,
                                 bool showScale = true,
                                 bool showTime = true,
                                 bool showYear = true,
                                 bool showGrid = true,
                                 bool showPrint = false);
    bool showHeaderPopupMenu() const;
    void setShowTimeTablePopupMenu( bool );
    bool showTimeTablePopupMenu() const;
    void setShapes( KDGanttViewItem::Type type,
                    KDGanttViewItem::Shape start,
                    KDGanttViewItem::Shape middle,
                    KDGanttViewItem::Shape end,
		    bool overwriteExisting = true  );
    bool shapes( KDGanttViewItem::Type type,
                 KDGanttViewItem::Shape& start,
                 KDGanttViewItem::Shape& middle,
                 KDGanttViewItem::Shape& end ) const;
    void setColors( KDGanttViewItem::Type type,
                    const TQColor& start, const TQColor& middle,
                    const TQColor& end,
		    bool overwriteExisting = true );
    bool colors( KDGanttViewItem::Type type,
                 TQColor& start, TQColor& middle, TQColor& end ) const;
    void setDefaultColor( KDGanttViewItem::Type type,
			  const TQColor&,
			  bool overwriteExisting = true );
    TQColor defaultColor( KDGanttViewItem::Type type ) const;
    void setHighlightColors( KDGanttViewItem::Type type,
                             const TQColor& start, const TQColor& middle,
                             const TQColor& end,
			     bool overwriteExisting = true );
    bool highlightColors( KDGanttViewItem::Type type,
                          TQColor& start, TQColor& middle, TQColor& end ) const;
    void setDefaultHighlightColor( KDGanttViewItem::Type type,
				   const TQColor&,
				   bool overwriteExisting = true );
    TQColor defaultHighlightColor( KDGanttViewItem::Type type ) const;
    void setTextColor( const TQColor& color );
    TQColor textColor() const;

    void setNoInformationBrush( const TQBrush& brush );
    TQBrush noInformationBrush() const;

    // Link-related stuff
    TQPtrList<KDGanttViewTaskLink> taskLinks() const;
    TQPtrList<KDGanttViewTaskLinkGroup> taskLinkGroups() const;

    // Legend-related stuff
    void addLegendItem( KDGanttViewItem::Shape shape, const TQColor& shapeColor, const TQString& text );
    void clearLegend();
    // Header-related stuff
    void setHorizonStart( const TQDateTime& start );
    TQDateTime horizonStart() const;
    void setHorizonEnd( const TQDateTime& start );
    TQDateTime horizonEnd() const;
    void setScale( Scale );
    Scale scale() const;
    void setMaximumScale( Scale );
    Scale maximumScale() const;
    void setMinimumScale( Scale );
    Scale minimumScale() const;
    void setAutoScaleMinorTickCount( int count );
    int autoScaleMinorTickCount() const;
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
    void setShowMajorTicks( bool );
    bool showMajorTicks() const;
    void setShowMinorTicks( bool );
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


    void setPaletteBackgroundColor(const TQColor& col);
    void setGvBackgroundColor ( const TQColor & );
    void setLvBackgroundColor ( const TQColor & );
    void setTimeHeaderBackgroundColor ( const TQColor & );
    void setLegendHeaderBackgroundColor ( const TQColor & );
    TQColor gvBackgroundColor () const ;
    TQColor lvBackgroundColor () const ;
    TQColor timeHeaderBackgroundColor () const ;
    TQColor legendHeaderBackgroundColor () const ;
    void addUserdefinedLegendHeaderWidget( TQWidget * w );

    void setWeekendDays( int start, int end );
    void weekendDays( int& start, int& end ) const;

    static TQPixmap getPixmap( KDGanttViewItem::Shape shape, const TQColor& shapeColor,const TQColor& backgroundColor, int itemSize);

    void setHeaderVisible( bool );
    bool headerVisible() const;

    void setShowLegendButton( bool show );
    bool showLegendButton() const;

    // Pass-through methods from QListView
    virtual int addColumn( const TQString& label, int width = -1 );
    virtual int addColumn( const TQIconSet& iconset, const TQString& label,
                           int width = -1 );
    virtual void removeColumn( int index );
    KDGanttViewItem* selectedItem() const;
    void setSelected( KDGanttViewItem*, bool );
    KDGanttViewItem* firstChild() const;
    KDGanttViewItem* lastItem() const;
    int childCount() const;
    void clear();

    void setDragEnabled( bool b );
    void setDropEnabled( bool b );
    void setDragDropEnabled( bool b );
    bool dragEnabled() const;
    bool dropEnabled() const;
    bool isDragEnabled() const;
    bool isDropEnabled() const;

    virtual bool lvDropEvent ( TQDropEvent *e, KDGanttViewItem*, KDGanttViewItem*);
    virtual void lvStartDrag (KDGanttViewItem*);
    virtual bool lvDragMoveEvent (TQDragMoveEvent * e,KDGanttViewItem*, KDGanttViewItem*);
    virtual void lvDragEnterEvent (TQDragEnterEvent * e );
    virtual TQSize sizeHint() const;
    KDGanttViewItem* getItemByName( const TQString& name ) const;
    TQDateTime getDateTimeForCoordX(int coordX, bool global = true ) const;
    KDGanttViewItem* getItemByListViewPos( const TQPoint& pos ) const;
    KDGanttViewItem* getItemByGanttViewPos( const TQPoint& pos ) const;
    KDGanttViewItem* getItemAt( const TQPoint& pos , bool global = true ) const;

    // setting the vertical scrollbars of the listview and the timetable
    // default values: always off for the listview, always on for the timetable
    void setLvVScrollBarMode( TQScrollView::ScrollBarMode );
    void setGvVScrollBarMode( TQScrollView::ScrollBarMode );

    void setLinkItemsEnabled(bool on);
    bool isLinkItemsEnabled() const;

    KDTimeTableWidget * timeTableWidget() { return myTimeTable; }
    KDTimeHeaderWidget * timeHeaderWidget() { return myTimeHeader; }

    void setFixedHorizon( bool f ) { mFixedHorizon = f; }

public slots:
    void editItem( KDGanttViewItem* );
    void zoomToSelection( const TQDateTime& start,  const TQDateTime&  end);

signals:
    void timeIntervallSelected( const TQDateTime& start,  const TQDateTime&  end);
    void timeIntervalSelected( const TQDateTime& start,  const TQDateTime&  end);
    void rescaling( KDGanttView::Scale );
    void intervalColorRectangleMoved( const TQDateTime& start, const TQDateTime& end );

    // the following signals are emitted if an item is clicked in the
    // listview (inclusive) or in the ganttview
    void itemLeftClicked( KDGanttViewItem* );
    void itemMidClicked( KDGanttViewItem* );
    void itemRightClicked( KDGanttViewItem* );
    void itemDoubleClicked( KDGanttViewItem* );

    // The following signal is emitted when two items shall be linked
    void linkItems( KDGanttViewItem* from, KDGanttViewItem* to, int linkType );

    // the following signals are emitted if an item is clicked in the
    // listview (exlusive) or in the ganttview
    // gv... means item in ganttview clicked

    void gvCurrentChanged( KDGanttViewItem* );
    void gvItemLeftClicked( KDGanttViewItem* );
    void gvItemMidClicked( KDGanttViewItem* );
    void gvItemRightClicked( KDGanttViewItem* );
  // the point is the global position!!
    void gvMouseButtonClicked ( int button, KDGanttViewItem* item, const TQPoint & pos);
    void gvItemDoubleClicked( KDGanttViewItem* );
    // the point is the global position!!
    void gvContextMenuRequested ( KDGanttViewItem * item, const TQPoint & pos );
    void gvItemMoved( KDGanttViewItem* );

    // lv... means item in listview clicked
    void lvCurrentChanged( KDGanttViewItem* );
    void lvItemRenamed( KDGanttViewItem* , int col, const TQString & text  );
    void lvMouseButtonPressed(  int button, KDGanttViewItem * item, const TQPoint & pos, int c );
    void lvItemLeftClicked( KDGanttViewItem* );
    void lvItemMidClicked( KDGanttViewItem* );
    void lvItemRightClicked( KDGanttViewItem* );
    void lvContextMenuRequested ( KDGanttViewItem * item, const TQPoint & pos, int col );
    void lvMouseButtonClicked ( int button, KDGanttViewItem* item, const TQPoint & pos, int c );
    void lvItemDoubleClicked( KDGanttViewItem* );
    void lvSelectionChanged( KDGanttViewItem* );

    void itemConfigured( KDGanttViewItem* );

    void taskLinkLeftClicked( KDGanttViewTaskLink* );
    void taskLinkMidClicked( KDGanttViewTaskLink* );
    void taskLinkRightClicked( KDGanttViewTaskLink* );
    void taskLinkDoubleClicked( KDGanttViewTaskLink* );

    void dateTimeDoubleClicked (const TQDateTime& );

    void dropped ( TQDropEvent * e, KDGanttViewItem* droppedItem, KDGanttViewItem* itemBelowMouse);
private slots:
    void forceRepaint( int val = 0 );
    void slotSelectionChanged( TQListViewItem* item );
    void slotCurrentChanged ( TQListViewItem * item );
    void slotItemRenamed ( TQListViewItem * item, int col, const TQString & text  );
    void slotMouseButtonPressed ( int button, TQListViewItem * item, const TQPoint & pos, int c );
    void slotmouseButtonClicked ( int button, TQListViewItem * item, const TQPoint & pos, int c );
    void slotcontextMenuRequested ( TQListViewItem * item, const TQPoint & pos, int col );
    void slotHeaderSizeChanged();
    void addTickRight();
    void addTickLeft();
    void enableAdding( int );
    void slot_lvDropped(TQDropEvent* e, KDGanttViewItem* droppedItem, KDGanttViewItem* itemBelowMouse );
private:
    struct legendItem {
      KDGanttViewItem::Shape shape;
      TQColor color;
      TQString text;
    };
    bool loadXML( const TQDomDocument& doc );
    TQDomDocument saveXML( bool withPI = true ) const;

    void emptySpaceDoubleClicked( TQMouseEvent* e );

    static TQString scaleToString( Scale scale );
    static TQString yearFormatToString( YearFormat format );
    static TQString hourFormatToString( HourFormat format );
    static Scale stringToScale( const TQString& string );
    static YearFormat stringToYearFormat( const TQString& string );
    static HourFormat stringToHourFormat( const TQString& string );

    // PENDING(lutz) Review these
    friend class KDGanttCanvasView;
    friend class KDGanttViewEventItem;
    friend class KDGanttViewItem;
    friend class KDGanttViewTaskItem;
    friend class KDGanttViewSummaryItem;
    friend class KDGanttViewTaskLink;
    friend class KDGanttViewCalendarItem;
    friend class KDTimeTableWidget;
    friend class KDTimeHeaderWidget;
    friend class KDListView;
    friend class KDGanttViewTaskLinkGroup;
    friend class KDLegendWidget;

    KDListView * myListView;
    KDGanttCanvasView *myCanvasView;
    KDTimeHeaderWidget * myTimeHeader;
    KDTimeTableWidget * myTimeTable;
    KDLegendWidget * myLegend;
    itemAttributeDialog* myItemAttributeDialog;
    TQVBox * leftWidget,  * rightWidget;
    TQHBox * spacerLeft;
    TQScrollView* myTimeHeaderScroll;
    TQHBox* myTimeHeaderContainer ;
    TQWidget* timeHeaderSpacerWidget;
    TQWidget *spacerRight;

    bool listViewIsVisible;
    bool chartIsEditable;
    bool editorIsEnabled;
    bool _displaySubitemsAsGroup;
    bool _displayEmptyTasksAsLine;
    bool _showLegendButton;
    bool _showHeader;
    bool _enableAdding;
    bool fCenterTimeLineAfterShow;
    bool fDragEnabled;
    bool fDropEnabled;
    bool closingBlocked;
    TQDateTime dtCenterTimeLineAfterShow;
    KDGanttViewItem::Shape myDefaultShape [9];
    TQColor myColor[9],myColorHL[9];
    bool undefinedShape[3],undefinedColor[3],undefinedColorHL[3];
    TQColor myTextColor;
    TQColor myDefaultColor[3],myDefaultColorHL[3];
    TQPtrList<KDGanttViewTaskLinkGroup> myTaskLinkGroupList;
    TQPtrList<legendItem> *myLegendItems;
    void addTaskLinkGroup(KDGanttViewTaskLinkGroup*);
    void removeTaskLinkGroup(KDGanttViewTaskLinkGroup*);
    int getIndex( KDGanttViewItem::Type ) const;
    void notifyEditdialog( KDGanttViewItem * );
    void initDefaults();
    KDGanttViewItem* myCurrentItem;
    KDGanttMinimizeSplitter *mySplitter;
    bool  mFixedHorizon;
protected:
  virtual TQDragObject * dragObject ();
  virtual void startDrag ();
};



#endif
