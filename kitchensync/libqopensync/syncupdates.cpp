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

#include <osengine/engine.h>

#include "syncupdates.h"

using namespace QSync;

SyncMemberUpdate::SyncMemberUpdate()
{
}

SyncMemberUpdate::SyncMemberUpdate( OSyncMemberUpdate *update )
{
  switch ( update->type ) {
    case MEMBER_CONNECTED:
      mType = Connected;
      break;
    case MEMBER_SENT_CHANGES:
      mType = SentChanges;
      break;
    case MEMBER_COMMITTED_ALL:
      mType = CommittedAll;
      break;
    case MEMBER_DISCONNECTED:
      mType = Disconnected;
      break;
    case MEMBER_CONNECT_ERROR:
      mType = ConnectError;
      break;
    case MEMBER_GET_CHANGES_ERROR:
      mType = GetChangesError;
      break;
    case MEMBER_COMMITTED_ALL_ERROR:
      mType = CommittedAllError;
      break;
    case MEMBER_SYNC_DONE_ERROR:
      mType = SyncDoneError;
      break;
    case MEMBER_DISCONNECT_ERROR:
      mType = DisconnectedError;
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
    case CHANGE_RECEIVED:
      mType = Received;
      break;
    case CHANGE_RECEIVED_INFO:
      mType = ReceivedInfo;
      break;
    case CHANGE_SENT:
      mType = Sent;
      break;
    case CHANGE_WRITE_ERROR:
      mType = WriteError;
      break;
    case CHANGE_RECV_ERROR:
      mType = ReceiveError;
      break;
  }

  if ( update->error )
    mResult = Result( &(update->error) );

  mChange = SyncChange( update->change );
  mMemberId = update->member_id;
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

int SyncChangeUpdate::memberId() const
{
  return mMemberId;
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
    case MAPPING_SOLVED:
      mType = Solved;
      break;
    case MAPPING_SYNCED:
      mType = Synced;
      break;
    case MAPPING_WRITE_ERROR:
      mType = WriteError;
      break;
  }

  if ( update->error )
    mResult = Result( &(update->error) );

  mWinner = update->winner;
  mMapping.mEngine = engine;
  mMapping.mMapping = update->mapping;
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
    case ENG_ENDPHASE_CON:
      mType = EndPhaseConnected;
      break;
    case ENG_ENDPHASE_READ:
      mType = EndPhaseRead;
      break;
    case ENG_ENDPHASE_WRITE:
      mType = EndPhaseWrite;
      break;
    case ENG_ENDPHASE_DISCON:
      mType = EndPhaseDisconnected;
      break;
    case ENG_ERROR:
      mType = Error;
      break;
    case ENG_SYNC_SUCCESSFULL:
      mType = SyncSuccessfull;
      break;
    case ENG_PREV_UNCLEAN:
      mType = PrevUnclean;
      break;
    case ENG_END_CONFLICTS:
      mType = EndConflicts;
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

