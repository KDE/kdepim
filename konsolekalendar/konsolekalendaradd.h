/********************************************************************************
 *   konsolekalendaradd.h                                                       *
 *                                                                              *
 *   KonsoleKalendar is console frontend to calendar                            *
 *   Copyright (C) 2002-2004  Tuukka Pasanen <illuusio@mailcity.com>            * 
 *   Copyright (C) 2003-2004  Allen Winter                                      *
 *                                                                              *
 *   This library is free software; you can redistribute it and/or              * 
 *   modify it under the terms of the GNU Lesser General Public                 *
 *   License as published by the Free Software Foundation; either               *
 *   version 2.1 of the License, or (at your option) any later version.         *
 *                                                                              *
 *   This library is distributed in the hope that it will be useful,            * 
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU          *
 *   Lesser General Public License for more details.                            *
 *                                                                              *
 *   You should have received a copy of the GNU Lesser General Public           *
 *   License along with this library; if not, write to the Free Software        *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA  * 
 *                                                                              *
 ********************************************************************************/
 
#ifndef _KONSOLEKALENDARADD_H_
#define _KONSOLEKALENDARADD_H_


#include "konsolekalendarvariables.h"
namespace KCal
{

class KonsoleKalendarAdd
{
public:
    KonsoleKalendarAdd( KonsoleKalendarVariables *variables );
    ~KonsoleKalendarAdd();

  /**
   * Adds one event
   */

   bool addEvent();

   /**
    * Imports calendar file to Current calendar
    */

   bool addImportedCalendar();


private:

   /**
    * Print event specs for dryrun and verbose options
    */
   void printSpecs();

   /**
    * Variable to how to make it
    */

   KonsoleKalendarVariables *m_variables;
   
};

}
#endif
