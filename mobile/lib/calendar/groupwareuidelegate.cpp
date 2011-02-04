/*
    Copyright (c) 2011 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "groupwareuidelegate.h"

#include "incidenceview.h"

#include <akonadi/changerecorder.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/session.h>
#include <calendarsupport/calendar.h>
#include <calendarsupport/calendarmodel.h>
#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/utils.h>
#include <klocale.h>
#include <ksystemtimezone.h>

GroupwareUiDelegate::GroupwareUiDelegate()
  : mCalendar( 0 )
{
}

void GroupwareUiDelegate::setCalendar( CalendarSupport::Calendar *calendar )
{
  mCalendar = calendar;
}

void GroupwareUiDelegate::createCalendar()
{
  Akonadi::Session *session = new Akonadi::Session( "GroupwareIntegration", this );
  Akonadi::ChangeRecorder *monitor = new Akonadi::ChangeRecorder( this );

  Akonadi::ItemFetchScope scope;
  scope.fetchFullPayload( true );

  monitor->setSession( session );
  monitor->setCollectionMonitored( Akonadi::Collection::root() );
  monitor->fetchCollection( true );
  monitor->setItemFetchScope( scope );
  monitor->setMimeTypeMonitored( "text/calendar" );
  monitor->setMimeTypeMonitored( KCalCore::Event::eventMimeType(), true );
  monitor->setMimeTypeMonitored( KCalCore::Todo::todoMimeType(), true );
  monitor->setMimeTypeMonitored( KCalCore::Journal::journalMimeType(), true );

  CalendarSupport::CalendarModel *calendarModel =
    new CalendarSupport::CalendarModel( monitor, this );
  calendarModel->setObjectName( "Groupware calendar model" );

  mCalendar = new CalendarSupport::Calendar( calendarModel, calendarModel,
                                             KSystemTimeZones::local() );
  mCalendar->setObjectName( "Groupware calendar" );
  mCalendar->setOwner( KCalCore::Person( CalendarSupport::KCalPrefs::instance()->fullName(),
                                         CalendarSupport::KCalPrefs::instance()->email() ) );
}

void GroupwareUiDelegate::requestIncidenceEditor( const Akonadi::Item &item )
{
  const KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( item );
  if ( !incidence ) {
    kWarning() << "Incidence is null, won't open the editor";
    return;
  }

  IncidenceView *editor = new IncidenceView;
  editor->setWindowTitle( i18n( "KDE Calendar" ) );
  editor->load( item, QDate::currentDate() );

  editor->setIsCounterProposal( true );
  editor->show();
}
