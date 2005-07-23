/*******************************************************************************
 * konsolekalendardelete.h                                                     *
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

#ifndef _KONSOLEKALENDARDELETE_H_
#define _KONSOLEKALENDARDELETE_H_

#include "konsolekalendarvariables.h"

/**
 * @file konsolekalendardelete.h
 * Provides the KonsoleKalendarDelete class definition.
 */

namespace KCal
{
  /**
   * Class to manage the Event removal capability.
   * @author Tuukka Pasanen
   * @author Allen Winter
   */
  class KonsoleKalendarDelete
  {
  public:

    /**
     * Constructs a KonsoleKalendarDelete object from command line arguments.
     */
    KonsoleKalendarDelete( KonsoleKalendarVariables *vars );
    /**
     * Destructor
     */
    ~KonsoleKalendarDelete();

    /**
     * Delete the Event.
     */
    bool deleteEvent();

  private:

    /**
     * Print event specs for dryrun and verbose options.
     */
    void printSpecs( Event *event );

    /**
     * What we need to delete
     */
    KonsoleKalendarVariables *m_variables;
  };

}
#endif
