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


#include "KDGanttViewTaskLink.h"
#include "KDGanttViewTaskLinkGroup.h"
#include "KDGanttViewSubwidgets.h"
#include "KDGanttXMLTools.h"


/*! \class KDGanttViewTaskLink KDGanttViewTaskLink.h
  This class represents a link between a number of Gantt chart items.

  It always connects source items with target items. Task links can
  be grouped into KDGanttViewTaskLinkGroup objects.
  If a Gantt view item is deleted, it is removed from the fromList or
  from the toList.
  If one of the lists becomes empty, the complete task link is deleted
  as well.
*/


/*!
  Creates a task link that connects all items in the source item list from
  to all items in the destination item list to.

  \param from the source items
  \param to the target items
*/
KDGanttViewTaskLink::KDGanttViewTaskLink( QPtrList<KDGanttViewItem> from,
                                          QPtrList<KDGanttViewItem> to )
{
  fromList= from;
  toList = to;
  myGroup = 0;
  initTaskLink();
}


/*!
  \overload

  Creates a task link that connects two items.
  Note, that the from() and to() functions are returning a list,
  in this case containing only one item.

  \param from the source item
  \param to the target item
*/
KDGanttViewTaskLink::KDGanttViewTaskLink( KDGanttViewItem*  from,
                                          KDGanttViewItem* to )
{
  fromList.append(from);
  toList.append(to);
  myGroup = 0;
  initTaskLink();

}

/*!
  Creates a task link that connects all items in the source item list from
  to all items in the destination item list to.
  Inserts the link directly into a link group.

  \param group the link group to insert this link into
  \param from the source items
  \param to the target items
*/

KDGanttViewTaskLink::KDGanttViewTaskLink( KDGanttViewTaskLinkGroup* group,
                                          QPtrList<KDGanttViewItem> from,
                                          QPtrList<KDGanttViewItem> to )
{
  fromList = from;
  toList = to;
  myGroup = 0;
  initTaskLink();
  setGroup(group);
}

/*!
  \overload

  Creates a task link that connects two items and inserts the link
  directly into a link group.
  Note, that the from() and to() functions are returning a list,
  in this case containing only one item.

  \param group the link group to insert this link into
  \param from the source item
  \param to the target item
*/

KDGanttViewTaskLink::KDGanttViewTaskLink( KDGanttViewTaskLinkGroup* group,
                                          KDGanttViewItem*  from,
                                          KDGanttViewItem* to )
{
  fromList.append(from);
  toList.append(to);
  myGroup = 0;
  initTaskLink();
  setGroup(group);
}


KDGanttViewTaskLink::~KDGanttViewTaskLink( )
{
  setGroup(0);
  myTimeTable->myTaskLinkList.remove(this);
  delete horLineList;
  delete verLineList;
  delete topList;
}


void KDGanttViewTaskLink::initTaskLink()
{
  horLineList = new QPtrList<KDCanvasLine>;
  verLineList = new QPtrList<KDCanvasLine>;
  topList = new QPtrList<KDCanvasPolygon>;
  horLineList->setAutoDelete( true );
  verLineList->setAutoDelete( true );
  topList->setAutoDelete( true );
  myTimeTable = fromList.getFirst()->myGanttView->myTimeTable;
  KDCanvasLine* horLine,*verLine;
  KDCanvasPolygon* top;
  unsigned int i, j;
  for ( i = 0;i < fromList.count();++i) {
    for ( j = 0;j < toList.count();++j) {
      horLine = new KDCanvasLine(myTimeTable,this,Type_is_KDGanttTaskLink);
      verLine = new KDCanvasLine(myTimeTable,this,Type_is_KDGanttTaskLink);
      top = new KDCanvasPolygon(myTimeTable,this,Type_is_KDGanttTaskLink);
      QPointArray arr = QPointArray(3);
      arr.setPoint(0,-4,-5);
      arr.setPoint(1,4,-5);
      arr.setPoint(2,0,0);
      top->setPoints(arr);
      horLineList->append(horLine);
      verLineList->append(verLine);
      topList->append(top);
      horLine->setZ(1);
      verLine->setZ(1);
      top->setZ(1);
    }
  }

  setTooltipText( "Tasklink" );
  setWhatsThisText( "Tasklink" );
  myTimeTable->myTaskLinkList.append(this);
  setHighlight( false);
  setHighlightColor(Qt::red );
  setColor(Qt::black);
  setVisible(true);
}


