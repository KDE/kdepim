/*******************************************************************************
 * konsolekalendarepoch.cpp                                                    *
 *                                                                             *
 * KonsoleKalendar is a command line interface to KDE calendars                *
 * Copyright (C) 2002-2004  Tuukka Pasanen <illuusio@mailcity.com>             *
 * Copyright (C) 2003-2004  Allen Winter <winter@kde.org>                      *
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

#include <stdlib.h>
#include <iostream>

#include <qdatetime.h>
#include "konsolekalendarepoch.h"

using namespace KCal;
using namespace std;

KonsoleKalendarEpoch::KonsoleKalendarEpoch( )
{
}

KonsoleKalendarEpoch::~KonsoleKalendarEpoch()
{
}

// By "epoch" we mean the number of seconds since 00:00:00 UTC on January 1 1970

// Function to convert an epoch value into a QDateTime
QDateTime KonsoleKalendarEpoch::epoch2QDateTime( uint epoch )
{
  QDateTime dt;
  dt.setTime_t (epoch,Qt::UTC);
  return (dt);
}

// Function to convert a QDateTime value into an epoch
uint KonsoleKalendarEpoch::QDateTime2epoch( QDateTime dt )
{
  // THIS FUNCTION CAN BE OFF BY 1 HOUR DUE TO DAYLIGHT SAVINGS TIME.
  // SORRY QT DOESN'T HANDLE DAYLIGHT SAVINGS TIME.

  // Compute #seconds to subtract for local timezone difference from UTC.
  int offset = QDateTime::currentDateTime(Qt::UTC).toTime_t()
               - QDateTime::currentDateTime(Qt::LocalTime).toTime_t();
  return(dt.toTime_t()-offset);
}

#if defined (TEST)
// Pass -DTEST to the compile command to create the test program, e.g:
// cc -DTEST -I/usr/local/KDE/include  konsolekalendarepoch.cpp
//           -L/usr/local/KDE/lib -lqt-mt -pthread
main()
{
  uint epoch;
  QDateTime dt;

  cout << endl;
  cout << "NOTE: Some tests may be off by 1 hour (3600 secs) "
       << "due to daylight savings time"
       << endl << endl;

  // Test1
  epoch=0;
  dt = KonsoleKalendarEpoch::epoch2QDateTime(epoch);
  cout << "TEST 1:" << endl;
  cout << "epoch="
       << epoch
       << " converts to "
       << dt.toString(Qt::TextDate)
       << endl;

  epoch = KonsoleKalendarEpoch::QDateTime2epoch(dt);
  cout << "date="
       << dt.toString(Qt::TextDate)
       << " converts to "
       << "epoch="
       << epoch
       << endl;

  // Test2
  epoch=100000;
  dt = KonsoleKalendarEpoch::epoch2QDateTime(epoch);
  cout << "TEST 2:" << endl;
  cout << "epoch="
       << epoch
       << " converts to "
       << dt.toString(Qt::TextDate)
       << endl;

  epoch = KonsoleKalendarEpoch::QDateTime2epoch(dt);
  cout << "date="
       << dt.toString(Qt::TextDate)
       << " converts to "
       << "epoch="
       << epoch
       << endl;

  // Test3
  epoch=10000000;
  dt = KonsoleKalendarEpoch::epoch2QDateTime(epoch);
  cout << "TEST 3:" << endl;
  cout << "epoch="
       << epoch
       << " converts to "
       << dt.toString(Qt::TextDate)
       << endl;

  epoch = KonsoleKalendarEpoch::QDateTime2epoch(dt);
  cout << "date="
       << dt.toString(Qt::TextDate)
       << " converts to "
       << "epoch="
       << epoch
       << endl;

  // Test4
  epoch=1000000000;
  dt = KonsoleKalendarEpoch::epoch2QDateTime(epoch);
  cout << "TEST 4:" << endl;
  cout << "epoch="
       << epoch
       << " converts to "
       << dt.toString(Qt::TextDate)
       << endl;

  epoch = KonsoleKalendarEpoch::QDateTime2epoch(dt);
  cout << "date="
       << dt.toString(Qt::TextDate)
       << " converts to "
       << "epoch="
       << epoch
       << endl;

  // Test5
  epoch=10000000000;
  dt = KonsoleKalendarEpoch::epoch2QDateTime(epoch);
  cout << "TEST 5:" << endl;
  cout << "epoch="
       << epoch
       << " converts to "
       << dt.toString(Qt::TextDate)
       << endl;

  epoch = KonsoleKalendarEpoch::QDateTime2epoch(dt);
  cout << "date="
       << dt.toString(Qt::TextDate)
       << " converts to "
       << "epoch="
       << epoch
       << endl;
}
#endif
