/*
    This file is part of the exchange resource.
    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#ifndef KABC_EXCHANGECONVERTERCONTACT_H
#define KABC_EXCHANGECONVERTERCONTACT_H

#include <qstring.h>
#include <qdom.h>

#include <kabc/addressee.h>

namespace KABC {

class ExchangeConverterContact
{
  public:

    ExchangeConverterContact();

    static void createRequest( QDomDocument &doc, QDomElement &root );

    QDomDocument createWebDAV( Addressee addr );

    Addressee::List parseWebDAV( const QDomDocument& davdata );
    bool readAddressee( const QDomElement &node, Addressee &addressee );

  protected:
    bool extractAddress( const QDomElement &node, Addressee &addressee, int type,
      const QString &street, const QString &pobox, const QString &location, 
      const QString &postalcode, const QString &state, const QString &country, 
      const QString &countycode );
};

}

#endif
