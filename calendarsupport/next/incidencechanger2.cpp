/*
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (C) 2010 Sérgio Martins <iamsergio@gmail.com>

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
#include "incidencechanger.h"
#include "incidencechanger_p.h"
#include "calendar.h"

using namespace CalendarSupport;

IncidenceChanger2::IncidenceChanger2( CalendarSupport::Calendar *calendar ) : QObject(), d( new Private )
{

}

IncidenceChanger2::~IncidenceChanger2()
{
  delete d;
}

bool IncidenceChanger2::addIncidence( const KCalCore::Incidence::Ptr &incidence,
                                      const Akonadi::Collection &collection,
                                      uint atomicOperationId,
                                      QWidget *parent )
{
  Q_ASSERT_X( incidence, "addIncidence()", "Invalid incidences not allowed" );
}

bool IncidenceChanger2::deleteIncidence( const Akonadi::Item &item,
                                         uint atomicOperationId,
                                         QWidget *parent )
{
  // Too harsh?
  Q_ASSERT_X( item.isValid(), "deleteIncidence()", "Invalid items not allowed" );
}

uint IncidenceChanger2::startAtomicOperation()
{
  static uint latestAtomicOperationId = 0;
  return ++latestAtomicOperationId;
}

void IncidenceChanger2::endAtomicOperation( uint atomicOperationId )
{
  //d->mOperationStatus.remove( atomicOperationId );
}
