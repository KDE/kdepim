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
#include <kapplication.h>
#include "KDGanttViewTaskItem.h"
#include "KDGanttViewSummaryItem.h"
#include "KDGanttViewEventItem.h"
#include "itemAttributeDialog.h"

#include "KDXMLTools.h"


/*!
  \class KDGanttViewItem KDGanttViewItem.h
  This class represents an item in a Gantt chart.

  This class is an abstract base class, it cannot be instantiated
  directly. Instead, you should create items of one of the
  subclasses. This class provides methods common to all Gantt items.

  The initialization of the shapes/colors of the item is as follows:

  Shapes:
  When a new item is created, the shapes are set to the default values
  for items of the type of this item, defined in the KDGanttView class with
  void setShapes( KDGanttViewItem::Type type,
  KDGanttViewItem::Shape start,
  KDGanttViewItem::Shape middle,
  KDGanttViewItem::Shape end );
  If there is no default value defined for this type,
  the shapes are set as follows:
  For TaskViewItems all three shapes are set to Square.
  For SummaryViewItems all three shapes are set to TriangleDown.
  For EventViewItems all three shapes are set to Diamond.

  Colors:
  When a new item is created, the colors are set to the default values
  for items of the type of this item, defined in the KDGanttView class with
  void setColors( KDGanttViewItem::Type type,
  const QColor& start,
  const QColor& middle,
  const QColor& end );
  If there is no default value defined for this type,
  the colors of the shapes are set to the default color for items of this type,
  defined in the KDGanttView class with:
  void setDefaultColor( KDGanttViewItem::Type type, const QColor& );
  The initial default color in the KDGanttView class is set to
  blue for KDGanttViewItem::Event,
  green for KDGanttViewItem::Task,
  cyan for KDGanttViewItem::Summary.

  Highlight Colors:
  When a new item is created, the highlight colors are set to the default
  values for items of the type of this item,
  defined in the KDGanttView class with:
  void setHighlightColors( KDGanttViewItem::Type type,
  const QColor& start,
  const QColor& middle,
  const QColor& end );
  If there is no default value defined for this type,
  the highlight colors of the shapes are set to the default color for
  items of this type,defined in the KDGanttView class with:
  void setDefaultHighlightColor( KDGanttViewItem::Type type, const QColor& );
  The initial default highlight color in the KDGanttView class is set to red,
  for all types.

  Start/End time:
  When a new item is created,the start time and the end time is set automatically.
  The time , which is currently displayed in the middle of the Gantt View, is set
  as start/end time. At startup of a newly created gantt view,
  this is the current time.

  The priority:
  The priority is set with \a setPriority().
  The priority determines which item is painted over other items.
  The item with the highest priority is painted on top over the others.
  The priority for an item may change between 1 and 199.
  A priority less than 100 means, that the item is painted in the
  Gantt chart below the grid.
  For Task items, the default priority is 50, for all other items it is 150.
  This feature makes only sense for an item, which is a child of another item,
  which \a displaySubitemsAsGroup() property is set to true.

  The display mode:
  The display mode is set with \a setDisplaySubitemsAsGroup().
  In the normal view mode (set with setDisplaySubitemsAsGroup( false ); )
  an item is displayed in the same manner, when its child items are shown or not.
  In the other mode, (set with setDisplaySubitemsAsGroup( true ); ),
  a so called calendar mode, the item is displaed as follows:
  If the item has no childs, the item is displayed as usual.
  If the item is opened (i.e. its childs are displayed), start/end time of
  this item is computed automatically according to the earliest start time/
  latest end time of its childs. The item and its childs are displayed as usual.
  if the item is closed (i.e. its childs are hidden in the left listview),
  the item itself is hidden and its childs are displayed on the timeline of
  this item instead. To manage the painting of overlapping childs, call
  \a setPriority() for the childs.


  Blocking of user interaction to open item:
  If you want to block users to open items used as parents of calendar items,
  call \a KDGanttView::setCalendarMode( true );

  Example 1, Color:
  If you create an instance of a KDGanttView class and add a SummaryViewItem
  without setting any color/shape values, you get an item with three shapes
  of the form TriangleDown in the color magenta. If the item is highlighted,
  the color will change to the highlight color red.

  Example 2, Calender View:
  To use a gantt view as a calendar view, call
  \a KDGanttView::setCalendarMode( true );
  \a KDGanttView::setDisplaySubitemsAsGroup( true );
  Insert root items in the gantt view.
  Insert items as childs of these root item in the gantt view.
  You may use any item type as parent and child. There is no limitation.
  But it is recommended to use KDGanttViewTaskItems
  Actually, you may add  child items to the childs itself.
  Then, such a child behaves like a parent.
  Now set start/end time of the childs to determine a time interval for these items.

*/


QDict<KDGanttViewItem> KDGanttViewItem::sItemDict;

/*!
  Constructs an empty Gantt item.

  \param view the Gantt view to insert this item into
  \param lvtext the text to show in the listview
  \param name the name by which the item can be identified. If no name
  is specified, a unique name will be generated
*/


KDGanttViewItem::KDGanttViewItem( Type type, KDGanttView* view,
                                  const QString& lvtext,
                                  const QString& name ) :
    QListViewItem(view->myListView,lvtext)
{
    initColorAndShapes(type);
    generateAndInsertName( name );
}


/*!
  Constructs an empty Gantt item.

  \param parent a parent item under which this one goes
  \param lvtext the text to show in the listview
  \param name the name by which the item can be identified. If no name
  is specified, a unique name will be generated
*/

KDGanttViewItem::KDGanttViewItem( Type type, KDGanttViewItem* parentItem,
                                  const QString& lvtext,
                                  const QString& name ) :
    QListViewItem(parentItem,lvtext)
{
    initColorAndShapes(type);
    generateAndInsertName( name );
}


/*!
  Constructs an empty Gantt item.

  \param view the Gantt view to insert this item into
  \param after another item at the same level behind which this one should go
  \param lvtext the text to show in the listview
  \param name the name by which the item can be identified. If no name
  is specified, a unique name will be generated
*/

KDGanttViewItem::KDGanttViewItem( Type type, KDGanttView* view,
                                  KDGanttViewItem* after,
                                  const QString& lvtext,
                                  const QString& name ) :
    QListViewItem(view->myListView,after, lvtext)
{
    initColorAndShapes(type);
    generateAndInsertName( name );
}


/*!
  Constructs an empty Gantt item.

  \param parent a parent item under which this one goes
  \param after another item at the same level behind which this one should go
  \param lvtext the text to show in the listview
  \param name the name by which the item can be identified. If no name
  is specified, a unique name will be generated
*/

KDGanttViewItem::KDGanttViewItem( Type type, KDGanttViewItem* parentItem,
                                  KDGanttViewItem* after,
                                  const QString& lvtext,
                                  const QString& name ) :
    QListViewItem( parentItem, after, lvtext )
{
    initColorAndShapes(type);
    generateAndInsertName( name );
}


/*!
  Destroys the object and frees any allocated resources.

*/

