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

#ifndef SCHEMA_ATTRIBUTE_H
#define SCHEMA_ATTRIBUTE_H

#include <tqstring.h>
#include <tqvaluelist.h>

namespace Schema {

class Attribute
{
  public:
    typedef TQValueList<Attribute> List;
    typedef TQValueList<Attribute*> PtrList;

    Attribute();

    Attribute( const TQString &name, int type, bool qualified = false,
               const TQString &defaultValue = TQString(),
               const TQString &fixedValue = TQString(),
               bool use = true );

    TQString name() const;
    int type() const;

    void setTypeName( const TQString &typeName );
    TQString typeName() const;

    TQString defaultValue() const;
    TQString fixedValue() const;

    bool isQualified() const;
    bool isUsed() const;

  private:
    TQString mName;
    int mType;
    TQString mTypeName;
    bool mQualified;
    TQString mDefaultValue;
    TQString mFixedValue;
    bool mUse;
};

}

#endif
