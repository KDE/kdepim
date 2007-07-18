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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "gwconverter.h"

#include <libkdepim/kpimprefs.h>

#include <kdebug.h>
//Added by qt3to4:
#include <QByteArray>

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
  if ( !str ) return QString();
  return QString::fromUtf8( str->c_str() );
}

char* GWConverter::qStringToChar( const QString &string )
{
  QByteArray str = string.utf8();

  char* charStr = (char*)soap_malloc( mSoap, str.length() + 1 );
  memcpy( charStr, str, str.length() );
  charStr[ str.length() ] = 0;

  return charStr;
}

QDate GWConverter::charToQDate( const char *str )
{
  if ( !str ) return QDate(); // FIXME: Qt::ISODate is probably no good here because it expects yyyy-MM-dd not yyyyMMdd 
  return QDate::fromString( QString::fromUtf8( str ), Qt::ISODate );
}

char *GWConverter::qDateTimeToChar( const QDateTime &dt,
                                    const KDateTime::Spec &timeSpec )
{
  QDateTime qdt = dt;
  qdt.setTimeSpec( Qt::LocalTime );
  return qDateTimeToChar( KDateTime( qdt, timeSpec ).toUtc().dateTime() );
}

char *GWConverter::qDateTimeToChar( const QDateTime &dt )
{
  return qStringToChar( dt.toString( "yyyyMMddThhmmZ" ) );
}

std::string* GWConverter::qDateTimeToString( const QDateTime &dt, const KDateTime::Spec &timeSpec )
{
  QDateTime qdt = dt;
  qdt.setTimeSpec( Qt::LocalTime );
  return qDateTimeToString( KDateTime( qdt, timeSpec ).toUtc().dateTime() );
}

std::string* GWConverter::qDateTimeToString( const QDateTime &dt )
{
  return qStringToString( dt.toString( "yyyyMMddThhmmZ" ) );
}

QDateTime GWConverter::stringToQDateTime( const std::string* str )
{
  QDateTime dt = QDateTime::fromString( QString::fromUtf8( str->c_str() ), Qt::ISODate );
  return dt;
}

KDateTime GWConverter::stringToKDateTime( const std::string* str, const KDateTime::Spec &timeSpec )
{
  QDateTime dt = QDateTime::fromString( QString::fromUtf8( str->c_str() ), Qt::ISODate );
  return KDateTime( dt, timeSpec );
}

char* GWConverter::qDateToChar( const QDate &date )
{
  return qStringToChar( date.toString( "yyyyMMdd" ) );
}

std::string* GWConverter::qDateToString( const QDate &date )
{
  return qStringToString( date.toString( "yyyyMMdd" ) );
}

QDate GWConverter::stringToQDate( std::string* str )
{
  return QDate::fromString( QString::fromUtf8( str->c_str() ) );
}

QDateTime GWConverter::charToQDateTime( const char *str )
{
  if ( !str ) return QDateTime();
//  kDebug() << "charToQDateTime(): " << str << endl;
  QDateTime dt = QDateTime::fromString( QString::fromUtf8( str ), Qt::ISODate );
//  kDebug() << "  " << dt.toString() << endl;
  return dt;
}

KDateTime GWConverter::charToKDateTime( const char *str,
                                        const KDateTime::Spec &timeSpec )
{
  if ( !str ) return KDateTime();
  QDateTime utc = charToQDateTime( str );
  utc.setTimeSpec( Qt::UTC );
  return KDateTime( utc, timeSpec );
}

QDateTime GWConverter::charToQDateTime( const char *str,
                                        const KDateTime::Spec &timeSpec )
{
  return charToKDateTime( str, timeSpec ).dateTime();
}
