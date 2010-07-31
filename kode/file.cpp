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

#include "file.h"

#include <kdebug.h>

#include <tqfile.h>
#include <tqtextstream.h>

using namespace KODE;

File::File()
  : mProject( "KDE" )
{
}

void File::setFilename( const TQString &filename )
{
  mFilename = filename;
}

TQString File::filename() const
{
  if ( !mFilename.isEmpty() ) return mFilename;

  if ( !mClasses.isEmpty() ) {
    TQString className = mClasses[ 0 ].name();
    return className.lower();
  }

  return TQString::null;
}

void File::setNameSpace( const TQString &n )
{
  mNameSpace = n;
}

void File::setProject( const TQString &project )
{
  if ( project.isEmpty() ) return;
  mProject = project;
}

void File::addCopyright( int year, const TQString &name, const TQString &email )
{
  TQString str = "Copyright (c) " + TQString::number( year ) + " " + name + " <" +
                email + ">";
  mCopyrightStrings.append( str );
}

void File::setLicense( const License &l )
{
  mLicense = l;
}

void File::addInclude( const TQString &i )
{
  TQString include = i;
  if( !include.endsWith( ".h" ) ) include.append( ".h" );

  if ( mIncludes.find( include ) == mIncludes.end() ) {
    mIncludes.append( include );
  }
}

void File::insertClass( const Class &c )
{
  Class::List::Iterator it;
  for( it = mClasses.begin(); it != mClasses.end(); ++it ) {
    if ( (*it).name() == c.name() ) {
      it = mClasses.remove( it );
      mClasses.insert( it, c );
      return;
    }
  }
  
  mClasses.append( c );
}

bool File::hasClass( const TQString &name )
{
  Class::List::ConstIterator it;
  for( it = mClasses.begin(); it != mClasses.end(); ++it ) {
    if ( (*it).name() == name ) break;
  }
  return it != mClasses.end();
}

Class File::findClass( const TQString &name )
{
  Class::List::ConstIterator it;
  for( it = mClasses.begin(); it != mClasses.end(); ++it ) {
    if ( (*it).name() == name ) return *it;
  }
  return Class();
}

void File::addFileVariable( const Variable &v )
{
  mFileVariables.append( v );
}

void File::addFileFunction( const Function &f )
{
  mFileFunctions.append( f );
}

void File::addExternCDeclaration( const TQString &s )
{
  mExternCDeclarations.append( s );
}

void File::clearClasses()
{
  mClasses.clear();
}

void File::clearFileFunctions()
{
  mFileFunctions.clear();
}

void File::clearFileVariables()
{
  mFileVariables.clear();
}

void File::clearCode()
{
  clearClasses();
  clearFileFunctions();
  clearFileVariables();
}

void File::addFileCode( const Code &c )
{
  mFileCode = c;
}
