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

#include "jobitem.h"

#include <QtGui/QPainter>
#include <QtGui/QGraphicsSceneHoverEvent>
#include <QtGui/QGraphicsSceneEvent>

#include <libkmobiletools/jobxp.h>

#include <KDE/KIconLoader>
#include <KDE/KColorScheme>
#include <KDE/KLocale>
#include <KDE/KDebug>
#include <KDE/KIconEffect>

JobItem::JobItem( KMobileTools::JobXP* job, QGraphicsItem* parent )
 : QGraphicsItem( parent )
{
    connect( job, SIGNAL(done(ThreadWeaver::Job*)),
             this, SLOT(jobSuccessful(ThreadWeaver::Job*)) );

    connect( job, SIGNAL(progressChanged(int)),
             this, SLOT(jobProgressChanged(int)) );

    m_firstPaint = true;
    m_progress = job->progress();
    m_hoverCancel = false;

    setAcceptsHoverEvents( true );
    setHandlesChildEvents( true );

    switch( job->jobType() ) {
        case KMobileTools::JobXP::fetchAddressbook:
            m_caption = i18n( "Fetching address book" );
            m_pixmap = KIconLoader::global()->loadIcon( "x-office-address-book",
                                                        KIconLoader::NoGroup,
                                                        KIconLoader::SizeMedium );
            break;

        case KMobileTools::JobXP::fetchInformation:
            m_caption = i18n( "Fetching phone information" );
            m_pixmap = KIconLoader::global()->loadIcon( "phone",
                                                        KIconLoader::NoGroup,
                                                        KIconLoader::SizeMedium );
            break;

        /// @todo add the left job type possibilities

        default:
            m_caption = i18n( "Unknown job" );
            m_pixmap = KIconLoader::global()->loadIcon( "system-run",
                                                        KIconLoader::NoGroup,
                                                        KIconLoader::SizeMedium );
            break;
    }
}


JobItem::~JobItem()
{
}

QRectF JobItem::boundingRect() const
{
    return m_boundingRect;
}

void JobItem::paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    //
    // prepare caption
    //
    QRectF textRect = painter->boundingRect( QRectF(), Qt::TextSingleLine, m_caption );
    // determine text color
    KColorScheme colorScheme( QPalette::Normal );
    QColor color = colorScheme.shade( colorScheme.foreground( KColorScheme::NormalText ),
                                      KColorScheme::LightShade );

    // draw pixmap
    QRectF pixmapRect = QRectF( textRect.width() / 2 - m_pixmap.width() / 2, 0,
                                m_pixmap.width(), m_pixmap.height() );
    painter->drawPixmap( pixmapRect.x(), pixmapRect.y(), m_pixmap );
    textRect.moveTop( m_pixmap.height() + 5 );

    // draw caption
    painter->setPen( color );
    painter->drawText( textRect, m_caption );

    // draw progress
    if( m_progress > 0 )
        painter->drawArc( pixmapRect, 90 * 16, -16 * 3.6 * m_progress );

    // draw cancel button
    QPixmap cancelPixmap = KIconLoader::global()->loadIcon( "dialog-cancel",
                                                            KIconLoader::NoGroup,
                                                            KIconLoader::SizeSmall );

    m_cancelRect = QRectF( pixmapRect.x() + pixmapRect.width() - cancelPixmap.width(),
                           pixmapRect.y() + pixmapRect.height() - cancelPixmap.height(),
                           cancelPixmap.width(),
                           cancelPixmap.height() );

    if( !m_hoverCancel )
        KIconEffect::semiTransparent( cancelPixmap );

    painter->drawPixmap( pixmapRect.x() + pixmapRect.width() - cancelPixmap.width(),
                         pixmapRect.y() + pixmapRect.height() - cancelPixmap.height(),
                         cancelPixmap );

    m_boundingRect.setTop( 0 );
    m_boundingRect.setBottom( textRect.height() + m_pixmap.height() + 5 );
    m_boundingRect.setLeft( 0 );
    m_boundingRect.setRight( textRect.width() );

    if( m_firstPaint ) {
        update();
        m_firstPaint = false;
    }
}

void JobItem::jobSuccessful( ThreadWeaver::Job* ) {
    emit removeItem( this );
}

void JobItem::jobProgressChanged( int progress ) {
    m_progress = progress;
    update();
}

bool JobItem::sceneEvent( QEvent* event ) {
    if( event->type() == QEvent::GraphicsSceneHoverMove ) {
        QGraphicsSceneHoverEvent* sceneEvent = dynamic_cast<QGraphicsSceneHoverEvent*>( event );
        QPointF pos = sceneEvent->pos();
        m_hoverCancel = m_cancelRect.contains( pos );

        update();
    }
    else if( event->type() == QEvent::GraphicsSceneMouseRelease ) {
        QGraphicsSceneMouseEvent* sceneEvent = dynamic_cast<QGraphicsSceneMouseEvent*>( event );
        if( sceneEvent->button() == Qt::LeftButton ) {
            QPointF pos = sceneEvent->pos();
            if( m_cancelRect.contains( pos ) )
                kDebug() << "triggering job cancellation...";
        }
    }

    return true;
}
#include "jobitem.moc"
