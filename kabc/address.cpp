/*
    This file is part of libkabc.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

#include <kapplication.h>
#include <kdebug.h>

#include "address.h"

using namespace KABC;

Address::Address() :
  mEmpty( true ), mType( 0 )
{
  mId = KApplication::randomString( 10 );
}

bool Address::isEmpty() const
{
  return mEmpty;
}

void Address::setId( const QString &id )
{
  mEmpty = false;

  mId = id;
}

QString Address::id() const
{
  return mId;
}

void Address::setType( int type )
{
  mEmpty = false;

  mType = type;
}

int Address::type() const
{
  return mType;
}

void Address::setPostOfficeBox( const QString &s )
{
  mEmpty = false;

  mPostOfficeBox = s;
}

QString Address::postOfficeBox() const
{
  return mPostOfficeBox;
}


void Address::setExtended( const QString &s )
{
  mEmpty = false;

  mExtended = s;
}

QString Address::extended() const
{
  return mExtended;
}


void Address::setStreet( const QString &s )
{
  mEmpty = false;

  mStreet = s;
}

QString Address::street() const
{
  return mStreet;
}


void Address::setLocality( const QString &s )
{
  mEmpty = false;

  mLocality = s;
}

QString Address::locality() const
{
  return mLocality;
}


void Address::setRegion( const QString &s )
{
  mEmpty = false;

  mRegion = s;
}

QString Address::region() const
{
  return mRegion;
}


void Address::setPostalCode( const QString &s )
{
  mEmpty = false;

  mPostalCode = s;
}

QString Address::postalCode() const
{
  return mPostalCode;
}


void Address::setCountry( const QString &s )
{
  mEmpty = false;

  mCountry = s;
}

QString Address::country() const
{
  return mCountry;
}


void Address::setLabel( const QString &s )
{
  mEmpty = false;

  mLabel = s;
}

QString Address::label() const
{
  return mLabel;
}

void Address::dump() const
{
  kdDebug() << "  Address {" << endl;
  kdDebug() << "    Id: " << id() << endl;
  kdDebug() << "    Extended: " << extended() << endl;
  kdDebug() << "    Street: " << street() << endl;
  kdDebug() << "    Postal Code: " << postalCode() << endl;
  kdDebug() << "    Locality: " << locality() << endl;
  kdDebug() << "  }" << endl;
}
