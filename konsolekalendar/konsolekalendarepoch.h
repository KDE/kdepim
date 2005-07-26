/*******************************************************************************
 * konsolekalendarepoch.h                                                      *
 *                                                                             *
 * KonsoleKalendar is a command line interface to KDE calendars                *
 * Copyright (C) 2002-2004  Tuukka Pasanen <illuusio@mailcity.com>             *
 * Copyright (C) 2003-2005  Allen Winter <winter@kde.org>                      *
 *                                                                             *
 * This program is free software; you can redistribute it and/or modify        *
 * it under the terms of the GNU General Public License as published by        *
 * the Free Software Foundation; either version 2 of the License, or           *
 * (at your option) any later version.                                         *
 *                                                                             *
 * This program is distributed in the hope that it will be useful,             *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                *
 * GNU General Public License for more details.                                *
 *                                                                             *
 * You should have received a copy of the GNU General Public License           *
 * along with this program; if not, write to the Free Software                 *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. *
 *                                                                             *
 * As a special exception, permission is given to link this program            *
 * with any edition of Qt, and distribute the resulting executable,            *
 * without including the source code for Qt in the source distribution.        *
 *                                                                             *
 ******************************************************************************/

#ifndef _KONSOLEKALENDAREPOCH_H_
#define _KONSOLEKALENDAREPOCH_H_

#include <qdatetime.h>

/**
 * @file konsolekalendarepoch.h
 * Provides the KonsoleKalendarEpoch class definition.
 */

namespace KCal
{
  /**
   * Class for timestamps expressed as epochs.
   * @author Allen Winter
   */
  class KonsoleKalendarEpoch
  {
  public:
    /**
     * Constructor.
     */
    KonsoleKalendarEpoch();
    /**
     * Destructor
     */
    ~KonsoleKalendarEpoch();

    /**
     * Converts epoch time to QDateTime format.
     * @param epoch epoch time.
     */
    static QDateTime epoch2QDateTime( uint epoch );

    /**
     * Converts QT DateTime to epoch format.
     * @param dt is a QDateTime to convert to an epoch.
     */
    static uint QDateTime2epoch( QDateTime dt );

  };

}

#endif
