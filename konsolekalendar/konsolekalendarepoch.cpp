/***************************************************************************
        konsolekalendarepoch.cpp - epoch calcuations
        ----------------------------------
    begin                : Thu Jul 10 2003
    copyright            : (C) 2003 by Allen Winter
    email                : winterz@earthlink.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
QDateTime KonsoleKalendarEpoch::epoch2QDateTime( time_t epoch )
{
  QDateTime dt;
  dt.setTime_t ((uint)epoch);
  return (dt);
}

// Function to convert a QDateTime value into an epoch
time_t KonsoleKalendarEpoch::QDateTime2epoch( QDateTime dt )
{
  return(dt.toTime_t());
}

#if defined (TEST)
// Pass -DTEST to the compile command to create the test program, e.g:
// cc -DTEST -I/usr/local/KDE/include  epochstuff.cpp -L/usr/local/KDE/lib -lqt-mt -pthread
main()
{
    time_t epoch=100000;
    QDateTime dt = epoch2QDateTime(epoch);
    cout << "epoch=" << epoch << " converts to " << dt.toString(Qt::TextDate) << endl;

    epoch = QDateTime2epoch(dt);
    cout << "date=" << dt.toString(Qt::TextDate) << " converts to " << "epoch=" << epoch << endl;
}
#endif
