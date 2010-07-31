/* 
    This file is part of KDE Schema Parser

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>
                       based on wsdlpull parser by Vivek Krishna

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

#ifndef SCHEMA_PARSER_H
#define SCHEMA_PARSER_H

#include <tqdom.h>
#include <tqvaluelist.h>

#include "types.h"
#include "typestable.h"

namespace Schema {

class Parser
{
  public:
    Parser( const TQString &nameSpace = TQString() );

    ~Parser();

    Types types() const;

    void clear();
    void setSchemaBaseUrl( const TQString& );

    void parseNameSpace( const TQDomElement &element );
    bool parseSchemaTag( const TQDomElement &element );

    const XSDType *type( const QualifiedName &type );
    const XSDType *type( int id ) const;
    int numTypes() const;

    TQString typeName( int id ) const;

    Element *element( const QualifiedName &element ) const;
    Element *element( int id ) const;
    Element::PtrList elements() const;
    int numElements() const;

    Attribute *attribute( const QualifiedName &attribute ) const;
    Attribute *attribute( int id ) const;
    int numAttributes() const;

    TQString targetNamespace() const;

    int typeId( const QualifiedName &name, bool create = false );

    bool isBasicType( int sType ) const;

    bool finalize();

    int elementId( const QualifiedName &type );
    int elementType( const QualifiedName &type );
    int attributeId( const QualifiedName &type ) const;
    int attributeType( const QualifiedName &type );

  private:
    void parseImport( const TQDomElement& );
    void parseElement( const TQDomElement& );
    void parseAttribute( const TQDomElement& );

    void parseAnnotation( const TQDomElement& );
    void parseAnnotation( const TQDomElement&, TQString& );
    void parseAnnotation( const TQDomElement&, ComplexType* );
    void parseAnnotation( const TQDomElement&, SimpleType* );
    XSDType *parseComplexType( const TQDomElement& );

    void all( const TQDomElement&, ComplexType* );
    void cs( const TQDomElement&, ComplexType* );

    void addElement( const TQDomElement&, ComplexType* );

    void addAttribute( const TQDomElement&, ComplexType* );
    void addAny( const TQDomElement&, ComplexType* );
    void addAnyAttribute( const TQDomElement&, ComplexType* );
    int addExternalElement( const TQString&, int );

    XSDType *parseSimpleType( const TQDomElement& );
    void parseRestriction( const TQDomElement&, SimpleType* );
    void parseComplexContent( const TQDomElement&, ComplexType* );
    void parseSimpleContent( const TQDomElement&, ComplexType* );


    void resolveForwardElementRefs();
    void resolveForwardAttributeRefs();
    void resolveForwardDerivations();
    bool shouldResolve();

    void importSchema( const TQString &location );

    bool mElementQualified;
    bool mAttributeQualified;
    TQMap<TQString, TQString> mNameSpaceMap;
    TQString mNameSpace;
    TQString mPrefix;
  
    TypesTable mTypesTable;
    Element::PtrList mElements;
    Attribute::PtrList mAttributes;

    QualifiedName::List mForwardElementRef;
    QualifiedName::List mForwardAttributeRef;

    typedef struct {
      QualifiedName type;
      QualifiedName baseType;
      ComplexType::Derivation derivation;
    } ForwardDerivation;

    TQValueList<ForwardDerivation> mForwardDerivations;
    TQStringList mImportedSchemas;
    TQString mSchemaBaseUrl;
};

}

#endif


