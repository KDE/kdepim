/***************************************************************************
   Copyright (C) 2007
   by Matthias Lechner <matthias@lmme.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef JOBQUEUEVIEW_H
#define JOBQUEUEVIEW_H

#include <QtGui/QGraphicsScene>

#include "jobitem.h"

class QGraphicsTextItem;
/**
 * @author Matthias Lechner <matthias@lmme.de>
 */
class JobQueueView : public QGraphicsScene
{
    Q_OBJECT
public:
    JobQueueView( QObject* parent = 0 );
    ~JobQueueView();

    void addJob( JobItem* jobItem );

protected:
    void drawBackground( QPainter* painter, const QRectF& rect );

private Q_SLOTS:
    void removeJob( JobItem* jobItem );

private:
    QList<JobItem*> m_jobItems;
    QGraphicsTextItem* m_noJobsItem;
    QGraphicsItemGroup* m_jobItemGroup;
};

#endif
