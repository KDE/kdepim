/*
    This file is part of kdepim.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "gwconverter.h"

#include <libkdepim/kpimprefs.h>

#include <kdebug.h>

GWConverter::GWConverter( struct soap* soap )
  : mSoap( soap )
{
  Q_ASSERT( mSoap );
}

struct soap* GWConverter::soap() const
{
  return mSoap;
}

std::string* GWConverter::qStringToString( const QString &string )
{
  std::string *str = soap_new_std__string( mSoap, -1 );
  str->append( string.utf8() );

  return str;
}

QString GWConverter::stringToQString( const std::string &str )
{
  return QString::fromUtf8( str.c_str() );
}

QString GWConverter::stringToQString( std::string *str )
{
  if ( !str ) return QString::null;
  return QString::fromUtf8( str->c_str() );
}

char* GWConverter::qStringToChar( const QString &string )
{
  QCString str = string.utf8();

  char* charStr = (char*)soap_malloc( mSoap, str.length() + 1 );
  memcpy( charStr, str, str.length() );
  charStr[ str.length() ] = 0;

  return charStr;
}

QDate GWConverter::charToQDate( const char *str )
{
  if ( !str ) return QDate();
  return QDate::fromString( QString::fromUtf8( str ), Qt::ISODate );
}

char *GWConverter::qDateTimeToChar( const QDateTime &dt,
                                    const QString &timezone )
{
  return qDateTimeToChar( KPimPrefs::localTimeToUtc( dt, timezone ) );
}

char *GWConverter::qDateTimeToChar( const QDateTime &dt )
{
  return qStringToChar( dt.toString( "yyyyMMddThhmmZ" ) );
}

char* GWConverter::qDateToChar( const QDate &date )
{
  return qStringToChar( date.toString( Qt::ISODate ) );
}

QDateTime GWConverter::charToQDateTime( const char *str )
{
  if ( !str ) return QDateTime();
//  kdDebug() << "charToQDateTime(): " << str << endl;
  QDateTime dt = QDateTime::fromString( QString::fromUtf8( str ), Qt::ISODate );
//  kdDebug() << "  " << dt.toString() << endl;
  return dt;
}

QDateTime GWConverter::charToQDateTime( const char *str,
                                        const QString &timezone )
{
  if ( !str ) return QDateTime();
  QDateTime utc = charToQDateTime( str );
  return KPimPrefs::utcToLocalTime( utc, timezone );
}
