/***************************************************************************
   Copyright (C) 2007 by Matthias Lechner <matthias@lmme.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "serviceloader.h"

#include <KService>
#include <KServiceTypeTrader>

#include <QtCore/QMutex>
#include <QtCore/QHash>

#include <libkmobiletools/coreservice.h>
#include <libkmobiletools/deviceloader.h>
#include <libkmobiletools/enginexp.h>

namespace KMobileTools {

class ServiceLoaderInstance {
public:
    ServiceLoader m_uniqueInstance;
};

class ServiceLoaderPrivate {
public:
    QHash<QString,KMobileTools::CoreService*> m_loadedServices;
};

K_GLOBAL_STATIC(ServiceLoaderInstance, serviceLoaderInstance)

ServiceLoader::ServiceLoader()
: QObject( 0 ), d( new ServiceLoaderPrivate )
{
}

ServiceLoader* ServiceLoader::instance() {
    // instance is automatically created
    return &serviceLoaderInstance->m_uniqueInstance;
}


ServiceLoader::~ServiceLoader()
{
    // unloading all services
    QHashIterator<QString,KMobileTools::CoreService*> i( d->m_loadedServices );
    while( i.hasNext() ) {
        i.next();
        unloadServices( i.key() );
    }

    delete d;
}

void ServiceLoader::loadServices( const QString& deviceName ) {
    // services already loaded?
    if( d->m_loadedServices.contains( deviceName ) )
        return;

    // query the required service from KDE's services
    KService::List serviceOffers = KServiceTypeTrader::self()->query( "KMobileTools/CoreService" );

    // no services available at all?
    if( serviceOffers.empty() )
        return;

    // get engine for the device
    KMobileTools::EngineXP* engine = KMobileTools::DeviceLoader::instance()->engine( deviceName );
    if( !engine )
        return;

    // iterate over the services and look which one we can use
    KMobileTools::CoreService* service;
    QStringList deviceNameList; // needed KService::createInstance
    deviceNameList << deviceName;
    for( int i=0; i<serviceOffers.size(); i++ ) {
        QObject* serviceObject = KService::createInstance<QObject>( serviceOffers.at( i ), (QObject*) 0, deviceNameList );
        if( !serviceObject )
            continue;

        service = qobject_cast<KMobileTools::CoreService*>( serviceObject );
        if( !service )
            continue;

        bool fulfillsRequirements = true;
        QStringList requirements = service->requires();
        for( int j=0; j<requirements.size(); j++ ) {
            if( !engine->implements( requirements.at(i) ) )
                fulfillsRequirements = false;
        }

        // service fulfills our requirements
        if( fulfillsRequirements ) {
            d->m_loadedServices.insert( deviceName, service );
            emit serviceLoaded( deviceName, service );
        } else
            delete service;
    }
}

void ServiceLoader::unloadServices( const QString& deviceName ) {
    if( d->m_loadedServices.contains( deviceName ) ) {
        QList<KMobileTools::CoreService*> services = d->m_loadedServices.values( deviceName );
        for( int i=0; i<services.size(); i++ ) {
            emit aboutToUnloadService( deviceName, services.value( i ) );
            /// @warning deleting the service here could cause a problem
            /// since a slot that is connected to the aboutToUnloadService signal
            /// could work with the service while it could become invalid
            services.value( i )->deleteLater();
            emit serviceUnloaded( deviceName, services.value( i ) );
        }

        d->m_loadedServices.remove( deviceName );
    }
}

QList<KMobileTools::CoreService*> ServiceLoader::service( const QString& deviceName, const QStringList& interfaces ) const {
    /// @todo implement me
}

}

#include "serviceloader.moc"
