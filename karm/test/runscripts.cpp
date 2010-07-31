/* This file is part of the KDE project
   Copyright (C) 2004 Mark Bucciarelli <mark@hubcapconsulting.com>

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

#include <kdebug.h>
#include <tqdir.h>
#include <tqfile.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqtextstream.h>

#include "script.h"

static TQString srcdir();
static int runscripts
( const TQString &interpreter, const TQString &extension, const TQString &path );

const TQString dots = ".................................................."; 
const TQString not_a_test_filename_prefix = "__";

// Read srcdir from Makefile (for builddir != srcdir).
TQString srcdir()
{
  bool found = false;
  TQString dir;

  TQFile file( "Makefile" );
  if ( !file.open( IO_ReadOnly  | IO_Translate ) ) return "";

  TQTextStream in( &file );
  TQString line;
  while ( !found && !in.atEnd() )
  {
    line = in.readLine(); 
    if ( line.startsWith( "srcdir = " ) )
    {
      dir = line.mid( 9 );
      found = true;
    }
  }

  if ( !found ) dir = "";

  return dir;
}

int runscripts
( const TQString &interpreter, const TQString &extension, const TQString &path )
{
  int rval = 0;
  int oneBadApple = 0;
  TQStringList files;

  TQDir dir( path );

  Script* s = new Script( dir );

  dir.setNameFilter( extension );
  dir.setFilter( TQDir::Files );
  dir.setSorting( TQDir::Name | TQDir::IgnoreCase );
  const QFileInfoList *list = dir.entryInfoList();
  QFileInfoListIterator it( *list );
  TQFileInfo *fi;
  while ( !rval && ( fi = it.current() ) != 0 ) 
  {
    // Don't run scripts that are shared routines.
    if ( ! fi->fileName().startsWith( not_a_test_filename_prefix ) ) 
    {
      s->addArgument( interpreter );
      s->addArgument( path + TQDir::separator() + fi->fileName().latin1() );

      // Thorsten's xautomation tests run with user interaction by default.
      if ( interpreter == "sh" ) s->addArgument( "--batch" );
      if ( interpreter == "php" ) s->addArgument( "--batch" );

      rval = s->run();

      kdDebug() << "runscripts: " << fi->fileName() 
        << " " << dots.left( dots.length() - fi->fileName().length() )
        << " " << ( ! rval ? "PASS" : "FAIL" ) << endl;

      // Don't abort if one test files--run them all
      if ( rval ) 
      {
        oneBadApple = 1;
        rval = 0;
      }

      delete s;
      s = new Script( dir );
    }
    ++it;
  }
  delete s;
  s = 0;
  
  return oneBadApple;
}

int main( int, char** )
{
  int rval = 0;

  TQString path = srcdir();

  if ( !rval ) rval = runscripts( "python", "*.py *.Py *.PY *.pY", path );

  if ( !rval ) rval = runscripts( "sh", "*.sh *.Sh *.SH *.sH", path );

  if ( !rval ) rval = runscripts( "perl", "*.pl *.Pl *.PL *.pL", path );

  if ( !rval ) rval = runscripts( "php", "*.php *.php3 *.php4 *.php5", path );

  return rval;
}
