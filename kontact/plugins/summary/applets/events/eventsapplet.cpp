#ifndef EVENTSAPPLET_CPP
#define EVENTSAPPLET_CPP
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

#include "eventsapplet.h"
#include "eventwidget.h"
#include "ui_configui.h"

#include <plasma/dataengine.h>
#include <plasma/widgets/scrollwidget.h>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kdatetime.h>
#include <kconfigdialog.h>

#include <akonadi/collection.h>
#include <akonadi/collectionfetchjob.h>
#include <akonadi/collectionfetchscope.h>

#include <qtimer.h>
#include <qgraphicslinearlayout.h>

K_EXPORT_PLASMA_APPLET(events, EventsApplet)

EventsApplet::EventsApplet( QObject* parent, QVariantList args )
    : Plasma::Applet( parent, args ),
      m_timer( 0 ),
      m_scrollWidget( 0 ),
      m_scroller( 0 ),
      m_scrollerLayout( 0 )
{
}

void EventsApplet::init()
{
    setBusy( true );

    ui();
    configChanged();
    updateEvents();
    updateUI();

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    m_timer->setInterval(1000);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateUI()));
}

void EventsApplet::ui()
{
    // Start by setting up the Applet with a layout
    QGraphicsLinearLayout* lay = new QGraphicsLinearLayout(Qt::Vertical);
    setLayout(lay);

    // Create the scroll widget and set the desired size policy if different than the default
    // this ScrollWidget will handle the scrollbars as well actual scrolling, including
    // kinetic and gesture-based (e.g. flicking) scrolling for us!
    m_scrollWidget = new Plasma::ScrollWidget(this);
    m_scrollWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    // Create the QGraphicsWidget we will be scrolling across
    m_scroller = new QGraphicsWidget(m_scroller);
    m_scrollerLayout = new QGraphicsLinearLayout(Qt::Vertical, m_scroller);

    // Put that widget inside the ScrollWidget
    m_scrollWidget->setWidget(m_scroller);

    // add the scrollwidget to the top level layout of the Applet
    lay->addItem(m_scrollWidget);
}

void EventsApplet::configChanged()
{
    KConfigGroup cg = config();

    // This can be either "Todo", "Events" or "Agenda" (merged of both)
    m_incidenceType = cg.readEntry( "incidenceType", "Agenda" );
    m_numDays = cg.readEntry( "numDays", 31 );
}

void EventsApplet::updateEvents()
{
    dataEngine( "calendar" )->disconnectSource( m_source, this );


    m_source = "events:%1:%2";
    QDate currentDate = QDate::currentDate();
    QDate endDate = currentDate.addDays( m_numDays );

    m_source = m_source.arg( currentDate.toString( Qt::ISODate ) );
    m_source = m_source.arg( currentDate.toString( Qt::ISODate ) );

    kDebug() << "Connecting source:" << m_source;

    dataEngine( "calendar" )->connectSource( m_source, this );
}

void EventsApplet::updateUI()
{
    setBusy( false );

    // Clear the current layout...
    for ( int i = 0; i < m_scrollerLayout->count(); i++ ) {
        m_scrollerLayout->removeItem( m_scrollerLayout->itemAt( i ) );
    }

    // ... and repopulate it.
    QMapIterator<QString,EventWidget*> it( m_incidences );
    while ( it.hasNext() ) {
        it.next();
        m_scrollerLayout->addItem( it.value() );
        kDebug() << "adding" << it.value()->summary() << it.key();
    }
}

void EventsApplet::createConfigurationInterface( KConfigDialog *parent)
{
    KConfigGroup cg = config();

    m_configWidget = new QWidget();
    m_configUi.setupUi(m_configWidget);

    m_configUi.daysSpinBox->setValue(cg.readEntry("numDays", 7));

    QString incType = cg.readEntry("incidenceType", "Agenda");

    m_configUi.calendarCB->setChecked(true);
    m_configUi.todoCB->setChecked(true);
    if ( incType == "Todo" ) {
        m_configUi.calendarCB->setChecked( false );
    } else if ( incType == "Events" ) {
        m_configUi.todoCB->setChecked( false );
    }

    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

    parent->addPage(m_configWidget, i18n("Configuration"),
                    "view-pim-calendar",
                    i18n("Configure Events applet."));
}

void EventsApplet::configAccepted()
{
    KConfigGroup cg = config();

    cg.writeEntry( "numDays", m_configUi.daysSpinBox->value() );

    if ( m_configUi.calendarCB->isChecked() && m_configUi.todoCB->isChecked() ) {
        cg.writeEntry( "incidenceType", "Agenda" );
    } else if ( m_configUi.calendarCB->isChecked() ) {
        cg.writeEntry( "incidenceType", "Events" );
    } else if ( m_configUi.todoCB->isChecked() ) {
        cg.writeEntry( "incidenceType", "Todo" );
    }

    // Clear the incidences before updating to keep stale entries out.
    if ( m_numDays > m_configUi.daysSpinBox->value() ) {
        QMapIterator<QString,EventWidget*> it( m_incidences );
        while ( it.hasNext() ) {
            it.next();
            delete it.value();
            m_incidences.erase( it.key() );
        }
    }

    configChanged();
    updateEvents();
}

void EventsApplet::dataUpdated( QString source, Plasma::DataEngine::Data pimData )
{
    if ( source.startsWith( "events" ) ) { // ++insurance
        // Start by purging old data
        // m_incidences.clear();

        QHashIterator<QString,QVariant> it( pimData );
        while ( it.hasNext() ) {
            it.next();
            QVariantHash data = it.value().toHash();

            kDebug() << it.key();
            kDebug() << data;

            // Start by making sure it's a type we are looking for
            if ( data[ "Type" ].toString() == m_incidenceType || m_incidenceType == "Agenda" ) {
                KDateTime sd = data.value("StartDate").value<KDateTime>();

                int difference = 0;

                if( sd.isValid() ) {
                    QDate date = sd.date();
                    date = QDate( QDate::currentDate().year(), date.month(), date.day() );
                    sd = KDateTime( date, sd.time() );

                    // Since it seems like the calendar dataengine is broken and returning ALL data, let's
                    // filter it a little bit by hand... FIXME
                    difference = KDateTime::currentDateTime( sd.timeSpec() ).daysTo( sd ) % 365 ; // XXX different locales with different calendars?
                }

                if ( difference <= m_numDays && difference >= 0 ) {
                    QString key = sd.toString();
                    if (m_incidences[ key ]) {
                        int i = 0;
                        while ( i <= 255 ) {
                            if ( m_incidences[ key ]->summary() == data[ "Summary" ] ) { // Event already exists with same summary, 
                                                                                         // assume it's the same event
                                break;
                            }

                            i++;
                            key = sd.toString()+QString( i );

                            if (!m_incidences[ key ]) { // Found a unique ID, use it.
                                break;
                            }
                        }
                    }
                    kDebug() << "Adding" << data[ "Summary" ] << key;
                    if ( !m_incidences[ key ] ) {
                        EventWidget* widget = new EventWidget( data, this );
                        m_incidences[ key ] = widget;
                    } else {
                        m_incidences[ key ]->updateSummaryUI();
                        m_incidences[ key ]->updateFullUI();
                    }
                }
            } 
            // kDebug() << data;
        }

        // Wait for more data before updating UI. I like doing it
        // this way. QTimer::start() restarts the timer from the
        // beginning every time it starts, which is fun :)
        m_timer->start();
    }
}

#include "eventsapplet.moc"

#endif // EVENTSAPPLET_CPP