KDGanttViewItem::~KDGanttViewItem()
{
  delete startLine;
  delete endLine  ;
  delete startLineBack  ;
  delete endLineBack ;
  delete actualEnd  ;
  delete textCanvas   ;
  delete startShape ;
  delete midShape  ;
  delete endShape  ;
  delete startShapeBack  ;
  delete midShapeBack   ;
  delete endShapeBack  ;
  if ( listView() ) {
     if ( parent() )
        parent()->takeItem( this );
     else
        myGantView->myListView->takeItem( this );
     myGantView->myTimeTable->updateMyContent();
  }
  myGantView->myTimeTable->removeItemFromTasklinks( this );
  myGantView->myCanvasView->resetCutPaste( this );
}


/*
  Generates a unique name if necessary and inserts it into the item
  dictionary.
*/
void KDGanttViewItem::generateAndInsertName( const QString& name )
{
    // First check if we already had a name. This can be the case if
    // the item was reconstructed from an XML file.
    if( !_name.isEmpty() )
        // We had a name, remove it
        sItemDict.remove( _name );
    QString newName = name;
    // if name is empty, we take the text of the listviewitem
    // or the number of the pointer of this
    if ( newName.isEmpty() )
        newName =  QListViewItem::text(0);
    if ( newName.isEmpty() )
        newName = KApplication::randomString(7);
    // Append _0 until we find a name that doesn't exist yet
    while( sItemDict.find( newName ) ) {
        newName += "_0";
    }
    sItemDict.insert( newName, this );
    _name = newName;
}


/*!
  Returns the unique name that can identify the item.

  \return the unique item name
*/
QString KDGanttViewItem::name() const
{
    return _name;
}


/*!
  Returns the item with the specified name.

  \param name the name to search for
  \return the item with the specified name; 0 if no group
  with that name exists
*/

KDGanttViewItem* KDGanttViewItem::find( const QString& name )
{
    return sItemDict.find( name );
}



/*!
  Returns the type of the item.
  This may be Event, Task, Summary.

  \return the type of the item
*/

KDGanttViewItem::Type KDGanttViewItem::type() const
{

    return myType;
}

/*!
  Specifies whether this item is enabled. If disabled, the item stays in the 
  gantt view and  the item shows gray color to show the item is disabled.
  All signals of this item (like itemLeftClicked( this ) ) are blocked.
  If the item displays its subitems (childs) as a group, 
  (displaySubitemsAsGroup() == true)
  all changes apply to all subitems as well.

  \param editable pass true to make this item editable
  \sa enabled ()
*/

void KDGanttViewItem::setEnabled( bool on )
{
  _enabled = on;
  if ( displaySubitemsAsGroup() ) {
    myGantView->myTimeTable->inc_blockUpdating();
    KDGanttViewItem* temp = (KDGanttViewItem*) firstChild();
    while (temp != 0) {
      temp->setEnabled(  on );
      temp = temp->nextSibling();
    }
    QListViewItem::setEnabled( on );
    myGantView->myTimeTable->dec_blockUpdating();
  }
  updateCanvasItems();
}
/*!
  Returns whether this item is enabled.

  \return true if this item is enabled, false otherwise
  \sa setEnabled()
*/

bool KDGanttViewItem::enabled () const
{
  return _enabled;
}
/*!
  Specifies whether this item is editable. The whole Gantt view needs
  to be editable as well for this to have any effect.

  \param editable pass true to make this item editable
  \sa editable(), KDGanttView::setEditable(), KDGanttView::editable()
*/

void KDGanttViewItem::setEditable( bool editable )
{
    isEditable = editable;
}


/*!
  Returns whether this item is editable.

  \return true if this item is editable, false otherwise
  \sa setEditable(), KDGanttView::setEditable(), KDGanttView::editable()
*/

bool KDGanttViewItem::editable() const
{
    return isEditable;
}
/*!
  Specifies whether this item shows hidden subitems on its timeline.
  Useful to get a so called calendar view table with many items in one row.
  When \a displaySubitemsAsGroup() is set to true, this item has a normal view,
  when it is expanded. If it is not expanded, (and has at least one child)
  the item itself is hidden  and all childs are displayed instead.
  To manage the painting priority of the childs ( if overlapping )
  you may set \a priority() of these items.

  \param show pass true to make this item displaying hidden subitems
  \sa editable(), KDGanttView::setEditable(), KDGanttView::editable(), setPriority()
*/

void KDGanttViewItem::setDisplaySubitemsAsGroup( bool show )
{
  if ( !show && _displaySubitemsAsGroup)
    isVisible = true;
    _displaySubitemsAsGroup = show;
    updateCanvasItems();
}


/*!
  Returns whether this item displays hidden subitems.
  Initial set to false.

  \return true if this item displays hidden subitems, false otherwise
  \sa setDisplaySubitemsAsGroup()
*/

bool KDGanttViewItem::displaySubitemsAsGroup() const
{
    return _displaySubitemsAsGroup;
}

/*!
  Specifies the priority of this item.
  Valid values are between 1 and 199.
  A priority less than 100 means, that the item is painted in the
  Gantt chart below the grid. A priority more than 100 means,
  that the item is painted in the Gantt chart over the grid.
  For a value of 100, the behaviour is undefined.
  An item with a higher priority is painted over an item with a lower
  priority in the Gantt chart. The painting of items with same priority is undefined.
  For Calendar items, the default priority is 50, for all other items it is 150.
  This feature makes only sense for an item, which is a child of another item,
  which \a displaySubitemsAsGroup() property is set to true.

  \param prio the new priority of this item.
  \sa priority(), displaySubitemsAsGroup()
*/

void KDGanttViewItem::setPriority( int prio )
{
  if ( prio < 1 )
    prio = 1;
  if (prio > 199 )
    prio = 199;
  _priority = prio;
  updateCanvasItems();
}


/*!
  Returns the priority of this item.
  \return the priority of this item
  \sa setDisplaySubitemsAsGroup()
*/

int KDGanttViewItem::priority()
{
    return _priority;
}


/*!
  Specifies the start time of this item. The parameter must be valid
  and non-null. If the parameter is invalid or null, no value is set.
  Reimplemented in the subclasses.

  \param start the start time
  \sa startTime(), setEndTime(), endTime()
*/

void KDGanttViewItem::setStartTime( const QDateTime&  )
{
  //qDebug("KDGanttViewItem::setStartTime():This method has to be reimplemented ");

}


/*!
  Returns the start time of this item.

  \return the start time of this item
  \sa setStartTime(), setEndTime(), endTime()
*/

QDateTime KDGanttViewItem::startTime() const
{
    return myStartTime;
}


/*!
  Specifies the end time of this item.The parameter must be valid
  and non-null. If the parameter is invalid or null, no value is set.
  Reimplemented in the subclasses

  \param end the end time
  \sa endTime(), setStartTime(), startTime()

*/

void KDGanttViewItem::setEndTime( const QDateTime& end )
{
    switch( type() ) {
    case Event:
        qDebug( "KDGantt:Event Item has no end time" );
	break;
    case Summary:
        ((KDGanttViewSummaryItem*)this)->setEndTime( end );
	break;
    case Task:
        qDebug( "KDGantt:Task Item has no end time" );
	break;
    default:
        qDebug( "Unknown type in KDGanttViewItem::typeToString()" );
    }
}


/*!
  Returns the end time of this item.

  \return the end time of this item
  \sa setEndTime(), setStartTime(), startTime()

*/

QDateTime KDGanttViewItem::endTime() const
{
    return myEndTime;
}


