/*
    This file is part of KDE.

    Copyright (c) 2006 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "dbusclient.h"

#include <libqopensync/environment.h>
#include <libqopensync/group.h>

#include <dbus/qdbuserror.h>
#include <dbus/qdbusconnection.h>
#include <dbus/qdbusmessage.h>
#include <dbus/qdbusproxy.h>

#include <qapplication.h>
#include <qtimer.h>

#include <iostream>

OpenSyncService::OpenSyncService()
  : mConnection( 0 )
{
}

void OpenSyncService::setConnection( QDBusConnection *connection )
{
  mConnection = connection;
  mConnection->registerObject( "/ABC", this );
}

bool OpenSyncService::handleMethodCall( const QDBusMessage &message )
{
  qDebug( "OpenSyncService::handleMethodCall()" );

  qDebug( "  Interface: %s", message.interface().latin1() );
  qDebug( "  Path: %s", message.path().latin1() );
  qDebug( "  Member: %s", message.member().latin1() );
  qDebug( "  Sender: %s", message.sender().latin1() );

  if ( message.interface() != "org.opensync.SyncEngine" ) return false;

  QDBusMessage reply;

  QString function = message.member();
  if ( function == "hello" ) {
    reply = hello( message );
  } else if ( function == "randomNumber" ) {
    reply = randomNumber( message );
  } else if ( function == "listGroups" ) {
    reply = listGroups( message );
  } else if ( function == "listPlugins" ) {
    reply = listPlugins( message );
  } else if ( function == "showGroup" ) {
    reply = showGroup( message );
  } else if ( function == "showMember" ) {
    reply = showMember( message );
  } else {
    return false;
  }

  mConnection->send( reply );

  return true;
}

QDBusMessage OpenSyncService::showMember( const QDBusMessage &message )
{
  if ( message.count() != 2 ) {
    return error( message, "arg_count",
      QString("Wrong number of arguments. Expected 2, got %1.")
      .arg( message.count() ) );
  }

  QString groupName = message[ 0 ].toString();
  if ( groupName.isEmpty() ) {
    return error( message, "missing_arg", "Missing argument: group name." );
  }

  bool ok;
  int memberId = message[ 1 ].toInt( &ok );
  if ( !ok ) {
    return error( message, "missing_arg", "Missing argument: member id." );
  }

  QDBusMessage reply;

  QSync::Environment env;
  QSync::Result result = env.initialize();
  if ( result.isError() ) {
    return error( message, result.name(), result.message() ); 
  } else {
    reply = QDBusMessage::methodReply( message );

    QSync::Group group = env.groupByName( groupName );
    if ( !group.isValid() ) {
      return error( message, "group_name", QString("Unknown group '%1'.")
        .arg( groupName ) );
    }

    QSync::Member member = group.memberById( memberId );
    if ( !member.isValid() ) {
      return error( message, "member_id",
        QString("Unknown member id '%1' in group '%2'.").arg( groupName )
          .arg( memberId ) );
    }

    reply.append( memberId );
    reply.append( member.pluginName() );
      
    env.finalize();
  }

  return reply;
}


QDBusMessage OpenSyncService::showGroup( const QDBusMessage &message )
{
  if ( message.count() != 1 ) {
    return error( message, "arg_count",
      QString("Wrong number of arguments. Expected 1, got %1")
      .arg( message.count() ) );
  }

  QString groupName = message[ 0 ].toString();
  if ( groupName.isEmpty() ) {
    return error( message, "missing_arg", "Missing argument group name." );
  }

  QDBusMessage reply;

  QSync::Environment env;
  QSync::Result result = env.initialize();
  if ( result.isError() ) {
    return error( message, result.name(), result.message() ); 
  } else {
    reply = QDBusMessage::methodReply( message );

    QSync::Group group = env.groupByName( groupName );
    if ( !group.isValid() ) {
      return error( message, "group_name", QString("Unknown group '%1'")
        .arg( groupName ) );
    }

    QSync::Group::Iterator it( &group );
    for( it = group.begin(); it != group.end(); ++it ) {
      QSync::Member member = *it;
      reply.append( QVariant( member.id() ) );
    }
  
    env.finalize();
  }

  return reply;
}

QDBusMessage OpenSyncService::error( const QDBusMessage &message,
  const QString &errorCode,
  const QString &errorMessage )
{
  QDBusError error( "org.opensync." + errorCode, errorMessage );
  return QDBusMessage::methodError( message, error );
}

QDBusMessage OpenSyncService::listPlugins( const QDBusMessage &message )
{
  QDBusMessage reply;

  QSync::Environment env;
  QSync::Result result = env.initialize();
  if ( result.isError() ) {
    QDBusError error( result.name(), result.message() ); 
    reply = QDBusMessage::methodError( message, error );
  } else {
    reply = QDBusMessage::methodReply( message );

    QSync::Environment::PluginIterator it( env.pluginBegin() );
    for ( ; it != env.pluginEnd(); ++it ) {
      reply.append( QVariant( (*it).name() ) );
    }
  
    env.finalize();
  }

  return reply;
}

QDBusMessage OpenSyncService::listGroups( const QDBusMessage &message )
{
  QDBusMessage reply;

  QSync::Environment env;
  QSync::Result result = env.initialize();
  if ( result.isError() ) {
    QDBusError error( result.name(), result.message() ); 
    reply = QDBusMessage::methodError( message, error );
  } else {
    reply = QDBusMessage::methodReply( message );

    QSync::Environment::GroupIterator it( env.groupBegin() );
    for ( ; it != env.groupEnd(); ++it ) {
      reply.append( QVariant( (*it).name() ) );
    }
  
    env.finalize();
  }

  return reply;
}

QDBusMessage OpenSyncService::hello( const QDBusMessage &message )
{
  QDBusMessage reply = QDBusMessage::methodReply( message );

// QDBusError error;
// reply = QDBusMessage::methodError( message, error );

  reply.append( QVariant( QString( "Hello!" ) ) );
  
  return reply;
}

QDBusMessage OpenSyncService::randomNumber( const QDBusMessage &message )
{
  QDBusMessage reply = QDBusMessage::methodReply( message );

  int number = rand();

  reply.append( QVariant( number ) );
  
  return reply;
}

int main( int argc, char *argv[] )
{
  QApplication app(argc, argv);

  std::cout << "Hello" << std::endl;

  QDBusConnection connection = QDBusConnection::addConnection(
    QDBusConnection::SessionBus );

  if ( !connection.isConnected() ) {
    qFatal("Cannot connect to session bus");
  }

  connection.requestName( "org.opensync.SyncEngine",
    QDBusConnection::NoReplace );

  OpenSyncService service;
  service.setConnection( &connection );

  return app.exec();
}

//#include "dbusclient.moc"
