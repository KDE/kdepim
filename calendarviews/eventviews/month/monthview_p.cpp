/*
  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Author: Bertjan Broeksema, broeksema@kde.org

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

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#include "monthview_p.h"

#include <calendarsupport/calendarsearch.h>
#include <calendarsupport/collectionselection.h>
#include <calendarsupport/collectionselectionproxymodel.h>

using namespace EventViews;

MonthViewPrivate::MonthViewPrivate( MonthView *qq )
  : EventViewPrivate( qq )
  , q( qq )
  , calendarSearch( new CalendarSupport::CalendarSearch( qq ) )
{ }

MonthViewPrivate::~MonthViewPrivate()
{ }

void MonthViewPrivate::setUpModels()
{
  EventViewPrivate::setUpModels();

  if ( collectionSelectionModel ) {
    calendarSearch->setSelectionModel( collectionSelectionModel->selectionModel() );
  } else {
    calendarSearch->setSelectionModel( sGlobalCollectionSelection->model() );
  }
#if 0
  QDialog *dlg = new QDialog( q );
  dlg->setModal( false );
  QVBoxLayout *layout = new QVBoxLayout( dlg );
  EntityTreeView *testview = new EntityTreeView( dlg );
  layout->addWidget( testview );
  testview->setModel( calendarSearch->model() );
  dlg->show();
#endif
}

void MonthViewPrivate::addIncidence( const Akonadi::Item &incidence )
{
  Q_UNUSED( incidence );
  //TODO: add some more intelligence here...
  q->reloadIncidences();
}

void MonthViewPrivate::moveStartDate( int weeks, int months )
{
  KDateTime start = q->startDateTime();
  KDateTime end = q->endDateTime();
  start = start.addDays( weeks * 7 );
  end = end.addDays( weeks * 7 );
  start = start.addMonths( months );
  end = end.addMonths( months );
  q->setDateRange( start, end );
}

void MonthViewPrivate::triggerDelayedReload()
{
//   if ( !mReloadTimer.isActive() ) {
//     mReloadTimer.start( 50 );
//   }
}
