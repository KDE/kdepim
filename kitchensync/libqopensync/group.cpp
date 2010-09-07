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

/** hack includes **/
#include <tqdom.h>
#include <tqfile.h>
/** hack includes **/

#include <opensync/opensync.h>
#include <opensync/opensync-group.h>

#include "conversion.h"
#include "filter.h"
#include "member.h"
#include "plugin.h"
#include "result.h"

#include "group.h"

using namespace QSync;

Group::Group()
  : mGroup( 0 )
{
}

Group::~Group()
{
}

bool Group::isValid() const
{
  return ( mGroup != 0 );
}

void Group::setName( const TQString &name )
{
  Q_ASSERT( mGroup );

  osync_group_set_name( mGroup, name.latin1() );
}

TQString Group::name() const
{
  Q_ASSERT( mGroup );

  return TQString::fromLatin1( osync_group_get_name( mGroup ) );
}

void Group::setLastSynchronization( const TQDateTime &dateTime )
{
  Q_ASSERT( mGroup );

  if ( dateTime.isValid() )
    osync_group_set_last_synchronization( mGroup, dateTime.toTime_t() );
}

TQDateTime Group::lastSynchronization() const
{
  Q_ASSERT( mGroup );

  TQDateTime dateTime;
  time_t time = osync_group_get_last_synchronization( mGroup );
  if ( time != 0 )
    dateTime.setTime_t( time );

  return dateTime;
}

Group::LockType Group::lock()
{
  Q_ASSERT( mGroup );

  OSyncLockState state = osync_group_lock( mGroup );
  switch ( state ) {
    default:
    case OSYNC_LOCK_OK:
      return LockOk;
      break;
    case OSYNC_LOCKED:
      return Locked;
      break;
    case OSYNC_LOCK_STALE:
      return LockStale;
      break;
  }
}

void Group::unlock()
{
  Q_ASSERT( mGroup );

  osync_group_unlock( mGroup );
}

Member Group::addMember( const QSync::Plugin &plugin )
{
  Q_ASSERT( mGroup );

  OSyncError *error = 0;

  OSyncMember *omember = osync_member_new( &error );
  osync_group_add_member( mGroup, omember );
  osync_member_set_pluginname( omember, plugin.name().utf8() );

  Member member;
  member.mMember = omember;

  save();

  return member;
}

void Group::removeMember( const Member &member )
{
  Q_ASSERT( mGroup );

  osync_group_remove_member( mGroup, member.mMember );
}

int Group::memberCount() const
{
  Q_ASSERT( mGroup );

  return osync_group_num_members( mGroup );
}

Member Group::memberAt( int pos ) const
{
  Q_ASSERT( mGroup );

  Member member;

  if ( pos < 0 || pos >= memberCount() )
    return member;

  member.mMember = osync_group_nth_member( mGroup, pos );

  return member;
}

int Group::filterCount() const
{
  Q_ASSERT( mGroup );

  return osync_group_num_filters( mGroup );
}

Filter Group::filterAt( int pos )
{
  Q_ASSERT( mGroup );

  Filter filter;

  if ( pos < 0 || pos >= filterCount() )
    return filter;

  filter.mFilter = osync_group_nth_filter( mGroup, pos );

  return filter;
}

Result Group::save()
{
  Q_ASSERT( mGroup );

  OSyncError *error = 0;
  if ( !osync_group_save( mGroup, &error ) )
    return Result( &error );
  else
    return Result();
}

void Group::setUseMerger( bool use )
{
  Q_ASSERT( mGroup );
  osync_group_set_merger_enabled( mGroup, use );
}

bool Group::useMerger() const
{
  Q_ASSERT( mGroup );
  return osync_group_get_merger_enabled( mGroup );
}

void Group::setUseConverter( bool use )
{
  Q_ASSERT( mGroup );
  osync_group_set_converter_enabled( mGroup, use );
}

bool Group::useConverter() const
{
  Q_ASSERT( mGroup );
  return osync_group_get_converter_enabled( mGroup );
}

void Group::setObjectTypeEnabled( const TQString &objectType, bool enabled )
{
  Q_ASSERT( mGroup );

  osync_group_set_objtype_enabled( mGroup, objectType.utf8(), enabled );
}

bool Group::isObjectTypeEnabled( const TQString &objectType ) const
{
  return osync_group_objtype_enabled( mGroup, objectType.utf8() );
}

Result Group::cleanup() const
{
  Q_ASSERT( mGroup );

  OSyncError *error = 0;
  if ( !osync_group_delete( mGroup, &error ) )
    return Result( &error );
  else
    return Result();
}
