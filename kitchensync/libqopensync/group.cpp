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

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtXml/QDomDocument>

#include <opensync/opensync.h>
#include <opensync/opensync-group.h>

#include "conversion.h"
#include "group.h"

using namespace QSync;

/**
  This class is a quick hack for OpenSync 0.19 and 0.20 because
  the engine doesn't stores the filter settings itself when calling
  osync_group_set_objtype_enabled(), so we have to store it for every
  group in a separated config file. This class encapsulates it.
 */
GroupConfig::GroupConfig()
  : mGroup( 0 )
{
}

QStringList GroupConfig::activeObjectTypes() const
{
  Q_ASSERT( mGroup );

  const QString fileName =
    QString( "%1/filter.conf" ).arg( osync_group_get_configdir( mGroup ) );

  QFile file( fileName );
  if ( !file.open( QIODevice::ReadOnly ) ) {
    return QStringList();
  }

  QDomDocument document;

  QString message;
  if ( !document.setContent( &file, &message ) ) {
    qDebug( "Error on loading %s: %s",
            qPrintable( fileName ), qPrintable( message ) );
    return QStringList();
  }
  file.close();

  QStringList objectTypes;

  QDomElement element = document.documentElement();
  QDomNode node = element.firstChild();
  while ( !node.isNull() ) {
    QDomElement childElement = node.toElement();
    if ( !childElement.isNull() ) {
      objectTypes.append( childElement.tagName() );
    }

    node = node.nextSibling();
  }

  return objectTypes;
}

void GroupConfig::setActiveObjectTypes( const QStringList &objectTypes )
{
  Q_ASSERT( mGroup );

  QDomDocument document( "Filter" );
  document.appendChild( document.createProcessingInstruction(
                        "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );

  QDomElement element = document.createElement( "filter" );
  document.appendChild( element );

  for ( int i = 0; i < objectTypes.count(); ++i ) {
    QDomElement entry = document.createElement( objectTypes[ i ] );
    element.appendChild( entry );
  }

  const QString fileName =
    QString( "%1/filter.conf" ).arg( osync_group_get_configdir( mGroup ) );

  QFile file( fileName );
  if ( !file.open( QIODevice::WriteOnly ) ) {
    return;
  }

  QTextStream s( &file );
  s.setCodec( "UTF-8" );
  s << document.toString();
  file.close();
}

Group::Group()
  : mGroup( 0 )
{
}

Group::~Group()
{
}

bool Group::isValid() const
{
  return mGroup != 0;
}

void Group::setName( const QString &name )
{
  Q_ASSERT( mGroup );

  osync_group_set_name( mGroup, name.toLatin1() );
}

QString Group::name() const
{
  Q_ASSERT( mGroup );

  return QString::fromLatin1( osync_group_get_name( mGroup ) );
}

void Group::setLastSynchronization( const QDateTime &dateTime )
{
  Q_ASSERT( mGroup );

  if ( dateTime.isValid() ) {
    osync_group_set_last_synchronization( mGroup, dateTime.toTime_t() );
  }
}

QDateTime Group::lastSynchronization() const
{
  Q_ASSERT( mGroup );

  QDateTime dateTime;
  time_t time = osync_group_get_last_synchronization( mGroup );
  if ( time != 0 ) {
    dateTime.setTime_t( time );
  }

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
  osync_member_set_pluginname( omember, plugin.name().toUtf8() );

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

  if ( pos < 0 || pos >= memberCount() ) {
    return member;
  }

  member.mMember = osync_group_nth_member( mGroup, pos );

  return member;
}

int Group::filterCount() const
{
  Q_ASSERT( mGroup );

  return osync_group_num_filters( mGroup );
}

Filter Group::filterAt( int pos ) const
{
  Q_ASSERT( mGroup );

  Filter filter;

  if ( pos < 0 || pos >= filterCount() ) {
    return filter;
  }

  filter.mFilter = osync_group_nth_filter( mGroup, pos );

  return filter;
}

Result Group::save()
{
  Q_ASSERT( mGroup );

  OSyncError *error = 0;
  if ( !osync_group_save( mGroup, &error ) ) {
    return Result( &error );
  } else {
    return Result();
  }
}

void Group::setUseMerger( bool use )
{
  Q_ASSERT( mGroup );
  osync_group_set_use_merger( mGroup, use );
}

bool Group::useMerger() const
{
  Q_ASSERT( mGroup );
  return osync_group_get_use_merger( mGroup );
}

void Group::setUseConverter( bool use )
{
  Q_ASSERT( mGroup );
  osync_group_set_use_converter( mGroup, use );
}

bool Group::useConverter() const
{
  Q_ASSERT( mGroup );
  return osync_group_get_use_converter( mGroup );
}

void Group::setObjectTypeEnabled( const QString &objectType, bool enabled )
{
  Q_ASSERT( mGroup );

  osync_group_set_objtype_enabled( mGroup, objectType.toUtf8(), enabled );
}

bool Group::isObjectTypeEnabled( const QString &objectType ) const
{
  return osync_group_objtype_enabled( mGroup, objectType.toUtf8() );
}

GroupConfig Group::config() const
{
  Q_ASSERT( mGroup );

  GroupConfig config;
  config.mGroup = mGroup;

  return config;
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
