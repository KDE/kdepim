/*
  This file is part of KAddressBook.
  Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef VCARD_XXPORT_H
#define VCARD_XXPORT_H

#include "xxport.h"

class VCardXXPort : public XXPort
{
  public:
    explicit VCardXXPort( QWidget *parent = 0 );

    bool exportContacts( const KABC::Addressee::List &contacts ) const;
    KABC::Addressee::List importContacts() const;

  private:
    KABC::Addressee::List parseVCard( const QByteArray &data ) const;
    bool doExport( const KUrl &url, const QByteArray &data ) const;

    void addKey( KABC::Addressee &addr, KABC::Key::Type type ) const;

    KABC::Addressee::List filterContacts( const KABC::Addressee::List & ) const;
};

#endif
