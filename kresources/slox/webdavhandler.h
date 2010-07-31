/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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

#include <tqvaluelist.h>
#include <tqstring.h>
#include <tqdatetime.h>
#include <tqdom.h>

#include <kdepimmacros.h>
#include <kabc/addressee.h>

namespace KCal {
class Incidence;
}

class SloxBase;

class KDE_EXPORT SloxItem
{
  public:
    enum Status { Invalid, Delete, Create, New };

    SloxItem();

    TQDomNode domNode;
    TQString sloxId;
    TQString clientId;
    Status status;
    TQString response;
    TQString responseDescription;
};

class KDE_EXPORT WebdavHandler
{
  public:
    WebdavHandler();

    void setUserId( const TQString & );
    TQString userId() const;
    void setResource( SloxBase *res ) { mRes = res; }

    void log( const TQString & );

    static TQDomElement addElement( TQDomDocument &, TQDomNode &,
                                   const TQString &tag );
    static TQDomElement addDavElement( TQDomDocument &, TQDomNode &,
                                      const TQString &tag );
    static TQDomElement addSloxElement( SloxBase *res,
                                       TQDomDocument &, TQDomNode &,
                                       const TQString &tag,
                                       const TQString &text = TQString::null );

    static TQDateTime sloxToQDateTime( const TQString &str );
    static TQDateTime sloxToQDateTime( const TQString &str,
                                      const TQString &timeZoneId );
    static TQString qDateTimeToSlox( const TQDateTime &dt );
    static TQString qDateTimeToSlox( const TQDateTime &dt,
                                    const TQString &timeZoneId );

    static TQValueList<SloxItem> getSloxItems( SloxBase *res, const TQDomDocument &doc );

    void clearSloxAttributeStatus();
    void parseSloxAttribute( const TQDomElement & );
    void setSloxAttributes( KCal::Incidence * );
    void setSloxAttributes( KABC::Addressee & );

  private:
    TQString mLogFile;
    int mLogCount;
    SloxBase *mRes;

    TQString mUserId;

    bool mWritable;
};

#endif
