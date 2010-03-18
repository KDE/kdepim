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

#include <KTimeZone>

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
  str->append( string.toUtf8().data() );

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
  QByteArray str = string.toUtf8();

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

char *GWConverter::kDateTimeToChar( const KDateTime &dt,
                                    const KDateTime::Spec &timeSpec )
{
  KDateTime kdt = dt;
  kdt.setTimeSpec( KDateTime::Spec::LocalZone() );
  return kDateTimeToChar( kdt );
}

char *GWConverter::kDateTimeToChar( const KDateTime &dt )
{
  return qStringToChar( dt.toString( "yyyyMMddThhmmZ" ) );
}

std::string* GWConverter::kDateTimeToString( const KDateTime &dt, const KDateTime::Spec &timeSpec )
{
  KDateTime kdt = dt;
  kdt.setTimeSpec( KDateTime::Spec::LocalZone() );
  return kDateTimeToString( kdt );
}

std::string* GWConverter::kDateTimeToString( const KDateTime &dt )
{
  return qStringToString( dt.toString( "yyyyMMddThhmmZ" ) );
}

KDateTime GWConverter::stringToKDateTime( const std::string* str )
{
  KDateTime dt = KDateTime::fromString( QString::fromUtf8( str->c_str() ), KDateTime::ISODate );
  return dt;
}

KDateTime GWConverter::stringToKDateTime( const std::string* str, const KDateTime::Spec &timeSpec )
{
  KDateTime dt = KDateTime::fromString( QString::fromUtf8( str->c_str() ), KDateTime::ISODate );
  dt.setTimeSpec( timeSpec );
  return dt;
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
  return QDate::fromString( QString::fromUtf8( str->c_str() ), Qt::ISODate );
}

KDateTime GWConverter::charToKDateTime( const char *str )
{
  if ( !str ) return KDateTime();
//  kDebug() <<"charToKDateTime():" << str;
  KDateTime dt = KDateTime::fromString( QString::fromUtf8( str ), KDateTime::ISODate );
//  kDebug() << dt.toString();
  return dt;
}

bool GWConverter::emailsMatch( const QString & email1, const QString & email2 )
{
    // eg demo3.po1.dom1@dmz1.provo.novell.com == demo3@dmz1.provo.novell.com
    if ( email1 == email2 )
        return true;

    QString shorter, longer;
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

    QString shortStem = shorter.section( '@', 0, 0 );
    QString longStem = longer.section( '@', 0, 0 );
    QString shortHost = shorter.section( '@', 1, 1 );
    QString longHost = longer.section( '@', 1, 1 );

    QString extension = longStem.right( longStem.length() - shortStem.length() );

    kDebug() << "gwconverter::emailsMatch(): " << shorter << " = " << longer;
    kDebug() << "shortStem: " << shortStem << ", longStem: " << longStem << ", extension: " << extension;

    if ( longStem.startsWith( shortStem ) && extension.startsWith( "." ) && (
    shortHost == longHost ) )
    {
        kDebug() << "Looks like a match!";
        return true;
    }
    return false;
}


KDateTime GWConverter::charToKDateTime( const char *str,
                                        const KDateTime::Spec &timeSpec )
{
  if ( !str ) return KDateTime();
  KDateTime utc = charToKDateTime( str );
  utc.setTimeSpec( KDateTime::UTC );
  return utc;
}


