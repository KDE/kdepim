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

#include "function.h"

#include <kdebug.h>

using namespace KODE;

Function::Function()
  : mAccess( Public ), mIsConst( false ), mIsStatic( false )
{
}

Function::Function( const TQString &name, const TQString &returnType,
                    int access, bool isStatic )
  :  mAccess( access ), mIsConst( false ), mIsStatic( isStatic ),
     mReturnType( returnType ), mName( name )
{
}
 
void Function::setConst( bool isConst )
{
  mIsConst = isConst;
} 
   
void Function::setStatic( bool isStatic )
{
  mIsStatic = isStatic;
}
   
void Function::addArgument( const TQString &argument )
{
  mArguments.append( argument );
}

void Function::addInitializer( const TQString &initializer )
{
  mInitializers.append( initializer );
}

void Function::setArgumentString( const TQString &argumentString )
{
  mArguments.clear();
  TQStringList arguments = TQStringList::split( ",", argumentString );
  TQStringList::ConstIterator it;
  for( it = arguments.begin(); it != arguments.end(); ++it ) {
    addArgument( *it );
  }
}

void Function::setBody( const TQString &body )
{
  mBody = body;
}

void Function::setBody( const Code &body )
{
  mBody = body.text();
}

void Function::addBodyLine( const TQString &bodyLine )
{
  mBody.append( bodyLine );
  if ( bodyLine.right( 1 ) != "\n" ) mBody.append( '\n' );
}

void Function::setAccess( int a )
{
  mAccess = a;
}

TQString Function::accessAsString() const
{
  TQString access;

  if ( mAccess & Public )
    access = "public";
  if ( mAccess & Protected )
    access = "protected";
  if ( mAccess & Private )
    access = "private";

  if ( mAccess & Signal )
    access = "signals";
  if ( mAccess & Slot )
    access += " slots";

  return access;
}

void Function::setReturnType( const TQString &str )
{
  mReturnType = str;
}

void Function::setName( const TQString &str )
{
  mName = str;
}

void Function::setDocs( const TQString &str )
{
  mDocs = str;
}
