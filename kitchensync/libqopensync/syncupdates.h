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

#ifndef QSYNC_SYNCUPDATES_H
#define QSYNC_SYNCUPDATES_H

#include <libqopensync/member.h>
#include <libqopensync/result.h>
#include <libqopensync/syncchange.h>
#include <libqopensync/syncmapping.h>

class OSyncMemberUpdate;
class OSyncChangeUpdate;
class OSyncMappingUpdate;
class OSyncEngineUpdate;
class OSyncMemberUpdate;

namespace QSync {

class SyncMemberUpdate
{
  friend class CallbackHandler;

  public:
    enum Type {
      Connected,
      SentChanges,
      CommittedAll,
      Disconnected,
      ConnectError,
      GetChangesError,
      CommittedAllError,
      SyncDoneError,
      DisconnectedError
    };

    SyncMemberUpdate();
    SyncMemberUpdate( OSyncMemberUpdate* );
    ~SyncMemberUpdate();

    Type type() const;
    Result result() const;
    Member member() const;

  private:
    Type mType;
    Result mResult;
    Member mMember;
};

class SyncChangeUpdate
{
  friend class CallbackHandler;

  public:
    enum Type {
      Received = 1,
      ReceivedInfo,
      Sent,
      WriteError,
      ReceiveError
    };

    SyncChangeUpdate();
    SyncChangeUpdate( OSyncChangeUpdate* );
    ~SyncChangeUpdate();

    Type type() const;
    Result result() const;
    SyncChange change() const;
    int memberId() const;
    int mappingId() const;

  private:
    Type mType;
    Result mResult;
    SyncChange mChange;
    int mMemberId;
    int mMappingId;
};

class SyncMappingUpdate
{
  friend class CallbackHandler;

  public:
    enum Type {
      Solved = 1,
      Synced,
      WriteError
    };

    SyncMappingUpdate();
    SyncMappingUpdate( OSyncMappingUpdate*, OSyncEngine* );
    ~SyncMappingUpdate();

    Type type() const;
    Result result() const;
    long long int winner() const;
    SyncMapping mapping() const;

  private:
    Type mType;
    Result mResult;
    long long int mWinner;
    SyncMapping mMapping;
};

class SyncEngineUpdate
{
  friend class CallbackHandler;

  public:
    enum Type {
      EndPhaseConnected = 1,
      EndPhaseRead,
      EndPhaseWrite,
      EndPhaseDisconnected,
      Error,
      SyncSuccessfull,
      PrevUnclean,
      EndConflicts
    };

    SyncEngineUpdate();
    SyncEngineUpdate( OSyncEngineUpdate* );
    ~SyncEngineUpdate();

    Type type() const;
    Result result() const;

  private:
    Type mType;
    Result mResult;
};

}

#endif
