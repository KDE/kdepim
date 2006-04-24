/*
    This file is part of libkholidays.
    Copyright (c) 2004 Allen Winter <winter@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include "astroseasons.h"

using namespace KHolidays;

AstroSeasons::AstroSeasons()
{
}

AstroSeasons::~AstroSeasons()
{
}

QString AstroSeasons::seasonStr( const QDate &date ) const
{
  return seasonName( season( date ) );
}

QString AstroSeasons::seasonName( AstroSeasons::Season season )
{
  switch ( season ) {
  case JuneSolstice:
    return( i18n( "June Solstice" ) );
    break;
  case DecemberSolstice:
    return( i18n( "December Solstice" ) );
    break;
  case MarchEquinox:
    return( i18n( "March Equinox" ) );
    break;
  case SeptemberEquinox:
    return( i18n( "September Equinox" ) );
    break;
  default:
  case None:
    return( QString::null );
    break;
  }
}

AstroSeasons::Season AstroSeasons::season( const QDate &date ) const
{
  Season retSeason = None;

  int year = date.year();
  //Use dumb method for now
  if ( date == QDate( year, 6, 22 ) )
    return( JuneSolstice );
  if ( date == QDate( year, 12, 22 ) )
    return( DecemberSolstice );
  if ( date == QDate( year, 3, 22 ) )
    return( MarchEquinox );
  if ( date == QDate( year, 9, 22 ) )
    return( SeptemberEquinox );

  return( retSeason );
}
