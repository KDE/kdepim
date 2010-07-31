/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KODE_CLASS_H
#define KODE_CLASS_H

#include "enum.h"
#include "function.h"
#include "membervariable.h"
#include "typedef.h"

#include <kdepimmacros.h>
#include <tqvaluelist.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqptrlist.h>

namespace KODE {

class KDE_EXPORT Class
{
  public:
    typedef TQValueList<Class> List;
    
    Class();
    Class( const TQString &name, const TQString &nameSpace = TQString::null );

    Class( const Class & );
    Class &operator=( const Class &c );

    bool isValid() const;

    void addInclude( const TQString &file,
      const TQString &forwardDeclaration = TQString::null );
    void addHeaderInclude( const TQString &file );
    void addHeaderIncludes( const TQStringList &files );
    void addBaseClass( const Class & );
    void addFunction( const Function & );
    void addMemberVariable( const MemberVariable &v );
    void addTypedef( const Typedef & );
    void addEnum( const Enum & );

    void setName( const TQString &name );
    TQString name() const { return mName; }
    void setNameSpace( const TQString &nameSpace );
    TQString nameSpace() const { return mNameSpace; }
    TQStringList includes() const { return mIncludes; }
    TQStringList headerIncludes() const { return mHeaderIncludes; }
    TQStringList forwardDeclarations() const { return mForwardDeclarations; }
    Function::List functions() const { return mFunctions; }
    MemberVariable::List memberVariables() const { return mMemberVariables; }
    Class::List baseClasses() const;
    Typedef::List typedefs() const { return mTypedefs; }
    Enum::List enums() const { return mEnums; }

    void setDocs( const TQString & );
    TQString docs() const { return mDocs; }

    bool hasFunction( const TQString &name ) const;
    
    bool isQObject() const;
    
  private:
    // WARNING: If you add member variables, you have to adjust the copy
    //          constructor.
    TQString mName;
    TQString mNameSpace;
    Function::List mFunctions;
    MemberVariable::List mMemberVariables;
    TQStringList mIncludes;
    TQStringList mForwardDeclarations;
    TQStringList mHeaderIncludes;
    TQPtrList<Class> mBaseClasses;
    Typedef::List mTypedefs;
    Enum::List mEnums;
    TQString mDocs;
};

}

#endif
