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

#include <plasma/dataengine.h>
#include <plasma/widgets/scrollwidget.h>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kdatetime.h>

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
    ui();
    configChanged();
    updateEvents();
    updateUI();

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    m_timer->setInterval(1000);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateUI()));

    setBusy( true );
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
    m_numDays = cg.readEntry( "numDays", 7 );
}

void EventsApplet::updateEvents()
{
    QString source = "events:%2:%3";
    QDate currentDate = QDate::currentDate();
    QDate endDate = currentDate.addDays( m_numDays );

    source = source.arg( currentDate.toString( Qt::ISODate ) );
    source = source.arg( currentDate.toString( Qt::ISODate ) );

    kDebug() << "Connecting source:" << source;

    dataEngine( "calendar" )->connectSource( source, this, 6000 );
}

void EventsApplet::updateUI()
{
    setBusy( false );
    kDebug() << "updateing ui";

    // Clear the current layout...
    for ( int i = 0; i < m_scrollerLayout->count(); i++ ) {
        m_scrollerLayout->removeItem( m_scrollerLayout->itemAt( i ) );
    }

    // ... and repopulate it.
    QMapIterator<QString,EventWidget*> it( m_incidences );
    while ( it.hasNext() ) {
        it.next();
        m_scrollerLayout->addItem( it.value() );
    }
}

void EventsApplet::dataUpdated( QString source, Plasma::DataEngine::Data data )
{
    if ( source.startsWith( "events" ) ) { // ++insurance
        QHashIterator<QString,QVariant> it( data );
        while ( it.hasNext() ) {
            it.next();
            QVariantHash data = it.value().toHash();
            KDateTime sd = qVariantValue<KDateTime>( data[ "StartDate" ] );

            m_incidences[ sd.toString() ] = new EventWidget( data, this );
            // kDebug() << data;
        }

        // Wait for more data before updating UI. I like doing it
        // this way. QTimer::start() restarts the timer from the
        // beginning every time it starts, which is fun :)
        m_timer->start();
    }
}

#include "eventsapplet.moc"

