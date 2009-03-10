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


#ifndef KDGANTTVIEWITEM_H
#define KDGANTTVIEWITEM_H


#include <QDateTime>
#include <QString>
#include <QColor>
#include <QPixmap>
#include <QFont>


#include "kdgantt_qt3_compat.h"


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
    enum Type { Event = 0, Task, Summary, UnknownType };
    enum Shape { TriangleDown, TriangleUp, Diamond, Square, Circle };
    enum Connector { NoConnector = 0, Start, Middle, End, Move, ActualEnd, Lead, TaskLinkStart, TaskLinkEnd };

protected:
    KDGanttViewItem( Type type, KDGanttView* view,
                     const QString& lvtext = QString(),
                     const QString& name = QString() );
    KDGanttViewItem( Type type, KDGanttViewItem* parent,
                     const QString& lvtext = QString(),
                     const QString& name = QString() );
    KDGanttViewItem( Type type, KDGanttView* view, KDGanttViewItem* after,
                     const QString& lvtext = QString(),
                     const QString& name = QString() );
    KDGanttViewItem( Type type, KDGanttViewItem* parent,
                     KDGanttViewItem* after,
                     const QString& lvtext = QString(),
                     const QString& name = QString() );

    void checkCoord( int * );
    bool isVisibleInGanttView;
    void updateCanvasItems();
    int getCoordY();
    KDCanvasLine * startLine, *endLine,
      * startLineBack, *endLineBack, *actualEnd ;
    KDCanvasPolygonItem* startShape,* midShape, *endShape,
      * startShapeBack,* midShapeBack, *endShapeBack,
      * progressShape, * floatStartShape, * floatEndShape;

    KDGanttView* myGanttView;
    KDCanvasText* mTextCanvas;
    QString textCanvasText;
    QDateTime myStartTime, myEndTime;
    bool isHighlighted, isEditable;
    int myItemSize;
    bool blockUpdating;
    QCanvasText* textcanvas();
    void generateAndInsertName( const QString& name );
    QString mUid;

    void moveTextCanvas(int x, int y);

    int myProgress;
    QDateTime myFloatStartTime;
    QDateTime myFloatEndTime;

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
    void setShowNoInformationBeforeAndAfter( bool show );
    bool showNoInformation();
    bool showNoInformationBeforeAndAfter();
    void setDisplaySubitemsAsGroup( bool show );
    bool displaySubitemsAsGroup() const;
    void setPriority( int prio );
    int priority();
    virtual void setStartTime( const QDateTime& start );
    QDateTime startTime() const;
    virtual void setEndTime( const QDateTime& end );
    QDateTime endTime() const;

    void setItemText( const QString& text );
    QString itemText() const;
    void setListViewText( const QString& text, int column = 0 );
    void setListViewText( int column, const QString& text );
    QString listViewText( int column = 0 ) const;
    void setFont( const QFont& font );
    QFont font() const;
    void setTooltipText( const QString& text );
    QString tooltipText() const;
    void setWhatsThisText( const QString& text );
    QString whatsThisText() const;
    void setPixmap( int column, const QPixmap& pixmap );
    void setPixmap( const QPixmap& pixmap );
    const QPixmap* pixmap( int column = 0 ) const;

    void setHighlight( bool );
    bool highlight() const;

    bool subitemIsCalendar() const;

    void setShapes( Shape start, Shape middle, Shape end );
    void shapes( Shape& start, Shape& middle, Shape& end ) const;
    void setDefaultColor( const QColor& );
    QColor defaultColor() const;
    void setColors( const QColor& start, const QColor& middle,
                    const QColor& end );
    void colors( QColor& start, QColor& middle, QColor& end ) const;
    void setDefaultHighlightColor( const QColor& );
    QColor defaultHighlightColor() const;
    void setHighlightColors( const QColor& start, const QColor& middle,
                             const QColor& end );
    void highlightColors( QColor& start, QColor& middle, QColor& end ) const;
    void setTextColor( const QColor& color );
    QColor textColor() const;

    void setProgress(int percent);
    void setFloatStartTime(const QDateTime &start);
    void setFloatEndTime(const QDateTime &end);

    KDGanttViewItem* firstChild() const;
    KDGanttViewItem* nextSibling() const;
    KDGanttViewItem* parent() const;
    KDGanttViewItem* itemAbove();
    KDGanttViewItem* itemBelow( bool includeDisabled = true );
    KDGanttViewItem* getChildByName( const QString& name );
    KDGanttViewItem* getChildByUid( const QString& name );

    void createNode( QDomDocument& doc,
                     QDomElement& parentElement );
    static KDGanttViewItem* createFromDomElement( KDGanttView* view,
                                                  QDomElement& element );
    static KDGanttViewItem* createFromDomElement( KDGanttView* view,
                                                  KDGanttViewItem* previous,
                                                  QDomElement& element );
    static KDGanttViewItem* createFromDomElement( KDGanttViewItem* parent,
                                                  QDomElement& element );
    static KDGanttViewItem* createFromDomElement( KDGanttViewItem* parent,
                                                  KDGanttViewItem* previous,
                                                  QDomElement& element );
    QString name() const;
    static KDGanttViewItem* find( const QString& name );
    virtual QString uid() const;
    virtual void setUid( const QString& text );
    unsigned int getChildTimeForTimespan( const QDateTime& start, const QDateTime& end );
    unsigned int getAllSubChildTimeForTimespan( const QDateTime& start, const QDateTime& end );
    virtual unsigned int getTimeForTimespan( const QDateTime& start, const QDateTime& end );
    QPtrList <KDGanttViewItem> getChildListForTimespan( const QDateTime& start, const QDateTime& end );
    QDateTime myChildStartTime();
    QDateTime myChildEndTime();
    virtual QString typeString() const;
    virtual Connector getConnector( QPoint p );
    virtual bool moveConnector( Connector, QPoint p );
    void updateItemsOnCanvas( bool forceUpdate = false );

