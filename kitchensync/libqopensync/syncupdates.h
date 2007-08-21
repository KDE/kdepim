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

#include <libqopensync/qopensync_export.h>
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

class QSYNC_EXPORT SyncMemberUpdate
{
  friend class CallbackHandler;

  public:
    enum Type {
      Connected,
      Disconnected,
      Read,
      Written,
      SyncDone,
      Discovered,
      Error
    };

    SyncMemberUpdate();
    SyncMemberUpdate( OSyncMemberUpdate *update );
    ~SyncMemberUpdate();

    Type type() const;
    Result result() const;
    Member member() const;

  private:
    Type mType;
    Result mResult;
    Member mMember;
};

class QSYNC_EXPORT SyncChangeUpdate
{
  friend class CallbackHandler;

  public:
    enum Type {
      Read = 1,
      Written,
      Error
    };

    SyncChangeUpdate();
    SyncChangeUpdate( OSyncChangeUpdate *update );
    ~SyncChangeUpdate();

    Type type() const;
    Result result() const;
    SyncChange change() const;
    Member member() const;
    int mappingId() const;

  private:
    Type mType;
    Result mResult;
    SyncChange mChange;
    Member mMember;
    int mMappingId;
};

class QSYNC_EXPORT SyncMappingUpdate
{
  friend class CallbackHandler;

  public:
    enum Type {
      Solved = 1,
      //Synced,
      Error
    };

    SyncMappingUpdate();
    SyncMappingUpdate( OSyncMappingUpdate *update, OSyncEngine *engine );
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

class QSYNC_EXPORT SyncEngineUpdate
{
  friend class CallbackHandler;

  public:
    enum Type {
      Connected = 1,
      Read,
      Written,
      Disconnected,
      Error,
      Successful,
      PrevUnclean,
      EndConflicts,
      SyncDone
    };

    SyncEngineUpdate();
    SyncEngineUpdate( OSyncEngineUpdate *update );
    ~SyncEngineUpdate();

    Type type() const;
    Result result() const;

  private:
    Type mType;
    Result mResult;
};

}

#endif
