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

#include "converter.h"

using namespace KWSDL;

static TQString capitalize( const TQString &str )
{
  return str[ 0 ].upper() + str.mid( 1 );
}

static TQString escapeEnum( const TQString &str )
{
  TQString enumStr = capitalize( str );

  return enumStr.replace( "-", "_" );
}

Converter::Converter()
{
  mQObject = KODE::Class( "TQObject" );
}

void Converter::setWSDL( const WSDL &wsdl )
{
  mWSDL = wsdl;
  mTypeMapper.setTypes( wsdl.types() );
}

KODE::Class::List Converter::classes() const
{
  return mClasses;
}

void Converter::convert()
{
  createUtilClasses();
  createTransportClass();

  convertTypes( mWSDL.types() );

  mClasses.append( mSerializer );

  convertService( mWSDL.service() );
}

void Converter::convertTypes( const Schema::Types &types )
{
  Schema::SimpleType::List simpleTypes = types.simpleTypes();
  Schema::SimpleType::List::ConstIterator simpleIt;
  for ( simpleIt = simpleTypes.begin(); simpleIt != simpleTypes.end(); ++simpleIt )
    convertSimpleType( &(*simpleIt) );

  Schema::ComplexType::List complexTypes = types.complexTypes();
  Schema::ComplexType::List::ConstIterator complexIt;
  for ( complexIt = complexTypes.begin(); complexIt != complexTypes.end(); ++complexIt )
    convertComplexType( &(*complexIt) );
}

void Converter::convertSimpleType( const Schema::SimpleType *type )
{
  KODE::Class newClass( type->name() );
  newClass.addInclude( "serializer.h" );

  KODE::Code ctorCode, dtorCode;

  if ( !type->documentation().isEmpty() )
    newClass.setDocs( type->documentation().simplifyWhiteSpace() );

  if ( type->subType() == Schema::SimpleType::TypeRestriction ) {
    /**
      Use setter and getter method for enums as well.
     */
    if ( type->facetType() & Schema::SimpleType::ENUM ) {
      TQStringList enums = type->facetEnums();
      for ( uint i = 0; i < enums.count(); ++i )
        enums[ i ] = escapeEnum( enums[ i ] );

      newClass.addEnum( KODE::Enum( "Type", enums ) );

      // member variables
      KODE::MemberVariable variable( "type", "Type" );
      newClass.addMemberVariable( variable );

      // setter method
      KODE::Function setter( "setType", "void" );
      setter.addArgument( "Type type" );
      setter.setBody( variable.name() + " = type;" );

      // getter method
      KODE::Function getter( "type", capitalize( newClass.name() ) + "::Type" );
      getter.setBody( " return " + variable.name() + ";" );
      getter.setConst( true );

      newClass.addFunction( setter );
      newClass.addFunction( getter );
    }

    /**
      A class can't derive from basic types (e.g. int or unsigned char), so
      we add setter and getter methods to set the value of this class.
     */
    if ( type->baseType() != Schema::XSDType::ANYTYPE &&
         type->baseType() != Schema::XSDType::INVALID &&
         !(type->facetType() & Schema::SimpleType::ENUM) ) {

      const TQString baseName = type->baseTypeName();
      const TQString typeName = mTypeMapper.type( baseName );

      // include header
      TQMap<TQString, TQString> headerDec = mTypeMapper.headerDec( baseName );
      TQMap<TQString, TQString>::ConstIterator it;
      for ( it = headerDec.begin(); it != headerDec.end(); ++it ) {
        if ( !it.key().isEmpty() )
          newClass.addInclude( it.key(), it.data() );

        if ( it.data().isEmpty() )
          newClass.addHeaderInclude( it.key() );
      }

      // member variables
      KODE::MemberVariable variable( "value", typeName + "*" );
      newClass.addMemberVariable( variable );

      ctorCode += variable.name() + " = 0;";
      dtorCode += "delete " + variable.name() + ";";
      dtorCode += variable.name() + " = 0;";

      // setter method
      KODE::Function setter( "setValue", "void" );
      setter.addArgument( mTypeMapper.argument( "value", baseName ) );
      setter.setBody( variable.name() + " = value;" );

      // getter method
      KODE::Function getter( "value", typeName + "*" );
      getter.setBody( " return " + variable.name() + ";" );
      getter.setConst( true );

      newClass.addFunction( setter );
      newClass.addFunction( getter );
    }
  } else if ( type->subType() == Schema::SimpleType::TypeList ) {
    newClass.addHeaderInclude( "tqptrlist.h" );
    const TQString baseName = type->listTypeName();
    const TQString typeName = mTypeMapper.type( baseName );

    // include header
    TQMap<TQString, TQString> headerDec = mTypeMapper.headerDec( baseName );
    TQMap<TQString, TQString>::ConstIterator it;
    for ( it = headerDec.begin(); it != headerDec.end(); ++it ) {
      if ( !it.key().isEmpty() )
        newClass.addInclude( it.key(), it.data() );

      if ( it.data().isEmpty() )
        newClass.addHeaderInclude( it.key() );
    }

    // member variables
    KODE::MemberVariable variable( "entries", "TQPtrList<" + typeName + ">*" );
    newClass.addMemberVariable( variable );

    ctorCode += variable.name() + " = 0;";
    dtorCode += "delete " + variable.name() + ";";
    dtorCode += variable.name() + " = 0;";

    // setter method
    KODE::Function setter( "setEntries", "void" );
    setter.addArgument( mTypeMapper.argument( "entries", baseName, true ) );
    setter.setBody( variable.name() + " = entries;" );

    // getter method
    KODE::Function getter( "entries", "TQPtrList<" + typeName + ">*" );
    getter.setBody( " return " + variable.name() + ";" );
    getter.setConst( true );

    newClass.addFunction( setter );
    newClass.addFunction( getter );
  }

  createSimpleTypeSerializer( type );

  KODE::Function ctor( capitalize( newClass.name() ) );
  KODE::Function dtor( "~" + capitalize( newClass.name() ) );

  ctor.setBody( ctorCode );
  dtor.setBody( dtorCode );

  newClass.addFunction( ctor );
  newClass.addFunction( dtor );

  mClasses.append( newClass );
}