/*!
  Sets the text to be shown in this item in the Gantt view.
  For a KDGanttViewTaskItem witht displaySubitemsAsGroup() == true,
  the text is shown in the item itself and
  the text is truncated automatically, if it fits not in the item.
  For all other item types, the text is shown right of the item.

  \param text the text to be shown
  \sa text(), setTextColor(), textColor(), setListViewText(),
  listViewText()
*/

void KDGanttViewItem::setText( const QString& text )
{
    textCanvas->setText(text);
    textCanvasText = text;
    updateCanvasItems();
}


/*!
  Returns the text to be shown in this item in the Gantt view.

  \return the text to be shown in this item
  \sa setText(), setTextColor(), textColor(), setListViewText(),
  listViewText()
*/

QString KDGanttViewItem::text() const
{
    return textCanvasText;
}


/*!
  \obsolete
*/

void KDGanttViewItem::setListViewText( const QString& text, int column )
{
    QListViewItem::setText( column, text );
}


/*!
  Sets the text to be shown in this item in the list view.

  \param column the column in which the text will be shown
  \param text the text to be shown
  \sa text(), setTextColor(), textColor(), setText(), listViewText()
*/

void KDGanttViewItem::setListViewText( int column, const QString& text )
{
    QListViewItem::setText( column, text );
}


/*!
  Returns the text to be shown in this item in the list view.

  \param the column in which the text will be shown
  \return the text to be shown in this item
  \sa setText(), setTextColor(), textColor(), text(),
  setListViewText()
*/

QString KDGanttViewItem::listViewText( int column ) const
{
    return QListViewItem::text( column );
}


/*!
  Sets the font to be used for the text in this item.

  \param font the font to be shown
  \sa font()
*/

void KDGanttViewItem::setFont( const QFont& font )
{
    textCanvas->setFont(font);
    updateCanvasItems();
}


/*!
  Returns the font used for the text in this item.

  \return the font used for the text in this item
  \sa setFont()
*/

QFont KDGanttViewItem::font() const
{
    return textCanvas->font();
}


/*!
  Sets the text to show in a tooltip for this item.

  \param text the tooltip text
  \sa tooltipText()
*/

void KDGanttViewItem::setTooltipText( const QString& text )
{
    myToolTipText = text;
}


/*!
  Returns the tooltip text of this item

  \param the tooltip text
  \sa setTooltipText()
*/

QString KDGanttViewItem::tooltipText() const
{

    return myToolTipText;
}


/*!
  Sets the text to show in a What's This window for this item.

  \param text the what's this text
  \sa whatsThisText()

*/

void KDGanttViewItem::setWhatsThisText( const QString& text )
{
    myWhatsThisText = text;
}


/*!
  Returns the what's this text of this item

  \param the what's this text
  \sa setWhatsThisText()

*/

QString KDGanttViewItem::whatsThisText() const
{
    return myWhatsThisText;
}


/*!
  Specifies whether this item should be shown highlighted. The user
  can also highlight items with the mouse.
  If the item displays its subitems (childs) as a group, 
  (displaySubitemsAsGroup() == true)
  all changes apply to all subitems as well.

  \param highlight true in order to highlight, false, in order to turn
  highlighting off for this item
  \sa highlight()
*/

void KDGanttViewItem::setHighlight( bool highlight )
{
    isHighlighted = highlight;
    if ( displaySubitemsAsGroup() ) {
      myGantView->myTimeTable->inc_blockUpdating();
      KDGanttViewItem* temp = (KDGanttViewItem*) firstChild();
      while (temp != 0) {
	temp->setHighlight( highlight );
	temp = temp->nextSibling();
      }
      myGantView->myTimeTable->dec_blockUpdating();
    }
    updateCanvasItems();
}


/*!
  Returns whether this item is highlighted, either programmatically
  with setHighlight() or by the user with the mouse.

  \return true if the item is highlighted
  \sa setHighlight()
*/

bool KDGanttViewItem::highlight() const
{
    return isHighlighted;
}


/*!
  Specifies the shapes to be used for this item.

  It is advisable not to use this method, but rather set the shapes
  for all items of a type with KDGanttView::setShapes() in order to
  get a uniform Gantt view.

  \param start the start shape
  \param middle the middle shape
  \param end the end shape
  \sa shapes(), setColors(), colors()
*/

void KDGanttViewItem::setShapes( Shape start, Shape middle, Shape end )
{

    myStartShape =  start;
    myMiddleShape= middle;
    myEndShape=  end;
    createShape(startShape,startShapeBack,start);
    createShape(midShape,midShapeBack,middle);
    midShape->setZ( 4 );
    createShape(endShape,endShapeBack,end);
    updateCanvasItems();
}
/*!
  Creates shapes of the specified type \a shape.
  The background shape color is set to black and the background shape
  is a little bit bigger than the foregroundshape to have a black border
  around the foregroundshape.

  \param itemShape the foreground shape
  \param middle itemShapeBack  the background shape
  \param shape the type of the shape
   (may be TriangleDown, TriangleUp, Diamond, Square, Circle)
  \sa shapes(), setColors(), colors()
*/