/*!
  Specifies whether this task link should be visible or not.

  \param visible pass true to make this task link visible, and false
  to hide it
  \sa isVisible()
*/

void KDGanttViewTaskLink::setVisible( bool visible )
{
    showMe ( visible );
    myTimeTable->updateMyContent();
}
void KDGanttViewTaskLink::showMe( bool visible )
{

    isvisible = visible;
    int wid = 1;
    QPen p;
    QBrush b;
    p.setWidth(wid);
    b.setStyle(Qt::SolidPattern);
    if (ishighlighted) {
        b.setColor(myColorHL);
        p.setColor(myColorHL);

    } else {
        b.setColor(myColor);
        p.setColor(myColor);
    }
    QPoint start, end;
    QPtrListIterator<KDCanvasLine> horIt(*horLineList);
    QPtrListIterator<KDCanvasLine> verIt(*verLineList);
    QPtrListIterator<KDCanvasPolygon> topIt(*topList);
    QPtrListIterator<KDGanttViewItem> fromIt(fromList);
    QPtrListIterator<KDGanttViewItem> toIt(toList);
    for ( ; fromIt.current(); ++fromIt ) {
        toIt.toFirst();
        for ( ; toIt.current(); ++toIt ) {
            if (!isvisible || ! (*fromIt)->isVisibleInGanttView || !(*toIt)->isVisibleInGanttView || !myTimeTable->taskLinksVisible) {
                (*horIt)->hide();
                (*verIt)->hide();
                (*topIt)->hide();
                ++horIt;
                ++verIt;
                ++topIt;
            } else {
                (*horIt)->setPen(p);
                (*verIt)->setPen(p);
                (*topIt)->setBrush(b);
                end = (*toIt)->getTaskLinkEndCoord();
                start = (*fromIt)->getTaskLinkStartCoord(end);
                (*horIt)->setPoints(start.x(),start.y(),end.x()+wid,start.y());
                (*verIt)->setPoints(end.x()+wid/2,start.y(),end.x()+wid/2,end.y()-2);
                (*topIt)->move(end.x()+wid/2,end.y());
                (*horIt)->show();
                (*verIt)->show();
                (*topIt)->show();
                ++horIt;
                ++verIt;
                ++topIt;

            }
        }
    }
    while ( horIt.current() ) {
        (*horIt)->hide();
        (*verIt)->hide();
        (*topIt)->hide();
        ++horIt;
        ++verIt;
        ++topIt;
    }
}


/*!
  Returns whether this task link should be visible or not.

  \return true if the task link is visible
  \sa setVisible()
*/
bool KDGanttViewTaskLink::isVisible() const
{

    return isvisible;
}


/*!
  Returns the group (if any) to which this task link belongs.

  \return the group to which this task link belongs; 0 if it does not
  belong to any group.
  \sa KDGanttViewTaskLinkGroup
*/
KDGanttViewTaskLinkGroup* KDGanttViewTaskLink::group()
{
    return myGroup;
}


/*!
  Inserts this task link in a group.
  If the parameter is 0, the task link is removed from any group

  \param group the group, this task link has to be inserted

  \sa KDGanttViewTaskLinkGroup
*/
void KDGanttViewTaskLink::setGroup(KDGanttViewTaskLinkGroup* group)
{

  myTimeTable->myGanttView->addTaskLinkGroup(group);
    if(myGroup == group)
      return;
    if (myGroup != 0)
      myGroup->removeItem(this);
    myGroup = group;
    if (myGroup != 0)
      myGroup->insertItem(this);
}


/*!
  Specifies whether this task link should be shown highlighted. The
  user can also highlight a task link with the mouse.

  \param highlight pass true in order to highlight this task link
  \sa highlight()
*/
void KDGanttViewTaskLink::setHighlight( bool highlight )
{
   ishighlighted =  highlight ;
   // if ( isvisible) setVisible(true );
   myTimeTable->updateMyContent();
}


/*!
  Returns whether this task link is highlighted, either
  programmatically by setHighlight() or by the user with the mouse.

  \return true if the task link is highlighted
  \sa setHighlight()
*/
bool KDGanttViewTaskLink::highlight() const
{
    return ishighlighted;
}


/*!
  Specifies the color to draw this task link in.

  \param color the color to draw this task link in
  \sa color()
*/
void KDGanttViewTaskLink::setColor( const QColor& color )
{
  myColor = color;
  //if ( isvisible) setVisible(true );
 myTimeTable->updateMyContent();
}


