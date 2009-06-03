/*
    This file is part of KContactManager.
    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

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

#include "xxport.h"

XXPort::XXPort( QWidget *parent )
  : mParentWidget( parent )
{
}

XXPort::~XXPort()
{
}

KABC::Addressee::List XXPort::importContacts() const
{
  return KABC::Addressee::List();
}

bool XXPort::exportContacts( const KABC::Addressee::List& ) const
{
  return false;
}

void XXPort::setOption( const QString &key, const QString &value )
{
  mOptions.insert( key, value );
}

QString XXPort::option( const QString &key ) const
{
  return mOptions.value( key );
}

QWidget *XXPort::parentWidget() const
{
  return mParentWidget;
}

QString XXPort::contactFileName( const KABC::Addressee &contact ) const
{
  if ( !contact.givenName().isEmpty() && !contact.familyName().isEmpty() )
    return QString( "%1_%2" ).arg( contact.givenName() ).arg( contact.familyName() );

  if ( !contact.familyName().isEmpty() )
    return contact.familyName();

  if ( !contact.givenName().isEmpty() )
    return contact.givenName();

  if ( !contact.organization().isEmpty() )
    return contact.organization();

  return contact.uid();
}
