/*
    This file is part of libkpimexchange
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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

#include <qdatetime.h>

extern "C" {
  #include <ical.h>
}

#include "utils.h"

/** In a document doc with node node, add an element with name ns and tagname tag. Return the new element 
 */
QDomElement addElement( QDomDocument& doc, QDomNode& node, const QString& ns, const QString& tag )
{
  QDomElement el = doc.createElementNS( ns, tag );
  node.appendChild( el );
  return el;
}

/**
 In a document doc with node node, add an element with namespace ns and tagname tag. Add a textnode in
 the element with text contents text. Return the new element.
 */
QDomElement addElement( QDomDocument& doc, QDomNode& node, const QString& ns, const QString& tag, const QString& text )
{
  QDomElement el = doc.createElementNS( ns, tag );
  QDomText textnode = doc.createTextNode( text );
  el.appendChild( textnode );
  node.appendChild( el );
  return el;
}

QDateTime utcAsZone( const QDateTime& utc, const QString& timeZoneId )
{
  QDateTime epoch;
  epoch.setTime_t( 0 );
  time_t v = epoch.secsTo( utc );
  struct icaltimetype tt = icaltime_from_timet( v, 0 ); // 0: is_date=false
  int offset = icaltime_utc_offset( tt, timeZoneId.local8Bit() );
  return utc.addSecs( offset );
}

QDateTime zoneAsUtc( const QDateTime& zone, const QString& timeZoneId )
{
  QDateTime epoch;
  epoch.setTime_t( 0 );
  time_t v = epoch.secsTo( zone );
  struct icaltimetype tt = icaltime_from_timet( v, 0 ); // 0: is_date=false
  int offset = icaltime_utc_offset( tt, timeZoneId.local8Bit() );
  return zone.addSecs( - offset );
}

KURL toDAV( const KURL& url ) {
  KURL result( url );
  if ( result.protocol() == "http" )
    result.setProtocol( "webdav" );
  else if ( result.protocol() = "https" )
    result.setProtocol( "https" );
  return result;
}

KURL* toDAV( const KURL* url ) {
  KURL* result = new KURL( *url );
  if ( result->protocol() == "http" )
     result->setProtocol( "webdav" );
  else if (  result->protocol() = "https" )
     result->setProtocol( "https" );
  return result;
}

