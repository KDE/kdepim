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

#include <libkmobiletools/jobxp.h>

#include <KDE/KIconLoader>
#include <KDE/KColorScheme>
#include <KDE/KLocale>

JobItem::JobItem( KMobileTools::JobXP* job, QGraphicsItem* parent )
 : QGraphicsItem( parent )
{
    connect( job, SIGNAL(done(ThreadWeaver::Job*)),
             this, SLOT(jobSuccessful(ThreadWeaver::Job*)) );

    m_firstPaint = true;

    switch( job->jobType() ) {
        case KMobileTools::JobXP::fetchAddressbook:
            m_caption = i18n( "Fetching address book" );
            m_pixmap = KIconLoader::global()->loadIcon( "book2",
                                                        KIconLoader::NoGroup,
                                                        KIconLoader::SizeMedium );
            break;

        case KMobileTools::JobXP::fetchInformation:
            m_caption = i18n( "Fetching phone information" );
            m_pixmap = KIconLoader::global()->loadIcon( "phone",
                                                        KIconLoader::NoGroup,
                                                        KIconLoader::SizeMedium );

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
    // draw caption
    //
    QRectF textRect = painter->boundingRect( QRectF(), Qt::TextSingleLine, m_caption );
    // determine text color
    KColorScheme colorScheme( QPalette::Normal );
    QColor color = colorScheme.shade( colorScheme.foreground( KColorScheme::NormalText ),
                                      KColorScheme::LightShade );

    // draw pixmap
    painter->drawPixmap( textRect.width() / 2 - m_pixmap.width() / 2, 0, m_pixmap );
    textRect.moveTop( m_pixmap.height() + 5 );
    painter->setPen( color );
    painter->drawText( textRect, m_caption );

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

#include "jobitem.moc"
