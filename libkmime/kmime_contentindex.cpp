/*
    Copyright (c) 2006 Volker Krause <volker.krause@rwth-aachen.de>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "kmime_contentindex.h"

#include <QStringList>

KMime::ContentIndex::ContentIndex()
{
}

KMime::ContentIndex::ContentIndex( const QString & index )
{
  QStringList l = index.split( '.' );
  foreach ( QString s, l ) {
    bool ok;
    unsigned int i = s.toUInt( &ok );
    if ( !ok ) {
      mIndex.clear();
      break;
    }
    mIndex.append( i );
  }
}

bool KMime::ContentIndex::isValid() const
{
  return !mIndex.isEmpty();
}

unsigned int KMime::ContentIndex::pop()
{
  return mIndex.takeFirst();
}

void KMime::ContentIndex::push( unsigned int index )
{
  mIndex.prepend( index );
}

QString KMime::ContentIndex::toString() const
{
  QStringList l;
  foreach ( unsigned int i, mIndex )
    l.append( QString::number( i ) );
  return l.join( "." );
}

bool KMime::ContentIndex::operator ==( const ContentIndex & index ) const
{
  return mIndex == index.mIndex;
}

bool KMime::ContentIndex::operator !=( const ContentIndex & index ) const
{
  return mIndex != index.mIndex;
}
