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

#ifndef RESOURCEACTIONS_H
#define RESOURCEACTIONS_H

#include <kconfigpropagator.h>
#include <klocale.h>
#include <kurl.h>

class ResourceAction : public KConfigPropagator::Change
{
  public:
    ResourceAction( const QString &title )
      : KConfigPropagator::Change( title )
    {
    }

    void setResourceName( const QString &name ) { mResourceName = name; }
    void setUrl( const KURL &url ) { mUrl = url; }
    void setDomain( const QString &domain ) { mDomain = domain; }
    void setUser( const QString &user ) { mUser = user; }
    void setPassword( const QString &password ) { mPassword = password; }

  protected:
    QString mResourceName;
    KURL mUrl;
    QString mDomain;
    QString mUser;
    QString mPassword;
};

class CreateResource : public ResourceAction
{
  public:
    CreateResource( const QString &title )
      : ResourceAction( title )
    {
    }
};

class UpdateResource : public ResourceAction
{
  public:
    UpdateResource( const QString &uid, const QString &title )
      : ResourceAction( title ), mResourceUid( uid )
    {
    }

  protected:
    QString mResourceUid;
};

class DeleteResource : public ResourceAction
{
  public:
    DeleteResource( const QString &uid, const QString &title )
      : ResourceAction( title ), mResourceUid( uid )
    {
    }

  protected:
    QString mResourceUid;
};

class ActivateResource : public ResourceAction
{
  public:
    ActivateResource( const QString &uid, bool active, const QString &title )
      : ResourceAction( title ), mResourceUid( uid ), mActive( active )
    {
    }

  protected:
    QString mResourceUid;
    bool mActive;
};

class CreateKCalResource : public CreateResource
{
  public:
    CreateKCalResource()
      : CreateResource( i18n( "Create eGroupware calendar" ) )
    {}

    void apply()
    {
      KCal::CalendarResourceManager manager( "calendar" );
      manager.readConfig();
      KCal::ResourceXMLRPC *resource = new KCal::ResourceXMLRPC();
      resource->setResourceName( mResourceName );
      resource->setURL( mUrl );
      resource->setDomain( mDomain );
      resource->setUser( mUser );
      resource->setPassword( mPassword );
      manager.add( resource );
      manager.writeConfig();
    }
};

class UpdateKCalResource : public UpdateResource
{
  public:
    UpdateKCalResource( const QString &uid )
      : UpdateResource( uid, i18n( "Update eGroupware calendar" ) )
    {}

    void apply()
    {
      KCal::CalendarResourceManager manager( "calendar" );
      manager.readConfig();

      KCal::CalendarResourceManager::Iterator it;
      for ( it = manager.begin(); it != manager.end(); ++it ) {
        if ( (*it)->identifier() == mResourceUid ) {
          KCal::ResourceXMLRPC *resource = static_cast<KCal::ResourceXMLRPC*>( *it );
          resource->setResourceName( mResourceName );
          resource->setURL( mUrl );
          resource->setDomain( mDomain );
          resource->setUser( mUser );
          resource->setPassword( mPassword );
        }
      }

      manager.writeConfig();
    }
};

class DeleteKCalResource : public DeleteResource
{
  public:
    DeleteKCalResource( const QString &uid )
      : DeleteResource( uid, i18n( "Delete eGroupware calendar" ) )
    {}

    void apply()
    {
      KCal::CalendarResourceManager manager( "calendar" );
      manager.readConfig();

      KCal::CalendarResourceManager::Iterator it;
      for ( it = manager.begin(); it != manager.end(); ++it ) {
        if ( (*it)->identifier() == mResourceUid ) {
          KCal::ResourceXMLRPC *resource = static_cast<KCal::ResourceXMLRPC*>( *it );
          manager.remove( resource );
          break;
        }
      }

      manager.writeConfig();
    }
};

class ActivateKCalResource : public ActivateResource
{
  public:
    ActivateKCalResource( const QString &uid, bool active )
      : ActivateResource( uid, active, i18n( "Activate eGroupware calendar" ) )
    {}

    void apply()
    {
      KCal::CalendarResourceManager manager( "calendar" );
      manager.readConfig();

      KCal::CalendarResourceManager::Iterator it;
      for ( it = manager.begin(); it != manager.end(); ++it ) {
        if ( (*it)->identifier() == mResourceUid ) {
          KCal::ResourceXMLRPC *resource = static_cast<KCal::ResourceXMLRPC*>( *it );
          resource->setActive( mActive );
          break;
        }
      }

      manager.writeConfig();
    }
};

