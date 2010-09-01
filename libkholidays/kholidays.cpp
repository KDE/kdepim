/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Allen Winter <winter@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <tqfile.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include "kholidays.h"
#include "kholidays_version.h"

extern "C" {
  char *parse_holidays( const char *, int year, short force );
  /** \internal */
  struct holiday {
    char            *string;        /* name of holiday, 0=not a holiday */
    int             color;          /* color code, see scanholiday.lex */
    unsigned short  dup;            /* reference count */
    holiday         *next;          /* single-linked list if more than one holida appears on a given date */
  };
  extern struct holiday holidays[366];
}

TQStringList KHolidays::locations()
{
  TQStringList files =
    KGlobal::dirs()->findAllResources( "data", "libkholidays/" + generateFileName( "*" ),
                                       false, true );
  TQStringList locs;

  TQStringList::ConstIterator it;
  for ( it = files.begin(); it != files.end(); ++it )
    locs.append( (*it).mid((*it).findRev('_') + 1) );

  return locs;
}

TQString KHolidays::fileForLocation( const TQString &location )
{
  return locate( "data", "libkholidays/" + generateFileName( location ) );
}

TQString KHolidays::userPath( bool create )
{
  return KGlobal::dirs()->saveLocation( "data", "libkholidays/", create );
}

TQString KHolidays::generateFileName( const TQString &location )
{
  return "holiday_" + location;
}




KHolidays::KHolidays( const TQString& location )
  : mLocation( location )
{
  mHolidayFile = fileForLocation( location );

  mYearLast = 0;
}

KHolidays::~KHolidays()
{
}

TQString KHolidays::location() const
{
  return mLocation;
}

TQString KHolidays::shortText( const TQDate &date )
{
  TQValueList<KHoliday> lst = getHolidays( date );
  if ( !lst.isEmpty() )
    return lst.first().text;
  else return TQString::null;
}

bool KHolidays::parseFile( const TQDate &date )
{
// kdDebug()<<"KHolidays::parseFile( date=" << date << ")"<<endl;
  int lastYear = 0; //current year less 1900

  if ( mHolidayFile.isNull() || mHolidayFile.isEmpty() || date.isNull() || !date.isValid() )
    return false;

  if ( ( date.year() != mYearLast ) || ( mYearLast == 0 ) ) {
// kdDebug()<<kdBacktrace();
    mYearLast = date.year();
    lastYear = date.year() - 1900; // silly parse_year takes 2 digit year...
    parse_holidays( TQFile::encodeName( mHolidayFile ), lastYear, 1 );
  }

  return true;
}

TQString KHolidays::getHoliday( const TQDate &date )
{
  TQValueList<KHoliday> lst = getHolidays( date );
  if ( !lst.isEmpty() )
    return lst.first().text;
  else return TQString::null;
}

TQValueList<KHoliday> KHolidays::getHolidays( const TQDate &date )
{
  TQValueList<KHoliday> list;
  if ( !date.isValid() ) {
    return list;
  }

  if ( !parseFile( date ) ) return list;
  struct holiday *hd = &holidays[date.dayOfYear()-1];
  while ( hd ) {
    if ( hd->string ) {
      KHoliday holiday;
      holiday.text = TQString::fromUtf8( hd->string );
      holiday.shortText = holiday.text;
      holiday.Category = (hd->color == 2/*red*/) || (hd->color == 9/*weekend*/) ? HOLIDAY : WORKDAY;
      list.append( holiday );
    }
    hd = hd->next;
  }
  return list;
}

int KHolidays::category( const TQDate &date )
{
  if ( !parseFile(date) ) return WORKDAY;

  return (holidays[date.dayOfYear()-1].color == 2/*red*/) ||
         (holidays[date.dayOfYear()-1].color == 9/*weekend*/) ? HOLIDAY : WORKDAY;
}
