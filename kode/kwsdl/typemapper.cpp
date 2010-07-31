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

#include <tqmap.h>

#include <schema/attribute.h>
#include <schema/complextype.h>
#include <schema/element.h>
#include <schema/simpletype.h>
#include <schema/xsdtype.h>

#include "typemapper.h"

using namespace KWSDL;

TypeMapper::TypeMapper()
{
  mMap.insert( "any", TypeInfo( "any", "TQString", "tqstring.h" ) );
  mMap.insert( "anyURI", TypeInfo( "anyUri", "TQString", "tqstring.h" ) );
  mMap.insert( "base64Binary", TypeInfo( "base64Binary", "TQByteArray", "tqcstring.h" ) );
  mMap.insert( "binary", TypeInfo( "binary", "TQByteArray", "tqcstring.h" ) );
  mMap.insert( "boolean", TypeInfo( "boolean", "bool", "" ) );
  mMap.insert( "byte", TypeInfo( "byte", "char", "" ) );
  mMap.insert( "date", TypeInfo( "date", "TQDate", "tqdatetime.h" ) );
  mMap.insert( "dateTime", TypeInfo( "dateTime", "TQDateTime", "tqdatetime.h" ) );
  mMap.insert( "decimal", TypeInfo( "decimal", "float", "" ) );
  mMap.insert( "double", TypeInfo( "double", "double", "" ) );
  mMap.insert( "duration", TypeInfo( "duration", "TQString", "tqstring.h" ) ); // TODO: add duration class
  mMap.insert( "int", TypeInfo( "int", "int", "" ) );
  mMap.insert( "language", TypeInfo( "language", "TQString", "tqstring.h" ) );
  mMap.insert( "short", TypeInfo( "short", "short", "" ) );
  mMap.insert( "string", TypeInfo( "string", "TQString", "tqstring.h" ) );
  mMap.insert( "unsignedByte", TypeInfo( "unsignedByte", "unsigned char", "" ) );
  mMap.insert( "unsignedInt", TypeInfo( "unsignedInt", "unsigned int", "" ) );
}

void TypeMapper::setTypes( const Schema::Types &types )
{
  mTypes = types;
}

TQString TypeMapper::type( const Schema::XSDType *type ) const
{
  TQString typeName = type->name();
  typeName[ 0 ] = typeName[ 0 ].upper();

  return typeName;
}

TQString TypeMapper::type( const Schema::Element *element ) const
{
  TQString typeName = element->typeName();

  TQString type;
  // check basic types
  TQMap<TQString, TypeInfo>::ConstIterator it = mMap.find( typeName );
  if ( it != mMap.end() )
    type = it.data().type;

  if ( type.isEmpty() ) {
    type = typeName;
    type[ 0 ] = type[ 0 ].upper();
  }

  return type;
}

TQString TypeMapper::type( const Schema::Attribute *attribute ) const
{
  TQString typeName = attribute->typeName();

  TQString type;
  // check basic types
  TQMap<TQString, TypeInfo>::ConstIterator it = mMap.find( typeName );
  if ( it != mMap.end() )
    type = it.data().type;

  if ( type.isEmpty() ) {
    type = typeName;
    type[ 0 ] = type[ 0 ].upper();
  }

  return type;
}

TQString TypeMapper::type( const TQString &typeName ) const
{
  // check basic types
  TQMap<TQString, TypeInfo>::ConstIterator it = mMap.find( typeName );
  if ( it != mMap.end() )
    return it.data().type;

  Schema::SimpleType::List simpleTypes = mTypes.simpleTypes();
  Schema::SimpleType::List::ConstIterator simpleIt;
  for ( simpleIt = simpleTypes.begin(); simpleIt != simpleTypes.end(); ++simpleIt ) {
    if ( (*simpleIt).name() == typeName ) {
      return type( &(*simpleIt) );
    }
  }

  Schema::ComplexType::List complexTypes = mTypes.complexTypes();
  Schema::ComplexType::List::ConstIterator complexIt;
  for ( complexIt = complexTypes.begin(); complexIt != complexTypes.end(); ++complexIt ) {
    if ( (*complexIt).name() == typeName ) {
      return type( &(*complexIt) );
    }
  }

  Schema::Element::List elements = mTypes.elements();
  Schema::Element::List::ConstIterator elemIt;
  for ( elemIt = elements.begin(); elemIt != elements.end(); ++elemIt ) {
    if ( (*elemIt).name() == typeName ) {
      return type( &(*elemIt) );
    }
  }

  return TQString();
}

TQStringList TypeMapper::header( const Schema::XSDType *type ) const
{
  return type->name().lower() + ".h";
}

