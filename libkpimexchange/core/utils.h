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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KDEPIM_EXCHANGE_UTILS_H
#define KDEPIM_EXCHANGE_UTILS_H

#include <tqstring.h>
#include <tqdom.h>

#include <kurl.h>

/** In a document doc with node node, add an element with name ns and tagname tag. Return the new element 
 */
TQDomElement addElement( TQDomDocument& doc, TQDomNode& node, const TQString& ns, const TQString& tag );

/**
 In a document doc with node node, add an element with namespace ns and tagname tag. Add a textnode in
 the element with text contents text. Return the new element.
 */
TQDomElement addElement( TQDomDocument& doc, TQDomNode& node, const TQString& ns, const TQString& tag, const TQString& text );

/**
 * Return the representation of utc time in the time zone indicated by timeZoneId
 */
TQDateTime utcAsZone( const TQDateTime& utc, const TQString& timeZoneId );

/**
 * Return the UTC representation of local time in the time zone indicated by timeZoneId 
 */
TQDateTime zoneAsUtc( const TQDateTime& zone, const TQString& timeZoneId );

/**
 * Convert http:// url to webdav:// and https:// to webdavs://
 */
KURL toDAV( const KURL& url );
KURL* toDAV( const KURL* url );

#endif

