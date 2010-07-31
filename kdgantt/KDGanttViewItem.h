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


#ifndef KDGANTTVIEWITEM_H
#define KDGANTTVIEWITEM_H

#include <tqdatetime.h>
#include <tqstring.h>
#include <tqcolor.h>
#include <tqpixmap.h>
#include <tqfont.h>
#include <tqlistview.h>
#include <tqcanvas.h>
#include <tqdom.h>
#include <tqdict.h>

class KDGanttView;
class KDTimeTableWidget;
class KDTimeHeaderWidget;
class KDGanttViewTaskLink;
class KDCanvasLine;
class KDCanvasText;
class KDCanvasPolygonItem;
class KDGanttViewTaskLinkGroup;

class KDGanttViewItem : public QListViewItem
{
public:
    enum Type { Event, Task, Summary };
    enum Shape { TriangleDown, TriangleUp, Diamond, Square, Circle };

protected:
    KDGanttViewItem( Type type, KDGanttView* view,
                     const TQString& lvtext = TQString::null,
                     const TQString& name = TQString::null );
    KDGanttViewItem( Type type, KDGanttViewItem* parent,
                     const TQString& lvtext = TQString::null,
                     const TQString& name = TQString::null );
    KDGanttViewItem( Type type, KDGanttView* view, KDGanttViewItem* after,
                     const TQString& lvtext = TQString::null,
                     const TQString& name = TQString::null );
    KDGanttViewItem( Type type, KDGanttViewItem* parent,
                     KDGanttViewItem* after,
                     const TQString& lvtext = TQString::null,
                     const TQString& name = TQString::null );

  //bool _isCalendar;
    bool isVisibleInGanttView;
    void updateCanvasItems();
    int getCoordY();
    TQDateTime myChildStartTime();
    TQDateTime myChildEndTime();
    void generateAndInsertName( const TQString& name );
    KDCanvasLine * startLine, *endLine,
      * startLineBack, *endLineBack, *actualEnd ;
    KDCanvasPolygonItem* startShape,* midShape, *endShape, *progressShape,
      * startShapeBack,* midShapeBack, *endShapeBack,
      * floatStartShape, * floatEndShape;
    KDGanttView* myGanttView;
    KDCanvasText* textCanvas;
    TQString textCanvasText;
    TQDateTime myStartTime, myEndTime;
    bool isHighlighted, isEditable;
    int myItemSize;
    bool blockUpdating;

    void moveTextCanvas(int x, int y);
    int myProgress;
    TQDateTime myFloatStartTime;
    TQDateTime myFloatEndTime;

public:
    virtual ~KDGanttViewItem();

    Type type() const;
    void setEnabled( bool on );
    bool enabled () const;
    virtual void setOpen( bool o );
    void setItemVisible( bool on );
    bool itemVisible () const;
    void setEditable( bool editable );
    bool editable() const;
    void setShowNoInformation( bool show );
    bool showNoInformation();
    void setDisplaySubitemsAsGroup( bool show );
    bool displaySubitemsAsGroup() const;
    void setPriority( int prio );
    int priority();
    virtual void setStartTime( const TQDateTime& start );
    TQDateTime startTime() const;
    virtual void setEndTime( const TQDateTime& end );
    TQDateTime endTime() const;

    void setText( const TQString& text );
    TQString text() const;
    void setListViewText( const TQString& text, int column = 0 );
    void setListViewText( int column, const TQString& text );
    TQString listViewText( int column = 0 ) const;
    void setFont( const TQFont& font );
    TQFont font() const;
    void setTooltipText( const TQString& text );
    TQString tooltipText() const;
    void setWhatsThisText( const TQString& text );
    TQString whatsThisText() const;
    void setPixmap( int column, const TQPixmap& pixmap );
    void setPixmap( const TQPixmap& pixmap );
    const TQPixmap* pixmap( int column = 0 ) const;

    void setHighlight( bool );
    bool highlight() const;

    bool subitemIsCalendar() const;
  //void setIsCalendar( bool );
  //bool isCalendar( ) const;