TQStringList TypeMapper::header( const Schema::Element *element ) const
{
  TQString typeName = element->typeName();

  TQStringList headers;

  // check basic types
  TQMap<TQString, TypeInfo>::ConstIterator it = mMap.find( typeName );
  if ( it != mMap.end() ) {
    if ( !it.data().header.isEmpty() )
      headers.append( it.data().header );
  } else
    headers.append( typeName.lower() + ".h" );
  
  if ( element->maxOccurs() > 1 )
    headers.append( "tqptrlist.h" );

  return headers;
}

TQMap<TQString, TQString> TypeMapper::headerDec( const Schema::Element *element ) const
{
  TQString typeName = element->typeName();

  TQMap<TQString, TQString> headers;

  // check basic types
  TQMap<TQString, TypeInfo>::ConstIterator it = mMap.find( typeName );
  if ( it != mMap.end() ) {
    if ( !it.data().header.isEmpty() ) {
      if ( it.data().type == "TQByteArray" )
        headers.insert( it.data().header, TQString() );
      else
        headers.insert( it.data().header, it.data().type );
    }
  } else {
    typeName[ 0 ] = typeName[ 0 ].upper();
    headers.insert( typeName.lower() + ".h", typeName );
  }

  if ( element->maxOccurs() > 1 )
    headers.insert( "tqptrlist.h", TQString() );

  return headers;
}

TQStringList TypeMapper::header( const Schema::Attribute *attribute ) const
{
  TQString typeName = attribute->typeName();

  TQStringList headers;

  // check basic types
  TQMap<TQString, TypeInfo>::ConstIterator it = mMap.find( typeName );
  if ( it != mMap.end() ) {
    if ( !it.data().header.isEmpty() )
      headers.append( it.data().header );
  } else
    headers.append( typeName.lower() + ".h" );
  
  return headers;
}

TQMap<TQString, TQString> TypeMapper::headerDec( const Schema::Attribute *attribute ) const
{
  TQString typeName = attribute->typeName();

  TQMap<TQString, TQString> headers;

  // check basic types
  TQMap<TQString, TypeInfo>::ConstIterator it = mMap.find( typeName );
  if ( it != mMap.end() ) {
    if ( !it.data().header.isEmpty() ) {
      if ( it.data().type == "TQByteArray" )
        headers.insert( it.data().header, TQString() );
      else
        headers.insert( it.data().header, it.data().type );
    }
  } else {
    typeName[ 0 ] = typeName[ 0 ].upper();
    headers.insert( typeName.lower() + ".h", typeName );
  }

  return headers;
}

TQStringList TypeMapper::header( const TQString &typeName ) const
{
  TQMap<TQString, TypeInfo>::ConstIterator it = mMap.find( typeName );
  if ( it != mMap.end() )
    return it.data().header;

  const TQString convertedType = type( typeName );
  if ( isBaseType( convertedType ) ) {
    for ( it = mMap.begin(); it != mMap.end(); ++it )
      if ( it.data().type == convertedType )
        return it.data().header;
  } else
    return typeName.lower() + ".h";

  return TQStringList();
}

TQMap<TQString, TQString> TypeMapper::headerDec( const TQString &typeName ) const
{
  TQString type( typeName );
  TQMap<TQString, TQString> headers;

  // check basic types
  TQMap<TQString, TypeInfo>::ConstIterator it = mMap.find( typeName );
  if ( it != mMap.end() ) {
    if ( !it.data().header.isEmpty() ) {
      if ( it.data().type == "TQByteArray" )
        headers.insert( it.data().header, TQString() );
      else
        headers.insert( it.data().header, it.data().type );
    }
  } else {
    type[ 0 ] = type[ 0 ].upper();
    headers.insert( typeName.lower() + ".h", type );
  }

  return headers;
}

TQString TypeMapper::argument( const TQString &name, const Schema::Element *element ) const
{
  TQString typeName = type( element );

  if ( element->maxOccurs() > 1 )
    return "TQPtrList<" + typeName + ">* " + name;
  else
    return typeName + "* " + name;
}

TQString TypeMapper::argument( const TQString &name, const Schema::Attribute *attribute ) const
{
  return type( attribute ) + "* " + name;
}

TQString TypeMapper::argument( const TQString &name, const TQString &typeName, bool isList ) const
{
  if ( isList ) {
    return "TQPtrList<" + type( typeName ) + ">* " + name;
  } else {
    return type( typeName ) + "* " + name;
  }
}

bool TypeMapper::isBaseType( const TQString &type ) const
{
  TQMap<TQString, TypeInfo>::ConstIterator it;
  for ( it = mMap.begin(); it != mMap.end(); ++it )
    if ( it.data().type == type )
      return true;

  return false;
}
