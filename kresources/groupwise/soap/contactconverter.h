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

#ifndef KABC_GW_CONTACTCONVERTER_H
#define KABC_GW_CONTACTCONVERTER_H

#include <kabc/addressee.h>

#include "gwconverter.h"

class ContactConverter : public GWConverter
{
  public:
    ContactConverter( struct soap* );

    KABC::Addressee convertFromContact( ngwt__Contact* );
    ngwt__Contact* convertToContact( const KABC::Addressee& );
    KABC::Addressee convertFromResource( ngwt__Resource* );
    KABC::Addressee convertFromGroup( ngwt__Group* );

  private:
    KABC::Addressee convertFromAddressBookItem( ngwt__AddressBookItem * );
    KABC::PhoneNumber convertPhoneNumber( ngwt__PhoneNumber* ) const;
    ngwt__PhoneNumber* convertPhoneNumber( const KABC::PhoneNumber& ) const;

    KABC::Address convertPostalAddress( ngwt__PostalAddress* ) const;
    ngwt__PostalAddress* convertPostalAddress( const KABC::Address& );
    /* we convert all IM addresses in the addressee at once,
    because multiple values per IM system are stored in a custom field each
    which is a different structure to that used for phone numbers, email addresses etc */
    ngwt__ImAddressList* convertImAddresses( const KABC::Addressee& );
    // splits up an arbitrary custom field
    void splitField( const QString &str, QString &app, QString &name, QString &value );
    static QMap<QString, QString > mIMProtocolMap;
};

#endif