protected:
    int mCurrentCoord_Y;
    int mCurrentConnectorCoordX;
    int mCurrentConnectorDiffX;
    virtual void userReadFromElement( QDomElement& element );
    virtual void userWriteToElement( QDomDocument& doc,
                                     QDomElement& parentElement );

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

    virtual KDGanttViewItem::Connector getConnector( QPoint p, bool linkMode );

    static QString shapeToString( Shape shape );
    static Shape stringToShape( const QString& string );
    static QString typeToString( Type type );
    static Type stringToType(  const QString& type );

    Type myType;
    void setAllSubitemsExpanded ( bool );
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
    void loadFromDomElement( QDomElement& element );

    //QFont myFont;
    QString myToolTipText,myWhatsThisText;
    void paintBranches ( QPainter * p, const QColorGroup & cg, int w, int y, int h );
    bool _displaySubitemsAsGroup;
    bool _showNoInformation;
    bool _showNoInformationBeforeAndAfter;
    bool _enabled;
    bool _callListViewOnSetOpen;
    Shape myStartShape,myMiddleShape,myEndShape;
    QColor myStartColor,myMiddleColor,myEndColor;
    QColor myStartColorHL,myMiddleColorHL,myEndColorHL;
    QColor myDefaultColor,myDefaultColorHL;
    QColor myTextColor;
    bool colorDefined,colorHLDefined;
    QPoint getTaskLinkStartCoord(QPoint);
    QPoint getTaskLinkEndCoord();
    QPoint middleLeft();
    QPoint middleRight();
    void moveTextCanvas();
    void setTextOffset(QPoint p);
    bool isMyTextCanvas(QCanvasItem *tc);
    QPoint myTextOffset;
    QString _name;
    bool shapeDefined;
    int _priority;
    static QDict<KDGanttViewItem> sItemDict;

    bool _isMoveable;
    bool _isResizeable;
};


#endif
