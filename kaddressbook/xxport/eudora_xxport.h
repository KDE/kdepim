/*
    This file is part of KAddressbook.
    Copyright (c) 2003 Daniel Molkentin <molkentin@kde.org>

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

#ifndef EUDORA_XXPORT_H
#define EUDORA_XXPORT_H

#include <xxport.h>

class EudoraXXPort : public KAB::XXPort
{
  Q_OBJECT

  public:
    EudoraXXPort( KABC::AddressBook *ab, QWidget *parent, const char *name = 0 );

    QString identifier() const { return "eudora"; }

  public slots:
    KABC::AddresseeList importContacts( const QString &data ) const;

  private:
    QString get( const QString& line, const QString& key ) const;
    QString comment( const QString& line ) const;
    QString email( const QString& line ) const;
    QString key( const QString& line ) const;
    int find( const QString& key ) const;
};

#endif
