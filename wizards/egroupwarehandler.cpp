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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <libkcal/resourcecalendar.h>
#include <kurl.h>

#include "kresources/egroupware/kcal_resourcexmlrpc.h"
#include "kresources/egroupware/kabc_resourcexmlrpc.h"

#include "editdialog.h"
#include "resourceactions.h"

#include "egroupwarehandler.h"

ServerType* EGroupwareHandlerFactory::serverType( QObject*, const char*)
{
  return new EGroupwareHandler();
}

extern "C"
{
  void *init_libegroupwarewizard()
  {
    return ( new EGroupwareHandlerFactory() );
  }
}

static bool resourcesEquals( KABC::ResourceXMLRPC *kabcResource, KCal::ResourceXMLRPC *kcalResource );
static QString createUrl( bool useSSL, const QString &server );

EGroupwareHandler::EGroupwareHandler()
  : ServerType( 0, "egroupware" )
{
  collectConnections();
}

EGroupwareHandler::~EGroupwareHandler()
{
}

ServerType::ConnectionInfoList EGroupwareHandler::connectionInfo() const
{
  return mInfoList;
}

void EGroupwareHandler::addConnection()
{
  EditDialog dlg( 0 );

  dlg.setDomain( "default" );

  if ( !dlg.exec() )
    return;

  CreateKABCResource *kabcResource = new CreateKABCResource;
  kabcResource->setResourceName( dlg.resourceName() );
  kabcResource->setUrl( createUrl( dlg.useSSLConnection(), dlg.server() ) );
  kabcResource->setDomain( dlg.domain() );
  kabcResource->setUser( dlg.user() );
  kabcResource->setPassword( dlg.password() );

  CreateKCalResource *kcalResource = new CreateKCalResource;
  kcalResource->setResourceName( dlg.resourceName() );
  kcalResource->setUrl( createUrl( dlg.useSSLConnection(), dlg.server() ) );
  kcalResource->setDomain( dlg.domain() );
  kcalResource->setUser( dlg.user() );
  kcalResource->setPassword( dlg.password() );

  mChanges.append( kabcResource );
  mChanges.append( kcalResource );

  collectConnections();
}

void EGroupwareHandler::editConnection( const QString& uid )
{
  EditDialog dlg( 0 );

  int colon = uid.find( ':' );
  QString kabcUid = uid.left( colon );
  QString kcalUid = uid.mid( colon + 1 );


  QString name, url, domain, user, password;
  if ( !kabcUid.isEmpty() )
    kabcResourceSettings( kabcUid, name, url, domain, user, password );
  else
    kcalResourceSettings( kcalUid, name, url, domain, user, password );

  KURL fullUrl( url );

  dlg.setResourceName( name );
  dlg.setServer( fullUrl.host() );
  dlg.setDomain( domain );
  dlg.setUser( user );
  dlg.setPassword( password );
  dlg.setUseSSLConnection( fullUrl.protocol() == "https" );

  if ( !dlg.exec() )
    return;

  if ( !kabcUid.isEmpty() ) {
    UpdateKABCResource *kabcResource = new UpdateKABCResource( kabcUid );
    kabcResource->setResourceName( dlg.resourceName() );
    kabcResource->setUrl( createUrl( dlg.useSSLConnection(), dlg.server() ) );
    kabcResource->setDomain( dlg.domain() );
    kabcResource->setUser( dlg.user() );
    kabcResource->setPassword( dlg.password() );
    
    mChanges.append( kabcResource );
  }

  if ( !kcalUid.isEmpty() ) {
    UpdateKCalResource *kcalResource = new UpdateKCalResource( kcalUid );
    kcalResource->setResourceName( dlg.resourceName() );
    kcalResource->setUrl( createUrl( dlg.useSSLConnection(), dlg.server() ) );
    kcalResource->setDomain( dlg.domain() );
    kcalResource->setUser( dlg.user() );
    kcalResource->setPassword( dlg.password() );

    mChanges.append( kcalResource );
  }

  collectConnections();
}

void EGroupwareHandler::deleteConnection( const QString& uid )
{
  int colon = uid.find( ':' );
  QString kabcUid = uid.left( colon );
  QString kcalUid = uid.mid( colon + 1 );

  if ( !kabcUid.isEmpty() ) {
    DeleteKABCResource *kabcResource = new DeleteKABCResource( kabcUid );

    mChanges.append( kabcResource );
  }

  if ( !kcalUid.isEmpty() ) {
    DeleteKCalResource *kcalResource = new DeleteKCalResource( kcalUid );

    mChanges.append( kcalResource );
  }

  collectConnections();
}