void KDGanttViewItem::createShape( KDCanvasPolygonItem* &itemShape,
                                   KDCanvasPolygonItem* &itemShapeBack,
                                   Shape shape )
{
  if ( itemShape && type() ==  Task )
    return;
    if (itemShape) delete itemShape;
    if (itemShapeBack) delete itemShapeBack;

    QCanvasPolygonalItem * item;
    QCanvasPolygonalItem * itemBack;
    int size = myItemSize+2;
    int hei = (myItemSize/3)/2;
    switch (shape) {
    case TriangleDown:
        {
            item = new KDCanvasPolygon(myGantView->myTimeTable, this,Type_is_KDGanttViewItem);
            QPointArray arr = QPointArray(3);
            arr.setPoint(0,-size/2,-hei);
            arr.setPoint(1,size/2,-hei);
            arr.setPoint(2,0,((size/2)-hei));
            ((QCanvasPolygon*)item)->setPoints(arr);
            size += 4;hei +=1;
            itemBack = new KDCanvasPolygon(myGantView->myTimeTable, this,Type_is_KDGanttViewItem);
            arr.setPoint(0,-size/2,-hei);
            arr.setPoint(1,size/2,-hei);
            arr.setPoint(2,0,((size/2)-hei));
            ((QCanvasPolygon*)itemBack)->setPoints(arr);

            break;
        }
    case TriangleUp:
        {
            // I really do not know why, but  we get only an TriangleUp-icon
            // of the same size as a TriangleDown-icon, if we increment the size by 2
            size+=2;
            item = new KDCanvasPolygon(myGantView->myTimeTable, this,Type_is_KDGanttViewItem);
            QPointArray arr = QPointArray(3);
            arr.setPoint(0,-size/2,hei);
            arr.setPoint(1,size/2,hei);
            arr.setPoint(2,0,(-size/2)+hei);
            ((QCanvasPolygon*)item)->setPoints(arr);
            size += 4;hei +=1;
            itemBack = new KDCanvasPolygon(myGantView->myTimeTable, this,Type_is_KDGanttViewItem);
            arr.setPoint(0,-size/2,hei);
            arr.setPoint(1,size/2,hei);
            arr.setPoint(2,0,(-size/2)+hei);
            ((QCanvasPolygon*)itemBack)->setPoints(arr);

            break;
        }

    case Diamond:
        {
            item = new KDCanvasPolygon(myGantView->myTimeTable, this,Type_is_KDGanttViewItem);
            QPointArray arr = QPointArray(4);
            arr.setPoint(0,0,-size/2);
            arr.setPoint(1,size/2,0);
            arr.setPoint(2,0,size/2);
            arr.setPoint(3,-size/2,0);
            ((QCanvasPolygon*)item)->setPoints(arr);
            size += 2;hei +=1;
            itemBack = new KDCanvasPolygon(myGantView->myTimeTable, this,Type_is_KDGanttViewItem);
            arr.setPoint(0,0,-size/2);
            arr.setPoint(1,size/2,0);
            arr.setPoint(2,0,size/2);
            arr.setPoint(3,-size/2,0);
            ((QCanvasPolygon*)itemBack)->setPoints(arr);
            break;
        }

    case Square:
        {
            size -=2;
            item = new KDCanvasPolygon(myGantView->myTimeTable, this,Type_is_KDGanttViewItem);
            QPointArray arr = QPointArray(4);
            arr.setPoint(0,-size/2,-size/2);
            arr.setPoint(1,size/2,-size/2);
            arr.setPoint(2,size/2,size/2);
            arr.setPoint(3,-size/2,size/2);
            ((QCanvasPolygon*)item)->setPoints(arr);
            size += 2;hei +=1;
            itemBack = new KDCanvasPolygon(myGantView->myTimeTable, this,Type_is_KDGanttViewItem);
            arr.setPoint(0,-size/2,-size/2);
            arr.setPoint(1,size/2,-size/2);
            arr.setPoint(2,size/2,size/2);
            arr.setPoint(3,-size/2,size/2);
            ((QCanvasPolygon*)itemBack)->setPoints(arr);
            break;
        }

    case Circle:
        {
            size -= 2;
            item = new KDCanvasEllipse(myGantView->myTimeTable, this,Type_is_KDGanttViewItem);
            ((KDCanvasEllipse*)item)->setSize(size,size);
            size += 2;hei +=1;
            itemBack = new KDCanvasEllipse(myGantView->myTimeTable, this,Type_is_KDGanttViewItem);
            ((KDCanvasEllipse*)itemBack)->setSize(size,size);
            break;
        }
    default:
        // Uninitialized shape, can e.g. be the case with free-busy
        // items which don't have any shapes
        return;
    }
    item->setBrush(Qt::SolidPattern);
    item->setZ(5);
    itemShape = (KDCanvasPolygonItem*) item;
    itemBack->setBrush(Qt::SolidPattern);
    itemBack->setZ(3);
    itemShapeBack = (KDCanvasPolygonItem*) itemBack;

}


/*!
  Returns the shapes used for this item

  \param start returns the start shape
  \param middle returns the middle shape
  \param end returns the end shape
  \sa setShapes(), setColors(), colors()
*/

void KDGanttViewItem::shapes( Shape& start, Shape& middle, Shape& end ) const
{
    start = myStartShape;
    middle = myMiddleShape;
    end = myEndShape;
}


/*!
  Specifies the colors in which to draw the shapes of this item.

  It is advisable not to use this method, but rather set the colors
  for all items of a type with KDGanttView::setColors() in order to
  get a uniform Gantt view.

  \param start the color for the start shape
  \param middle the color for the middle shape
  \param end the color for the end shape
  \sa colors(), setShapes(), shapes(), setDefaultColor(), defaultColor()
*/

void KDGanttViewItem::setColors( const QColor& start, const QColor& middle,
                                 const QColor& end )
{

    myStartColor=start ;
    myMiddleColor= middle;
    myEndColor= end;
    if ( displaySubitemsAsGroup() ) {
      myGantView->myTimeTable->inc_blockUpdating();
      KDGanttViewItem* temp = (KDGanttViewItem*) firstChild();
      while (temp != 0) {
	temp->setColors( start, middle, end );
	temp = temp->nextSibling();
      }
      myGantView->myTimeTable->dec_blockUpdating();
    }
    updateCanvasItems();
}


/*!
  Returns the colors used for this item

  \param start returns the start color
  \param middle returns the middle color
  \param end returns the end color
  \sa setColors(), setShapes(), shapes(), setDefaultColor(), defaultColor()
*/

void KDGanttViewItem::colors( QColor& start, QColor& middle, QColor& end ) const
{
    start = myStartColor ;
    middle =  myMiddleColor;
    end  = myEndColor;

}


/*!
  Specifies the highlight colors in which to draw the shapes of this item.

  It is advisable not to use this method, but rather set the highlight
  colors for all items of a type with
  KDGanttView::setHighlightColors() in order to get a uniform Gantt
  view.

  If the item displays its subitems (childs) as a group, 
  (displaySubitemsAsGroup() == true)
  all changes apply to all subitems as well.

  \param start the highlight color for the start shape
  \param middle the highlight color for the middle shape
  \param end the highlight color for the end shape
  \sa highlightColors(), setShapes(), shapes()

*/

void KDGanttViewItem::setHighlightColors( const QColor& start, const QColor& middle, const QColor& end )
{
    myStartColorHL=start ;
    myMiddleColorHL= middle;
    myEndColorHL= end;  
    if ( displaySubitemsAsGroup() ) {
      myGantView->myTimeTable->inc_blockUpdating();
      KDGanttViewItem* temp = (KDGanttViewItem*) firstChild();
      while (temp != 0) {
	temp->setHighlightColors( start,  middle,  end );
	temp = temp->nextSibling();
      }
      myGantView->myTimeTable->dec_blockUpdating();
    }
    updateCanvasItems();
}


/*!
  Returns the highlight colors used for this item

  \param start returns the start highlight color
  \param middle returns the middle highlight color
  \param end returns the end highlight color
  \sa setHighlightColors(), setShapes(), shapes()

*/

void KDGanttViewItem::highlightColors( QColor& start, QColor& middle, QColor& end ) const
{
    start = myStartColorHL ;
    middle =  myMiddleColorHL;
    end  = myEndColorHL;
}


/*!
  Specifies the color to be used for the text of this item.

  It is advisable not to use this method, but rather set the text color
  for all items with KDGanttView::setTextColor() in order to get a
  uniform Gantt view.
  If the item displays its subitems (childs) as a group, 
  (displaySubitemsAsGroup() == true)
  all changes apply to all subitems as well.
  \param color the text color
  \sa textColor(), setText(), text()
*/

void KDGanttViewItem::setTextColor( const QColor& color )
{
   
   myTextColor = color;
   if ( displaySubitemsAsGroup() ) {
      myGantView->myTimeTable->inc_blockUpdating();
      KDGanttViewItem* temp = (KDGanttViewItem*) firstChild();
      while (temp != 0) {
	temp->setTextColor(color);
	temp = temp->nextSibling();
      }
      myGantView->myTimeTable->dec_blockUpdating();
    }
    updateCanvasItems();
}


/*!
  Returns the color used for the text of this item.

  \return the text color
  \sa setTextColor(), setText(), text()
*/

QColor KDGanttViewItem::textColor() const
{
    return myTextColor;
}