void Converter::createSimpleTypeSerializer( const Schema::SimpleType *type )
{
  const TQString typeName = mTypeMapper.type( type );
  const TQString baseType = mTypeMapper.type( type->baseTypeName() );

  KODE::Function marshal( "marshal", "void" );
  marshal.setStatic( true );
  marshal.addArgument( "TQDomDocument &doc" );
  marshal.addArgument( "TQDomElement &parent" );
  marshal.addArgument( "const TQString &name" );
  marshal.addArgument( "const " + typeName + "* value" );

  KODE::Function demarshal( "demarshal", "void" );
  demarshal.setStatic( true );
  demarshal.addArgument( "const TQDomElement &parent" );
  demarshal.addArgument( typeName + "* value" );

  KODE::Code marshalCode, demarshalCode, code;

  // include header
  TQMap<TQString, TQString> headerDec = mTypeMapper.headerDec( type->name() );
  TQMap<TQString, TQString>::ConstIterator it;
  for ( it = headerDec.begin(); it != headerDec.end(); ++it ) {
    if ( !it.key().isEmpty() )
      mSerializer.addInclude( it.key(), it.data() );

    if ( it.data().isEmpty() )
      mSerializer.addHeaderInclude( it.key() );
  }

  if ( type->subType() == Schema::SimpleType::TypeRestriction ) {
    // is an enumeration
    if ( type->facetType() & Schema::SimpleType::ENUM ) {
      TQStringList enums = type->facetEnums();
      TQStringList escapedEnums;
      for ( uint i = 0; i < enums.count(); ++i )
        escapedEnums.append( escapeEnum( enums[ i ] ) );

      // marshal value
      KODE::Function marshalValue( "marshalValue", "TQString" );
      marshalValue.setStatic( true );
      marshalValue.addArgument( "const " + typeName + "* value" );
      code += "switch ( value->type() ) {";
      code.indent();
      for ( uint i = 0; i < enums.count(); ++i ) {
        code += "case " + typeName + "::" + escapedEnums[ i ] + ":";
        code.indent();
        code += "return \"" + enums[ i ] + "\";";
        code += "break;";
        code.unindent();
      }
      code += "default:";
      code.indent();
      code += "qDebug( \"Unknown enum %d passed.\", value->type() );";
      code += "break;";
      code.unindent();
      code.unindent();
      code += "}";
      code.newLine();
      code += "return TQString();";
      marshalValue.setBody( code );

      // marshal
      marshalCode += "TQDomElement root = doc.createElement( name );";
      marshalCode += "root.setAttribute( \"xsi:type\", \"ns1:" + type->name() + "\" );";
      marshalCode += "parent.appendChild( root );";
      marshalCode += "root.appendChild( doc.createTextNode( Serializer::marshalValue( value ) ) );";

      // demarshal value
      KODE::Function demarshalValue( "demarshalValue", "void" );
      demarshalValue.setStatic( true );
      demarshalValue.addArgument( "const TQString &str" );
      demarshalValue.addArgument( typeName + "* value" );
      code.clear();
      for ( uint i = 0; i < enums.count(); ++i ) {
        code += "if ( str == \"" + enums[ i ] + "\" )";
        code.indent();
        code += "value->setType( " + typeName + "::" + escapedEnums[ i ] + " );";
        code.unindent();
        code.newLine();
      }
      demarshalValue.setBody( code );

      // demarshal
      demarshalCode += "Serializer::demarshalValue( parent.text(), value );";

      mSerializer.addFunction( marshalValue );
      mSerializer.addFunction( demarshalValue );
    } else if ( type->baseType() != Schema::XSDType::INVALID ) {
      marshalCode += "if ( value->value() ) {";
      marshalCode.indent();
      marshalCode += "Serializer::marshal( doc, parent, name, value->value() );";
      marshalCode.unindent();
      marshalCode += "}";

      demarshalCode += "const TQString text = parent.text();";
      demarshalCode.newLine();
      demarshalCode += "if ( !text.isEmpty() ) {";
      demarshalCode.indent();
      demarshalCode += baseType + "* data = new " + baseType + ";";
      demarshalCode += "Serializer::demarshal( parent, value );";
      demarshalCode += "value->setValue( data );";
      demarshalCode.unindent();
      demarshalCode += "}";

      KODE::Function marshalValue( "marshalValue", "TQString" );
      marshalValue.setStatic( true );
      marshalValue.addArgument( "const " + typeName + "* value" );
      marshalValue.setBody( "return Serializer::marshalValue( value->value() );" );

      mSerializer.addFunction( marshalValue );

      KODE::Function demarshalValue( "demarshalValue", "void" );
      demarshalValue.setStatic( true );
      demarshalValue.addArgument( "const TQString &str" );
      demarshalValue.addArgument( typeName + "* value" );
      KODE::Code code;
      code += baseType + "* data = new " + baseType + ";";
      code += "Serializer::demarshalValue( str, data );";
      code += "value->setValue( data );";
      demarshalValue.setBody( code );

      mSerializer.addFunction( demarshalValue );
    }
  } else if ( type->subType() == Schema::SimpleType::TypeList ) {
    const TQString listType = mTypeMapper.type( type->listTypeName() );

    mSerializer.addInclude( "tqstringlist.h" );

    marshalCode += "if ( value->entries() ) {";
    marshalCode.indent();
    marshalCode += "TQStringList list;";
    marshalCode += "TQPtrList<" + listType + ">* entries = value->entries();";
    marshalCode += "TQPtrListIterator<" + listType + "> it( *entries );";
    marshalCode += "while ( it.current() != 0 ) {";
    marshalCode.indent();
    marshalCode += "list.append( Serializer::marshalValue( it.current() ) );";
    marshalCode += "++it;";
    marshalCode.unindent();
    marshalCode += "}";
    marshalCode.newLine();
    marshalCode += "TQDomElement element = doc.createElement( name );";
    marshalCode += "parent.appendChild( element );";
    marshalCode += "element.appendChild( doc.createTextNode( list.join( \" \" ) ) );";
    marshalCode.unindent();
    marshalCode += "}";

    demarshalCode += "const TQStringList list = TQStringList::split( \" \", parent.text(), false );";
    demarshalCode += "if ( !list.isEmpty() ) {";
    demarshalCode.indent();
    demarshalCode += "TQPtrList<" + listType + ">* entries = new TQPtrList<" + listType + ">;";
    demarshalCode += "entries->setAutoDelete( true );";
    demarshalCode += "TQStringList::ConstIterator it;";
    demarshalCode += "for ( it = list.begin(); it != list.end(); ++it ) {";
    demarshalCode.indent();
    demarshalCode += listType + " *entry = new " + listType + ";";
    demarshalCode += "Serializer::demarshalValue( *it, entry );";
    demarshalCode += "entries->append( entry );";
    demarshalCode.unindent();
    demarshalCode += "}";
    demarshalCode.newLine();
    demarshalCode += "value->setEntries( entries );";
    demarshalCode.unindent();
    demarshalCode += "}";
  }

  marshal.setBody( marshalCode );
  mSerializer.addFunction( marshal );

  demarshal.setBody( demarshalCode );
  mSerializer.addFunction( demarshal );
}

