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

GWConverter::GWConverter( struct soap* soap )
  : mSoap( soap )
{
  Q_ASSERT( mSoap );
}

struct soap* GWConverter::soap() const
{
  return mSoap;
}

std::string* GWConverter::qStringToString( const TQString &string )
{
  std::string *str = soap_new_std__string( mSoap, -1 );
  str->append( string.utf8() );

  return str;
}

TQString GWConverter::stringToQString( const std::string &str )
{
  return TQString::fromUtf8( str.c_str() );
}

TQString GWConverter::stringToQString( std::string *str )
{
  if ( !str ) return TQString::null;
  return TQString::fromUtf8( str->c_str() );
}

char* GWConverter::qStringToChar( const TQString &string )
{
  TQCString str = string.utf8();

  char* charStr = (char*)soap_malloc( mSoap, str.length() + 1 );
  memcpy( charStr, str, str.length() );
  charStr[ str.length() ] = 0;

  return charStr;
}

TQDate GWConverter::charToQDate( const char *str )
{
  if ( !str ) return TQDate(); // FIXME: Qt::ISODate is probably no good here because it expects yyyy-MM-dd not yyyyMMdd 
  return TQDate::fromString( TQString::fromUtf8( str ), Qt::ISODate );
}

char *GWConverter::qDateTimeToChar( const TQDateTime &dt,
                                    const TQString &timezone )
{
  return qDateTimeToChar( KPimPrefs::localTimeToUtc( dt, timezone ) );
}

char *GWConverter::qDateTimeToChar( const TQDateTime &dt )
{
  return qStringToChar( dt.toString( "yyyyMMddThhmmZ" ) );
}

std::string* GWConverter::qDateTimeToString( const TQDateTime &dt, const TQString &timezone )
{
  return qDateTimeToString( KPimPrefs::localTimeToUtc( dt, timezone ) );
}

std::string* GWConverter::qDateTimeToString( const TQDateTime &dt )
{
  return qStringToString( dt.toString( "yyyyMMddThhmmZ" ) );
}

TQDateTime GWConverter::stringToQDateTime( const std::string* str )
{
  TQDateTime dt = TQDateTime::fromString( TQString::fromUtf8( str->c_str() ), Qt::ISODate );
  return dt;
}

char* GWConverter::qDateToChar( const TQDate &date )
{
  return qStringToChar( date.toString( "yyyyMMdd" ) );
}

std::string* GWConverter::qDateToString( const TQDate &date )
{
  return qStringToString( date.toString( "yyyyMMdd" ) );
}

TQDate GWConverter::stringToQDate( std::string* str )
{
  //NB this ISODate may become unnecessary, if GW stops sending in yyyy-mm-dd format again
  return TQDate::fromString( TQString::fromLatin1( str->c_str() ), Qt::ISODate );
}

TQDateTime GWConverter::charToQDateTime( const char *str )
{
  if ( !str ) return TQDateTime();
//  kdDebug() << "charToQDateTime(): " << str << endl;
  // as above re Qt::ISODate
  TQDateTime dt = TQDateTime::fromString( TQString::fromUtf8( str ), Qt::ISODate );
//  kdDebug() << "  " << dt.toString() << endl;
  return dt;
}

TQDateTime GWConverter::charToQDateTime( const char *str,
                                        const TQString &timezone )
{
  if ( !str ) return TQDateTime();
  TQDateTime utc = charToQDateTime( str );
  return KPimPrefs::utcToLocalTime( utc, timezone );
}

bool GWConverter::emailsMatch( const TQString & email1, const TQString & email2 )
{
    // eg demo3.po1.dom1@dmz1.provo.novell.com == demo3@dmz1.provo.novell.com
    if ( email1 == email2 )
        return true;

    TQString shorter, longer;
    if ( email1.length() < email2.length() )
    {
        shorter = email1;
        longer = email2;
    }
    else
    {
        shorter = email2;
        longer = email1;
    }

    TQString shortStem = shorter.section( '@', 0, 0 );
    TQString longStem = longer.section( '@', 0, 0 );
    TQString shortHost = shorter.section( '@', 1, 1 );
    TQString longHost = longer.section( '@', 1, 1 );

    TQString extension = longStem.right( longStem.length() - shortStem.length() );

    kdDebug() << "gwconverter::emailsMatch(): " << shorter << " = " << longer << endl;
    kdDebug() << "shortStem: " << shortStem << ", longStem: " << longStem << ", extension: " << extension << endl;

    if ( longStem.startsWith( shortStem ) && extension.startsWith( "." ) && (
    shortHost == longHost ) )
    {
        kdDebug() << "Looks like a match!" << endl;
        return true;
    }
    return false;
}

