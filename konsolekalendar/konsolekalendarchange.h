/*******************************************************************************
 * konsolekalendarchange.h                                                     *
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. *
 *                                                                             *
 * As a special exception, permission is given to link this program            *
 * with any edition of Qt, and distribute the resulting executable,            *
 * without including the source code for Qt in the source distribution.        *
 *                                                                             *
 ******************************************************************************/

#ifndef _KONSOLEKALENDARCHANGE_H_
#define _KONSOLEKALENDARCHANGE_H_

#include "konsolekalendarvariables.h"

/**
 * @file konsolekalendarchange.h
 * Provides the KonsoleKalendarChange class definition.
 */

namespace KCal
{
  /**
   * Class to manage the Event modification capability.
   * @author Tuukka Pasanen
   * @author Allen Winter
   */
  class KonsoleKalendarChange
  {
  public:

    /**
     * Constructs a KonsoleKalendarChange object from command line arguments.
     * @param vars is a KonsoleKalendarVariable object with Event information.
     */
    KonsoleKalendarChange( KonsoleKalendarVariables *vars );
    /**
     * Destructor
     */
    ~KonsoleKalendarChange();

    /**
     * Modify the Event.
     */
    bool changeEvent();

  private:

    /**
     * Print Event specs for dryrun and verbose options.
     * @param event is the Event to print.
     */
    void printSpecs( Event *event );
    /*
     * Print Event specs as provided from the command line arguments.
     */
    void printSpecs();

    /**
     * Variables are here
     */
    KonsoleKalendarVariables *m_variables;
  };

}
#endif