void Converter::convertComplexType( const Schema::ComplexType *type )
{
  KODE::Class newClass( type->name() );
  newClass.addInclude( "serializer.h" );

  KODE::Code ctorCode, dtorCode;

  if ( type->baseType() != Schema::XSDType::ANYTYPE && !type->isArray() ) {
    TQString baseName = mTypeMapper.type( type->baseTypeName() );
    newClass.addBaseClass( KODE::Class( baseName ) );
    newClass.addHeaderIncludes( mTypeMapper.header( type->baseTypeName() ) );
  }

  if ( !type->documentation().isEmpty() )
    newClass.setDocs( type->documentation().simplifyWhiteSpace() );

  // elements
  Schema::Element::List elements = type->elements();
  Schema::Element::List::ConstIterator elemIt;
  for ( elemIt = elements.begin(); elemIt != elements.end(); ++elemIt ) {
    TQString typeName = mTypeMapper.type( &*elemIt );

    if ( (*elemIt).maxOccurs() > 1 )
      typeName = "TQPtrList<" + typeName + ">";

    // member variables
    KODE::MemberVariable variable( (*elemIt).name(), typeName + "*" );
    newClass.addMemberVariable( variable );

    ctorCode += variable.name() + " = 0;";
    dtorCode += "delete " + variable.name() + ";";
    dtorCode += variable.name() + " = 0;";

    TQString upperName = (*elemIt).name();
    TQString lowerName = (*elemIt).name();
    upperName[ 0 ] = upperName[ 0 ].upper();
    lowerName[ 0 ] = lowerName[ 0 ].lower();

    // setter method
    KODE::Function setter( "set" + upperName, "void" );
    setter.addArgument( mTypeMapper.argument( mNameMapper.escape( lowerName ), &*elemIt ) );
    setter.setBody( variable.name() + " = " + mNameMapper.escape( lowerName ) + ";" );

    // getter method
    KODE::Function getter( mNameMapper.escape( lowerName ), typeName + "*" );
    getter.setBody( " return " + variable.name() + ";" );
    getter.setConst( true );

    newClass.addFunction( setter );
    newClass.addFunction( getter );

    // include header
    TQMap<TQString, TQString> headerDec = mTypeMapper.headerDec( &*elemIt);
    TQMap<TQString, TQString>::ConstIterator it;
    for ( it = headerDec.begin(); it != headerDec.end(); ++it ) {
      if ( !it.key().isEmpty() )
        newClass.addInclude( it.key(), it.data() );

      if ( it.data().isEmpty() )
        newClass.addHeaderInclude( it.key() );
    }
  }

  // attributes
  Schema::Attribute::List attributes = type->attributes();
  Schema::Attribute::List::ConstIterator attrIt;
  for ( attrIt = attributes.begin(); attrIt != attributes.end(); ++attrIt ) {
    const TQString typeName = mTypeMapper.type( &*attrIt );

    // member variables
    KODE::MemberVariable variable( (*attrIt).name(), typeName + "*" );
    newClass.addMemberVariable( variable );

    ctorCode += variable.name() + " = 0;";
    dtorCode += "delete " + variable.name() + ";";
    dtorCode += variable.name() + " = 0;";

    TQString upperName = (*attrIt).name();
    TQString lowerName = (*attrIt).name();
    upperName[ 0 ] = upperName[ 0 ].upper();
    lowerName[ 0 ] = lowerName[ 0 ].lower();

    // setter method
    KODE::Function setter( "set" + upperName, "void" );
    setter.addArgument( mTypeMapper.argument( mNameMapper.escape( lowerName ), &*attrIt ) );
    setter.setBody( variable.name() + " = " + mNameMapper.escape( lowerName ) + ";" );

    // getter method
    KODE::Function getter( mNameMapper.escape( lowerName ), typeName + "*" );
    getter.setBody( " return " + variable.name() + ";" );
    getter.setConst( true );

    newClass.addFunction( setter );
    newClass.addFunction( getter );

    // include header
    TQMap<TQString, TQString> headerDec = mTypeMapper.headerDec( &*attrIt);
    TQMap<TQString, TQString>::ConstIterator it;
    for ( it = headerDec.begin(); it != headerDec.end(); ++it ) {
      if ( !it.key().isEmpty() )
        newClass.addInclude( it.key(), it.data() );

      if ( it.data().isEmpty() )
        newClass.addHeaderInclude( it.key() );
    }
  }

  createComplexTypeSerializer( type );

  KODE::Function ctor( capitalize( newClass.name() ) );
  KODE::Function dtor( "~" + capitalize( newClass.name() ) );

  ctor.setBody( ctorCode );
  dtor.setBody( dtorCode );

  newClass.addFunction( ctor );
  newClass.addFunction( dtor );

  mClasses.append( newClass );
}

