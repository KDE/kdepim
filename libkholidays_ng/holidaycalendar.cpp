/*
    This file is part of libkholidays.
    Copyright (c) 2004,2006 Allen Winter <winter@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "holidaycalendar.h"

using namespace KHolidays;

HolidayCalendar::HolidayCalendar() {
}

HolidayCalendar::~HolidayCalendar() {
}

QDate HolidayCalendar::christmas( int year ) {
  return QDate( year, 12, 25 );
}

QDate HolidayCalendar::easter( int year ) {
  int G, C, H, I, J, L, month, day;

  G = year % 19;
  C = int( year / 100 );
  H = ( C - int( C / 4 ) - int( ( 8 * C ) / 25 ) + 19 * G + 15 ) % 30;
  I = H - int( H / 28 ) *
      ( 1 - int( H / 28 ) * int( 29 / ( H + 1 ) ) * int( ( 21 - G ) / 11 ) );
  J     = ( year + int( year / 4 ) + I + 2 - C + int( C / 4 ) ) % 7;
  L     = I - J;
  month = 3 + int( ( L + 40 ) / 44 );
  day   = L + 28 - ( 31 * int( month / 4 ) );

  return QDate( year, month, day );
}

QDate HolidayCalendar::orthodoxEaster( int year ) {

  int epakti, month, fullmoon, fullmoonday, easter;

    epakti = ( ( ( year - 2 ) % 19 ) * 11 ) % 30;

    if( epakti > 23 ) {
      month = 4;
    } else {
      month = 3;
    }
    fullmoon = 44 - epakti + 13;

    if( ( month == 4 ) && ( fullmoon > 30 ) ) {
      month++;
      fullmoon -= 30;
    } else if( ( month == 3 ) && ( fullmoon > 31 ) ) {
      month++;
      fullmoon -= 31;
    }
    fullmoonday = QDate( year, month, fullmoon ).dayOfWeek();
    easter = fullmoon + ( 7 - fullmoonday );

    if( ( month == 3 ) && ( easter > 31 ) ) {
      month++;
      easter -= 31;
    } else if( ( month == 4 ) && ( easter > 30 ) ) {
      month++;
      easter -= 30;
    }
    return QDate( year, month, easter );
}

