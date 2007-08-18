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

#include "deviceloader.h"

#include <libkmobiletools/enginexp.h>

#include <KService>
#include <KServiceTypeTrader>

#include <QtCore/QMutex>
#include <QtCore/QHash>

namespace KMobileTools {

class DeviceLoaderInstance {
public:
    DeviceLoader m_uniqueInstance;
};

class DeviceLoaderPrivate {
public:
    QHash<QString,KMobileTools::EngineXP*> m_loadedDevices;
    QHash<QString,KPluginInfo> m_engineInformation;

    /**
     * A list of engines available as KDE service
     */
    KService::List m_engineOffers;
};

K_GLOBAL_STATIC(DeviceLoaderInstance, deviceLoaderInstance)

DeviceLoader::DeviceLoader()
: QObject( 0 ), d( new DeviceLoaderPrivate )
{
}

DeviceLoader* DeviceLoader::instance() {
    // instance is automatically created
    return &deviceLoaderInstance->m_uniqueInstance;
}


DeviceLoader::~DeviceLoader()
{
    // unloading all devices...
    QHashIterator<QString,KMobileTools::EngineXP*> i( d->m_loadedDevices );
    while (i.hasNext()) {
        i.next();
        unloadDevice( i.key() );
    }

    delete d;
}

bool DeviceLoader::loadDevice( const QString& deviceName, const QString& engineName ) {
    // device already loaded?
    if( d->m_loadedDevices.contains( deviceName ) )
        return false;

    // query the required service from KDE's services if necessary
    if( d->m_engineOffers.empty() )
        d->m_engineOffers = KServiceTypeTrader::self()->query( "KMobileTools/EngineXP" );

    // no engines available at all?
    if( d->m_engineOffers.empty() )
        return false;

    // check if a there's a suitable engine service in the list
    int serviceNumber = 0;
    bool engineFound = false;
    for( ; serviceNumber < d->m_engineOffers.size(); serviceNumber++ ) {
        if( d->m_engineOffers.at( serviceNumber )->name() == engineName ) {
            engineFound = true;
            break;
        }
    }

    // no suitable engine available?
    if( !engineFound )
        return false;

    // try to load the engine
    QStringList argDeviceName( deviceName );
    KMobileTools::EngineXP* engine = KService::createInstance<KMobileTools::EngineXP>
                                     ( d->m_engineOffers.at( serviceNumber ), (QObject*) 0, argDeviceName );
    if( !engine )
        return false;

    // retrieve information about the engine
    d->m_engineInformation.insert( deviceName, KPluginInfo( d->m_engineOffers.at( serviceNumber ) ) );

    d->m_loadedDevices.insert( deviceName, engine );
    emit deviceLoaded( deviceName );

    return true;
}

bool DeviceLoader::unloadDevice( const QString& deviceName ) {
    if( d->m_loadedDevices.contains( deviceName ) ) {
        delete d->m_loadedDevices.value( deviceName );
        d->m_loadedDevices.remove( deviceName );
        d->m_engineInformation.remove( deviceName );

        emit deviceUnloaded( deviceName );
        return true;
    }
    return false;
}

EngineXP* DeviceLoader::engine( const QString& deviceName ) const {
    if( d->m_loadedDevices.contains( deviceName ) )
        return d->m_loadedDevices.value( deviceName );

    return 0;
}

KPluginInfo DeviceLoader::engineInformation( const QString& deviceName ) const {
    if( d->m_engineInformation.contains( deviceName ) )
        return d->m_engineInformation.value( deviceName );

    return KPluginInfo();
}

}

#include "deviceloader.moc"
