#ifndef _KONSOLEKALENDAREPOCH_H_
#define _KONSOLEKALENDAREPOCH_H_

/***************************************************************************
        konsolekalendarepoch.h  -  Epoch parsing routines
           -------------------
    begin                : Sun Jan 6 2003
    copyright            : (C) 2003 by Allen Winter
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