/*!
  \enum KDGanttViewItem::Shape

  This enum is used in order to specify the shapes of a Gantt chart
  item.
*/


/*!
  \enum KDGanttViewItem::Type

  This enum is used in order to return the type of a Gantt chart item.
*/


/*!
  Sets the pixmap that is shown in the listview.

  \param column the column in which the pixmap is shown
  \param pixmap the pixmap to show
  \sa pixmap()
*/

void KDGanttViewItem::setPixmap( int column, const QPixmap& pixmap )
{
    QListViewItem::setPixmap( column, pixmap );
}

/*!
  \obsolete

  Sets the pixmap in column 0 of the listview. Use setPixmap( 0,
  pixmap ) instead.
*/

void KDGanttViewItem::setPixmap( const QPixmap& pixmap )
{
    QListViewItem::setPixmap( 0, pixmap );
}


/*!
  Returns a pixmap that is shown in the listview.

  \param column the column for which to query the pixmap
  \return a pointer to the pixmap shown
  \sa setPixmap()
*/

const QPixmap* KDGanttViewItem::pixmap( int column ) const
{
    return QListViewItem::pixmap( column );
}


/*!
  Sets the default color that is used for the item if no specific
  start, middle, or end colors are set.

  It is advisable not to use this method, but rather set the colors
  for all items of a type with KDGanttView::setDefaultColor() in order to
  get a uniform Gantt view.

  If the item displays its subitems (childs) as a group, 
  (displaySubitemsAsGroup() == true)
  all changes apply to all subitems as well.


  \param color the default color to use
  \sa defaultColor(), setColors(), colors()
*/

void KDGanttViewItem::setDefaultColor( const QColor& color )
{
    myDefaultColor = color;


   if ( displaySubitemsAsGroup() ) {
      myGantView->myTimeTable->inc_blockUpdating();
      KDGanttViewItem* temp = (KDGanttViewItem*) firstChild();
      while (temp != 0) {
	temp->setDefaultColor( color );
	temp = temp->nextSibling();
      }
      myGantView->myTimeTable->dec_blockUpdating();
    }
    updateCanvasItems();
}



/*!
  Returns the default color that is used for the item if no specific
  start, middle, or end colors are set.

  \return color the default color used
  \sa setDefaultColor(), setColors(), colors()
*/

QColor KDGanttViewItem::defaultColor() const
{
    return myDefaultColor;
}


/*!
  Sets the default highlighting color that is used for the item if no
  specific start, middle, or end colors are set.

  It is advisable not to use this method, but rather set the colors
  for all items of a type with KDGanttView::setDefaultHighlightColor()
  in order to get a uniform Gantt view.

  If the item displays its subitems (childs) as a group, 
  (displaySubitemsAsGroup() == true)
  all changes apply to all subitems as well.

  \param color the default highlighting color to use
  \sa defaultHighlightColor(), setHighlightColors(), highlightColors()
*/

void KDGanttViewItem::setDefaultHighlightColor( const QColor& color )
{
    myDefaultColorHL = color;
   if ( displaySubitemsAsGroup() ) {
      myGantView->myTimeTable->inc_blockUpdating();
      KDGanttViewItem* temp = (KDGanttViewItem*) firstChild();
      while (temp != 0) {
	temp->setDefaultHighlightColor( color );
	temp = temp->nextSibling();
      }
      myGantView->myTimeTable->dec_blockUpdating();
    }
    updateCanvasItems();
}



/*!
  Returns the default highlighting color that is used for the item if
  no specific start, middle, or end colors are set.

  \return color the default highlighting color used
  \sa setDefaultHighlightColor(), setHighlightColors(), highlightColors()
*/

QColor KDGanttViewItem::defaultHighlightColor() const
{
    return myDefaultColorHL;
}


/*!
  Returns the first child of this item.

  \return the first child of this item, 0 if this item has no children
*/

KDGanttViewItem* KDGanttViewItem::firstChild() const
{

    return (KDGanttViewItem* )QListViewItem::firstChild();
}


/*!
  Returns the next sibling item of this item

  \return the next sibling item of this item, 0 if this item has no
  more siblings
*/

KDGanttViewItem* KDGanttViewItem::nextSibling() const
{
    return (KDGanttViewItem* )QListViewItem::nextSibling();
}


/*!
  Returns the parent item of this item

  \return the parent item of this item, 0 if this item is a top-level
  item
*/

KDGanttViewItem* KDGanttViewItem::parent() const
{
    return (KDGanttViewItem*)QListViewItem::parent();
}


/*!
  Returns the item above this item in the listview

  \return the item above this item, 0 if this is the first item
*/

KDGanttViewItem* KDGanttViewItem::itemAbove()
{
    return (KDGanttViewItem* )QListViewItem::itemAbove();
}


/*!
  Returns the item below this item in the listview.
  It can be specified wether the disabled items are taken
  into account as well.

  \param includeDisabled if true, disabled items are considered as well
  \return the item below this item, 0 if this is the last item
*/

KDGanttViewItem* KDGanttViewItem::itemBelow( bool includeDisabled )
{
  if ( !includeDisabled ) {
    return (KDGanttViewItem* )QListViewItem::itemBelow();
  }
  if ( isOpen() )
    return firstChild();
  if ( nextSibling() )
    return nextSibling();
  if ( parent() )
    return parent()->nextSibling();
  return 0;
}

// updating colors, not the coordinates
void KDGanttViewItem::updateCanvasItems()
{
    if (blockUpdating) return;
    QPen p,pBack;
    QBrush b;
    b.setStyle(Qt::SolidPattern);
    if ( enabled() ) { 
      textCanvas->setColor(myTextColor);
      if (isHighlighted) {
        b.setStyle(Qt::SolidPattern);
        b.setColor(myStartColorHL);
        startShape->setBrush(b);
        b.setColor(myMiddleColorHL);
        midShape->setBrush(b);
        b.setColor(myEndColorHL);
        endShape->setBrush(b);
        p.setWidth(myItemSize/3 -1);
        p.setColor(myStartColorHL);
        startLine->setPen(p);
        p.setColor(myEndColorHL);
        endLine->setPen(p);
      } else {
        b.setStyle(Qt::SolidPattern);
        b.setColor(myStartColor);
	//  qDebug("update color  %s %s", listViewText().latin1(),myStartColor.name().latin1() );
        startShape->setBrush(b);
        b.setColor(myMiddleColor);
        midShape->setBrush(b);
        b.setColor(myEndColor);
        endShape->setBrush(b);
        p.setWidth(myItemSize/3-1);
        p.setColor(myStartColor);
        startLine->setPen(p);
        p.setColor(myEndColor);
        endLine->setPen(p);
      }
    } else {
      //QColor discol = Qt::lightGray;
        QColor discol = QColor(232,232,232);
	textCanvas->setColor( QColor(150,150,150) );
        b.setStyle(Qt::SolidPattern);
        b.setColor(discol);
        startShape->setBrush(b);
        midShape->setBrush(b);
        endShape->setBrush(b);
        p.setWidth(myItemSize/3 -1);
        p.setColor(discol);
        startLine->setPen(p);
        endLine->setPen(p);
    }
    pBack.setWidth((myItemSize/3-1)+2);
    startLineBack->setPen(pBack);
    endLineBack->setPen(pBack);
    QFont f = textCanvas->font();
    f.setPixelSize(myItemSize);
    textCanvas->setFont(f);
    //if (isVisible) {
        myGantView->myTimeTable->updateMyContent();
	//}
}
void KDGanttViewItem::initItem()
{
  //  qDebug("KDGanttViewItem::initItem():Error:This virtual method has to be reimplemented ");
}

