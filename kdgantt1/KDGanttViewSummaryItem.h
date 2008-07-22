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


#ifndef KDGANTTVIEWSUMMARYITEM_H
#define KDGANTTVIEWSUMMARYITEM_H

#include "KDGanttViewItem.h"

class KDGanttViewSummaryItem : public KDGanttViewItem
{
public:
    KDGanttViewSummaryItem( KDGanttView* view,
                            const QString& lvtext = QString(),
                            const QString& name = QString() );
    KDGanttViewSummaryItem( KDGanttViewItem* parent,
                            const QString& lvtext = QString(),
                            const QString& name = QString() );
    KDGanttViewSummaryItem( KDGanttView* view, KDGanttViewItem* after,
                            const QString& lvtext = QString(),
                            const QString& name = QString() );
    KDGanttViewSummaryItem( KDGanttViewItem* parent, KDGanttViewItem* after,
                            const QString& lvtext = QString(),
                            const QString& name = QString() );
    virtual ~KDGanttViewSummaryItem();

    virtual bool moveConnector( Connector, QPoint p );
    virtual Connector getConnector( QPoint p );
    void setMiddleTime( const QDateTime& );
    QDateTime middleTime() const;
    void setActualEndTime( const QDateTime& end );
    QDateTime actualEndTime() const;
    void setStartTime( const QDateTime& start );
    void setEndTime( const QDateTime& end );
private:
    virtual KDGanttViewItem::Connector getConnector( QPoint p, bool linkMode );
    virtual void showItem( bool show = true, int coordY = 0 );
    QDateTime* myActualEndTime,*myMiddleTime;
    virtual void initItem();
    void hideMe();


};

#endif
