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

#ifndef INCIDENCEEDITOR_GROUPWAREINTEGRATION_H
#define INCIDENCEEDITOR_GROUPWAREINTEGRATION_H

#include "incidenceeditors-ng_export.h"
#include <Akonadi/Calendar/ETMCalendar>

namespace Akonadi {
  class GroupwareUiDelegate;
}

namespace IncidenceEditorNG {

class INCIDENCEEDITORS_NG_EXPORT GroupwareIntegration
{
  public:
    static bool isActive();
    static void activate( const Akonadi::ETMCalendar::Ptr &calendar = Akonadi::ETMCalendar::Ptr() );
    static void setGlobalUiDelegate( Akonadi::GroupwareUiDelegate *delegate );

  private:
    static bool sActivated;
    static Akonadi::GroupwareUiDelegate *sDelegate;
};

}

#endif
