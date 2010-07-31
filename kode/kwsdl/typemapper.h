/*
    This file is part of KDE.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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

#ifndef KWSDL_TYPEMAPPER_H
#define KWSDL_TYPEMAPPER_H

#include <tqstringlist.h>

#include <schema/types.h>

namespace Schema {
class Attribute;
class Element;
class XSDType;
};

namespace KWSDL {

class TypeInfo
{
  public:
    TypeInfo()
    {}

    TypeInfo( const TQString &_xsdType, const TQString &_type, const TQString &_header )
      : xsdType( _xsdType ), type( _type ), header( _header )
    {}

    TQString xsdType;
    TQString type;
    TQString header;
};

class TypeMapper
{
  public:
    TypeMapper();

    void setTypes( const Schema::Types &types );

    TQString type( const Schema::XSDType *type ) const;
    TQString type( const Schema::Element *element ) const;
    TQString type( const Schema::Attribute *attribute ) const;
    TQString type( const TQString &typeName ) const;

    TQStringList header( const Schema::XSDType *type ) const;
    TQStringList header( const Schema::Element *element ) const;
    TQMap<TQString,TQString> headerDec( const Schema::Element *element ) const;
    TQStringList header( const Schema::Attribute *attribute ) const;
    TQMap<TQString,TQString> headerDec( const Schema::Attribute *attribute ) const;
    TQStringList header( const TQString &typeName ) const;
    TQMap<TQString,TQString> headerDec( const TQString &typeName ) const;

    TQString argument( const TQString &name, const Schema::Element *element ) const;
    TQString argument( const TQString &name, const Schema::Attribute *attribute ) const;
    TQString argument( const TQString &name, const TQString &typeName, bool isList = false ) const;

  private:
    bool isBaseType( const TQString& ) const;

    TQMap<TQString, TypeInfo> mMap;
    Schema::Types mTypes;
};

}

#endif
