/*
    This file is part of kdepim.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>
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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "idmapper.h"

#include <kstandarddirs.h>
#include <kdebug.h>

#include <qfile.h>

using namespace KPIM;

IdMapper::IdMapper()
{
}

IdMapper::IdMapper( const QString &path, const QString &identifier )
  : mPath( path ), mIdentifier( identifier )
{
}

IdMapper::~IdMapper()
{
}

void IdMapper::setPath( const QString &path )
{
  mPath = path;
}

void IdMapper::setIdentifier( const QString &identifier )
{
  mIdentifier = identifier;
}

QString IdMapper::filename()
{
  QString file = mPath;
  if ( !file.endsWith( "/" ) ) file += "/";
  file += mIdentifier;

  return locateLocal( "data", file );
}

bool IdMapper::load()
{
  QFile file( filename() );
  if ( !file.open( IO_ReadOnly ) ) {
    kdError(5800) << "Can't read uid map file '" << filename() << "'" << endl;
    return false;
  }

  clear();

  QString line;
  while ( file.readLine( line, 1024 ) != -1 ) {
    line.truncate( line.length() - 2 ); // strip newline

    QStringList parts = QStringList::split( "\x02\x02", line, true );
    mIdMap.insert( parts[ 0 ], parts[ 1 ] );
  }

  file.close();

  return true;
}

bool IdMapper::save()
{
  QFile file( filename() );
  if ( !file.open( IO_WriteOnly ) ) {
    kdError(5800) << "Can't write uid map file '" << filename() << "'" << endl;
    return false;
  }

  QString content;

  QMap<QString, QVariant>::Iterator it;
  for ( it = mIdMap.begin(); it != mIdMap.end(); ++it ) {
    content += it.key() + "\x02\x02" + it.data().toString() + "\r\n";
  }

  file.writeBlock( content.latin1(), qstrlen( content.latin1() ) );
  file.close();

  return true;
}

void IdMapper::clear()
{
  mIdMap.clear();
}

void IdMapper::setRemoteId( const QString &localId, const QString &remoteId )
{
  mIdMap.insert( localId, remoteId );
}

void IdMapper::removeRemoteId( const QString &remoteId )
{
  QMap<QString, QVariant>::Iterator it;
  for ( it = mIdMap.begin(); it != mIdMap.end(); ++it )
    if ( it.data().toString() == remoteId ) {
      mIdMap.remove( it );
      return;
    }
}

QString IdMapper::remoteId( const QString &localId ) const
{
  QMap<QString, QVariant>::ConstIterator it;
  it = mIdMap.find( localId );

  if ( it != mIdMap.end() )
    return it.data().toString();
  else
    return QString::null;
}

QString IdMapper::localId( const QString &remoteId ) const
{
  QMap<QString, QVariant>::ConstIterator it;
  for ( it = mIdMap.begin(); it != mIdMap.end(); ++it )
    if ( it.data().toString() == remoteId )
      return it.key();

  return QString::null;
}

QString IdMapper::asString() const
{
  QString content;

  QMap<QString, QVariant>::ConstIterator it;
  for ( it = mIdMap.begin(); it != mIdMap.end(); ++it ) {
    content += it.key() + "\t" + it.data().toString() + "\r\n";
  }

  return content;
}
