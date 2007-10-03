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

#include "jobqueueview.h"

#include <QtGui/QGraphicsScene>
#include <QtGui/QGradient>
#include <QtGui/QPainter>
#include <QtGui/QGraphicsTextItem>
#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsItemAnimation>
#include <QtCore/QTimeLine>

#include <KDE/KColorScheme>
#include <KDE/KLocale>

JobQueueView::JobQueueView( QObject* parent )
 : QGraphicsScene( parent )
{
    // create no jobs item
    m_noJobsItem = new QGraphicsTextItem( i18n( "There are no active jobs." ) );

    KColorScheme colorScheme( QPalette::Normal );
    QColor color = colorScheme.shade( colorScheme.foreground( KColorScheme::NormalText ),
                                      KColorScheme::LightShade );
    m_noJobsItem->setDefaultTextColor( color );
    addItem( m_noJobsItem );

    m_jobItemGroup = new QGraphicsItemGroup();
    addItem( m_jobItemGroup );
}


JobQueueView::~JobQueueView()
{
}

void JobQueueView::addJob( JobItem* jobItem )
{
    // hide "no jobs" label if there are active jobs
    if( !m_jobItems.size() )
        m_noJobsItem->hide();

    // create the animation for the job
    QTimeLine* timer = new QTimeLine( 1000 );
    timer->setFrameRange( 0, 25 );

    QGraphicsItemAnimation* animation = new QGraphicsItemAnimation;
    animation->setItem( jobItem );
    animation->setTimeLine( timer );

    if( m_jobItems.size() )
        jobItem->setPos( QPointF( m_jobItems.last()->pos().x() +
                                  m_jobItems.last()->boundingRect().width() + 10, 0 ) );
    else
        jobItem->setPos( QPointF( 10, 0 ) );

    animation->setScaleAt( 0, 0, 0 );
    animation->setScaleAt( 1, 1, 1 );

    for( int i=0; i<views().size(); i++ )
        views().at( i )->setAlignment( Qt::AlignLeft );

    jobItem->scale( 0, 0 );
    // add the new job to the group
    m_jobItemGroup->addToGroup( jobItem );
    m_jobItems.append( jobItem );

    // update scene rectangle
    setSceneRect( m_jobItemGroup->boundingRect() );

    connect( jobItem, SIGNAL(removeItem(JobItem*)),
             this, SLOT(removeJob(JobItem*)) );

    timer->start();
}

void JobQueueView::removeJob( JobItem* jobItem ) {
    // remove job item
    if( m_jobItems.contains( jobItem ) ) {
        // remove the item
        m_jobItems.removeAll( jobItem );
        m_jobItemGroup->removeFromGroup( jobItem );
        delete jobItem;

        // re-arrange the other items
        for( int i=0; i<m_jobItems.size(); i++ )
            m_jobItems.at( i )->moveBy( 10, 0 );
    }

    // if there are no active jobs, display "no jobs" label and update scene rectangle
    if( !m_jobItems.size() ) {
        m_noJobsItem->show();
        setSceneRect( m_noJobsItem->boundingRect() );
        for( int i=0; i<views().size(); i++ )
            views().at( i )->setAlignment( Qt::AlignCenter );
    } else
        setSceneRect( m_jobItemGroup->boundingRect() );
}

void JobQueueView::drawBackground( QPainter* painter, const QRectF& rect ) {
    QLinearGradient brush( 0, rect.top(), 0, rect.bottom() );
    KColorScheme colorScheme( QPalette::Normal );
    QColor color0 = colorScheme.shade( colorScheme.background( KColorScheme::NormalBackground ),
                                       KColorScheme::DarkShade );
    QColor color1 = colorScheme.shade( colorScheme.background( KColorScheme::NormalBackground ),
                                       KColorScheme::ShadowShade );

    brush.setColorAt( 0, color0 );
    brush.setColorAt( 1, color1 );

    painter->setBrush( brush );
    painter->drawRect( rect );
}



#include "jobqueueview.moc"
