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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "idmapper.h"

#include <kstandarddirs.h>
#include <kdebug.h>

#include <tqfile.h>

using namespace KPIM;

IdMapper::IdMapper()
{
}

IdMapper::IdMapper( const TQString &path, const TQString &identifier )
  : mPath( path ), mIdentifier( identifier )
{
}

IdMapper::~IdMapper()
{
}

void IdMapper::setPath( const TQString &path )
{
  mPath = path;
}

void IdMapper::setIdentifier( const TQString &identifier )
{
  mIdentifier = identifier;
}

TQString IdMapper::filename()
{
  TQString file = mPath;
  if ( !file.endsWith( "/" ) ) file += "/";
  file += mIdentifier;

  return locateLocal( "data", file );
}

bool IdMapper::load()
{
  TQFile file( filename() );
  if ( !file.open( IO_ReadOnly ) ) {
    kdError(5800) << "Can't read uid map file '" << filename() << "'" << endl;
    return false;
  }

  clear();

  TQString line;
  while ( file.readLine( line, 1024 ) != -1 ) {
    line.truncate( line.length() - 2 ); // strip newline

    TQStringList parts = TQStringList::split( "\x02\x02", line, true );
    mIdMap.insert( parts[ 0 ], parts[ 1 ] );
    mFingerprintMap.insert( parts[ 0 ], parts[ 2 ] );
  }

  file.close();

  return true;
}

bool IdMapper::save()
{
  TQFile file( filename() );
  if ( !file.open( IO_WriteOnly ) ) {
    kdError(5800) << "Can't write uid map file '" << filename() << "'" << endl;
    return false;
  }

  TQString content;

  TQMap<TQString, TQVariant>::Iterator it;
  for ( it = mIdMap.begin(); it != mIdMap.end(); ++it ) {
    TQString fingerprint( "" );
    if ( mFingerprintMap.contains( it.key() ) )
      fingerprint = mFingerprintMap[ it.key() ];
    content += it.key() + "\x02\x02" + it.data().toString() + "\x02\x02" + fingerprint + "\r\n";
  }

  file.writeBlock( content.latin1(), qstrlen( content.latin1() ) );
  file.close();

  return true;
}

void IdMapper::clear()
{
  mIdMap.clear();
  mFingerprintMap.clear();
}

void IdMapper::setRemoteId( const TQString &localId, const TQString &remoteId )
{
  mIdMap.replace( localId, remoteId );
}

void IdMapper::removeRemoteId( const TQString &remoteId )
{
  TQMap<TQString, TQVariant>::Iterator it;
  for ( it = mIdMap.begin(); it != mIdMap.end(); ++it )
    if ( it.data().toString() == remoteId ) {
      mIdMap.remove( it );
      mFingerprintMap.remove( it.key() );
      return;
    }
}

TQString IdMapper::remoteId( const TQString &localId ) const
{
  TQMap<TQString, TQVariant>::ConstIterator it;
  it = mIdMap.find( localId );

  if ( it != mIdMap.end() )
    return it.data().toString();
  else
    return TQString::null;
}

TQString IdMapper::localId( const TQString &remoteId ) const
{
  TQMap<TQString, TQVariant>::ConstIterator it;
  for ( it = mIdMap.begin(); it != mIdMap.end(); ++it )
    if ( it.data().toString() == remoteId )
      return it.key();

  return TQString::null;
}

TQString IdMapper::asString() const
{
  TQString content;

  TQMap<TQString, TQVariant>::ConstIterator it;
  for ( it = mIdMap.begin(); it != mIdMap.end(); ++it ) {
    TQString fp;
    if ( mFingerprintMap.contains( it.key() ) )
      fp = mFingerprintMap[ it.key() ];
    content += it.key() + "\t" + it.data().toString() + "\t" + fp + "\r\n";
  }

  return content;
}

void IdMapper::setFingerprint( const TQString &localId, const TQString &fingerprint )
{
  mFingerprintMap.insert( localId, fingerprint );
}

const TQString& IdMapper::fingerprint( const TQString &localId ) const
{
  if ( mFingerprintMap.contains( localId ) )
    return mFingerprintMap[ localId ];
  else 
    return TQString::null;
}

TQMap<TQString, TQString> IdMapper::remoteIdMap() const
{
  TQMap<TQString, TQString> reverseMap;
  TQMap<TQString, TQVariant>::ConstIterator it;
  for ( it = mIdMap.begin(); it != mIdMap.end(); ++it ) {
    reverseMap.insert( it.data().toString(), it.key() );
  }
  return reverseMap;
}