void Converter::createComplexTypeSerializer( const Schema::ComplexType *type )
{
  const TQString typeName = mTypeMapper.type( type );

  KODE::Function marshal( "marshal", "void" );
  marshal.setStatic( true );
  marshal.addArgument( "TQDomDocument &doc" );
  marshal.addArgument( "TQDomElement &parent" );
  marshal.addArgument( "const TQString &name" );
  marshal.addArgument( "const " + typeName + "* value" );

  KODE::Function demarshal( "demarshal", "void" );
  demarshal.setStatic( true );
  demarshal.addArgument( "const TQDomElement &parent" );
  demarshal.addArgument( typeName + "* value" );

  KODE::Code marshalCode, demarshalCode, demarshalFinalCode;

  // include header
  TQMap<TQString, TQString> headerDec = mTypeMapper.headerDec( type->name() );
  TQMap<TQString, TQString>::ConstIterator it;
  for ( it = headerDec.begin(); it != headerDec.end(); ++it ) {
    if ( !it.key().isEmpty() )
      mSerializer.addInclude( it.key(), it.data() );

    if ( it.data().isEmpty() )
      mSerializer.addHeaderInclude( it.key() );
  }

  marshalCode += "TQDomElement root = doc.createElement( name );";
  marshalCode += "root.setAttribute( \"xsi:type\", \"ns1:" + typeName + "\" );";
  marshalCode += "parent.appendChild( root );";

  demarshalCode += "TQDomNode node = parent.firstChild();";
  demarshalCode += "while ( !node.isNull() ) {";
  demarshalCode.indent();
  demarshalCode += "TQDomElement element = node.toElement();";
  demarshalCode += "if ( !element.isNull() ) {";
  demarshalCode.indent();

  // elements
  Schema::Element::List elements = type->elements();
  Schema::Element::List::ConstIterator elemIt;
  for ( elemIt = elements.begin(); elemIt != elements.end(); ++elemIt ) {
    const TQString typeName = mTypeMapper.type( &*elemIt );

    TQString upperName = (*elemIt).name();
    TQString lowerName = (*elemIt).name();
    upperName[ 0 ] = upperName[ 0 ].upper();
    lowerName[ 0 ] = lowerName[ 0 ].lower();

    KODE::Function setter( "set" + upperName, "void" );
    KODE::Function getter( mNameMapper.escape( lowerName ), typeName + "*" );

    if ( (*elemIt).maxOccurs() > 1 ) {
      marshalCode += "if ( value->" + mNameMapper.escape( lowerName ) + "() ) {";
      marshalCode.indent();
      marshalCode += "const TQPtrList<" + typeName + ">* list = value->" + mNameMapper.escape( lowerName ) + "();";
      marshalCode.newLine();
      marshalCode += "TQDomElement element = doc.createElement( name );";
      // no idea about the namespaces here...
      marshalCode += "element.setAttribute( \"xmlns:ns1\", \"http://schemas.xmlsoap.org/soap/encoding/\" );";
      marshalCode += "element.setAttribute( \"xsi:type\", \"ns1:Array\" );";
      marshalCode += "element.setAttribute( \"ns1:arrayType\", \"ns1:" + typeName + "[\" + TQString::number( list->count() ) + \"]\" );";
      marshalCode += "parent.appendChild( element );";
      marshalCode.newLine();
      marshalCode += "TQPtrListIterator<" + typeName + "> it( *list );";
      marshalCode += "while ( it.current() != 0 ) {";
      marshalCode.indent();
      marshalCode += "Serializer::marshal( doc, element, \"item\", it.current() );";
      marshalCode += "++it;";
      marshalCode.unindent();
      marshalCode += "}";
      marshalCode.unindent();
      marshalCode += "}";

      const TQString listName = mNameMapper.escape( lowerName ) + "List";
      // TODO: prepend the code somehow
      KODE::Code code;
      code += "TQPtrList<" + typeName + ">* " + listName + " = new TQPtrList<" + typeName + ">();";
      code += listName + "->setAutoDelete( true );";
      code += demarshalCode;
      demarshalCode = code;
      demarshalCode += "if ( element.tagName() == \"item\" ) {";
      demarshalCode.indent();
      demarshalCode += typeName + " *item = new " + typeName + ";";
      demarshalCode += "Serializer::demarshal( element, item );";
      demarshalCode += listName + "->append( item );";
      demarshalCode.unindent();
      demarshalCode += "}";

      demarshalFinalCode += "value->" + setter.name() + "( " + listName + " );";
    } else {
      marshalCode += "if ( value->" + getter.name() + "() ) {";
      marshalCode.indent();
      marshalCode += "Serializer::marshal( doc, root, \"" + (*elemIt).name() + "\", value->" + getter.name() + "() );";
      marshalCode.unindent();
      marshalCode += "}";

      demarshalCode += "if ( element.tagName() == \"" + (*elemIt).name() + "\" ) {";
      demarshalCode.indent();
      demarshalCode += typeName + "* item = new " + typeName + ";";
      demarshalCode += "Serializer::demarshal( element, item );";
      demarshalCode += "value->" + setter.name() + "( item );";
      demarshalCode.unindent();
      demarshalCode += "}";
    }
  }

  // attributes
  Schema::Attribute::List attributes = type->attributes();
  Schema::Attribute::List::ConstIterator attrIt;
  for ( attrIt = attributes.begin(); attrIt != attributes.end(); ++attrIt ) {
    const TQString typeName = mTypeMapper.type( &*attrIt );

    TQString upperName = (*attrIt).name();
    TQString lowerName = (*attrIt).name();
    upperName[ 0 ] = upperName[ 0 ].upper();
    lowerName[ 0 ] = lowerName[ 0 ].lower();

    KODE::Function setter( "set" + upperName, "void" );
    KODE::Function getter( mNameMapper.escape( lowerName ), typeName + "*" );

    marshalCode += "if ( value->" + mNameMapper.escape( lowerName ) + "() )";
    marshalCode.indent();
    marshalCode += "parent.setAttribute( \"" + (*attrIt).name() + "\","
                                         "Serializer::marshalValue( value->" + mNameMapper.escape( lowerName ) + "() ) );";
    marshalCode.newLine();

    demarshalCode += "if ( element.hasAttribute( \"" + (*attrIt).name() + "\" ) ) {";
    demarshalCode.indent();
    demarshalCode += typeName + "* item = new " + typeName + ";";
    demarshalCode += "Serializer::demarshalValue( element.attribute( \"" + (*attrIt).name() + "\" ), item );";
    demarshalCode += "value->" + setter.name() + "( item );";
    demarshalCode.unindent();
    demarshalCode += "}";
  }

  demarshalCode.unindent();
  demarshalCode += "}";
  demarshalCode += "node = node.nextSibling();";
  demarshalCode.unindent();
  demarshalCode += "}";
  demarshalCode.newLine();
  demarshalCode += demarshalFinalCode;

  marshal.setBody( marshalCode );
  mSerializer.addFunction( marshal );

  demarshal.setBody( demarshalCode );
  mSerializer.addFunction( demarshal );
}

