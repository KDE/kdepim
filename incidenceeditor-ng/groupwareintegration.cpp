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

#include <calendarsupport/utils.h>
#include <calendarsupport/calendarsingleton.h>

#include <Akonadi/Calendar/ITIPHandler>

#include <KSystemTimeZones>

using namespace IncidenceEditorNG;

namespace IncidenceEditorNG {

class GroupwareUiDelegate : public QObject, public Akonadi::GroupwareUiDelegate
{
  public:
    GroupwareUiDelegate()
      : mCalendar( 0 )
    {
    }

    void setCalendar( const Akonadi::ETMCalendar::Ptr &calendar )
    {
      mCalendar = calendar;
    }

    void createCalendar()
    {
      QStringList mimeTypes;
      mimeTypes << KCalCore::Event::eventMimeType() << KCalCore::Todo::todoMimeType();
      mCalendar = CalendarSupport::calendarSingleton();
    }

    void requestIncidenceEditor( const Akonadi::Item &item )
    {
#ifndef KDEPIM_MOBILE_UI
      const KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( item );
      if ( !incidence ) {
        kWarning() << "Incidence is null, won't open the editor";
        return;
      }

      IncidenceEditorNG::IncidenceDialog *dialog =
        IncidenceEditorNG::IncidenceDialogFactory::create( /*needs initial saving=*/ false,
                                                           incidence->type(), 0 );
      dialog->setIsCounterProposal( true );
      dialog->load( item, QDate::currentDate() );
#else
      Q_UNUSED( item );
#endif
    }

    Akonadi::ETMCalendar::Ptr mCalendar;
};

}

Akonadi::GroupwareUiDelegate *GroupwareIntegration::sDelegate = 0;

bool GroupwareIntegration::sActivated = false;

bool GroupwareIntegration::isActive()
{
  return sActivated;
}

void GroupwareIntegration::activate( const Akonadi::ETMCalendar::Ptr &calendar )
{
  if ( !sDelegate ) {
    sDelegate = new GroupwareUiDelegate;
  }

  EditorConfig::setEditorConfig( new KOrganizerEditorConfig );
  //Akonadi::Groupware::create( sDelegate ); TODO_SERGIO
  if ( calendar ) {
    sDelegate->setCalendar( calendar );
  } else {
    sDelegate->createCalendar();
  }
  sActivated = true;
}

void GroupwareIntegration::setGlobalUiDelegate( Akonadi::GroupwareUiDelegate *delegate )
{
  delete sDelegate;
  sDelegate = delegate;
}
