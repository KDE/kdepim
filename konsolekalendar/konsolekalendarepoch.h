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

  static QDateTime epoch2QDateTime( uint epoch );
  static uint QDateTime2epoch( QDateTime dt );

};

}

#endif
