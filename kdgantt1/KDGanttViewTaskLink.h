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

#ifndef KDGANTTVIEWTASKLINK_H
#define KDGANTTVIEWTASKLINK_H

#include <QColor>
#include <QString>

#include "kdgantt_qt3_compat.h"

#include "KDGanttViewItem.h"
class KDGanttViewTaskLinkGroup;
class KDCanvasPolygon;
class KDCanvasLine;

class KDGanttViewTaskLink
{
public:
    enum LinkType { None, FinishStart, StartStart, FinishFinish, StartFinish };

    KDGanttViewTaskLink( QPtrList<KDGanttViewItem> from,
                         QPtrList<KDGanttViewItem> to,
                         LinkType type = None );
    KDGanttViewTaskLink( KDGanttViewTaskLinkGroup* group,
                         QPtrList<KDGanttViewItem> from,
                         QPtrList<KDGanttViewItem> to,
                         LinkType type = None );
    KDGanttViewTaskLink( KDGanttViewTaskLinkGroup* group,
                         KDGanttViewItem*  from,
                         KDGanttViewItem* to,
                         LinkType type = None );
    KDGanttViewTaskLink( KDGanttViewItem*  from,
                         KDGanttViewItem* to,
                         LinkType type = None);
    ~KDGanttViewTaskLink();
    QPtrList<KDGanttViewItem> from() const;
    QPtrList<KDGanttViewItem> to() const;
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
    bool isFromToItem (KDGanttViewItem * item );

    int getLinkType() const;
    void setLinkType(int type);

private:
    void resetGroup();
    friend class KDGanttViewTaskLinkGroup;
    friend class KDTimeTableWidget;
    QPtrList<KDGanttViewItem> fromList,toList;
    QPtrList<KDCanvasLine>* horLineList;
    QPtrList<KDCanvasLine>* verLineList;
    QPtrList<KDCanvasPolygon>* topList;
    // used when linkType != None    
    QPtrList<KDCanvasLine>* horLineList2;
    QPtrList<KDCanvasLine>* verLineList2;
    QPtrList<KDCanvasLine>* horLineList3;
    QPtrList<KDCanvasPolygon>* topLeftList;
    QPtrList<KDCanvasPolygon>* topRightList;
    
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