void KDGanttViewItem::showItem( bool, int )
{

  // qDebug("KDGanttViewItem::showItem(bool show):Error:This virtual method has to be reimplemented ");
}

QPoint KDGanttViewItem::getTaskLinkStartCoord(QPoint p)
{
    textCanvas->move(p.x()+myItemSize, itemPos() + height()/2-myItemSize/2);
    return QPoint (myGantView->myTimeHeader->getCoordX(myEndTime) +myItemSize/2,itemPos()+height()/2);
}
QPoint KDGanttViewItem::getTaskLinkEndCoord()
{
    return QPoint (myGantView->myTimeHeader->getCoordX(myStartTime)-myItemSize/2 ,itemPos()-myItemSize/2+height()/2-2);
}

void KDGanttViewItem::hideSubtree()
{
  if (firstChild())
    firstChild()->hideSubtree();
  if ( nextSibling () )
    nextSibling ()->hideSubtree();
  showItem(false);
}


void KDGanttViewItem::initColorAndShapes(Type t)
{
    myType = t;
    blockUpdating = true;
    isVisible = false;
    startShape = 0;
    midShape = 0;
    endShape = 0;
    startShapeBack = 0;
    midShapeBack = 0;
    endShapeBack = 0;

    myItemSize = 10;
    myGantView = ((KDListView *)listView())->myGantView;
    myGantView->myTimeHeader->saveCenterDateTime();
    myStartTime = myGantView->myTimeHeader->myCenterDateTime;
    myEndTime = myStartTime;
    myToolTipText =QListViewItem::text(0);
    myWhatsThisText = QListViewItem::text(0);
    isHighlighted = false;
    isEditable = true;
    _displaySubitemsAsGroup = myGantView->displaySubitemsAsGroup();
    startLine = new KDCanvasLine(myGantView->myTimeTable,this,Type_is_KDGanttViewItem);//KDGanttViewItem );
    endLine = new KDCanvasLine(myGantView->myTimeTable,this,Type_is_KDGanttViewItem);
    startLine->setZ(2);endLine->setZ(2);
    startLineBack = new KDCanvasLine(myGantView->myTimeTable,this,Type_is_KDGanttViewItem);//KDGanttViewItem );
    endLineBack = new KDCanvasLine(myGantView->myTimeTable,this,Type_is_KDGanttViewItem);
    startLineBack->setZ(1);endLineBack->setZ(1);
    actualEnd = new KDCanvasLine(myGantView->myTimeTable,this,Type_is_KDGanttViewItem);
    actualEnd->setZ(5);
    actualEnd->setPen( QPen ( Qt::red, 3 ) );

    textCanvas = new KDCanvasText(myGantView->myTimeTable,this,Type_is_KDGanttViewItem);
    textCanvas->setText("");
    textCanvas->setZ(10);
    // set textcolor
    setTextColor( myGantView->textColor());
    // set default color
    setDefaultColor( myGantView->defaultColor(myType));
    // set default highlight color
    setDefaultHighlightColor(myGantView->defaultHighlightColor(myType));
    // set shapes
    if (!( shapeDefined = (myGantView->shapes(myType,myStartShape,myMiddleShape,myEndShape)))) {

        //qDebug("KDGantt::KDGanttViewItem created with not user defined shapes");
    };
    
    setShapes(myStartShape,myMiddleShape,myEndShape);
    if ( type() == Task ) {
      //qDebug("new task %s ", listViewText().latin1());
      if ( startShape )
	delete startShape;
      startShape = (KDCanvasPolygonItem*)new  KDCanvasRectangle(myGantView->myTimeTable,this,Type_is_KDGanttViewItem); 
    }
    
    // set color of shapes
    if (!( colorDefined = (myGantView->colors(myType,myStartColor,myMiddleColor,myEndColor)))) {

    };
    setColors(defaultColor(),defaultColor(), defaultColor());
    // set highlight color of shapes
    if (!( colorHLDefined = (myGantView->highlightColors(myType,myStartColorHL,myMiddleColorHL,myEndColorHL)))) {

    };
    setHighlightColors(defaultHighlightColor(),defaultHighlightColor(), defaultHighlightColor());
    setFont(myGantView->font());
    // if (type() ==  Task)
    //setText(QListViewItem::text(0)); // testing only
    //isVisible = true;
    _priority = 150;
    _showNoInformation = false;
    _enabled = true;
    blockUpdating = false;
    updateCanvasItems();
}


QString KDGanttViewItem::shapeToString( Shape shape )
{
    switch( shape ) {
    case TriangleDown:
        return "TriangleDown";
    case TriangleUp:
        return "TriangleUp";
    case Diamond:
        return "Diamond";
    case Square:
        return "Square";
    case Circle:
        return "Circle";
    }
    return "";
}


KDGanttViewItem::Shape KDGanttViewItem::stringToShape( const QString& string )
{
    if( string == "TriangleDown" )
        return TriangleDown;
    else if( string == "TriangleUp" )
        return TriangleUp;
    else if( string == "Diamond" )
        return Diamond;
    else if( string == "Square" )
        return Square;
    else if( string == "Circle" )
        return Circle;
    else
        return TriangleDown;
}


void KDGanttViewItem::createNode( QDomDocument& doc,
                                  QDomElement& parentElement )
{
    QDomElement itemElement = doc.createElement( "Item" );
    parentElement.appendChild( itemElement );
    itemElement.setAttribute( "Type", typeToString( type() ) );

    KDXML::createDateTimeNode( doc, itemElement, "StartTime", startTime() );
    KDXML::createDateTimeNode( doc, itemElement, "EndTime", endTime() );
    KDXML::createFontNode( doc, itemElement, "Font", font() );
    KDXML::createStringNode( doc, itemElement, "Text", text() );
    KDXML::createStringNode( doc, itemElement, "TooltipText", tooltipText() );
    KDXML::createStringNode( doc, itemElement, "WhatsThisText",
                             whatsThisText() );
    if( pixmap() )
        KDXML::createPixmapNode( doc, itemElement, "Pixmap", *pixmap() );
    if( !listViewText().isNull() )
        KDXML::createStringNode( doc, itemElement, "ListViewText",
                                 listViewText() );
    KDXML::createBoolNode( doc, itemElement, "Open", isOpen() );
    KDXML::createBoolNode( doc, itemElement, "Highlight", highlight() );
    Shape startShape, middleShape, endShape;
    shapes( startShape, middleShape, endShape );
    KDXML::createStringNode( doc, itemElement, "StartShape",
                             shapeToString( startShape ) );
    KDXML::createStringNode( doc, itemElement, "MiddleShape",
                             shapeToString( middleShape ) );
    KDXML::createStringNode( doc, itemElement, "EndShape",
                             shapeToString( endShape ) );
    KDXML::createColorNode( doc, itemElement, "DefaultColor", defaultColor() );
    QColor startColor, middleColor, endColor;
    colors( startColor, middleColor, endColor );
    KDXML::createColorNode( doc, itemElement, "StartColor", startColor );
    KDXML::createColorNode( doc, itemElement, "MiddleColor", middleColor );
    KDXML::createColorNode( doc, itemElement, "EndColor", endColor );
    KDXML::createColorNode( doc, itemElement, "DefaultHighlightColor",
                            defaultHighlightColor() );
    highlightColors( startColor, middleColor, endColor );
    KDXML::createColorNode( doc, itemElement, "StartHighlightColor",
                            startColor );
    KDXML::createColorNode( doc, itemElement, "MiddleHighlightColor",
                            middleColor );
    KDXML::createColorNode( doc, itemElement, "EndHighlightColor", endColor );
    KDXML::createColorNode( doc, itemElement, "TextColor", textColor() );
    KDXML::createStringNode( doc, itemElement, "Name", name() );
    QDomElement itemsElement = doc.createElement( "Items" );
    itemElement.appendChild( itemsElement );
    KDGanttViewItem* currentItem = firstChild();
    while( currentItem ) {
        currentItem->createNode( doc, itemsElement );
        currentItem = currentItem->nextSibling();
    }

}



