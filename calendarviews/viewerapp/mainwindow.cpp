/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
    Author: Kevin Krammer, krake@kdab.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "mainwindow.h"

#include "settings.h"

#include "agenda.h"
#include "agendaview.h"
#include "month/monthview.h"
#include "multiagenda/multiagendaview.h"
#include "timeline/timelineview.h"
#include "prefs.h"

#include <calendarsupport/collectionselection.h>

#include <Akonadi/Calendar/IncidenceChanger>
#include <KCalCore/Event>

#include <AkonadiCore/Collection>
#include <AkonadiCore/Control>

#include <KCheckableProxyModel>

using namespace Akonadi;
using namespace CalendarSupport;
using namespace EventViews;

MainWindow::MainWindow( const QStringList &viewNames )
  : QMainWindow(),
    mViewNames( viewNames ),
    mIncidenceChanger( 0 ),
    mSettings( 0 ),
    mViewPreferences( 0 )
{
  mUi.setupUi( this );
  mUi.tabWidget->clear();

  connect( mUi.addViewMenu, SIGNAL(triggered(QAction*)), this, SLOT(addViewTriggered(QAction*)) );

  Akonadi::Control::widgetNeedsAkonadi( this );

  setGeometry( 0, 0, 800, 600 );
  QMetaObject::invokeMethod( this, "delayedInit", Qt::QueuedConnection );
}

MainWindow::~MainWindow()
{
  delete mViewPreferences;
  delete mSettings;
}

void MainWindow::addView( const QString &viewName )
{
  EventView *eventView = 0;

  const KDateTime start = KDateTime::currentLocalDateTime().addDays( -1 );
  const KDateTime end = KDateTime::currentLocalDateTime().addDays( 1 );

  if ( viewName == QLatin1String( "agenda" ) ) {
    eventView = new AgendaView( start.date(), end.date(), true, false, this );
  } else if ( viewName == QLatin1String( "multiagenda" ) ) {
    eventView = new MultiAgendaView( this );
  } else if ( viewName == QLatin1String( "month" ) ) {
    eventView = new MonthView( MonthView::Visible, this );
  } else if ( viewName == QLatin1String( "timeline" ) ) {
    eventView = new TimelineView( this );
  }

  if ( eventView ) {
    eventView->setPreferences( *mViewPreferences );
    eventView->setCalendar( mCalendar );
    eventView->setIncidenceChanger( mIncidenceChanger );
    eventView->setDateRange( start, end );
    eventView->updateConfig();
    mUi.tabWidget->addTab( eventView, viewName );
  } else {
    qCritical() << "Cannot create view" << viewName;
  }
}

void MainWindow::delayedInit()
{
  // create our application settings
  mSettings = new Settings;

  // create view preferences so that matching values are retrieved from
  // application settings
  mViewPreferences = new PrefsPtr( new Prefs( mSettings ) );

  mCalendar = Akonadi::ETMCalendar::Ptr( new Akonadi::ETMCalendar() );
  KCheckableProxyModel *checkableProxy = mCalendar->checkableProxyModel();
  QItemSelectionModel *selectionModel = checkableProxy->selectionModel();

  CalendarSupport::CollectionSelection *collectionSelection = new CalendarSupport::CollectionSelection( selectionModel );
  EventViews::EventView::setGlobalCollectionSelection( collectionSelection );

  mIncidenceChanger = new IncidenceChanger( this );
  mCalendar->setCollectionFilteringEnabled( false );

  Q_FOREACH( const QString &viewName, mViewNames ) {
    addView( viewName );
  }
}

void MainWindow::addViewTriggered( QAction *action )
{
  QString viewName = action->text().toLower();
  viewName.remove( QLatin1Char( '&' ) );
  addView( viewName );
}


// kate: space-indent on; indent-width 2; replace-tabs on;
