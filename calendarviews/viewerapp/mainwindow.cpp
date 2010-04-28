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
#include "prefs.h"

#include <akonadi/kcal/calendar.h>
#include <akonadi/kcal/calendarmodel.h>
#include <akonadi/kcal/incidencechanger.h>
#include <akonadi/kcal/incidencemimetypevisitor.h>

#include <akonadi/changerecorder.h>
#include <akonadi/collection.h>
#include <akonadi/control.h>
#include <akonadi/entitydisplayattribute.h>
#include <akonadi/entitymimetypefiltermodel.h>
#include <akonadi/itemfetchscope.h>

#include <KSystemTimeZones>

using namespace Akonadi;
using namespace EventViews;

MainWindow::MainWindow( const QStringList &viewNames )
  : QMainWindow(),
    mViewNames( viewNames ),
    mChangeRecorder( 0 ),
    mCalendar( 0 ),
    mIncidenceChanger( 0 ),
    mSettings( 0 ),
    mViewPreferences( 0 )
{
  mUi.setupUi( this );
  mUi.tabWidget->clear();

  connect( mUi.addViewMenu, SIGNAL( triggered( QAction* ) ), this, SLOT( addViewTriggered( QAction* ) ) );

  Akonadi::Control::widgetNeedsAkonadi( this );

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

  if ( viewName == QLatin1String( "agenda" ) ) {
    eventView = new AgendaView( this );
  }

  if ( eventView != 0 ) {
    eventView->setPreferences( *mViewPreferences );
    eventView->setCalendar( mCalendar );
    eventView->setIncidenceChanger( mIncidenceChanger );
    eventView->setDateRange( KDateTime::currentLocalDateTime().addDays( -1 ),
                             KDateTime::currentLocalDateTime().addDays( 1 ) );
    eventView->updateConfig();
    mUi.tabWidget->addTab( eventView, viewName );
  } else {
    kError() << "Cannot create view" << viewName;
  }
}

void MainWindow::delayedInit()
{
  // create our application settings
  mSettings = new Settings;

  // create view preferences so that matching values are retrieved from
  // application settings
  mViewPreferences = new PrefsPtr( new Prefs( mSettings ) );

  mChangeRecorder = new ChangeRecorder( this );
  mChangeRecorder->setCollectionMonitored( Collection::root(), true );

  ItemFetchScope scope;
  scope.fetchFullPayload( true );
  scope.fetchAttribute<EntityDisplayAttribute>();

  mChangeRecorder->fetchCollection( true );
  mChangeRecorder->setItemFetchScope( scope );

  mChangeRecorder->setMimeTypeMonitored( IncidenceMimeTypeVisitor::eventMimeType(), true );

  CalendarModel* calendarModel = new CalendarModel( mChangeRecorder, this );

  // no collections, just items
  calendarModel->setCollectionFetchStrategy( EntityTreeModel::InvisibleFetch );

  EntityMimeTypeFilterModel *filterModel = new EntityMimeTypeFilterModel( this );
  filterModel->setHeaderGroup( EntityTreeModel::ItemListHeaders );
  filterModel->setSourceModel( calendarModel );
  filterModel->setSortRole( CalendarModel::SortRole );

  mCalendar = new Akonadi::Calendar( calendarModel, filterModel, KSystemTimeZones::local() );

  mIncidenceChanger = new IncidenceChanger( mCalendar, this, Collection() );

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

#include "mainwindow.moc"

// kate: space-indent on; indent-width 2; replace-tabs on;
