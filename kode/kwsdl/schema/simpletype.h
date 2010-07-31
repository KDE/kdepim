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

#ifndef SCHEMA_SIMPLETYPE_H
#define SCHEMA_SIMPLETYPE_H

#include <tqstringlist.h>

#include "qualifiedname.h"
#include "xsdtype.h"

namespace Schema {

class SimpleType : public XSDType
{
  public:
    typedef TQValueList<SimpleType> List;

    enum FacetType
    {
      NONE = 0,
      LENGTH = 1,
      MINLEN = 2,
      MAXLEN = 4,
      ENUM = 8,
      WSP = 16,
      MAXINC = 32,
      MININC = 64,
      MAXEX = 128,
      MINEX = 256,
      TOT = 512,
      FRAC = 1024,
      PATTERN = 2048
    };

    enum WhiteSpaceType
    {
      PRESERVE,
      REPLACE,
      COLLAPSE
    };

    enum SubType
    {
      TypeRestriction,
      TypeList,
      TypeUnion
    };

    SimpleType();
    SimpleType( const TQString &nameSpace );
    ~SimpleType();

    void setName( const TQString &name );
    TQString name() const;

    QualifiedName qualifiedName() const;

    void setDocumentation( const TQString &documentation );
    TQString documentation() const;

    void setType( int type );
    int type() const;

    void setBaseType( int baseType );
    int baseType() const;

    void setBaseTypeName( const TQString &baseTypeName );
    TQString baseTypeName() const;

    void setSubType( SubType subType );
    SubType subType() const;

    void setListType( int listType );
    void setListTypeName( const TQString &name );

    int listType() const;
    TQString listTypeName() const;

    void setContentModel( int contentModel );
    int contentModel() const;

    void setAnonymous( bool anonymous );
    bool isAnonymous() const;

    bool isValidFacet( const TQString &facet );
    void setFacetValue( const TQString &value );

    int facetType() const;

    int facetLength() const;
    int facetMinimumLength() const;
    int facetMaximumLength() const;
    TQStringList facetEnums() const;
    WhiteSpaceType facetWhiteSpace() const;
    int facetMinimumInclusive() const;
    int facetMaximumInclusive() const;
    int facetMinimumExclusive() const;
    int facetMaximumExclusive() const;
    int facetTotalDigits() const;
    int facetFractionDigits() const;
    TQString facetPattern() const;

  private:
    TQString mName;
    TQString mNameSpace;
    TQString mDocumentation;
    int mType;
    int mBaseType;
    TQString mBaseTypeName;
    int mContentModel;
    bool mRestriction;
    int mFacetId;
    bool mAnonymous;
    TQStringList mEnums;
    SubType mSubType;

    int mListType;
    TQString mListTypeName;

    typedef struct
    {
      int length;
      struct
      {
        int minlen, maxlen;
      } lenRange;
      WhiteSpaceType wsp;
      struct
      {
        int maxinc, mininc, maxex, minex;
      } valRange;
      int tot;
      int frac;
      TQString pattern;
    } FacetValueType;


    FacetValueType mFacetValue;
};

}

#endif
