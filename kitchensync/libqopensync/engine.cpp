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
#include <opensync/opensync-engine.h>

#include "group.h"
#include "member.h"
#include "result.h"

#include "engine.h"

using namespace QSync;

Engine::Engine( const Group &group )
{
  OSyncError *error = 0;
  mEngine = osync_engine_new( group.mGroup, &error );
}

Engine::~Engine()
{
  osync_engine_unref( mEngine );
  mEngine = 0;
}

Result Engine::initialize()
{
  Q_ASSERT( mEngine );

  OSyncError *error = 0;
  if ( !osync_engine_initialize	( mEngine, &error ) )
    return Result( &error );
  else
    return Result();
}

void Engine::finalize()
{
  Q_ASSERT( mEngine );

  OSyncError *error = 0;
  osync_engine_finalize( mEngine , &error );
}

Result Engine::synchronize()
{
  Q_ASSERT( mEngine );

  OSyncError *error = 0;
  if ( !osync_engine_synchronize( mEngine, &error ) )
    return Result( &error );
  else
    return Result();
}

Result Engine::discover( const Member &member )
{
  Q_ASSERT( mEngine );

  OSyncError *error = 0;
  if ( !osync_engine_discover_and_block( mEngine, member.mMember, &error ) )
    return Result( &error );
  else
    return Result();
}

void Engine::abort()
{
  Q_ASSERT( mEngine );

// TODO
//  osync_engine_abort( mEngine );
}

