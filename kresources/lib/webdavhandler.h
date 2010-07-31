/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>
    Copyright (c) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef WEBDAVHANDLER_H
#define WEBDAVHANDLER_H

#include <tqstring.h>
#include <tqstringlist.h>
#include <tqdom.h>
#include <tqdatetime.h>
#include <kurl.h>
#include <kdepimmacros.h>

class KDE_EXPORT WebdavHandler
{
  public:
    WebdavHandler();

    static KURL toDAV( const KURL& url );


    static TQDomElement addElementNS( TQDomDocument &doc, TQDomNode &node,
                                     const TQString &ns, const TQString &tag,
                                     const TQString &value = TQString::null );
    static TQDomElement addElement( TQDomDocument &, TQDomNode &,
                         const TQString &tag, const TQString &value = TQString::null );
    static TQDomElement addDavElement( TQDomDocument &, TQDomNode &,
                         const TQString &tag, const TQString &value = TQString::null );

    static bool extractBool( const TQDomElement &node, 
                         const TQString &entry, bool &value );
    static bool extractLong( const TQDomElement &node, 
                         const TQString &entry, long &value );
    static bool extractFloat( const TQDomElement &node, 
                         const TQString &entry, float &value );
    static bool extractDateTime( const TQDomElement &node, 
                         const TQString &entry, TQDateTime &value );
    static bool extractString( const TQDomElement &node, 
                         const TQString &entry, TQString &value );
    static bool extractStringList( const TQDomElement &node, 
                         const TQString &entry, TQStringList &value );
    
    /**
     * Returns the value of the "etag" header if it can be found in the headers.
     */
    static const TQString getEtagFromHeaders( const TQString& );


    /**
     * Return the representation of utc time in the time zone indicated by timeZoneId
     */
    static TQDateTime utcAsZone( const TQDateTime& utc, const TQString& timeZoneId );

    /**
     * Return the UTC representation of local time in the time zone indicated by timeZoneId 
     */
    static TQDateTime zoneAsUtc( const TQDateTime& zone, const TQString& timeZoneId );
};

#endif
