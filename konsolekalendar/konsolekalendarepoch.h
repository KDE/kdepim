/********************************************************************************
 *   konsolekalendarepoch.h                                                     *
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
 
#ifndef _KONSOLEKALENDAREPOCH_H_
#define _KONSOLEKALENDAREPOCH_H_

#include <qdatetime.h>
#include <qstring.h>

#include <iostream>

namespace KCal {

class KonsoleKalendarEpoch
{
  public:
    KonsoleKalendarEpoch();
    ~KonsoleKalendarEpoch();

  /**
   * Converts epoxh time to QT DateTime format
   *
   * @param epoch epoch time
   */
  
  static QDateTime epoch2QDateTime( uint epoch );

  /**
   * Converts QT DateTime to epoch format
   *
   * @param epoch epoch time
   */
  
  static uint QDateTime2epoch( QDateTime dt );

};

}

#endif
