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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <kdebug.h>
#include <qdir.h>
#include <qfile.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtextstream.h>

#include "script.h"

static QString srcdir();
static int runscripts
( const QString &interpreter, const QString &extension, const QString &path );

// Read srcdir from Makefile (for builddir != srcdir).
QString srcdir()
{
  bool found = false;
  QString dir;

  QFile file( "Makefile" );
  if ( !file.open( IO_ReadOnly  | IO_Translate ) ) return "";

  QTextStream in( &file );
  QString line;
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
( const QString &interpreter, const QString &extension, const QString &path )
{
  int rval = 0;
  Script* s = new Script();
  QStringList files;

  QDir dir( path );

  dir.setNameFilter( extension );
  dir.setFilter( QDir::Files );
  dir.setSorting( QDir::Name | QDir::IgnoreCase );
  const QFileInfoList *list = dir.entryInfoList();
  QFileInfoListIterator it( *list );
  QFileInfo *fi;
  while ( !rval && (fi = it.current()) != 0 ) 
  {
    s->addArgument( interpreter );
    s->addArgument( path + QDir::separator() + fi->fileName().latin1() );
    ++it;
    rval = s->run();
  }
  delete s;
  s = 0;
  
  return rval;
}

int main( int, char** )
{
  int rval = 0;

  QString path = srcdir();

  //if ( !rval ) rval = runscripts( "python", "*.py *.Py *.PY *.pY", path );

  if ( !rval ) rval = runscripts( "sh", "*.sh *.Sh *.SH *.sH", path );

  if ( !rval ) rval = runscripts( "perl", "*.pl *.Pl *.PL *.pL", path );

  return rval;
}
