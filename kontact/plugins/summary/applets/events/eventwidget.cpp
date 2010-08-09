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

#include "eventwidget.h"
#include "gradientprogresswidget.h"

#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/Applet>

#include <kdatetime.h>
#include <kicon.h>

#include <qgraphicslinearlayout.h>


EventWidget::EventWidget( QVariantHash args, QGraphicsWidget* parent )
    : Plasma::Frame( parent ),
      m_parent( parent )
{
    setData( args );
    initUI();
    updateUI();
}

void EventWidget::setData( QVariantHash args )
{
    QVariant var;
    var = args[ "StartDate" ];
    if ( var.isValid() ) {
        m_date = qVariantValue<KDateTime>( var );

        // make the date reference this year.
        QDateTime day = QDateTime(QDate( QDate::currentDate().year(), m_date.date().month(), m_date.date().day() ) );

        m_date = KDateTime( day );
    }

    var = args[ "Summary" ];
    if ( var.isValid() ) {
        m_summary = qVariantValue<QString>( var );
    }

    var = args[ "Type" ];
    if ( var.isValid() ) {
        m_type = qVariantValue<QString>( var );
    }

    var = args[ "AllDay" ];
    if ( var.isValid() ) {
        m_allDay = qVariantValue<bool>( var );
    }
}

void EventWidget::initUI()
{
    // Create the layout
    m_layout = new QGraphicsLinearLayout( Qt::Horizontal );

    // Create the icon
    m_icon = new Plasma::IconWidget( 0 );
    m_layout->addItem(m_icon);

    // Create the text
    m_text = new Plasma::Label();
    m_layout->addItem(m_text);

    // Create the time-'til
    m_timetil = new GradientProgressWidget( );
    m_layout->addItem(m_timetil);

    // Create the more info button
    m_moreInfoIcon = new Plasma::IconWidget( KIcon("arrow-down-double") );
    m_moreInfoIcon->setMaximumWidth(16);
    m_moreInfoIcon->setMaximumHieght(16);
    m_moreInfoIcon->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    m_layout->addItem(m_moreInfoIcon);
    m_layout->setAlignment(m_moreInfoIcon, Qt::AlignBottom|Qt::AlignHCenter);
    m_moreInfoIcon->setVisible(0);
}

void EventWidget::updateUI()
{
    // Set the icon
    KIcon icon;
    if ( m_type == "Event" ) {
        icon = KIcon( "view-calendar" );
    } else if ( m_type == "Todo" ) {
        icon = KIcon( "view-calendar-tasks" );
    }
    m_icon->setIcon( icon );

    // Set the text
    m_text->setText( m_summary );

    // Set the time-'til
    KConfigGroup config = qobject_cast<Plasma::Applet*>(m_parent)->config();
    int numDays = config.readEntry("numDays",31);

    int difference = m_date.daysTo( KDateTime( QDateTime::currentDateTime() ) );
    m_timetil->setEnd( numDays );
    m_timetil->setCurrent( difference );

    kDebug() << "update ui" << icon << m_summary << difference;
}

