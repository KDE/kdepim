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

#include <QtCore/QString>
#include <opensync/opensync.h>
#include <opensync/opensync-engine.h>

#include "syncmapping.h"

using namespace QSync;

SyncMapping::SyncMapping()
  : mEngine( 0 ), mMappingEngine( 0 )
{
}

SyncMapping::SyncMapping( OSyncMappingEngine *mapping, OSyncEngine *engine )
  : mEngine( engine ), mMappingEngine( mapping )
{
}

SyncMapping::~SyncMapping()
{
}

bool SyncMapping::isValid() const
{
  return ( mEngine != 0 && mMappingEngine != 0 );
}

/* TODO Method osync_mapping engine_get_id( OSyncMappingEngine ) doesn't seem to exist.
is it osync_mapping_get_id( OSyncMapping ) instead? -- anirudh 20070729
long long SyncMapping::id() const
{
  Q_ASSERT( mMappingEngine );

  return osync_mapping_engine_get_id( mMappingEngine );
}
*/
void SyncMapping::duplicate()
{
  Q_ASSERT( mEngine );
  Q_ASSERT( mMappingEngine );

  OSyncError *error = 0;
  osync_mapping_engine_duplicate( mMappingEngine, &error );
}

void SyncMapping::solve( const SyncChange &change )
{
  Q_ASSERT( mEngine );
  Q_ASSERT( mMappingEngine );
  Q_ASSERT( change.isValid() );

  OSyncError *error = 0;
  osync_mapping_engine_solve( mMappingEngine, change.mSyncChange, &error );
}

void SyncMapping::ignore()
{
  Q_ASSERT( mEngine );
  Q_ASSERT( mMappingEngine );

  //TODO: error should be returned as Result
  OSyncError *error = 0;
  osync_mapping_engine_ignore( mMappingEngine, &error );
}

int SyncMapping::changesCount() const
{
  Q_ASSERT( mMappingEngine );

  return osync_mapping_engine_num_changes( mMappingEngine );
}

SyncChange SyncMapping::changeAt( int pos ) const
{
  Q_ASSERT( mMappingEngine );

  if ( pos < 0 || pos >= osync_mapping_engine_num_changes( mMappingEngine ) ) {
    return SyncChange();
  }

  OSyncChange *ochange = osync_mapping_engine_nth_change( mMappingEngine, pos );

  return SyncChange( ochange );
}

