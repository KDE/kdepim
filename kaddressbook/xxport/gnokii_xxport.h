/*
    This file is part of KAddressbook.
    Copyright (c) 2003 - 2003 Helge Deller <deller@kde.org>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef GNOKII_XXPORT_H
#define GNOKII_XXPORT_H

#include <xxport.h>

class GNOKIIXXPort : public KAB::XXPort
{
  Q_OBJECT

  public:
    GNOKIIXXPort( KABC::AddressBook *ab, QWidget *parent, const char *name = 0 );

    QString identifier() const { return "gnokii"; }

  public slots:
    bool exportContacts( const KABC::AddresseeList &list, const QString &data );
    KABC::AddresseeList importContacts( const QString &data ) const;
};

#endif
