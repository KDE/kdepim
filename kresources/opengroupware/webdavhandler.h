/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifndef WEBDAVHANDLER_H
#define WEBDAVHANDLER_H

#include <qstring.h>
#include <qdom.h>
#include <kurl.h>

class WebdavHandler
{
  public:
    WebdavHandler();

    static KURL toDAV( const KURL& url );


    static QDomElement addElementNS( QDomDocument &doc, QDomNode &node,
                                   const QString &ns, const QString &tag,
                                   const QString &value = QString::null );
    static QDomElement addElement( QDomDocument &, QDomNode &,
                                   const QString &tag, const QString &value = QString::null );
    static QDomElement addDavElement( QDomDocument &, QDomNode &,
                                      const QString &tag, const QString &value = QString::null );
    
    /** 
     * Creates a request body which asks for the standard properties. Used as a stat.
     * */
    static QDomDocument createAllPropsRequest();
    /**
     * Creates a request body which asks only for the file name and the sequence 
     * number, which is what OGo uses to track revisions. */
    static QDomDocument createItemsAndVersionsPropsRequest();


    /**
     * Returns the value of the "etag" header if it can be found in the headers.
     */
    static const QString getEtagFromHeaders( const QString& );
};

#endif
