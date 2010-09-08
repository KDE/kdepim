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

#include <opensync/file.h>
#include <opensync/opensync.h>

#include "syncchange.h"

using namespace QSync;

SyncChange::SyncChange()
{
}

SyncChange::SyncChange( OSyncChange *change )
{
  mSyncChange = change;
}

SyncChange::~SyncChange()
{
}

bool SyncChange::isValid() const
{
  return ( mSyncChange != 0 );
}

void SyncChange::setUid( const TQString &uid )
{
  osync_change_set_uid( mSyncChange, uid.utf8() );
}

TQString SyncChange::uid() const
{
  return TQString::fromUtf8( osync_change_get_uid( mSyncChange ) );
}

void SyncChange::setHash( const TQString &hash )
{
  osync_change_set_hash( mSyncChange, hash.utf8() );
}

TQString SyncChange::hash() const
{
  return TQString::fromUtf8( osync_change_get_hash( mSyncChange ) );
}

void SyncChange::setData( const TQString &data )
{
  osync_change_set_data( mSyncChange, const_cast<char*>( data.utf8().data() ), data.utf8().size(), true );
}

TQString SyncChange::data() const
{
  int size = osync_change_get_datasize( mSyncChange );

  TQString content;
  if ( objectFormatName() == "file" ) {
    fileFormat *format = (fileFormat*)osync_change_get_data( mSyncChange );
    if ( format )
      content = TQString::fromUtf8( format->data, format->size );
  } else
    content = TQString::fromUtf8( osync_change_get_data( mSyncChange ), size );

  return content;
}

bool SyncChange::hasData() const
{
  return osync_change_has_data( mSyncChange );
}

TQString SyncChange::objectFormatName() const
{
  OSyncObjFormat *format = osync_change_get_objformat( mSyncChange );
  Q_ASSERT( format );

  return TQString::fromUtf8( osync_objformat_get_name( format ) );
}

Member SyncChange::member() const
{
  OSyncMember *omember = osync_change_get_member( mSyncChange );

  Member m;
  m.mMember = omember;

  return m;
}

void SyncChange::setChangeType( Type changeType )
{
  OSyncChangeType ochangeType;

  switch ( changeType ) {
    case AddedChange:
      ochangeType = CHANGE_ADDED;
      break;
    case UnmodifiedChange:
      ochangeType = CHANGE_UNMODIFIED;
      break;
    case DeletedChange:
      ochangeType = CHANGE_DELETED;
      break;
    case ModifiedChange:
      ochangeType = CHANGE_MODIFIED;
      break;
    case UnknownChange:
    default:
      ochangeType = CHANGE_UNKNOWN;
      break;
  }

  osync_change_set_changetype( mSyncChange, ochangeType );
}

SyncChange::Type SyncChange::changeType() const
{
  OSyncChangeType ochangeType = osync_change_get_changetype( mSyncChange );

  switch ( ochangeType ) {
    case CHANGE_ADDED:
      return AddedChange;
      break;
    case CHANGE_UNMODIFIED:
      return UnmodifiedChange;
      break;
    case CHANGE_DELETED:
      return DeletedChange;
      break;
    case CHANGE_MODIFIED:
      return ModifiedChange;
      break;
    case CHANGE_UNKNOWN:
    default:
      return UnknownChange;
      break;
  }
}