void Converter::convertService( const Service &service )
{
  KODE::Class newClass( service.name() );
  newClass.addBaseClass( mQObject );
  newClass.addHeaderInclude( "tqobject.h" );
  newClass.addHeaderInclude( "tqstring.h" );
  newClass.addHeaderInclude( "transport.h" );

  newClass.addInclude( "serializer.h" );

  KODE::Function ctor( service.name() );
  KODE::Function dtor( "~" + service.name() );
  KODE::Code ctorCode, dtorCode;

  const Service::Port::List servicePorts = service.ports();
  Service::Port::List::ConstIterator it;
  for ( it = servicePorts.begin(); it != servicePorts.end(); ++it ) {
    Binding binding = mWSDL.findBinding( (*it).mBinding );

    Port port = mWSDL.findPort( binding.type() );
    const Port::Operation::List operations = port.operations();
    Port::Operation::List::ConstIterator opIt;
    for ( opIt = operations.begin(); opIt != operations.end(); ++opIt ) {
      Message inputMessage = mWSDL.findMessage( (*opIt).input() );
      Message outputMessage = mWSDL.findMessage( (*opIt).output() );

      convertInputMessage( port, inputMessage, newClass );
      convertOutputMessage( port, outputMessage, newClass );

      KODE::MemberVariable transport( inputMessage.name() + "Transport", "Transport" );
      ctorCode += transport.name() + " = new Transport( \"" + (*it).mLocation + "\" );";

      ctorCode += "connect( " + transport.name() + ", TQT_SIGNAL( result( const TQString& ) ),";
      ctorCode.indent();
      ctorCode += "this, TQT_SLOT( " + outputMessage.name() + "Slot( const TQString& ) ) );";
      ctorCode.unindent();

      dtorCode += "delete " + transport.name() + ";";
      dtorCode += transport.name() + " = 0;";
    }
  }

  ctor.setBody( ctorCode );
  newClass.addFunction( ctor );

  dtor.setBody( dtorCode );
  newClass.addFunction( dtor );

  mClasses.append( newClass );
}

