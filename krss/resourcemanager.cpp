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
#include "netresource.h"
#include "tagidsattribute.h"
#include "subscriptionlabelscollectionattribute.h"
#include "feedpropertiescollectionattribute.h"
#include "config-nepomuk.h"

#ifndef HAVE_NEPOMUK
#include "defaulttagprovider/tagpropertiesattribute.h"
#endif

#include <akonadi/agentmanager.h>
#include <akonadi/agentinstance.h>
#include <akonadi/attributefactory.h>
#include <KDebug>

#include <QtCore/QHash>
#include <boost/shared_ptr.hpp>

using namespace KRss;
using boost::shared_ptr;

namespace KRss {

class ResourceManagerPrivate
{
public:
    QHash<QString, shared_ptr<NetResource> > m_resources;
};

} // namespace KRss

ResourceManager* ResourceManager::self()
{
    static ResourceManager s_instance;
    return &s_instance;
}

ResourceManager::ResourceManager()
    : d( new ResourceManagerPrivate() )
{
    QList<Akonadi::AgentInstance> instances = Akonadi::AgentManager::self()->instances();
    Q_FOREACH( const Akonadi::AgentInstance& instance, instances ) {
        kDebug() << "Instance:" << instance.identifier();
        kDebug() << "Capabilities:" << instance.type().capabilities();
        if ( instance.type().capabilities().contains( "RssResource" ) ) {
            NetResource* const resource = new NetResource( instance.identifier(), instance.name() );
            d->m_resources.insert( instance.identifier(), shared_ptr<NetResource>( resource ) );
        }
    }
}

ResourceManager::~ResourceManager()
{
    delete d;
}

shared_ptr<NetResource> ResourceManager::resource( const QString& resourceId ) const
{
    return d->m_resources.value( resourceId );
}

QStringList ResourceManager::identifiers() const
{
    return d->m_resources.keys();
}

QList<shared_ptr<Resource> > ResourceManager::resources() const
{
    QList<shared_ptr<Resource> > allResources;
    Q_FOREACH( const shared_ptr<NetResource>& resource, d->m_resources.values() )
        allResources.append( resource );

    return allResources;
}

QList<shared_ptr<NetResource> > ResourceManager::netResources() const
{
    return d->m_resources.values();
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
