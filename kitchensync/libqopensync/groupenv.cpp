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

#include <opensync/opensync.h>
#include <opensync/opensync-group.h>

#include "group.h"
#include "result.h"

#include "groupenv.h"

using namespace QSync;

GroupEnv::GroupEnv()
{
  OSyncError *error = 0;
  mGroupEnv = osync_group_env_new( &error );
}

GroupEnv::~GroupEnv()
{
  osync_group_env_free( mGroupEnv );
}

Result GroupEnv::initialize()
{
  Q_ASSERT( mGroupEnv );

  OSyncError *error = 0;
  if ( !osync_group_env_load_groups( mGroupEnv, NULL, &error ) )
    return Result( &error );
  else
    return Result();
}

void GroupEnv::finalize()
{
}

int GroupEnv::groupCount() const
{
  Q_ASSERT( mGroupEnv );

  return osync_group_env_num_groups( mGroupEnv );
}

Group GroupEnv::groupAt( int pos ) const
{
  Q_ASSERT( mGroupEnv );

  Group group;

  if ( pos < 0 || pos >= groupCount() )
    return group;

  OSyncGroup *ogroup = osync_group_env_nth_group( mGroupEnv, pos );
  group.mGroup = ogroup;

  return group;
}

Group GroupEnv::groupByName( const TQString &name ) const
{
  Q_ASSERT( mGroupEnv );

  Group group;

  OSyncGroup *ogroup = osync_group_env_find_group( mGroupEnv, name.latin1() );
  if ( ogroup )
    group.mGroup = ogroup;

  return group;
}

Group GroupEnv::addGroup( const TQString &name )
{
  Q_ASSERT( mGroupEnv );

  Group group;
  OSyncError *error = 0;

  OSyncGroup *ogroup = osync_group_new( &error );
  if ( ogroup )
    group.mGroup = ogroup;

  group.setName( name );

  if ( !osync_group_env_add_group( mGroupEnv, ogroup, &error ) ) {
    Result res( &error );
    qDebug( "Error on adding group: %s", res.message().latin1() );
  }

  return group;
}

void GroupEnv::removeGroup( const Group &group )
{
  Q_ASSERT( mGroupEnv );

  group.cleanup();

  osync_group_env_remove_group( mGroupEnv, group.mGroup );
}
