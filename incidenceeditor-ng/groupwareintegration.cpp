/*
  Copyright (c) 2010 Kevin Ottens <ervin@kde.org>

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

#include "groupwareintegration.h"
#include "editorconfig.h"
#include "incidencedialog.h"
#include "incidencedialogfactory.h"
#include "korganizereditorconfig.h"

#include <kcalprefs.h>

#include <calendarsupport/calendar.h>
#include <calendarsupport/calendarmodel.h>
#include <calendarsupport/groupware.h>
#include <calendarsupport/utils.h>

#include <Akonadi/ChangeRecorder>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/Session>

#include <KSystemTimeZones>

using namespace KCalCore;
using namespace IncidenceEditorNG;

class GroupwareUiDelegate : public QObject, public CalendarSupport::GroupwareUiDelegate
{
  public:
    GroupwareUiDelegate()
    {
      Akonadi::Session *session = new Akonadi::Session( "GroupwareIntegration", this );
      Akonadi::ChangeRecorder *monitor = new Akonadi::ChangeRecorder( this );

      Akonadi::ItemFetchScope scope;
      scope.fetchFullPayload( true );

      monitor->setSession( session );
      monitor->setCollectionMonitored( Akonadi::Collection::root() );
      monitor->fetchCollection( true );
      monitor->setItemFetchScope( scope );
      monitor->setMimeTypeMonitored( KCalCore::Event::eventMimeType(), true );
      monitor->setMimeTypeMonitored( KCalCore::Todo::todoMimeType(), true );
      monitor->setMimeTypeMonitored( KCalCore::Journal::journalMimeType(), true );

      CalendarSupport::CalendarModel *calendarModel =
        new CalendarSupport::CalendarModel( monitor, this );

      mCalendar = new CalendarSupport::Calendar( calendarModel, calendarModel,
                                                 KSystemTimeZones::local() );
      mCalendar->setOwner( Person( CalendarSupport::KCalPrefs::instance()->fullName(),
                                   CalendarSupport::KCalPrefs::instance()->email() ) );
    }

    void requestIncidenceEditor( const Akonadi::Item &item )
    {
      const Incidence::Ptr incidence = CalendarSupport::incidence( item );
      if ( !incidence ) {
        return;
      }

      IncidenceEditorNG::IncidenceDialog *dialog =
        IncidenceEditorNG::IncidenceDialogFactory::create( incidence->type() );
      dialog->setIsCounterProposal( true );
      dialog->load( item, QDate::currentDate() );
    }

    CalendarSupport::Calendar *mCalendar;
};

K_GLOBAL_STATIC( GroupwareUiDelegate, globalDelegate )

bool GroupwareIntegration::sActivated = false;

bool GroupwareIntegration::isActive()
{
  return sActivated;
}

void GroupwareIntegration::activate()
{
  EditorConfig::setEditorConfig( new KOrganizerEditorConfig );
  CalendarSupport::Groupware::create( globalDelegate->mCalendar, &*globalDelegate );
  sActivated = true;
}

