/*
    This file is part of kdepim.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

#include <qdatastream.h>
#include <qfile.h>

#include <kdebug.h>

#include "uidmapper.h"

UIDMapper::UIDMapper( const QString fileName )
  : QObject( 0, "" ), mFileName( fileName )
{
}

UIDMapper::~UIDMapper()
{
}

void UIDMapper::load()
{
  QFile file( mFileName );

  if ( !file.open( IO_ReadOnly ) ) {
    kdError() << "Couldn't open uid map file '" << file.name() << "' for reading." << endl;
    return;
  }

  QVariant variant;

  QDataStream stream( &file );
  stream >> variant;

  file.close();

  mMap = variant.toMap();
}

void UIDMapper::store()
{
  QFile file( mFileName );

  if ( !file.open( IO_WriteOnly ) ) {
    kdError() << "Couldn't open uid map file '" << file.name() << "' for writing." << endl;
    return;
  }

  QDataStream stream( &file );
  stream << QVariant( mMap );

  file.close();
}

void UIDMapper::add( const QString& local, const QString &remote )
{
  mMap.insert( local, QVariant( remote ) );
}

void UIDMapper::removeByLocal( const QString& local )
{
  mMap.remove( local );
}

void UIDMapper::removeByRemote( const QString& remote )
{
  QMap<QString, QVariant>::Iterator it;
  for ( it = mMap.begin(); it != mMap.end(); ++it ) {
    if ( it.data().toString() == remote ) {
      mMap.remove( it );
      break;
    }
  }
}

QString UIDMapper::remoteUid( const QString& local ) const
{
  return mMap[ local ].toString();
}

QString UIDMapper::localUid( const QString& remote ) const
{
  QMap<QString, QVariant>::ConstIterator it;
  for ( it = mMap.begin(); it != mMap.end(); ++it ) {
    if ( it.data().toString() == remote ) {
      return it.key();
    }
  }

  return QString::null;
}

#include "uidmapper.moc"
