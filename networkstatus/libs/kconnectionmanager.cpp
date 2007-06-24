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


// Connection manager itself
KConnectionManager::KConnectionManager( QObject * parent ) : QObject( parent ), d( new KConnectionManagerPrivate( this ) )
{
    connect( d->service, SIGNAL(statusChanged(uint)), this, SLOT(serviceStatusChanged(uint)) );

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

NetworkStatus::Status KConnectionManager::status()
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
          emit d->disconnected();
        } else if ( d->disconnectPolicy == OnNextChange ) {
          setDisconnectPolicy( Manual );
          emit d->disconnected();
        }
        break;
      case NetworkStatus::Online:
        if ( d->disconnectPolicy == Managed ) {
          emit d->connected();
        } else if ( d->disconnectPolicy == OnNextChange ) {
          setConnectPolicy( Manual );
          emit d->connected();
        }
        break;
      default:
        kDebug( 921 ) << k_funcinfo <<  "Unrecognised status code!" << endl;
    }
    emit statusChanged( d->status );
}

KConnectionManager::ConnectionPolicy KConnectionManager::connectPolicy() const
{
    return d->connectPolicy;
}

void KConnectionManager::setConnectPolicy( KConnectionManager::ConnectionPolicy policy )
{
    d->connectPolicy = policy;
}

KConnectionManager::ConnectionPolicy KConnectionManager::disconnectPolicy() const
{
    return d->disconnectPolicy;
}

void KConnectionManager::setDisconnectPolicy( KConnectionManager::ConnectionPolicy policy )
{
    d->disconnectPolicy = policy;
}

void KConnectionManager::setManualConnectionPolicies()
{
    d->connectPolicy = KConnectionManager::Manual;
    d->disconnectPolicy = KConnectionManager::Manual;
}

void KConnectionManager::setManagedConnectionPolicies()
{
    d->connectPolicy = KConnectionManager::Managed;
    d->disconnectPolicy = KConnectionManager::Managed;
}

void KConnectionManager::registerConnectSlot( QObject * receiver, const char * member )
{
    d->connectReceiver = receiver;
    d->connectSlot = member;
    connect( d, SIGNAL(connected()), receiver, member );
}

void KConnectionManager::forgetConnectSlot()
{
    disconnect( d, SIGNAL(connected()), d->connectReceiver, d->connectSlot );
    d->connectReceiver = 0;
    d->connectSlot = 0;
}

bool KConnectionManager::isConnectSlotRegistered() const
{
    return ( d->connectSlot != 0 );
}

void KConnectionManager::registerDisconnectSlot( QObject * receiver, const char * member )
{
    d->disconnectReceiver = receiver;
    d->disconnectSlot = member;
    connect( d, SIGNAL(disconnected()), receiver, member );
}

void KConnectionManager::forgetDisconnectSlot()
{
    disconnect( d, SIGNAL(disconnected()), d->disconnectReceiver, d->disconnectSlot );
    d->disconnectReceiver = 0;
    d->disconnectSlot = 0;
}

bool KConnectionManager::isDisconnectSlotRegistered() const
{
    return ( d->disconnectSlot != 0 );
}

#include "kconnectionmanager.moc"

