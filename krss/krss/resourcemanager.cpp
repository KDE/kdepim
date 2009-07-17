/*
    Copyright (C) 2008    Dmitry Ivanov <vonami@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "resourcemanager.h"
#include "resource.h"
#include "tagidsattribute.h"
#include "subscriptionlabelscollectionattribute.h"
#include "feedpropertiescollectionattribute.h"
#include "krss/config-nepomuk.h"

#ifndef HAVE_NEPOMUK
#include "defaulttagprovider/tagpropertiesattribute.h"
#endif

#include <akonadi/agentmanager.h>
#include <akonadi/agentinstance.h>
#include <akonadi/attributefactory.h>
#include <KDebug>

#include <QtCore/QHash>

using namespace KRss;

namespace KRss {

class ResourceManagerPrivate
{
public:

    explicit ResourceManagerPrivate()
    {
    }

    QHash<QString, Akonadi::AgentInstance> m_instances;
    QHash<QString, const Resource *> m_resources;
    static ResourceManager *m_self;
};

} // namespace KRss

ResourceManager* ResourceManagerPrivate::m_self = 0;

ResourceManager* ResourceManager::self()
{
    if ( ! ResourceManagerPrivate::m_self ) {
        ResourceManagerPrivate::m_self = new ResourceManager();
    }

    return ResourceManagerPrivate::m_self;
}

ResourceManager::ResourceManager()
    : QObject( 0 ), d( new ResourceManagerPrivate() )
{
    QList<Akonadi::AgentInstance> instances = Akonadi::AgentManager::self()->instances();
    Q_FOREACH( const Akonadi::AgentInstance &instance, instances ) {
        kDebug() << "Instance:" << instance.identifier();
        kDebug() << "Capabilities:" << instance.type().capabilities();
        if ( instance.type().capabilities().contains( "RssResource" ) ) {
            d->m_instances[ instance.identifier() ] = instance;
        }
    }
}

ResourceManager::~ResourceManager()
{
    delete d;
}

QStringList ResourceManager::identifiers() const
{
    return d->m_instances.keys();
}

QString ResourceManager::resourceName( const QString &resourceIdentifier ) const
{
    return d->m_instances.value( resourceIdentifier ).name();
}

const Resource* ResourceManager::resource( const QString &resourceIdentifier )
{
    // check if we allocated this resource earlier
    const Resource* resource1 = d->m_resources.value( resourceIdentifier );
    if ( resource1 )
        return resource1;

    // check if this resource exists at all
    Akonadi::AgentInstance instance = d->m_instances.value( resourceIdentifier );
    if ( !instance.isValid() )
        return 0;

    // ok, allocate a new resource
    const Resource* resource2 = new Resource( instance );
    d->m_resources[ resourceIdentifier ] = resource2;
    return resource2;
}

void ResourceManager::registerAttributes()
{
    Akonadi::AttributeFactory::registerAttribute<TagIdsAttribute>();
    Akonadi::AttributeFactory::registerAttribute<SubscriptionLabelsCollectionAttribute>();
    Akonadi::AttributeFactory::registerAttribute<FeedPropertiesCollectionAttribute>();
#ifndef HAVE_NEPOMUK
    Akonadi::AttributeFactory::registerAttribute<TagPropertiesAttribute>();
#endif
}

#include "resourcemanager.moc"
