/*
  Copyright (C) 2010 Bertjan Broeksema <broeksema@kde.org>

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

// NOTE: The code of the following methods is taken from
//       kdepim/korganizer/calendarview.cpp:
//       - makeIndependent (was incidence_unsub)
//       - makeChildrenIndependent

#include "calendarutils.h"

#include "calendar.h"
#include "incidencechanger.h"
#include "utils.h"

#include <KCalCore/Incidence>

using namespace Akonadi;
using namespace CalendarSupport;
using namespace KCalCore;

/// CalendarUtilsPrivate

namespace CalendarSupport {

struct CalendarUtilsPrivate
{
  /// Methods
  CalendarUtilsPrivate( Calendar *calendar, CalendarUtils *qq );

  /// Members
  Calendar         *mCalendar;
  IncidenceChanger *mChanger;

private:
  CalendarUtils * const q_ptr;
  Q_DECLARE_PUBLIC( CalendarUtils )
};

}

CalendarUtilsPrivate::CalendarUtilsPrivate( Calendar *calendar, CalendarUtils *qq )
  : mCalendar( calendar )
  , mChanger( new IncidenceChanger( calendar, qq ) )
  , q_ptr( qq )
{
  Q_ASSERT( mCalendar );
}

/// CalendarUtils

CalendarUtils::CalendarUtils( Calendar *calendar, QObject *parent)
  : QObject( parent )
  , d_ptr( new CalendarUtilsPrivate( calendar, this ) )
{
  Q_ASSERT( calendar );
}

CalendarUtils::~CalendarUtils()
{
  delete d_ptr;
}

Calendar *CalendarUtils::calendar() const
{
  Q_D( const CalendarUtils );
  return d->mCalendar;
}

bool CalendarUtils::makeIndependent( const Akonadi::Item &item )
{
  Q_D( CalendarUtils );
  const Incidence::Ptr inc = CalendarSupport::incidence( item );

  if ( !inc || inc->relatedTo().isEmpty() ) {
    return false;
  }

  Incidence::Ptr oldInc( inc->clone() );
  inc->setRelatedTo( 0 );
  // HACK: This is not a widget, so pass 0 for now
  d->mChanger->changeIncidence( oldInc, item, CalendarSupport::IncidenceChanger::RELATION_MODIFIED, 0 );

  return true;
}

bool CalendarUtils::makeChildrenIndependent( const Akonadi::Item &item )
{
  Q_D( CalendarUtils );
  const Incidence::Ptr inc = CalendarSupport::incidence( item );

  Akonadi::Item::List subIncs = d->mCalendar->findChildren( item );

  if ( !inc || subIncs.isEmpty() ) {
    return false;
  }
  //startMultiModify ( i18n( "Make sub-to-dos independent" ) );

  foreach( const Akonadi::Item &item, subIncs ) {
    makeIndependent( item );
  }

  //endMultiModify();
  return true;
}
