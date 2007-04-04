/*
    This file is part of libkdepim.

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <qstringlist.h>

#include <klocale.h>

#include "genericdiffalgo.h"

using namespace KSync;

#define MAX( a, b ) ( a > b ? a : b )

#ifndef KDE_USE_FINAL
// With --enable-final, we get the (identical) compareString from
// addresseediffalgo.cpp
static bool compareString( const QString &left, const QString &right )
{
  if ( left.isEmpty() && right.isEmpty() )
    return true;
  else
    return left == right;
}
#endif

GenericDiffAlgo::GenericDiffAlgo( const QString &leftData, const QString &rightData )
  : mLeftData( leftData ), mRightData( rightData )
{
}

void GenericDiffAlgo::run()
{
  begin();

  QStringList leftList = QStringList::split( '\n', mLeftData, true );
  QStringList rightList = QStringList::split( '\n', mRightData, true );

  uint lines = MAX( leftList.count(), rightList.count() );
  for ( uint i = 0; i < lines; ++i ) {
    if ( i < leftList.count() && i < rightList.count() ) {
      if ( !compareString( leftList[ i ], rightList[ i ] ) )
        conflictField( i18n( "Line %1" ).arg( i ), leftList[ i ], rightList[ i ] );
    } else if ( i < leftList.count() && i >= rightList.count() ) {
      additionalLeftField( i18n( "Line %1" ).arg( i ), leftList[ i ] );
    } else if ( i >= leftList.count() && i < rightList.count() ) {
      additionalRightField( i18n( "Line %1" ).arg( i ), rightList[ i ] );
    }
  }

  end();
}

