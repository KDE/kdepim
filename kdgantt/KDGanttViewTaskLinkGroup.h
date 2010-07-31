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

#ifndef KDGANTTVIEWTASKLINKGROUP_H
#define KDGANTTVIEWTASKLINKGROUP_H

#include "KDGanttViewTaskLink.h"
#include <tqptrlist.h>
#include <tqdict.h>

class KDTimeTableWidget;
class KDGanttView;

class KDGanttViewTaskLinkGroup : public QObject
{
public:
    KDGanttViewTaskLinkGroup( const TQString& name );
    KDGanttViewTaskLinkGroup();
    ~KDGanttViewTaskLinkGroup();
    void insert (KDGanttViewTaskLink*) ;
    bool remove (KDGanttViewTaskLink*);

    void setVisible( bool show );
    bool visible() const;

    void setHighlight( bool highlight );
    bool highlight() const;

    void setColor( const TQColor& color );
    TQColor color() const;
    void setHighlightColor( const TQColor& color );
    TQColor highlightColor() const;

    static KDGanttViewTaskLinkGroup* find( const TQString& name );

    void createNode( TQDomDocument& doc,
                     TQDomElement& parentElement );
    static KDGanttViewTaskLinkGroup* createFromDomElement( TQDomElement& );

    void generateAndInsertName( const TQString& name );

private:
    friend class KDTimeTableWidget;
    friend class KDGanttViewTaskLink;

    bool isvisible,ishighlighted;
    TQColor myColor, myColorHL;
    TQPtrList<KDGanttViewTaskLink> myTaskLinkList;
    TQString _name;

    void insertItem(KDGanttViewTaskLink*);
    void removeItem (KDGanttViewTaskLink*);

    static TQDict<KDGanttViewTaskLinkGroup> sGroupDict;
};

#endif