/*!
  Creates a KDGanttViewItem according to the specification in a DOM
  element.

  \param view the view in which the item will be inserted
  \param element the DOM element from which to read the specification
  \return the newly created element
*/

KDGanttViewItem* KDGanttViewItem::createFromDomElement( KDGanttView* view,
                                                        QDomElement& element )
{
    QString typeString = element.attribute( "Type" );
    Q_ASSERT( !typeString.isEmpty() );
    KDGanttViewItem* item;
    if( typeString == "Task" )
        item = new KDGanttViewTaskItem( view );
    else if( typeString == "Summary" )
        item = new KDGanttViewSummaryItem( view );
    else if( typeString == "Event" )
        item = new KDGanttViewEventItem( view );
    else {
        qDebug( "Unknown item type %s in KDGanttViewItem::createFromDomElement()", typeString.latin1() );
        return 0;
    }

    item->loadFromDomElement( element );
    return item;
}


/*!
  Creates a KDGanttViewItem according to the specification in a DOM
  element.

  \param view the view in which the item will be inserted
  \param previous to item behind this one should appear
  \param element the DOM element from which to read the specification
  \return the newly created element
*/

KDGanttViewItem* KDGanttViewItem::createFromDomElement( KDGanttView* view,
                                                        KDGanttViewItem* previous,
                                                        QDomElement& element )
{
    QString typeString = element.attribute( "Type" );
    Q_ASSERT( !typeString.isEmpty() );
    KDGanttViewItem* item;
    if( typeString == "Task" )
        item = new KDGanttViewTaskItem( view, previous );
    else if( typeString == "Summary" )
        item = new KDGanttViewSummaryItem( view, previous );
    else if( typeString == "Event" )
        item = new KDGanttViewEventItem( view, previous );
    else {
        qDebug( "Unknown item type in KDGanttViewItem::createFromDomElement()" );
        return 0;
    }

    item->loadFromDomElement( element );
    return item;
}




/*!
  Creates a KDGanttViewItem according to the specification in a DOM
  element.

  \param parent the parent item under which the item will be inserted
  \param element the DOM element from which to read the specification
  \return the newly created element
*/

KDGanttViewItem* KDGanttViewItem::createFromDomElement( KDGanttViewItem* parent,
                                                        QDomElement& element )
{
    QString typeString = element.attribute( "Type" );
    Q_ASSERT( !typeString.isEmpty() );
    KDGanttViewItem* item;
    if( typeString == "Task" )
        item = new KDGanttViewTaskItem( parent );
    else if( typeString == "Summary" )
        item = new KDGanttViewSummaryItem( parent );
    else if( typeString == "Event" )
        item = new KDGanttViewEventItem( parent );
    else {
        qDebug( "Unknown item type in KDGanttViewItem::createFromDomElement()" );
        return 0;
    }

    item->loadFromDomElement( element );
    return item;
}


/*!
  Creates a KDGanttViewItem according to the specification in a DOM
  element.

  \param parent the parent item under which the item will be inserted
  \param previous to item behind this one should appear
  \param element the DOM element from which to read the specification
  \return the newly created element
*/

KDGanttViewItem* KDGanttViewItem::createFromDomElement( KDGanttViewItem* parent,
                                                        KDGanttViewItem* previous,
                                                        QDomElement& element )
{
    QString typeString = element.attribute( "Type" );
    Q_ASSERT( !typeString.isEmpty() );
    KDGanttViewItem* item;
    if( typeString == "Task" )
        item = new KDGanttViewTaskItem( parent, previous );
    else if( typeString == "Summary" )
        item = new KDGanttViewSummaryItem( parent, previous );
    else if( typeString == "Event" )
        item = new KDGanttViewEventItem( parent, previous );
    else {
        qDebug( "Unknown item type in KDGanttViewItem::createFromDomElement()" );
        return 0;
    }

    item->loadFromDomElement( element );
    return item;
}


/*
  Fills in the values in the item by reading the DOM element.
*/
void KDGanttViewItem::loadFromDomElement( QDomElement& element )
{
    QDomNode node = element.firstChild();
    Shape startShape = TriangleDown, middleShape = TriangleDown,
            endShape = TriangleDown;
    QColor startColor, middleColor, endColor;
    QColor startHighlightColor, middleHighlightColor, endHighlightColor;
    QString tempName;
    while( !node.isNull() ) {
        QDomElement element = node.toElement();
        if( !element.isNull() ) { // was really an element
            QString tagName = element.tagName();
            if( tagName == "StartTime" ) {
                QDateTime value;
                if( KDXML::readDateTimeNode( element, value ) )
                    setStartTime( value );
            } else if( tagName == "EndTime" ) {
                QDateTime value;
                if( KDXML::readDateTimeNode( element, value ) )
                    setEndTime( value );
            } else if( tagName == "Text" ) {
                QString value;
                if( KDXML::readStringNode( element, value ) )
                    setText( value );
            } else if( tagName == "Font" ) {
                QFont value;
                if( KDXML::readFontNode( element, value ) )
                    setFont( value );
            } else if( tagName == "TooltipText" ) {
                QString value;
                if( KDXML::readStringNode( element, value ) )
                    setTooltipText( value );
            } else if( tagName == "WhatsThisText" ) {
                QString value;
                if( KDXML::readStringNode( element, value ) )
                    setWhatsThisText( value );
            } else if( tagName == "Pixmap" ) {
                QPixmap value;
                if( KDXML::readPixmapNode( element, value ) )
                    setPixmap( value );
            } else if( tagName == "ListViewText" ) {
                QString value;
                if( KDXML::readStringNode( element, value ) )
                    setListViewText( value );
            } else if( tagName == "Open" ) {
                bool value;
                if( KDXML::readBoolNode( element, value ) )
                    setOpen( value );
            } else if( tagName == "Highlight" ) {
                bool value;
                if( KDXML::readBoolNode( element, value ) )
                    setHighlight( value );
            } else if( tagName == "StartShape" ) {
                QString value;
                if( KDXML::readStringNode( element, value ) )
                    startShape = stringToShape( value );
            } else if( tagName == "MiddleShape" ) {
                QString value;
                if( KDXML::readStringNode( element, value ) )
                    middleShape = stringToShape( value );
            } else if( tagName == "EndShape" ) {
                QString value;
                if( KDXML::readStringNode( element, value ) )
                    endShape = stringToShape( value );
            } else if( tagName == "DefaultColor" ) {
                QColor value;
                if( KDXML::readColorNode( element, value ) )
                    setDefaultColor( value );
            } else if( tagName == "StartColor" ) {
                QColor value;
                if( KDXML::readColorNode( element, value ) )
                    startColor = value;
            } else if( tagName == "MiddleColor" ) {
                QColor value;
                if( KDXML::readColorNode( element, value ) )
                    middleColor = value;
            } else if( tagName == "EndColor" ) {
                QColor value;
                if( KDXML::readColorNode( element, value ) )
                    endColor = value;
            } else if( tagName == "DefaultHighlightColor" ) {
                QColor value;
                if( KDXML::readColorNode( element, value ) )
                    setDefaultHighlightColor( value );
            } else if( tagName == "StartHighlightColor" ) {
                QColor value;
                if( KDXML::readColorNode( element, value ) )
                    startHighlightColor = value;
            } else if( tagName == "MiddleHighlightColor" ) {
                QColor value;
                if( KDXML::readColorNode( element, value ) )
                    middleHighlightColor = value;
            } else if( tagName == "EndHighlightColor" ) {
                QColor value;
                if( KDXML::readColorNode( element, value ) )
                    endHighlightColor = value;
            } else if( tagName == "TextColor" ) {
                QColor value;
                if( KDXML::readColorNode( element, value ) )
                    setTextColor( value );
            } else if( tagName == "Name" ) {
                QString value;
                if( KDXML::readStringNode( element, value ) )
                    tempName = value;
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
            } else {
                qDebug( "Unrecognized tag name: %s", tagName.latin1() );
                Q_ASSERT( false );
            }
        }

        node = node.nextSibling();
    }

    setColors( startColor, middleColor, endColor );
    setHighlightColors( startHighlightColor, middleHighlightColor,
                        endHighlightColor );
    setShapes( startShape, middleShape, endShape );
    generateAndInsertName( tempName );
}


