/*
    This file is part of kdepim.

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

#include <tqdatastream.h>
#include <tqfile.h>

#include <kdebug.h>
#include <kstandarddirs.h>

#include "todostatemapper.h"

TQDataStream& operator<<( TQDataStream &stream, const TodoStateMapper::TodoStateMapEntry &entry )
{
  stream << entry.uid << entry.localState << entry.remoteState;

  return stream;
}

TQDataStream& operator>>( TQDataStream &stream, TodoStateMapper::TodoStateMapEntry &entry )
{
  stream >> entry.uid >> entry.localState >> entry.remoteState;

  return stream;
}

TodoStateMapper::TodoStateMapper()
{
}

TodoStateMapper::~TodoStateMapper()
{
}

void TodoStateMapper::setPath( const TQString &path )
{
  mPath = path;
}

void TodoStateMapper::setIdentifier( const TQString &identifier )
{
  mIdentifier = identifier;
}

bool TodoStateMapper::load()
{
  TQFile file( filename() );
  if ( !file.open( IO_ReadOnly ) ) {
    kdError() << "Can't read uid map file '" << filename() << "'" << endl;
    return false;
  }

  clear();

  TQDataStream stream;
  stream.setVersion( 6 );
  stream.setDevice( &file );
  stream >> mTodoStateMap;

  file.close();

  return true;
}

bool TodoStateMapper::save()
{
  TQFile file( filename() );
  if ( !file.open( IO_WriteOnly ) ) {
    kdError() << "Can't write uid map file '" << filename() << "'" << endl;
    return false;
  }

  TQDataStream stream;
  stream.setVersion( 6 );
  stream.setDevice( &file );
  stream << mTodoStateMap;

  file.close();

  return true;
}

void TodoStateMapper::clear()
{
  mTodoStateMap.clear();
}

void TodoStateMapper::addTodoState( const TQString &uid, int localState, const TQString &remoteState )
{
  TodoStateMapEntry entry;
  entry.uid = uid;
  entry.localState = localState;
  entry.remoteState = remoteState;

  mTodoStateMap.insert( uid, entry );
}

TQString TodoStateMapper::remoteState( const TQString &uid, int localState )
{
  if ( mTodoStateMap.find( uid ) == mTodoStateMap.end() )
    kdError() << "TodoStateMapper: no entry for " << uid << " found" << endl;

  TodoStateMapEntry entry = mTodoStateMap[ uid ];
  if ( entry.localState == localState )
    return entry.remoteState;
  else
    return toRemote( localState );
}

void TodoStateMapper::remove( const TQString &uid )
{
  mTodoStateMap.remove( uid );
}

int TodoStateMapper::toLocal( const TQString &remoteState )
{
  if ( remoteState == "offer" )
    return 0;
  else if ( remoteState == "ongoing" )
    return 50;
  else if ( remoteState == "done" || remoteState == "billed" )
    return 100;
  else {
    TQString number( remoteState );
    number.replace( "%", "" );
    return number.toInt();
  }
}

TQString TodoStateMapper::toRemote( int localState )
{
  if ( localState == 0 )
    return "offer";
  else if ( localState == 50 )
    return "ongoing";
  else if ( localState == 100 )
    return "done";
  else
    return TQString( "%1%" ).arg( localState );
}

TQString TodoStateMapper::filename()
{
  TQString file = mPath;
  if ( !file.endsWith( "/" ) ) file += "/";
  file += mIdentifier;

  return locateLocal( "data", file );
}

