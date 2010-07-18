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

#include "incidenceeditor-ng.h"
#include "eventortododialog.h"
#include "../korganizereditorconfig.h"
#include "../editorconfig.h"

#include <kcalprefs.h>

#include <kcalcore/visitor.h>


#include <akonadi/kcal/calendar.h>  //krazy:exclude=camelcase since kdepim/akonadi
#include <akonadi/kcal/calendarmodel.h> //krazy:exclude=camelcase since kdepim/akonadi
#include <akonadi/kcal/groupware.h> //krazy:exclude=camelcase since kdepim/akonadi
#include <akonadi/kcal/utils.h>     //krazy:exclude=camelcase since kdepim/akonadi

#include <Akonadi/ChangeRecorder>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/Session>
#include <Akonadi/KCal/IncidenceMimeTypeVisitor>

#include <KSystemTimeZones>

using namespace KCalCore;
using namespace IncidenceEditorsNG;

class EditorDialogVisitor : public Visitor
{
  public:
    EditorDialogVisitor() : Visitor(), mEditor( 0 ) {}
    IncidenceEditor *editor() const { return mEditor; }

  protected:
  bool visit( Event::Ptr  )
    {
      // TODO-NGPORT implement correctly
      return true;
    }
    bool visit( Todo::Ptr  )
    {
      // TODO-NGPORT implement correctly
      return true;
    }
    bool visit( Journal::Ptr  )
    {
      // TODO-NGPORT implement correctly
      return true;
    }
    bool visit( FreeBusy::Ptr  ) // to inhibit hidden virtual compile warning
    {
      return false;
    }

    IncidenceEditor *mEditor;
};

class GroupwareUiDelegate : public QObject, public Akonadi::GroupwareUiDelegate
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
      monitor->setMimeTypeMonitored( Akonadi::IncidenceMimeTypeVisitor::eventMimeType(), true );
      monitor->setMimeTypeMonitored( Akonadi::IncidenceMimeTypeVisitor::todoMimeType(), true );
      monitor->setMimeTypeMonitored( Akonadi::IncidenceMimeTypeVisitor::journalMimeType(), true );

      Akonadi::CalendarModel *calendarModel = new Akonadi::CalendarModel( monitor, this );

      mCalendar = new Akonadi::Calendar( calendarModel, calendarModel,
                                         KSystemTimeZones::local() );
      mCalendar->setOwner( Person( KCalPrefs::instance()->fullName(),
                                   KCalPrefs::instance()->email() ) );
    }

    void requestIncidenceEditor( const Akonadi::Item &item )
    {
      const Incidence::Ptr incidence = Akonadi::incidence( item );
      if ( !incidence ) {
        return;
      }

      EditorDialogVisitor v;
      if ( !incidence->accept( v, incidence ) ) {
        return;
      }

      // TODO-NGPORT Is this correct?
      EventOrTodoDialog* editor= new EventOrTodoDialog;
      editor->load( item );
    }

    Akonadi::Calendar *mCalendar;
};

K_GLOBAL_STATIC( GroupwareUiDelegate, globalDelegate )

bool GroupwareIntegration::sActivated = false;

bool GroupwareIntegration::isActive()
{
  return sActivated;
}

void GroupwareIntegration::activate()
{
  IncidenceEditors::EditorConfig::setEditorConfig( new IncidenceEditors::KOrganizerEditorConfig );
  Akonadi::Groupware::create( globalDelegate->mCalendar, &*globalDelegate );
  sActivated = true;
}