/*!
  Returns the color in which this task link is drawn.

  \return the color in which this task link is drawn
  \sa setColor()
*/
QColor KDGanttViewTaskLink::color() const
{
    return myColor;
}


/*!
  Specifies the highlight color to draw this task link in.

  \param color the highlight color to draw this task link in
  \sa highlightColor()
*/
void KDGanttViewTaskLink::setHighlightColor( const QColor& color )
{
  myColorHL = color;
  //if ( isvisible) setVisible(true );
 myTimeTable->updateMyContent();
}


/*!
  Returns the highlight color in which this task link is drawn.

  \return the highlight color in which this task link is drawn
  \sa setHighlightColor()
*/
QColor KDGanttViewTaskLink::highlightColor() const
{
    return myColorHL;
}


/*!
  Specifies the text to be shown as a tooltip for this task link.

  \param text the tooltip text
  \sa tooltipText()
*/
void KDGanttViewTaskLink::setTooltipText( const QString& text )
{
   myToolTipText = text;
}


/*!
  Returns the tooltip text of this task link.

  \return the tooltip text of this task link
  \sa setTooltipText()
*/
QString KDGanttViewTaskLink::tooltipText() const
{
    return myToolTipText;
}


/*!
  Specifies the text to be shown in a what's this window for this task link.

  \param text the what's this text
  \sa whatsThisText()
*/
void KDGanttViewTaskLink::setWhatsThisText( const QString& text )
{
  myWhatsThisText = text;

}


/*!
  Returns the what's this text of this task link.

  \return the what's this text of this task link
  \sa setWhatsThisText()
*/
QString KDGanttViewTaskLink::whatsThisText() const
{
  return myWhatsThisText;
}


/*!
  Returns the list of source item of this task link.

  \return the ist of source item of this task link
  \sa to()
*/
QPtrList<KDGanttViewItem>  KDGanttViewTaskLink::from() const
{
    return fromList;
}


/*!
  Removes a  KDGanttViewItem from the lists.

 \sa to() from()
*/
void KDGanttViewTaskLink::removeItemFromList( KDGanttViewItem* item )
{
    bool itemremoved = false;
    if (fromList.remove( item )) {
        itemremoved = true;
    }
    if ( toList.remove( item )) {
        itemremoved = true;
    }
    if ( itemremoved ) {
        setVisible( isvisible );
    }
}


/*!
  Returns the list of target items of this task link.

  \return the list of target item of this task link
  \sa from()
*/
QPtrList<KDGanttViewItem> KDGanttViewTaskLink::to() const
{
    return toList;
}


/*!
  Creates a DOM node that describes this task link.

  \param doc the DOM document to which the node belongs
  \param parentElement the element into which to insert this node
*/
void KDGanttViewTaskLink::createNode( QDomDocument& doc,
                                      QDomElement& parentElement )
{
    QDomElement taskLinkElement = doc.createElement( "TaskLink" );
    parentElement.appendChild( taskLinkElement );

    QDomElement fromItemsElement = doc.createElement( "FromItems" );
    taskLinkElement.appendChild( fromItemsElement );
    QPtrList<KDGanttViewItem> fromList = from();
    KDGanttViewItem* item;
    for( item = fromList.first(); item;
         item = fromList.next() )
        KDGanttXML::createStringNode( doc, fromItemsElement, "Item", item->name() );

    QDomElement toItemsElement = doc.createElement( "ToItems" );
    taskLinkElement.appendChild( toItemsElement );
    QPtrList<KDGanttViewItem> toList = to();
    for( item = toList.first(); item;
         item = toList.next() )
        KDGanttXML::createStringNode( doc, toItemsElement, "Item", item->name() );

    KDGanttXML::createBoolNode( doc, taskLinkElement, "Highlight", highlight() );
    KDGanttXML::createColorNode( doc, taskLinkElement, "Color", color() );
    KDGanttXML::createColorNode( doc, taskLinkElement, "HighlightColor",
                            highlightColor() );
    KDGanttXML::createStringNode( doc, taskLinkElement, "TooltipText",
                             tooltipText() );
    KDGanttXML::createStringNode( doc, taskLinkElement, "WhatsThisText",
                             whatsThisText() );
    if( group() )
        KDGanttXML::createStringNode( doc, taskLinkElement, "Group",
                                 group()->name() );
    KDGanttXML::createBoolNode( doc, taskLinkElement, "Visible",
                           isVisible() );
}


