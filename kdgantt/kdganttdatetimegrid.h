/****************************************************************************
 ** Copyright (C) 2001-2006 Klarälvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KD Gantt library.
 **
 ** This file may be distributed and/or modified under the terms of the
 ** GNU General Public License version 2 as published by the Free Software
 ** Foundation and appearing in the file LICENSE.GPL included in the
 ** packaging of this file.
 **
 ** Licensees holding valid commercial KD Gantt licenses may use this file in
 ** accordance with the KD Gantt Commercial License Agreement provided with
 ** the Software.
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** See http://www.kdab.net/kdgantt for
 **   information about KD Gantt Commercial License Agreements.
 **
 ** Contact info@kdab.net if any conditions of this
 ** licensing are not clear to you.
 **
 **********************************************************************/
#ifndef KDGANTTDATETIMEGRID_H
#define KDGANTTDATETIMEGRID_H

#include "kdganttabstractgrid.h"

#include <QDateTime>
#include <QSet>

namespace KDGantt {
    class KDGANTT_EXPORT DateTimeGrid : public AbstractGrid {
        Q_OBJECT
        KDGANTT_DECLARE_PRIVATE_DERIVED( DateTimeGrid )
    public:
		enum Scale { ScaleAuto, ScaleHour, ScaleDay };
	
        DateTimeGrid();
        virtual ~DateTimeGrid();

        QDateTime startDateTime() const;
        void setStartDateTime( const QDateTime& dt );

        qreal dayWidth() const;
        void setDayWidth( qreal );

        void setWeekStart( Qt::DayOfWeek );
        Qt::DayOfWeek weekStart() const;

        void setFreeDays( const QSet<Qt::DayOfWeek>& fd );
        QSet<Qt::DayOfWeek> freeDays() const;

	void setScale( Scale s );
	Scale scale() const;

        bool rowSeparators() const;
        void setRowSeparators( bool enable );

        /*reimp*/ Span mapToChart( const QModelIndex& idx ) const;
        /*reimp*/ bool mapFromChart( const Span& span, const QModelIndex& idx,
                                     const QList<Constraint>& constraints=QList<Constraint>() ) const;
        /*reimp*/ void paintGrid( QPainter* painter, const QRectF& sceneRect, const QRectF& exposedRect,
                                  AbstractRowController* rowController = 0,
                                  QWidget* widget=0 );
        /*reimp*/ void paintHeader( QPainter* painter, const QRectF& headerRect, const QRectF& exposedRect,
                                    qreal offset, QWidget* widget=0 );

	protected:
		virtual void paintHourScaleHeader( QPainter* painter, const QRectF& headerRect, const QRectF& exposedRect,
                                    qreal offset, QWidget* widget=0 );
		virtual void paintDayScaleHeader( QPainter* painter, const QRectF& headerRect, const QRectF& exposedRect,
                                    qreal offset, QWidget* widget=0 );

    };
}

#endif /* KDGANTTDATETIMEGRID_H */