QString KDGanttViewItem::typeToString( Type type )
{
    switch( type ) {
    case Event:
        return "Event";
    case Summary:
        return "Summary";
    case Task:
        return "Task";
    default:
        qDebug( "Unknown type in KDGanttViewItem::typeToString()" );
        return "Summary";
    }
    return "";
}
int KDGanttViewItem::getCoordY()
{
  return itemPos() + height()/2;
}
void KDGanttViewItem::showSubItems()
{
  showSubitemTree( getCoordY() );
}
void KDGanttViewItem::showSubitemTree( int CoordY )
{
  KDGanttViewItem* temp = firstChild();
  if (temp) {
    while (temp != 0) {
      if (temp->isOpen() || !temp->displaySubitemsAsGroup() ) {
	temp->showItem( true, CoordY );
	if ( temp->firstChild() )
	  temp->firstChild()->hideSubtree();
      } else {
	temp->showSubitemTree( CoordY );
      }
      temp = temp->nextSibling();
    }
    showItem( false );
  } else {
    showItem( true, CoordY );
  }
}
QDateTime KDGanttViewItem::myChildStartTime()
{
  QDateTime ret, tempTime;
  bool set = true;
  KDGanttViewItem* temp = (KDGanttViewItem*) firstChild();
  if (temp) {
    while (temp != 0) {
      if ( !temp->displaySubitemsAsGroup() ) {
	tempTime = temp->startTime();
      } else {
	tempTime = temp->myChildStartTime();
      }
      if ( set ) {
	set = false;
	ret = tempTime;
      } else {
	if ( tempTime < ret ) {
	  ret = tempTime;
	}
      }
      temp = temp->nextSibling();
    }
  } else {
    ret = startTime();
  }
  return ret;
}
QDateTime KDGanttViewItem::myChildEndTime()
{
  QDateTime ret, tempTime;
  bool set = true;
  KDGanttViewItem* temp = (KDGanttViewItem*) firstChild();
  if (temp) {
    while (temp != 0) {
      if ( !temp->displaySubitemsAsGroup() ) {
	tempTime = temp->endTime();
      } else {
	tempTime = temp->myChildEndTime();
      }
      if ( set ) {
	set = false;
	ret = tempTime;
      } else {
	if ( tempTime > ret ) {
	  ret = tempTime;
	}
      }
      temp = temp->nextSibling();
    }
  } else {
    ret = endTime();
  }
  return ret;
}
/*!
  Returns whether the 'showNoInformation' line  should be shown for this item

  \return true if showNoInformation line should be shown
 \sa setShowNoInformation(),  KDGanttView::setNoInformationBrush(), KDGanttView::noInformationBrush()
*/
bool KDGanttViewItem::showNoInformation()
{
  return _showNoInformation;
}
/*!
  Specifies whether the 'showNoInformation' line  should be shown for this item.
  The 'showNoInformation' line is drawn over the whole timeline.
  The height of the line is the height of the item.
  The brush of the line is specified by KDGanttView::setNoInformationBrush().
  (i.e. the same brush for all items of the ganttview).
  The default brush is QBrush(  QColor ( 100,100,100 ), Qt::FDiagPattern );
  \param if true, the 'showNoInformation' line  is shown for this item
  \sa  showNoInformation(), KDGanttView::setNoInformationBrush(), KDGanttView::noInformationBrush()

*/
void KDGanttViewItem::setShowNoInformation( bool show )
{
  _showNoInformation = show;
  myGantView->myTimeTable->updateMyContent();
}
/*!
  If the name of this item is \a name, (i.e. listViewText() == name )
  the pointer to this item is returned.  Otherwise, it looks for an 
  item with name \a name in the set of childs and subchilds  of this item.
  \param  name the name of the item
  \return the pointer to the item with name \a name

*/

KDGanttViewItem* KDGanttViewItem::getChildByName( const QString& name )
{
  if ( listViewText() == name )
    return this;
  KDGanttViewItem* temp =  firstChild(),* ret;
  while (temp != 0) {
    if ( (ret = temp->getChildByName( name )))
      return ret;
    temp = temp->nextSibling();
  }
  return 0;
}
/*
void  KDGanttViewItem::printinfo( QString s )
{
  qDebug("%s %s %d ", s.latin1(), listViewText().latin1(), (int) isOpen());
  KDGanttViewItem* temp =  firstChild();
  while (temp != 0) {
    temp->printinfo("  "+s );
    temp = temp->nextSibling();
  }
}
*/
int  KDGanttViewItem::computeHeight()
{
  int hei = 0;
  KDGanttViewItem* temp;
  bool show = true;
  if ( isOpen() ) { 
    temp = firstChild();
    while (temp != 0) {
      hei += temp->computeHeight();
      temp = temp->nextSibling();
    }
    hei += height();
  } else { // closed!
    hei = height();
    if ( !displaySubitemsAsGroup() ) {
      if ( firstChild() ) {
	firstChild()->hideSubtree();
      }
    } else {
      if ( firstChild() ) {
	showSubitemTree( getCoordY() );
	show =  false ;
      }
    }
  }
  if ( show )
    showItem( true );
  return hei;
}
