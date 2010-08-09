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
    updateSummaryUI();
    updateFullUI();
}

void EventWidget::setData( QVariantHash args )
{
    QVariant var;
    var = args[ "StartDate" ];
    if ( var.isValid() ) {
        m_startDate = qVariantValue<KDateTime>( var );

        // make the date reference this year
        QDate date = m_startDate.date();
        date = QDate( QDate::currentDate().year(), date.month(), date.day() );
        m_startDate = KDateTime( date, m_startDate.time() );

    }

    var = args[ "EndDate" ];
    if ( var.isValid() ) {
        m_endDate = qVariantValue<KDateTime>( var );

        // make the date reference this year
        QDate date = m_endDate.date();
        date = QDate( QDate::currentDate().year(), date.month(), date.day() );
        m_endDate = KDateTime( date, m_startDate.time() );
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
    // Create the master layout
    m_masterLayout = new QGraphicsLinearLayout( Qt::Vertical );

    // Create the layout where the summary resides
    m_summaryWidget = new QGraphicsWidget( this );
    QGraphicsLinearLayout* layout = new QGraphicsLinearLayout( Qt::Horizontal );
    m_summaryWidget->setLayout(layout);
    m_masterLayout->addItem(m_summaryWidget);

    // Create the icon
    m_icon = new Plasma::IconWidget( 0 );
    layout->addItem(m_icon);

    // Create the text
    m_summaryLabel = new Plasma::Label();
    layout->addItem(m_summaryLabel);

    // Create the time-'til
    m_timetil = new GradientProgressWidget( );
    layout->addItem(m_timetil);

    // Create the more info button
    m_moreInfoIcon = new Plasma::IconWidget( "arrow-down-double" );
    m_moreInfoIcon->setMaximumWidth(16);
    m_moreInfoIcon->setMaximumHeight(16);
    m_moreInfoIcon->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    layout->addItem(m_moreInfoIcon);
    layout->setAlignment(m_moreInfoIcon, Qt::AlignBottom|Qt::AlignHCenter);
    m_moreInfoIcon->setVisible(0); // Only shown when user mouses in.

    connect(m_moreInfoIcon, SIGNAL(clicked()), this, SLOT(toggleMoreInfo()));

    // XXX Create full view widget
    // Start with a layout
    m_fullViewWidget = new QGraphicsWidget( this );
    layout = new QGraphicsLinearLayout(Qt::Vertical);

    // Add description
    m_descriptionLabel = new Plasma::Label();
    layout->addItem(m_descriptionLabel);

    // Add date
    m_startDateLabel = new Plasma::Label();
    layout->addItem(m_startDateLabel);

    m_fullViewWidget->setLayout(layout);
    m_masterLayout->addItem(m_fullViewWidget);
    setLayout(m_masterLayout);
}

void EventWidget::mouseEnterEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    m_moreInfoIcon->setVisible(1);
}

void EventWidget::mouseLeaveEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    m_moreInfoIcon->setVisible(0);
}

void EventWidget::updateSummaryUI()
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
    m_summaryLabel->setText( m_summary );

    // Set the time-'til
    int numDays = 31;
    if (m_parent) {
        KConfigGroup config = qobject_cast<Plasma::Applet*>(m_parent)->config();
        numDays = config.readEntry("numDays",31);
    }

    int difference = m_startDate.daysTo( KDateTime( QDateTime::currentDateTime() ) );
    m_timetil->setEnd( numDays );
    m_timetil->setCurrent( numDays - difference );

    // kDebug() << "update ui" << icon << m_summary << difference;
}

void EventWidget::updateFullUI()
{
    // Set the description
    m_descriptionLabel->setText( m_description );

    // convert the date to a QString...
    QString text;
    if ( m_endDate.isValid() && m_startDate.isValid() ) {
        text = i18n( "%1 to %2");
        text = text.arg( m_startDate.toString(KDateTime::ISODate), m_endDate.toString(KDateTime::ISODate) );
    } else if ( m_startDate.isValid() ) {
        text = i18n("%1");
        text = text.arg( m_startDate.toString(KDateTime::ISODate) );
    } else if ( m_endDate.isValid() ) {
        text = i18n("Ends on %1");
        text = text.arg( m_endDate.toString(KDateTime::ISODate));
    } else {
        text = "";
    }

    // .. and set the text!
    m_startDateLabel->setText( text );

}

void EventWidget::toggleMoreInfo()
{
    if ( moreInfoVisible() ) {
        setMoreInfoVisible(false);
    } else {
        setMoreInfoVisible(true);
    }
}

void EventWidget::setMoreInfoVisible( bool visible )
{
    m_fullViewWidget->setVisible(visible);
    m_moreInfoVisible = visible;
}

bool EventWidget::moreInfoVisible()
{
    return m_moreInfoVisible;
}

QString EventWidget::summary()
{
    return m_summary;
}
