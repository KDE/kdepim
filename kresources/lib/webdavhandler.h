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

#include "kgroupware_export.h"

#include <kurl.h>

#include <QString>
#include <QStringList>
#include <qdom.h>
#include <QDateTime>

class KGROUPWAREDAV_EXPORT WebdavHandler
{
  public:
    WebdavHandler();

    static KUrl toDAV( const KUrl& url );


    static QDomElement addElementNS( QDomDocument &doc, QDomNode &node,
                                     const QString &ns, const QString &tag,
                                     const QString &value = QString() );
    static QDomElement addElement( QDomDocument &, QDomNode &,
                         const QString &tag, const QString &value = QString() );
    static QDomElement addDavElement( QDomDocument &, QDomNode &,
                         const QString &tag, const QString &value = QString() );

    static bool extractBool( const QDomElement &node, 
                         const QString &entry, bool &value );
    static bool extractLong( const QDomElement &node, 
                         const QString &entry, long &value );
    static bool extractFloat( const QDomElement &node, 
                         const QString &entry, float &value );
    static bool extractDateTime( const QDomElement &node, 
                         const QString &entry, QDateTime &value );
    static bool extractString( const QDomElement &node, 
                         const QString &entry, QString &value );
    static bool extractStringList( const QDomElement &node, 
                         const QString &entry, QStringList &value );
    
    /**
     * Returns the value of the "etag" header if it can be found in the headers.
     */
    static const QString getEtagFromHeaders( const QString& );


    /**
     * Return the representation of UTC time in the time zone indicated by timeZoneId
     *
     * @param utc UTC time. If its timeSpec() is not Qt::UTC, no conversion is done.
     * @param timeZoneId time zone name, e.g. Europe/Rekjavik
     */
    static QDateTime utcAsZone( const QDateTime& utc, const QString& timeZoneId );

    /**
     * Return the UTC representation of local time in the time zone indicated by timeZoneId 
     *
     * @param zone local time. If its timeSpec() is not Qt::LocalTime, no conversion is done.
     * @param timeZoneId time zone name, e.g. Europe/Rekjavik
     */
    static QDateTime zoneAsUtc( const QDateTime& zone, const QString& timeZoneId );
};

#endif