/*!
  Creates a KDGanttViewTaskLink according to the specification in a DOM
  element.

  \param element the DOM element from which to read the specification
  \return the newly created task link
*/
KDGanttViewTaskLink* KDGanttViewTaskLink::createFromDomElement( QDomElement& element )
{
    QDomNode node = element.firstChild();
    QStringList fromList, toList;
    bool highlight = false, visible = false;
    QColor color, highlightColor;
    QString tooltipText, whatsThisText, group;
    while( !node.isNull() ) {
        QDomElement element = node.toElement();
        if( !element.isNull() ) { // was really an element
            QString tagName = element.tagName();
            if( tagName == "FromItems" ) {
                QDomNode node = element.firstChild();
                while( !node.isNull() ) {
                    QDomElement element = node.toElement();
                    if( !element.isNull() ) { // was really an element
                        QString tagName = element.tagName();
                        if( tagName == "Item" ) {
                            QString value;
                            if( KDGanttXML::readStringNode( element, value ) )
                                fromList << value;
                        } else {
                            qDebug( "Unrecognized tag name: %s", tagName.latin1() );
                            Q_ASSERT( false );
                        }
                    }
                    node = node.nextSibling();
                }
            } else if( tagName == "ToItems" ) {
                QDomNode node = element.firstChild();
                while( !node.isNull() ) {
                    QDomElement element = node.toElement();
                    if( !element.isNull() ) { // was really an element
                        QString tagName = element.tagName();
                        if( tagName == "Item" ) {
                            QString value;
                            if( KDGanttXML::readStringNode( element, value ) )
                                toList << value;
                        } else {
                            qDebug( "Unrecognized tag name: %s", tagName.latin1() );
                            Q_ASSERT( false );
                        }
                    }
                    node = node.nextSibling();
                }
            } else if( tagName == "Highlight" ) {
                bool value;
                if( KDGanttXML::readBoolNode( element, value ) )
                    highlight = value;
            } else if( tagName == "Visible" ) {
                bool value;
                if( KDGanttXML::readBoolNode( element, value ) )
                    visible = value;
            } else if( tagName == "Color" ) {
                QColor value;
                if( KDGanttXML::readColorNode( element, value ) )
                    color = value;
            } else if( tagName == "HighlightColor" ) {
                QColor value;
                if( KDGanttXML::readColorNode( element, value ) )
                    highlightColor = value;
            } else if( tagName == "TooltipText" ) {
                QString value;
                if( KDGanttXML::readStringNode( element, value ) )
                    tooltipText = value;
            } else if( tagName == "WhatsThisText" ) {
                QString value;
                if( KDGanttXML::readStringNode( element, value ) )
                    whatsThisText = value;
            } else if( tagName == "Group" ) {
                QString value;
                if( KDGanttXML::readStringNode( element, value ) )
                    group = value;
            } else {
                qDebug( "Unrecognized tag name: %s", tagName.latin1() );
                Q_ASSERT( false );
            }
        }
        node = node.nextSibling();
    }

    QPtrList<KDGanttViewItem> fromItemList;
    QPtrList<KDGanttViewItem> toItemList;
    for( QStringList::const_iterator fromIt = fromList.begin();
         fromIt != fromList.end(); ++fromIt ) {
        KDGanttViewItem* item = KDGanttViewItem::find( *fromIt );
        if( item )
            fromItemList.append( item );
    }
    for( QStringList::const_iterator toIt = toList.begin();
         toIt != toList.end(); ++toIt ) {
        KDGanttViewItem* item = KDGanttViewItem::find( *toIt );
        if( item )
            toItemList.append( item );
    }
    // safeguard aginst incorrect names
    if (fromItemList.isEmpty()) {
        qDebug("Cannot create link: fromItemList is empty");
        return 0;
    }
    if (toItemList.isEmpty()) {
        qDebug("Cannot create link: toItemList is empty");
        return 0;
    }
    KDGanttViewTaskLink* tl = new KDGanttViewTaskLink( fromItemList,
                                                       toItemList );
    tl->setVisible( visible );
    tl->setHighlight( highlight );
    tl->setColor( color );
    tl->setHighlightColor( highlightColor );
    tl->setTooltipText( tooltipText );
    tl->setWhatsThisText( whatsThisText );
    KDGanttViewTaskLinkGroup* gr = KDGanttViewTaskLinkGroup::find( group );
    if( gr )
        tl->setGroup( gr );

    return tl;
}