void Converter::convertInputMessage( const Port &port, const Message &message, KODE::Class &newClass )
{
  KODE::MemberVariable transport( message.name() + "Transport", "Transport*" );
  newClass.addMemberVariable( transport );

  // call
  TQString messageName = message.name();
  messageName[ 0 ] = messageName[ 0 ].lower();
  KODE::Function callFunc( mNameMapper.escape( messageName ), "void", KODE::Function::Public );

  const Message::Part::List parts = message.parts();
  Message::Part::List::ConstIterator it;
  for ( it = parts.begin(); it != parts.end(); ++it ) {
    newClass.addHeaderIncludes( mTypeMapper.header( (*it).type() ) );
    TQString lowerName = (*it).name();
    lowerName[ 0 ] = lowerName[ 0 ].lower();
    callFunc.addArgument( mTypeMapper.argument( mNameMapper.escape( lowerName ), (*it).type() ) );
  }

  KODE::Code code;
  code += "TQDomDocument doc( \"kwsdl\" );";
  code += "doc.appendChild( doc.createProcessingInstruction( \"xml\", \"version=\\\"1.0\\\" encoding=\\\"UTF-8\\\"\" ) );";
  code += "TQDomElement env = doc.createElement( \"SOAP-ENV:Envelope\" );";
  code += "env.setAttribute( \"xmlns:SOAP-ENV\", \"http://schemas.xmlsoap.org/soap/envelope/\" );";
  code += "env.setAttribute( \"xmlns:xsi\", \"http://www.w3.org/1999/XMLSchema-instance\" );";
  code += "env.setAttribute( \"xmlns:xsd\", \"http://www.w3.org/1999/XMLSchema\" );";
  code += "doc.appendChild( env );";
  code += "TQDomElement body = doc.createElement( \"SOAP-ENV:Body\" );";
  code += "env.appendChild( body );";
  code += "TQDomElement method = doc.createElement( \"ns1:" + message.name() + "\" );";
  TQString nameSpace = mWSDL.findBindingOperation( port.name(), message.name() ).input().nameSpace();
  code += "method.setAttribute( \"xmlns:ns1\", \"" + nameSpace + "\" );";
  code += "method.setAttribute( \"SOAP-ENV:encodingStyle\", \"http://schemas.xmlsoap.org/soap/encoding/\" );";
  code += "body.appendChild( method );";
  code.newLine();

  for ( it = parts.begin(); it != parts.end(); ++it ) {
    TQString lowerName = (*it).name();
    lowerName[ 0 ] = lowerName[ 0 ].lower();
    code += "Serializer::marshal( doc, method, \"" + (*it).name() + "\", " + mNameMapper.escape( lowerName ) + " );";
    code += "delete " + mNameMapper.escape( lowerName ) + ";";
  }

  code += "qDebug( \"%s\", doc.toString().latin1() );";
  code += transport.name() + "->query( doc.toString() );";
  callFunc.setBody( code );

  newClass.addFunction( callFunc );
}