class CreateKABCResource : public CreateResource
{
  public:
    CreateKABCResource()
      : CreateResource( i18n( "Create eGroupware address book" ) )
    {}
    
    void apply()
    {
      KRES::Manager<KABC::Resource> manager( "contact" );
      manager.readConfig();

      KABC::ResourceXMLRPC *resource = new KABC::ResourceXMLRPC( mUrl, mDomain, mUser, mPassword );
      resource->setResourceName( mResourceName );
      manager.add( resource );
      manager.writeConfig();
    }
};

class UpdateKABCResource : public UpdateResource
{
  public:
    UpdateKABCResource( const QString &uid )
      : UpdateResource( uid, i18n( "Update eGroupware address book" ) )
    {}

    void apply()
    {
      KRES::Manager<KABC::Resource> manager( "contact" );
      manager.readConfig();

      KRES::Manager<KABC::Resource>::Iterator it;
      for ( it = manager.begin(); it != manager.end(); ++it ) {
        if ( (*it)->identifier() == mResourceUid ) {
          KABC::ResourceXMLRPC *resource = static_cast<KABC::ResourceXMLRPC*>( *it );
          resource->setResourceName( mResourceName );
          resource->setURL( mUrl );
          resource->setDomain( mDomain );
          resource->setUser( mUser );
          resource->setPassword( mPassword );
        }
      }

      manager.writeConfig();
    }
};

class DeleteKABCResource : public DeleteResource
{
  public:
    DeleteKABCResource( const QString &uid )
      : DeleteResource( uid, i18n( "Delete eGroupware address book" ) )
    {}

    void apply()
    {
      KRES::Manager<KABC::Resource> manager( "contact" );
      manager.readConfig();

      KRES::Manager<KABC::Resource>::Iterator it;
      for ( it = manager.begin(); it != manager.end(); ++it ) {
        if ( (*it)->identifier() == mResourceUid ) {
          KABC::ResourceXMLRPC *resource = static_cast<KABC::ResourceXMLRPC*>( *it );
          manager.remove( resource );
          break;
        }
      }

      manager.writeConfig();
    }
};

class ActivateKABCResource : public ActivateResource
{
  public:
    ActivateKABCResource( const QString &uid, bool active )
      : ActivateResource( uid, active, i18n( "Activate eGroupware address book" ) )
    {}

    void apply()
    {
      KRES::Manager<KABC::Resource> manager( "contact" );
      manager.readConfig();

      KRES::Manager<KABC::Resource>::Iterator it;
      for ( it = manager.begin(); it != manager.end(); ++it ) {
        if ( (*it)->identifier() == mResourceUid ) {
          KABC::ResourceXMLRPC *resource = static_cast<KABC::ResourceXMLRPC*>( *it );
          resource->setActive( mActive );
          break;
        }
      }

      manager.writeConfig();
    }
};

/**
  Helper function.
 */

static void kabcResourceSettings( const QString &uid, QString &name, QString &url, 
                                  QString &domain, QString &user, QString &password )
{
  KRES::Manager<KABC::Resource> manager( "contact" );
  manager.readConfig();

  KRES::Manager<KABC::Resource>::Iterator it;
  for ( it = manager.begin(); it != manager.end(); ++it ) {
    if ( (*it)->identifier() == uid ) {
      KABC::ResourceXMLRPC *resource = static_cast<KABC::ResourceXMLRPC*>( *it );
      name = resource->resourceName();
      url = resource->url().url();
      domain = resource->domain();
      user = resource->user();
      password = resource->password();

      break;
    }
  }
}

static void kcalResourceSettings( const QString &uid, QString &name, QString &url, 
                                  QString &domain, QString &user, QString &password )
{
  KCal::CalendarResourceManager manager( "calendar" );
  manager.readConfig();

  KCal::CalendarResourceManager::Iterator it;
  for ( it = manager.begin(); it != manager.end(); ++it ) {
    if ( (*it)->identifier() == uid ) {
      KCal::ResourceXMLRPC *resource = static_cast<KCal::ResourceXMLRPC*>( *it );
      name = resource->resourceName();
      url = resource->url().url();
      domain = resource->domain();
      user = resource->user();
      password = resource->password();

      break;
    }
  }
}

#endif
