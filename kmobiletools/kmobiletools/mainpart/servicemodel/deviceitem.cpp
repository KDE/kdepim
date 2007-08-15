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

#include "deviceitem.h"

#include <libkmobiletools/enginexp.h>
#include <libkmobiletools/deviceloader.h>
#include <libkmobiletools/ifaces/coreservice.h>
#include <libkmobiletools/ifaces/guiservice.h>

#include <KService>
#include <KServiceTypeTrader>

#include <QStringList>

#include "serviceitem.h"

DeviceItem::DeviceItem( const QString& name, TreeItem* parent )
: TreeItem( name, parent )
{
    queryServices();
}


DeviceItem::~DeviceItem()
{
}

void DeviceItem::queryServices() {
    KService::List serviceOffers = KServiceTypeTrader::self()->query( "KMobileTools/CoreService" );

    if( !serviceOffers.size() )
        return;

    KMobileTools::EngineXP* engine = KMobileTools::DeviceLoader::instance()->engine( data().toString() );
    if( !engine )
        return;

    // iterate over the services and look which one we can use
    KMobileTools::Ifaces::CoreService* service;
    QStringList deviceName;
    deviceName << data().toString();
    for( int i=0; i<serviceOffers.size(); i++ ) {
        QObject* serviceObject = KService::createInstance<QObject>( serviceOffers.at( i ), (QObject*) 0, deviceName );
        if( !serviceObject )
            continue;

        service = qobject_cast<KMobileTools::Ifaces::CoreService*>( serviceObject );
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
            ServiceItem* serviceItem = new ServiceItem( service->name(), this );
            serviceItem->setService( serviceObject );

            // now check if it's a gui service.. only gui services are worth being displayed ;-)
            KMobileTools::Ifaces::GuiService* guiService =
                        qobject_cast<KMobileTools::Ifaces::GuiService*>( serviceObject );

            if( guiService )
                serviceItem->setIcon( guiService->icon() );
            else
                serviceItem->setVisible( false );

            appendChild( serviceItem );
        }
        else
            delete service;
    }
}

#include "deviceitem.moc"
