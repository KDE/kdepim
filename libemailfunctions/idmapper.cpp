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

#include <QFile>
#include <QTextStream>

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
  if ( !file.open( QIODevice::ReadOnly ) ) {
    kError(5800) << "Can't read uid map file '" << filename() << "'" << endl;
    return false;
  }

  clear();

  QTextStream ts(&file);
  QString line;
  while ( !ts.atEnd() ) {
    line = ts.readLine( 1024 );
    QStringList parts = line.split( "\x02\x02", QString::KeepEmptyParts );
    mIdMap.insert( parts[ 0 ], parts[ 1 ] );
    mFingerprintMap.insert( parts[ 0 ], parts[ 2 ] );
  }

  file.close();

  return true;
}

bool IdMapper::save()
{
  QFile file( filename() );
  if ( !file.open( QIODevice::WriteOnly ) ) {
    kError(5800) << "Can't write uid map file '" << filename() << "'" << endl;
    return false;
  }

  QString content;

  QMap<QString, QVariant>::Iterator it;
  for ( it = mIdMap.begin(); it != mIdMap.end(); ++it ) {
    QString fingerprint;
    if ( mFingerprintMap.contains( it.key() ) )
      fingerprint = mFingerprintMap[ it.key() ];
    content += it.key() + "\x02\x02" + it.value().toString() + "\x02\x02" + fingerprint + "\r\n";
  }
  QTextStream ts(&file);
  ts << content;
  file.close();

  return true;
}

void IdMapper::clear()
{
  mIdMap.clear();
  mFingerprintMap.clear();
}

void IdMapper::setRemoteId( const QString &localId, const QString &remoteId )
{
  mIdMap.insert( localId, remoteId );
}

void IdMapper::removeRemoteId( const QString &remoteId )
{
  QMap<QString, QVariant>::Iterator it;
  for ( it = mIdMap.begin(); it != mIdMap.end(); ++it )
    if ( it.value().toString() == remoteId ) {
      mIdMap.erase( it );
      mFingerprintMap.remove( it.key() );
      return;
    }
}

QString IdMapper::remoteId( const QString &localId ) const
{
  QMap<QString, QVariant>::ConstIterator it;
  it = mIdMap.find( localId );

  if ( it != mIdMap.end() )
    return it.value().toString();
  else
    return QString();
}

QString IdMapper::localId( const QString &remoteId ) const
{
  QMap<QString, QVariant>::ConstIterator it;
  for ( it = mIdMap.begin(); it != mIdMap.end(); ++it )
    if ( it.value().toString() == remoteId )
      return it.key();

  return QString();
}

QString IdMapper::asString() const
{
  QString content;

  QMap<QString, QVariant>::ConstIterator it;
  for ( it = mIdMap.begin(); it != mIdMap.end(); ++it ) {
    QString fp;
    if ( mFingerprintMap.contains( it.key() ) )
      fp = mFingerprintMap[ it.key() ];
    content += it.key() + "\t" + it.value().toString() + "\t" + fp + "\r\n";
  }

  return content;
}

void IdMapper::setFingerprint( const QString &localId, const QString &fingerprint )
{
  mFingerprintMap.insert( localId, fingerprint );
}

QString IdMapper::fingerprint( const QString &localId ) const
{
  if ( mFingerprintMap.contains( localId ) )
    return mFingerprintMap[ localId ];
  else 
    return QString();
}

QMap<QString, QString> IdMapper::remoteIdMap() const
{
  QMap<QString, QString> reverseMap;
  QMap<QString, QVariant>::ConstIterator it;
  for ( it = mIdMap.begin(); it != mIdMap.end(); ++it ) {
    reverseMap.insert( it.value().toString(), it.key() );
  }
  return reverseMap;
}