    void setShapes( Shape start, Shape middle, Shape end );
    void shapes( Shape& start, Shape& middle, Shape& end ) const;
    void setDefaultColor( const TQColor& );
    TQColor defaultColor() const;
    void setColors( const TQColor& start, const TQColor& middle,
                    const TQColor& end );
    void colors( TQColor& start, TQColor& middle, TQColor& end ) const;
    void setDefaultHighlightColor( const TQColor& );
    TQColor defaultHighlightColor() const;
    void setHighlightColors( const TQColor& start, const TQColor& middle,
                             const TQColor& end );
    void highlightColors( TQColor& start, TQColor& middle, TQColor& end ) const;
    void setTextColor( const TQColor& color );
    TQColor textColor() const;

    void setProgress(int percent);
    void setFloatStartTime(const TQDateTime &start);
    void setFloatEndTime(const TQDateTime &end);

    KDGanttViewItem* firstChild() const;
    KDGanttViewItem* nextSibling() const;
    KDGanttViewItem* parent() const;
    KDGanttViewItem* itemAbove();
    KDGanttViewItem* itemBelow( bool includeDisabled = true );
    KDGanttViewItem* getChildByName( const TQString& name );
    TQString name() const;
    static KDGanttViewItem* find( const TQString& name );

    void createNode( TQDomDocument& doc,
                     TQDomElement& parentElement );
    static KDGanttViewItem* createFromDomElement( KDGanttView* view,
                                                  TQDomElement& element );
    static KDGanttViewItem* createFromDomElement( KDGanttView* view,
                                                  KDGanttViewItem* previous,
                                                  TQDomElement& element );
    static KDGanttViewItem* createFromDomElement( KDGanttViewItem* parent,
                                                  TQDomElement& element );
    static KDGanttViewItem* createFromDomElement( KDGanttViewItem* parent,
                                                  KDGanttViewItem* previous,
                                                  TQDomElement& element );

    void setMoveable( bool m );
    bool isMoveable() const;
    void setResizeable( bool r );
    bool isResizeable() const;

private:
    friend class KDGanttView;
    friend class KDTimeTableWidget;
    friend class KDTimeHeaderWidget;
    friend class KDListView;
    friend class KDGanttViewTaskLink;
    friend class KDGanttViewTaskLinkGroup;
    friend class KDGanttCanvasView;
    friend class KDGanttViewItemDrag;
    friend class itemAttributeDialog;

    static TQString shapeToString( Shape shape );
    static Shape stringToShape( const TQString& string );
    static TQString typeToString( Type type );

    Type myType;
    void initColorAndShapes(Type t);
    void resetSubitemVisibility();
    virtual void showItem( bool show = true, int coordY = 0 );
    virtual void initItem();
    int computeHeight();
    void showSubItems();
    void showSubitemTree( int );
    void hideSubtree();
    void setCallListViewOnSetOpen( bool call );
    bool showNoCross();
    void createShape(KDCanvasPolygonItem* &,KDCanvasPolygonItem* &, Shape);
    void loadFromDomElement( TQDomElement& element );

    //TQFont myFont;
    TQString myToolTipText,myWhatsThisText;
    void paintBranches ( TQPainter * p, const TQColorGroup & cg, int w, int y, int h );
    bool _displaySubitemsAsGroup;
    bool _showNoInformation;
    bool _enabled;
    bool _callListViewOnSetOpen;
    Shape myStartShape,myMiddleShape,myEndShape;
    TQColor myStartColor,myMiddleColor,myEndColor;
    TQColor myStartColorHL,myMiddleColorHL,myEndColorHL;
    TQColor myDefaultColor,myDefaultColorHL;
    TQColor myTextColor;
    bool colorDefined,colorHLDefined;
    TQPoint getTaskLinkStartCoord(TQPoint);
    TQPoint getTaskLinkEndCoord();
    TQPoint middleLeft();
    TQPoint middleRight();
    void moveTextCanvas();
    void setTextOffset(TQPoint p);
    bool isMyTextCanvas(TQCanvasItem *tc);
    TQPoint myTextOffset;
    TQString _name;
    bool shapeDefined;
    int _priority;
    static TQDict<KDGanttViewItem> sItemDict;

    bool _isMoveable;
    bool _isResizeable;
};


#endif
