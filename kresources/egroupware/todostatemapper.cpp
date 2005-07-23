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

#include <qdatastream.h>
#include <qfile.h>

#include <kdebug.h>
#include <kstandarddirs.h>

#include "todostatemapper.h"

QDataStream& operator<<( QDataStream &stream, const TodoStateMapper::TodoStateMapEntry &entry )
{
  stream << entry.uid << entry.localState << entry.remoteState;

  return stream;
}

QDataStream& operator>>( QDataStream &stream, TodoStateMapper::TodoStateMapEntry &entry )
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

void TodoStateMapper::setPath( const QString &path )
{
  mPath = path;
}

void TodoStateMapper::setIdentifier( const QString &identifier )
{
  mIdentifier = identifier;
}

bool TodoStateMapper::load()
{
  QFile file( filename() );
  if ( !file.open( IO_ReadOnly ) ) {
    kdError() << "Can't read uid map file '" << filename() << "'" << endl;
    return false;
  }

  clear();

  QDataStream stream;
  stream.setVersion( 6 );
  stream.setDevice( &file );
  stream >> mTodoStateMap;

  file.close();

  return true;
}

bool TodoStateMapper::save()
{
  QFile file( filename() );
  if ( !file.open( IO_WriteOnly ) ) {
    kdError() << "Can't write uid map file '" << filename() << "'" << endl;
    return false;
  }

  QDataStream stream;
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

void TodoStateMapper::addTodoState( const QString &uid, int localState, const QString &remoteState )
{
  TodoStateMapEntry entry;
  entry.uid = uid;
  entry.localState = localState;
  entry.remoteState = remoteState;

  mTodoStateMap.insert( uid, entry );
}

QString TodoStateMapper::remoteState( const QString &uid, int localState )
{
  if ( mTodoStateMap.find( uid ) == mTodoStateMap.end() )
    kdError() << "TodoStateMapper: no entry for " << uid << " found" << endl;

  TodoStateMapEntry entry = mTodoStateMap[ uid ];
  if ( entry.localState == localState )
    return entry.remoteState;
  else
    return toRemote( localState );
}

void TodoStateMapper::remove( const QString &uid )
{
  mTodoStateMap.remove( uid );
}

int TodoStateMapper::toLocal( const QString &remoteState )
{
  if ( remoteState == "offer" )
    return 0;
  else if ( remoteState == "ongoing" )
    return 50;
  else if ( remoteState == "done" || remoteState == "billed" )
    return 100;
  else {
    QString number( remoteState );
    number.replace( "%", "" );
    return number.toInt();
  }
}

QString TodoStateMapper::toRemote( int localState )
{
  if ( localState == 0 )
    return "offer";
  else if ( localState == 50 )
    return "ongoing";
  else if ( localState == 100 )
    return "done";
  else
    return QString( "%1%" ).arg( localState );
}

QString TodoStateMapper::filename()
{
  QString file = mPath;
  if ( !file.endsWith( "/" ) ) file += "/";
  file += mIdentifier;

  return locateLocal( "data", file );
}

