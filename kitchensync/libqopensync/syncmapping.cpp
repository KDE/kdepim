/*
    This file is part of libqopensync.

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

#include <qstring.h>
#include <osengine/engine.h>

#include "syncmapping.h"

using namespace QSync;

SyncMapping::SyncMapping()
  : mEngine( 0 ), mMapping( 0 )
{
}

SyncMapping::SyncMapping( OSyncMapping *mapping, OSyncEngine *engine )
  : mEngine( engine ), mMapping( mapping )
{
}

SyncMapping::~SyncMapping()
{
}

bool SyncMapping::isValid() const
{
  return ( mEngine != 0 && mMapping != 0 );
}

long long SyncMapping::id() const
{
  Q_ASSERT( mMapping );

  return osengine_mapping_get_id( mMapping );
}

void SyncMapping::duplicate()
{
  Q_ASSERT( mEngine );
  Q_ASSERT( mMapping );

  osengine_mapping_duplicate( mEngine, mMapping );
}

void SyncMapping::solve( const SyncChange &change )
{
  Q_ASSERT( mEngine );
  Q_ASSERT( mMapping );
  Q_ASSERT( change.isValid() );

  osengine_mapping_solve( mEngine, mMapping, change.mSyncChange );
}

void SyncMapping::ignore()
{
  Q_ASSERT( mEngine );
  Q_ASSERT( mMapping );

  //TODO: error should be returned as Result
  OSyncError *error = 0;
  osengine_mapping_ignore_conflict( mEngine, mMapping, &error );
}

int SyncMapping::changesCount() const
{
  Q_ASSERT( mMapping );

  return osengine_mapping_num_changes( mMapping );
}

SyncChange SyncMapping::changeAt( int pos )
{
  Q_ASSERT( mMapping );

  if ( pos < 0 || pos >= osengine_mapping_num_changes( mMapping ) )
    return SyncChange();

  OSyncChange *ochange = osengine_mapping_nth_change( mMapping, pos );

  return SyncChange( ochange );
}

