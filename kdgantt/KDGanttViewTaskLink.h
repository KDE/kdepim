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

#ifndef KDGANTTVIEWTASKLINK_H
#define KDGANTTVIEWTASKLINK_H

#include <qcolor.h>
#include <qstring.h>
#include <q3ptrlist.h>
#include <q3canvas.h>

#include "KDGanttViewItem.h"
class KDGanttViewTaskLinkGroup;
class KDCanvasPolygon;
class KDCanvasLine;

class KDGanttViewTaskLink
{
public:
    enum LinkType { None, FinishStart, StartStart, FinishFinish, StartFinish };
    
    KDGanttViewTaskLink( Q3PtrList<KDGanttViewItem> from,
                         Q3PtrList<KDGanttViewItem> to,
                         LinkType type=None );
    KDGanttViewTaskLink( KDGanttViewTaskLinkGroup* group,
                         Q3PtrList<KDGanttViewItem> from,
                         Q3PtrList<KDGanttViewItem> to,
                         LinkType type=None );
    KDGanttViewTaskLink( KDGanttViewTaskLinkGroup* group,
                         KDGanttViewItem*  from,
                         KDGanttViewItem* to,
                         LinkType type=None );
    KDGanttViewTaskLink( KDGanttViewItem*  from,
                         KDGanttViewItem* to,
                         LinkType type=None );
    ~KDGanttViewTaskLink();
    Q3PtrList<KDGanttViewItem> from() const;
    Q3PtrList<KDGanttViewItem> to() const;
    void removeItemFromList( KDGanttViewItem* );

    void setVisible( bool );
    bool isVisible() const;

    KDGanttViewTaskLinkGroup* group();
    void setGroup( KDGanttViewTaskLinkGroup*) ;

    void setHighlight( bool highlight );
    bool highlight() const;

    void setColor( const QColor& color );
    QColor color() const;
    void setHighlightColor( const QColor& color );
    QColor highlightColor() const;

    void setTooltipText( const QString& text );
    QString tooltipText() const;
    void setWhatsThisText( const QString& text );
    QString whatsThisText() const;

    void createNode( QDomDocument& doc,
                     QDomElement& parentElement );
    static KDGanttViewTaskLink* createFromDomElement( QDomElement& );

    int linkType();
    void setLinkType(int type);
    
private:
    friend class KDGanttViewTaskLinkGroup;
    friend class KDTimeTableWidget;
    Q3PtrList<KDGanttViewItem> fromList,toList;
    Q3PtrList<KDCanvasLine>* horLineList;
    Q3PtrList<KDCanvasLine>* verLineList;
    Q3PtrList<KDCanvasPolygon>* topList;

    // also used when linkType != None    
    Q3PtrList<KDCanvasLine>* horLineList2;
    Q3PtrList<KDCanvasLine>* verLineList2;
    Q3PtrList<KDCanvasLine>* horLineList3;
    Q3PtrList<KDCanvasPolygon>* topLeftList;
    Q3PtrList<KDCanvasPolygon>* topRightList;
    
    KDGanttViewTaskLinkGroup* myGroup;
    bool isvisible,ishighlighted;
    QColor myColor, myColorHL;
    QString myToolTipText,myWhatsThisText;
    KDTimeTableWidget*  myTimeTable;
    void initTaskLink();
    void showMe( bool );
    void showMeType( bool );
    void hide();    
    int xOffset(KDGanttViewItem *item);
    
    LinkType myLinkType;
    static QString linkTypeToString( LinkType type );
    static LinkType stringToLinkType( const QString type );
};

#endif
