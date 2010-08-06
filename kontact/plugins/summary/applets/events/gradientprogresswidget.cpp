/*
 *   Copyright 2010 Ryan Rix <ry@n.rix.si>
 * 
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "gradientprogresswidget.h"

#include <plasma/applet.h>
#include <plasma/theme.h>

#include <QGradient>
#include <qbrush.h>
#include <qpainter.h>

GradientProgressWidget::GradientProgressWidget(int end, int current, QGraphicsWidget* parent)
    : QGraphicsWidget( parent ),
      m_end(end),
      m_current(current)
{
}

void GradientProgressWidget::setEnd( int end )
{
    m_end = end;
}

void GradientProgressWidget::setCurrent( int current )
{
    m_current = current;
}

void GradientProgressWidget::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    int halfHeight = size().height()/2;
    int halfWidth = size().width()/2;
    QPointF center = QPointF( halfWidth, halfHeight );

    QString text = QString("%1").arg(m_end-m_current);
    // Account for text offset and calculate a center for the text drawing
    QFontMetrics metrics = Plasma::Theme::defaultTheme()->fontMetrics();
    QSize textSize = metrics.size( Qt::TextSingleLine, text );
    QPointF textCenter = QPointF(center.x() - textSize.width()/2, center.y() + textSize.height()/3 );

    float progress = 1.0*m_current/m_end;
    if( progress < 0.0 ) {
        progress = 0.0;
    }

    QRadialGradient gradient(halfWidth, halfHeight, size().width()/3);
    gradient.setColorAt(0,QColor::fromRgbF(1,0,0, progress));
    gradient.setColorAt(1,QColor::fromRgbF(0,0,0,0));

    QBrush brush(gradient);

    painter->save();

    painter->setBrush(brush);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(center, size().width()/3, size().height()/3); // TODO: Decide which is better
    //painter->drawPie(QRectF(0,0,size().width(), size().height()), 4320, 16*(360*m_current/m_end));

    painter->setPen(Qt::white);
    painter->drawText(textCenter, text);

    painter->restore();
}

#include "gradientprogresswidget.moc"
