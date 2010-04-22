/*
  This file is part of KOrganizer.

  Copyright (C) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef HELPER_H
#define HELPER_H

#include <KCal/Incidence>

#include <QColor>

namespace Akonadi {
  class Collection;
  class Item;
}

class QDate;

// Provides static methods that are useful to all views.

namespace Helper
{
  /**
    Returns a nice QColor for text, give the input color &c.
  */
  QColor getTextColor( const QColor &c );

  /**
    This method returns the proper resource / subresource color for the view.
    @return The resource color for the incidence. If the incidence belongs
    to a subresource, the color for the subresource is returned (if set).
    @param calendar the calendar for which the resource color should be obtained
    @param incidence the incidence for which the color is needed (to
                     determine which  subresource needs to be used)
  */
  QColor resourceColor( const Akonadi::Item & incidence );

  QColor resourceColor( const Akonadi::Collection &collection );

  /**
    Returns the number of years between the @p start QDate and the @p end QDate
  */
  qint64 yearDiff( const QDate &start, const QDate &end );


  /**
    Return true if it's the standard calendar
  */
  bool isStandardCalendar( const Akonadi::Collection &collection );
}

#endif
