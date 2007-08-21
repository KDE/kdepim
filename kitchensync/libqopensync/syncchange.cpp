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
#include <stdlib.h>

#include <opensync/file.h>

#include <opensync/opensync.h>
#include <opensync/opensync-data.h>
#include <opensync/opensync-format.h>

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
  return mSyncChange != 0;
}

void SyncChange::setUid( const QString &uid )
{
  osync_change_set_uid( mSyncChange, uid.toUtf8() );
}

QString SyncChange::uid() const
{
  return QString::fromUtf8( osync_change_get_uid( mSyncChange ) );
}

void SyncChange::setHash( const QString &hash )
{
  osync_change_set_hash( mSyncChange, hash.toUtf8() );
}

QString SyncChange::hash() const
{
  return QString::fromUtf8( osync_change_get_hash( mSyncChange ) );
}

void SyncChange::setData( const QString &data, OSyncObjFormat *format )
{
  OSyncError *error = 0;
  OSyncData *odata = osync_data_new( const_cast<char*>( data.toUtf8().data() ), data.toUtf8().size(), format, &error );
  osync_change_set_data( mSyncChange, odata );
}

QString SyncChange::data() const
{
  char *buf;
  unsigned int size;

  OSyncData *data = osync_change_get_data( mSyncChange );
  osync_data_get_data( data, &buf, &size );

  QString content;
  if ( objectFormatName() == "file" ) {
    OSyncFileFormat *format = (OSyncFileFormat*) buf;
    if ( format ) {
      content = QString::fromUtf8( format->data, format->size );
    }
  } else {
    content = QString::fromUtf8( buf, size );
  }
  free( buf );

  return content;
}

bool SyncChange::hasData() const
{
  return osync_data_has_data( osync_change_get_data( mSyncChange ) );
}

QString SyncChange::objectFormatName() const
{
  OSyncObjFormat *format = osync_data_get_objformat( osync_change_get_data( mSyncChange ) );
  Q_ASSERT( format );

  return QString::fromUtf8( osync_objformat_get_name( format ) );
}

/* TODO osync_change_get_member() doesn't seem to exist -- anirudh 20070730
Member SyncChange::member() const
{
  OSyncMember *omember = osync_change_get_member( mSyncChange );

  Member m;
  m.mMember = omember;

  return m;
}*/

void SyncChange::setChangeType( Type changeType )
{
  OSyncChangeType ochangeType;

  switch ( changeType ) {
    case AddedChange:
      ochangeType = OSYNC_CHANGE_TYPE_ADDED;
      break;
    case UnmodifiedChange:
      ochangeType = OSYNC_CHANGE_TYPE_UNMODIFIED;
      break;
    case DeletedChange:
      ochangeType = OSYNC_CHANGE_TYPE_DELETED;
      break;
    case ModifiedChange:
      ochangeType = OSYNC_CHANGE_TYPE_MODIFIED;
      break;
    case UnknownChange:
    default:
      ochangeType = OSYNC_CHANGE_TYPE_UNKNOWN;
      break;
  }

  osync_change_set_changetype( mSyncChange, ochangeType );
}

SyncChange::Type SyncChange::changeType() const
{
  OSyncChangeType ochangeType = osync_change_get_changetype( mSyncChange );

  switch ( ochangeType ) {
    case OSYNC_CHANGE_TYPE_ADDED:
      return AddedChange;
      break;
    case OSYNC_CHANGE_TYPE_UNMODIFIED:
      return UnmodifiedChange;
      break;
    case OSYNC_CHANGE_TYPE_DELETED:
      return DeletedChange;
      break;
    case OSYNC_CHANGE_TYPE_MODIFIED:
      return ModifiedChange;
      break;
    case OSYNC_CHANGE_TYPE_UNKNOWN:
    default:
      return UnknownChange;
      break;
  }
}
