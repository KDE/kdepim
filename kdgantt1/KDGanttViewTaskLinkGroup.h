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

#ifndef KDGANTTVIEWTASKLINKGROUP_H
#define KDGANTTVIEWTASKLINKGROUP_H


#include "kdgantt_qt3_compat.h"

#include "KDGanttViewTaskLink.h"


class KDTimeTableWidget;
class KDGanttView;

class KDGanttViewTaskLinkGroup : public QObject
{
public:
    KDGanttViewTaskLinkGroup( const QString& name );
    KDGanttViewTaskLinkGroup();
    ~KDGanttViewTaskLinkGroup();
    void insert (KDGanttViewTaskLink*) ;
    bool remove (KDGanttViewTaskLink*);

    void setVisible( bool show );
    bool visible() const;

    void setHighlight( bool highlight );
    bool highlight() const;

    void setColor( const QColor& color );
    QColor color() const;
    void setHighlightColor( const QColor& color );
    QColor highlightColor() const;

    static KDGanttViewTaskLinkGroup* find( const QString& name );

    void createNode( QDomDocument& doc,
                     QDomElement& parentElement );
    static KDGanttViewTaskLinkGroup* createFromDomElement( QDomElement& );

private:
    friend class KDTimeTableWidget;
    friend class KDGanttViewTaskLink;

    bool isvisible,ishighlighted;
    QColor myColor, myColorHL;
    QPtrList<KDGanttViewTaskLink> myTaskLinkList;
    QString _name;

    void insertItem(KDGanttViewTaskLink*);
    void removeItem (KDGanttViewTaskLink*);

    static QDict<KDGanttViewTaskLinkGroup> sGroupDict;
};

#endif