void EGroupwareHandler::activateConnection( const QString& uid, bool active )
{
  int colon = uid.find( ':' );
  QString kabcUid = uid.left( colon );
  QString kcalUid = uid.mid( colon + 1 );

  if ( !kabcUid.isEmpty() ) {
    ActivateKABCResource *kabcResource = new ActivateKABCResource( kabcUid,
                                                                   active );

    mChanges.append( kabcResource );
  }

  if ( !kcalUid.isEmpty() ) {
    ActivateKCalResource *kcalResource = new ActivateKCalResource( kcalUid,
                                                                   active );

    mChanges.append( kcalResource );
  }

  collectConnections();
}

void EGroupwareHandler::collectConnections()
{
  mInfoList.clear();

  KRES::Manager<KABC::Resource> kabcManager( "contact" );
  KCal::CalendarResourceManager kcalManager( "calendar" );
  kabcManager.readConfig();
  kcalManager.readConfig();

  // search equal kabc and kcal resources and combine them to a logical one
  KRES::Manager<KABC::Resource>::Iterator kabcIt;
  for ( kabcIt = kabcManager.begin(); kabcIt != kabcManager.end(); ++kabcIt ) {
    if ( (*kabcIt)->type() == "xmlrpc" ) {
      KABC::ResourceXMLRPC *kabcResource = static_cast<KABC::ResourceXMLRPC*>( *kabcIt );

      bool found = false;
      KCal::CalendarResourceManager::Iterator kcalIt;
      for ( kcalIt = kcalManager.begin(); kcalIt != kcalManager.end(); ++kcalIt ) {
        if ( (*kcalIt)->type() == "xmlrpc" ) {
          KCal::ResourceXMLRPC *kcalResource = static_cast<KCal::ResourceXMLRPC*>( *kcalIt );
          if ( resourcesEquals( kabcResource, kcalResource ) ) {
            ServerType::ConnectionInfo info;
            info.uid = kabcResource->identifier() + ":" + kcalResource->identifier();
            info.name = kabcResource->resourceName();
            info.active = kabcResource->isActive();
            mInfoList.append( info );

            found = true;
          }
        }
      }


      if ( !found ) {
        ServerType::ConnectionInfo info;
        info.uid = kabcResource->identifier() + ":";
        info.name = kabcResource->resourceName();
        info.active = kabcResource->isActive();
        mInfoList.append( info );
      }
    }
  }

  QStringList keys;

  ServerType::ConnectionInfoList::Iterator infoIt;
  for ( infoIt = mInfoList.begin(); infoIt != mInfoList.end(); ++infoIt )
    keys.append( (*infoIt).uid );

  QStringList::Iterator it;

  KCal::CalendarResourceManager::Iterator kcalIt;
  for ( kcalIt = kcalManager.begin(); kcalIt != kcalManager.end(); ++kcalIt ) {
    if ( (*kcalIt)->type() == "xmlrpc" ) {
      KCal::ResourceXMLRPC *kcalResource = static_cast<KCal::ResourceXMLRPC*>( *kcalIt );
      bool found = false;
      for ( it = keys.begin(); it != keys.end(); ++it ) {
        if ( (*it).endsWith( ":" + kcalResource->identifier() ) )
          found = true;
      }

      if ( !found ) {
        ServerType::ConnectionInfo info;
        info.uid = ":" + kcalResource->identifier();
        info.name = kcalResource->resourceName();
        info.active = kcalResource->isActive();
        mInfoList.append( info );
      }
    }
  }
}

bool resourcesEquals( KABC::ResourceXMLRPC *kabcResource, KCal::ResourceXMLRPC *kcalResource )
{
  return ( kabcResource->url().host() == kcalResource->url().host() &&
           kabcResource->domain() == kcalResource->domain() &&
           kabcResource->user() == kcalResource->user() );
}

QString createUrl( bool useSSL, const QString &server )
{
  KURL url;

  if ( useSSL )
    url.setProtocol( "https" );
  else
    url.setProtocol( "http" );

  url.setHost( server );
  url.setPath( "/egroupware/xmlrpc.php" );

  return url.url();
}

KConfigPropagator::Change::List EGroupwareHandler::changes()
{
  return mChanges;
}
