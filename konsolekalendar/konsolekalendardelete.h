/*******************************************************************************
 *   konsolekalendardelete.h                                                   *
 *                                                                             *
 *   KonsoleKalendar is a command line interface to KDE calendars              *
 *   Copyright (C) 2002-2004  Tuukka Pasanen <illuusio@mailcity.com>           *
 *   Copyright (C) 2003-2004  Allen Winter <awinterz@users.sourceforge.net>    *
 *                                                                             *
 *   This library is free software; you can redistribute it and/or             *
 *   modify it under the terms of the GNU Lesser General Public                *
 *   License as published by the Free Software Foundation; either              *
 *   version 2.1 of the License, or (at your option) any later version.        *
 *                                                                             *
 *   This library is distributed in the hope that it will be useful,           *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 *   Lesser General Public License for more details.                           *
 *                                                                             *
 *   You should have received a copy of the GNU Lesser General Public          *
 *   License along with this library; if not, write to the Free Software       *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA *
 *                                                                             *
 ******************************************************************************/

#ifndef _KONSOLEKALENDARDELETE_H_
#define _KONSOLEKALENDARDELETE_H_

#include "konsolekalendarvariables.h"

namespace KCal
{

  class KonsoleKalendarDelete
  {
  public:
    KonsoleKalendarDelete( KonsoleKalendarVariables *vars );
    ~KonsoleKalendarDelete();

    /**
     * Delete event
     */
    bool deleteEvent();

  private:

    /**
     * Print event specs for dryrun and verbose options
     */
    void printSpecs( Event *event );

    /**
     * What we need to delete
     */
    KonsoleKalendarVariables *m_variables;
  };

}
#endif