void Converter::convertOutputMessage( const Port&, const Message &message, KODE::Class &newClass )
{
  Message::Part part = message.parts().first();

  // signal
  TQString messageName = message.name();
  messageName[ 0 ] = messageName[ 0 ].lower();
  KODE::Function respSignal( messageName, "void", KODE::Function::Signal );

  /**
    If one output message is used by two input messages, don't define
    it twice.
   */
  if ( newClass.hasFunction( respSignal.name() ) )
    return;

  const Message::Part::List parts = message.parts();
  Message::Part::List::ConstIterator it;
  for ( it = parts.begin(); it != parts.end(); ++it ) {
    TQStringList headers = mTypeMapper.header( (*it).type() );
    for ( uint i = 0; i < headers.count(); ++i )
      if ( !headers[ i ].isEmpty() )
        newClass.addHeaderInclude( headers[ i ] );

    respSignal.addArgument( mTypeMapper.argument( "value", (*it).type() ) );
  }

  newClass.addFunction( respSignal );

  // slot
  KODE::Function respSlot( messageName + "Slot", "void", KODE::Function::Slot | KODE::Function::Private );
  respSlot.addArgument( "const TQString &xml" );

  KODE::Code code;
  code += "TQDomDocument doc;";
  code += "TQString errorMsg;";
  code += "int column, row;";
  code.newLine();
  code += "qDebug( \"%s\", xml.latin1() );";
  code.newLine();
  code += "if ( !doc.setContent( xml, true, &errorMsg, &row, &column ) ) {";
  code.indent();
  code += "qDebug( \"Unable to parse xml: %s (%d:%d)\", errorMsg.latin1(), row, column );";
  code += "return;";
  code.unindent();
  code += "}";
  code.newLine();
  code += mTypeMapper.type( part.type() ) + "* value = new " + mTypeMapper.type( part.type() ) + ";";
  code += "TQDomElement envelope = doc.documentElement();";
  code += "TQDomElement body = envelope.firstChild().toElement();";
  code += "TQDomElement method = body.firstChild().toElement();";
  code += "Serializer::demarshal( method.firstChild().toElement(), value );";
  code.newLine();
  code += "emit " + respSignal.name() + "( value );";
  respSlot.setBody( code );

  newClass.addFunction( respSlot );
}

void Converter::createUtilClasses()
{
  mSerializer = KODE::Class( "Serializer" );
  mSerializer.addHeaderInclude( "tqcstring.h" );
  mSerializer.addHeaderInclude( "tqdom.h" );
  mSerializer.addHeaderInclude( "tqdatetime.h" );
  mSerializer.addHeaderInclude( "tqstring.h" );
  mSerializer.addHeaderInclude( "tqptrlist.h" );
  mSerializer.addInclude( "kmdcodec.h" );

  typedef struct {
    TQString type;
    TQString xsdType;
    TQString marshalCode;
    TQString demarshalCode;
  } TypeEntry;

  /**
    I know the following code looks a bit ugly, but it saves us a lot
    of typing and is easier to maintain at the end ;)
   */
  TypeEntry types[] = {
    { "TQString", "xsd:string", "*value", "str" },
    { "bool", "xsd:boolean", "(*value ? \"true\" : \"false\")", "(str.lower() == \"true\" ? true : false)" },
    { "float", "xsd:TODO", "TQString::number( *value )", "str.toFloat()" },
    { "int", "xsd:int", "TQString::number( *value )", "str.toInt()" },
    { "unsigned int", "xsd:unsignedByte", "TQString::number( *value )", "str.toUInt()" },
    { "double", "xsd:double", "TQString::number( *value )", "str.toDouble()" },
    { "char", "xsd:byte", "TQString( TQChar( *value ) )", "str[ 0 ].latin1()" },
    { "unsigned char", "xsd:unsignedByte", "TQString( TQChar( *value ) )", "str[ 0 ].latin1()" },
    { "short", "xsd:short", "TQString::number( *value )", "str.toShort()" },
    { "TQByteArray", "xsd:base64Binary", "TQString::fromUtf8( KCodecs::base64Encode( *value ) )", "KCodecs::base64Decode( str.utf8() )" },
    { "TQDateTime", "xsd:dateTime", "value->toString( Qt::ISODate )", "TQDateTime::fromString( str, Qt::ISODate )" },
    { "TQDate", "xsd:date", "value->toString( Qt::ISODate )", "TQDate::fromString( str, Qt::ISODate )" }
  };

  for ( uint i = 0; i < sizeof( types ) / sizeof( TypeEntry ); ++i ) {
    KODE::Function marshal, demarshal;
    KODE::Code code;

    TypeEntry entry = types[ i ]; 

    marshal = KODE::Function( "marshalValue", "TQString" );
    marshal.setStatic( true );
    marshal.addArgument( "const " + entry.type + "* value" );

    code.clear();
    marshal.setBody( "return " + entry.marshalCode + ";" );

    mSerializer.addFunction( marshal );

    demarshal = KODE::Function( "demarshalValue", "void" );
    demarshal.setStatic( true );
    demarshal.addArgument( "const TQString &str" );
    demarshal.addArgument( entry.type + " *value" );
    demarshal.setBody( "*value = " + entry.demarshalCode + ";" );

    mSerializer.addFunction( demarshal );

    marshal = KODE::Function( "marshal", "void" );
    marshal.setStatic( true );
    marshal.addArgument( "TQDomDocument &doc" );
    marshal.addArgument( "TQDomElement &parent" );
    marshal.addArgument( "const TQString &name" );
    marshal.addArgument( "const " + entry.type + "* value" );

    code.clear();
    code += "TQDomElement element = doc.createElement( name );";
    code += "element.setAttribute( \"xsi:type\", \"" + entry.xsdType + "\" );";
    code += "element.appendChild( doc.createTextNode( Serializer::marshalValue( value ) ) );";
    code += "parent.appendChild( element );";
    marshal.setBody( code );

    mSerializer.addFunction( marshal );

    demarshal = KODE::Function( "demarshal", "void" );
    demarshal.setStatic( true );
    demarshal.addArgument( "const TQDomElement &element" );
    demarshal.addArgument( entry.type + "* value" );
    demarshal.setBody( "Serializer::demarshalValue( element.text(), value );" );

    mSerializer.addFunction( demarshal );
  }
}

