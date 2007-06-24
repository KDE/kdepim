/*  This file is part of kdepim
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


#include <QMap>

#include <KApplication>
#include <KDebug>

#include "network.h"
#include "clientadaptor.h"
#include "serviceadaptor.h"
#include "networkstatus.h"

extern "C" {
    KDE_EXPORT KDEDModule* create_networkstatus()
    {
        return new NetworkStatusModule();
    }
}

// INTERNALLY USED STRUCTS AND TYPEDEFS

typedef QMap< QString, Network * > NetworkMap;

class NetworkStatusModule::Private
{
public:
    Private() : status( NetworkStatus::NoNetworks )
    {

    }
    ~Private()
    {

    }
    NetworkMap networks;
    NetworkStatus::Status status;
};

// CTORS/DTORS

NetworkStatusModule::NetworkStatusModule() : KDEDModule(), d( new Private )
{
    new ClientAdaptor( this );
    new ServiceAdaptor( this );

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject( "/Networking", this );
    QDBusConnectionInterface * sessionBus = dbus.interface();

    connect( sessionBus, SIGNAL(serviceOwnerChanged(const QString&,const QString&,const QString&)), this, SLOT(serviceOwnerChanged(const QString&,const QString&,const QString&)) );
}

NetworkStatusModule::~NetworkStatusModule()
{
    foreach ( Network * net, d->networks ) {
        delete net;
    }

    delete d;
}

// CLIENT INTERFACE

int NetworkStatusModule::status()
{
    kDebug( 1222 ) << k_funcinfo << " status: " << (int)d->status << endl;
    return (int)d->status;
}

//protected:

void NetworkStatusModule::updateStatus()
{
    NetworkStatus::Status bestStatus = NetworkStatus::NoNetworks;
    const NetworkStatus::Status oldStatus = d->status;

    foreach ( Network * net, d->networks ) {
        if ( net->status() > bestStatus )
            bestStatus = net->status();
    }
    d->status = bestStatus;

    if ( oldStatus != d->status ) {
        emit statusChanged( (uint)d->status );
    }
}

void NetworkStatusModule::serviceOwnerChanged( const QString & name ,const QString & oldOwner, const QString & newOwner )
{
  if ( !oldOwner.isEmpty() && newOwner.isEmpty( ) ) {
    // unregister and delete any networks owned by a service that has just unregistered
    QMutableMapIterator<QString,Network*> it( d->networks );
    while ( it.hasNext() ) {
      it.next();
      if ( it.value()->service() == name )
      {
        kDebug( 1222 ) << "Departing service " << name << " owned network " << it.value()->name() << ", removing it" << endl;
        Network * removedNet = it.value();
        it.remove();
        updateStatus();
        delete removedNet;
      }
    }
  }
}

// SERVICE INTERFACE //

QStringList NetworkStatusModule::networks()
{
    if ( d->networks.count() ) {
      kDebug() << "Network status module is aware of " << d->networks.count() << " networks" << endl;
    } else {
      kDebug( 1222 ) << "Network status module is not aware of any networks" << endl;
    }
    return d->networks.keys();
}

void NetworkStatusModule::setNetworkStatus( const QString & networkName, int st )
{
    kDebug( 1222 ) << k_funcinfo << networkName << ", " << st << endl;
    NetworkStatus::Status changedStatus = (NetworkStatus::Status)st;
    if ( d->networks.contains( networkName ) ) {
      Network * net = d->networks[ networkName ];
        net->setStatus( changedStatus );
        updateStatus();
    } else {
      kDebug( 1222 ) << "  No network named '" << networkName << "' known." << endl;
    }
}

void NetworkStatusModule::registerNetwork( const QString & networkName, int status, const QString & serviceName )
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    QDBusConnectionInterface * sessionBus = dbus.interface();
    QString uniqueOwner = sessionBus->serviceOwner( serviceName ).value();

    kDebug( 1222 ) << k_funcinfo << networkName << ", with status " << status << " is owned by " << uniqueOwner<< endl;

    d->networks.insert( networkName, new Network( networkName, status, uniqueOwner ) );
    updateStatus();
}

void NetworkStatusModule::unregisterNetwork( const QString & networkName )
{
    kDebug( 1222 ) << k_funcinfo << networkName << " unregistered." << endl;

    d->networks.remove( networkName );
    updateStatus();
}

#include "networkstatus.moc"
// vim: set noet sw=4 ts=4:
