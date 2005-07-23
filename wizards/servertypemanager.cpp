/*
    This file is part of kdepim.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

#include <kstaticdeleter.h>

#include "egroupwarehandler.h"

#include "servertypemanager.h"

ServerTypeManager* ServerTypeManager::mSelf = 0;
static KStaticDeleter<ServerTypeManager> serverManagerDeleter;

ServerTypeManager::ServerTypeManager( QObject *parent, const char *name )
  : QObject( parent, name )
{
  loadPlugins();
}

ServerTypeManager::~ServerTypeManager()
{
}

ServerTypeManager* ServerTypeManager::self()
{
  if ( !mSelf )
    serverManagerDeleter.setObject( mSelf, new ServerTypeManager( 0, 
                                    "ServerTypeManager" ) );

  return mSelf;
}

QStringList ServerTypeManager::identifiers() const
{
  return mServerTypeFactoryMap.keys();
}

QString ServerTypeManager::title( const QString& identifier ) const
{
  ServerTypeFactoryMap::ConstIterator it = mServerTypeFactoryMap.find( identifier );
  if ( it == mServerTypeFactoryMap.end() )
    return QString::null;
  else
    return it.data()->title();
}

ServerType* ServerTypeManager::serverType( const QString& identifier )
{
  ServerTypeMap::ConstIterator serverIt = mServerTypeMap.find( identifier );
  if ( serverIt == mServerTypeMap.end() ) { // none server type loaded yet
    ServerTypeFactoryMap::Iterator it = mServerTypeFactoryMap.find( identifier );
    if ( it == mServerTypeFactoryMap.end() ) // no factory for this type
      return 0;

    ServerType *serverType = it.data()->serverType( 0, identifier.latin1() );
    if ( !serverType )
      return 0;

    mServerTypeMap.insert( identifier, serverType );
  }

  return mServerTypeMap[ identifier ];
}

void ServerTypeManager::loadPlugins()
{
  mServerTypeMap.clear();
  mServerTypeFactoryMap.clear();

  ServerTypeFactory *factory = new EGroupwareHandlerFactory();
  mServerTypeFactoryMap.insert( factory->identifier(), factory );
}

#include "servertypemanager.moc"
