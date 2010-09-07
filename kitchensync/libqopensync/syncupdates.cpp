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

#include "syncupdates.h"

using namespace QSync;

SyncMemberUpdate::SyncMemberUpdate()
{
}

SyncMemberUpdate::SyncMemberUpdate( OSyncMemberUpdate *update )
{
  switch ( update->type ) {
    case OSYNC_CLIENT_EVENT_CONNECTED:
      mType = Connected;
      break;
    case OSYNC_CLIENT_EVENT_DISCONNECTED:
      mType = Disconnected;
      break;
    case OSYNC_CLIENT_EVENT_READ:
      mType = Read; 
      break;
    case OSYNC_CLIENT_EVENT_WRITTEN:
      mType = Written;
      break;
    case OSYNC_CLIENT_EVENT_SYNC_DONE:
      mType = SyncDone;
      break;
    case OSYNC_CLIENT_EVENT_DISCOVERED:
      mType = Discovered;
      break;
    case OSYNC_CLIENT_EVENT_ERROR:
      mType = Error;
      break;
  }

  if ( update->error )
    mResult = Result( &(update->error) );

  mMember.mMember = update->member;
}

SyncMemberUpdate::~SyncMemberUpdate()
{
}

SyncMemberUpdate::Type SyncMemberUpdate::type() const
{
  return mType;
}

Result SyncMemberUpdate::result() const
{
  return mResult;
}

Member SyncMemberUpdate::member() const
{
  return mMember;
}


SyncChangeUpdate::SyncChangeUpdate()
{
}

SyncChangeUpdate::SyncChangeUpdate( OSyncChangeUpdate *update )
{
  switch ( update->type ) {
    case OSYNC_CHANGE_EVENT_READ:
      mType = Read;
      break;
    case OSYNC_CHANGE_EVENT_WRITTEN:
      mType = Written;
      break;
    case OSYNC_CHANGE_EVENT_ERROR:
      mType = Error;
      break;
  }

  if ( update->error )
    mResult = Result( &(update->error) );

  mChange = SyncChange( update->change );
  mMember.mMember = update->member;
  mMappingId = update->mapping_id;
}

SyncChangeUpdate::~SyncChangeUpdate()
{
}

SyncChangeUpdate::Type SyncChangeUpdate::type() const
{
  return mType;
}

Result SyncChangeUpdate::result() const
{
  return mResult;
}

SyncChange SyncChangeUpdate::change() const
{
  return mChange;
}

Member SyncChangeUpdate::member() const
{
  return mMember;
}

int SyncChangeUpdate::mappingId() const
{
  return mMappingId;
}

SyncMappingUpdate::SyncMappingUpdate()
{
}

SyncMappingUpdate::SyncMappingUpdate( OSyncMappingUpdate *update, OSyncEngine *engine )
{
  switch ( update->type ) {
    case OSYNC_MAPPING_EVENT_SOLVED:
      mType = Solved;
      break;
//    case OSYNC_MAPPING_EVENT_SYNCED:
  //    mType = Synced;
    //  break;
    case OSYNC_MAPPING_EVENT_ERROR:
      mType = Error;
      break;
  }

  if ( update->error )
    mResult = Result( &(update->error) );

  mWinner = update->winner;
  mMapping.mEngine = engine;

  // TODO PORTING
//  mMapping.mMapping = update->mapping;
}

SyncMappingUpdate::~SyncMappingUpdate()
{
}

SyncMappingUpdate::Type SyncMappingUpdate::type() const
{
  return mType;
}

Result SyncMappingUpdate::result() const
{
  return mResult;
}

long long int SyncMappingUpdate::winner() const
{
  return mWinner;
}

SyncMapping SyncMappingUpdate::mapping() const
{
  return mMapping;
}

SyncEngineUpdate::SyncEngineUpdate()
{
}

SyncEngineUpdate::SyncEngineUpdate( OSyncEngineUpdate *update )
{
  switch ( update->type ) {
    case OSYNC_ENGINE_EVENT_CONNECTED:
      mType = Connected;
      break;
    case OSYNC_ENGINE_EVENT_READ:
      mType = Read;
      break;
    case OSYNC_ENGINE_EVENT_WRITTEN:
      mType = Written;
      break;
    case OSYNC_ENGINE_EVENT_DISCONNECTED: 
      mType = Disconnected;
      break;
    case OSYNC_ENGINE_EVENT_ERROR:
      mType = Error;
      break;
    case OSYNC_ENGINE_EVENT_SUCCESSFUL:
      mType = SyncSuccessful;
      break;
    case OSYNC_ENGINE_EVENT_PREV_UNCLEAN:
      mType = PrevUnclean;
      break;
    case OSYNC_ENGINE_EVENT_END_CONFLICTS:
      mType = EndConflicts;
      break;
    case OSYNC_ENGINE_EVENT_SYNC_DONE:
      mType = SyncDone;
      break;
  }

  if ( update->error )
    mResult = Result( &(update->error) );
}

SyncEngineUpdate::~SyncEngineUpdate()
{
}

SyncEngineUpdate::Type SyncEngineUpdate::type() const
{
  return mType;
}

Result SyncEngineUpdate::result() const
{
  return mResult;
}

