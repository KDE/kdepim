/*
 *   Copyright 2010 Ryan Rix <ry@n.rix.si>
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

#include "special_dates.h"

#include <Plasma/Svg>
#include <Plasma/Theme>
#include <Plasma/DataEngine>

#include <QPainter>
#include <QFontMetrics>
#include <QSizeF>
#include <QGraphicsLinearLayout>
#include <QDate>

K_EXPORT_PLASMA_APPLET(special_dates, SpecialDatesApplet)

SpecialDatesApplet::SpecialDatesApplet( QObject* parent, QVariantList args )
    : Plasma::Applet( parent, args ),
      m_svg(this),
      m_numDays(7)
{
    Q_UNUSED(args);
    
    m_svg.setImagePath("widgets/background");
    setBackgroundHints(DefaultBackground);
}

void SpecialDatesApplet::init()
{
    m_layout = new QGraphicsLinearLayout(Qt::Vertical, this);
    m_engine = dataEngine("calendar");
    
    // Load configuration
    configChanged();
    
    updateSpecialDates();
    
}

void SpecialDatesApplet::paintInterface(QPainter* p,
                                     const QStyleOptionGraphicsItem* option, const QRect& contentsRect)
{
    p->setRenderHint(QPainter::SmoothPixmapTransform);
    p->setRenderHint(QPainter::Antialiasing);
    
    // Now we draw the applet, starting with our svg
    m_svg.resize((int)contentsRect.width(), (int)contentsRect.height());
    m_svg.paint(p, (int)contentsRect.left(), (int)contentsRect.top());
    
    p->save();
    p->setPen(Qt::white);
    p->drawText(contentsRect,
                Qt::AlignBottom | Qt::AlignHCenter,
                "Hello Plasmoid!");
    p->restore();
    
}

void SpecialDatesApplet::configChanged()
{
    m_numDays = config().readEntry("numDays").toInt();
    m_locale = config().readEntry("locale", "us_en-us"); // TODO default to system locale?
}

void SpecialDatesApplet::updateSpecialDates()
{
    // construct the DataEngine query
    QDate date = QDate::currentDate();
    
    QString query = QString("holidays:%1:%2:%3");
    query = query.arg(m_locale, date.toString("yyyy-MM-dd"), date.addDays(m_numDays).toString("yyyy-MM-dd"));
    
    kDebug() << "Query DataSource" << query;
    
    
}

#include "special_dates.moc"