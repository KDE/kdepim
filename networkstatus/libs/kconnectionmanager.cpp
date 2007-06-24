/*  This file is part of kdepim.
    Copyright (C) 2005,2007 Will Stephenson <wstephenson@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library.  If not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this library
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "kconnectionmanager.h"

#include <KApplication>
#include <KDebug>
#include <KStaticDeleter>

#include "clientinterface.h"

#include "kconnectionmanager_p.h"

// KConnectionManagerPrivate

KConnectionManagerPrivate::KConnectionManagerPrivate(QObject * parent ) : service( new OrgKdeSolidNetworkingClientInterface( "org.kde.kded", "/modules/networkstatus", QDBusConnection::sessionBus(), parent ) ), connectPolicy( KConnectionManager::Managed ), disconnectPolicy( KConnectionManager::Managed )
{
}

KConnectionManagerPrivate::~KConnectionManagerPrivate()
{
}

// Connection manager itself
KConnectionManager::KConnectionManager( QObject * parent ) : QObject( parent ), d( new KConnectionManagerPrivate( this ) )
{
    connect( d->service, SIGNAL(statusChanged(uint)), this, SLOT(serviceStatusChanged(uint)) );
    connect( QDBusConnection::sessionBus().interface(), SIGNAL(serviceOwnerChanged(const QString&, const QString&, const QString & ) ), SLOT(serviceOwnerChanged(const QString&, const QString&, const QString & ) ) );

    initialize();
}

KConnectionManager::~KConnectionManager()
{
    delete d;
}

KConnectionManager *KConnectionManager::s_self = 0L;

KConnectionManager *KConnectionManager::self()
{
    static KStaticDeleter<KConnectionManager> deleter;
    if(!s_self)
        deleter.setObject( s_self, new KConnectionManager( 0 ) );
    return s_self;
}

void KConnectionManager::initialize()
{
    // determine initial state and set the state object accordingly.
    uint status = d->service->status();

    d->status = ( NetworkStatus::Status )status;
}

NetworkStatus::Status KConnectionManager::status() const
{
    return d->status;
}

void KConnectionManager::serviceStatusChanged( uint status )
{
//    kDebug( 921 ) << k_funcinfo << endl;
    d->status = ( NetworkStatus::Status )status;
    switch ( status ) {
      case NetworkStatus::NoNetworks:
        break;
      case NetworkStatus::Unreachable:
        break;
      case NetworkStatus::OfflineDisconnected:
      case NetworkStatus::OfflineFailed:
      case NetworkStatus::TearingDown:
      case NetworkStatus::Offline:
      case NetworkStatus::Establishing:
        if ( d->disconnectPolicy == Managed ) {
          emit shouldDisconnect();
        } else if ( d->disconnectPolicy == OnNextStatusChange ) {
          setDisconnectPolicy( Manual );
          emit shouldDisconnect();
        }
        break;
      case NetworkStatus::Online:
        if ( d->disconnectPolicy == Managed ) {
          emit shouldConnect();
        } else if ( d->disconnectPolicy == OnNextStatusChange ) {
          setConnectPolicy( Manual );
          emit shouldConnect();
        }
        break;
      default:
        kDebug( 921 ) << k_funcinfo <<  "Unrecognised status code!" << endl;
    }
    emit statusChanged( d->status );
}

void KConnectionManager::serviceOwnerChanged( const QString & name, const QString & oldOwner, const QString & newOwner )
{
  Q_UNUSED( oldOwner );
  if ( name == "org.kde.kded" ) {
    if ( newOwner.isEmpty() ) {
      // kded quit on us
      d->status = NetworkStatus::NoNetworks;
      emit statusChanged( d->status );
    } else {
      // kded was replaced or started
      initialize();
      emit statusChanged( d->status );
      serviceStatusChanged( d->status );
    }
  }
}

KConnectionManager::ManagementPolicy KConnectionManager::connectPolicy() const
{
    return d->connectPolicy;
}

void KConnectionManager::setConnectPolicy( KConnectionManager::ManagementPolicy policy )
{
    d->connectPolicy = policy;
}

KConnectionManager::ManagementPolicy KConnectionManager::disconnectPolicy() const
{
    return d->disconnectPolicy;
}

void KConnectionManager::setDisconnectPolicy( KConnectionManager::ManagementPolicy policy )
{
    d->disconnectPolicy = policy;
}

#include "kconnectionmanager.moc"