void Converter::createTransportClass()
{
  KODE::Class transport( "Transport" );
  transport.addBaseClass( mQObject );
  transport.addHeaderInclude( "tqobject.h" );
  transport.addHeaderInclude( "kio/job.h" );

  transport.addInclude( "kdebug.h" );

  KODE::MemberVariable url( "url", "TQString" );
  transport.addMemberVariable( url );

  KODE::MemberVariable slotDataVar( "data", "TQByteArray" );
  transport.addMemberVariable( slotDataVar );

  // ctor
  KODE::Function ctor( "Transport" );
  ctor.addArgument( "const TQString &url" );
  ctor.setBody( url.name() + " = url;" );

  transport.addFunction( ctor );

  // query
  KODE::Function query( "query", "void" );
  query.addArgument( "const TQString &xml" );

  KODE::Code queryCode;
  queryCode += slotDataVar.name() + ".truncate( 0 );";
  queryCode.newLine();
  queryCode += "TQByteArray postData;";
  queryCode += "TQDataStream stream( postData, IO_WriteOnly );";
  queryCode += "stream.writeRawBytes( xml.utf8(), xml.utf8().length() );";
  queryCode.newLine();
  queryCode += "KIO::TransferJob* job = KIO::http_post( KURL( " + url.name() + " ), postData, false );";
  queryCode += "if ( !job ) {";
  queryCode.indent();
  queryCode += "kdWarning() << \"Unable to create KIO job for \" << " + url.name() + " << endl;";
  queryCode += "return;";
  queryCode.unindent();
  queryCode += "}";
  queryCode.newLine();
  queryCode += "job->addMetaData( \"UserAgent\", \"KWSDL\" );";
  queryCode += "job->addMetaData( \"content-type\", \"Content-Type: text/xml; charset=utf-8\" );";
  queryCode.newLine();
  queryCode += "connect( job, TQT_SIGNAL( data( KIO::Job*, const TQByteArray& ) ), this, TQT_SLOT( slotData( KIO::Job*, const TQByteArray& ) ) );";
  queryCode += "connect( job, TQT_SIGNAL( result( KIO::Job* ) ), this, TQT_SLOT( slotResult( KIO::Job* ) ) );";

  query.setBody( queryCode );

  transport.addFunction( query );

  // signal
  KODE::Function result( "result", "void", KODE::Function::Signal );
  result.addArgument( "const TQString &xml" );

  transport.addFunction( result );

  // data slot
  KODE::Function slotData( "slotData", "void", KODE::Function::Private | KODE::Function::Slot );

  slotData.addArgument( "KIO::Job*" );
  slotData.addArgument( "const TQByteArray &data" );

  KODE::Code slotDataCode;
  slotDataCode += "unsigned int oldSize = " + slotDataVar.name() + ".size();";
  slotDataCode += slotDataVar.name() + ".resize( oldSize + data.size() );";
  slotDataCode += "memcpy( " + slotDataVar.name() + ".data() + oldSize, data.data(), data.size() );";

  slotData.setBody( slotDataCode );

  transport.addFunction( slotData );

  // result slot
  KODE::Function slotResult( "slotResult", "void", KODE::Function::Private | KODE::Function::Slot );
  slotResult.addArgument( "KIO::Job* job" );

  KODE::Code slotResultCode;
  slotResultCode += "if ( job->error() != 0 ) {";
  slotResultCode.indent();
  slotResultCode += "kdWarning() << \"Error occurred \" << job->errorText() << endl;";
  slotResultCode += "kdWarning() << " + slotDataVar.name() + " << endl;";
  slotResultCode += "return;";
  slotResultCode.unindent();
  slotResultCode += "}";
  slotResultCode.newLine();

  slotResultCode += "emit result( TQString::fromUtf8( " + slotDataVar.name() + ".data(), " + slotDataVar.name() + ".size() ) );";

  slotResult.setBody( slotResultCode );

  transport.addFunction( slotResult );

  mClasses.append( transport );
}
