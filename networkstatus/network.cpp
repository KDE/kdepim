#include <kdebug.h>

#include "network.h"

Network::Network( const QString name, NetworkStatus::Properties properties )
	: m_name( name )
{
	kdDebug() << k_funcinfo << "constructing network '" << name << "', status: " << properties.status << endl;
	m_status = properties.status;
	m_netmasks = properties.netmasks;
	m_internet = properties.internet;
	m_service = properties.service;
	m_onDemandPolicy = properties.onDemandPolicy;
}

NetworkStatus::EnumStatus Network::reachabilityFor( const QString & host )
{
	// initially assume all networks are internet
	// TODO: compute reachability properly
	Q_UNUSED( host );
	if ( true /*nss.properties.internet && notPrivateNetwork( host )*/ )
	{
		NetworkStatus::EnumStatus status;
		if ( m_status == NetworkStatus::Establishing || m_status == NetworkStatus::Online )
			status = NetworkStatus::Online;
		else if ( m_status == NetworkStatus::ShuttingDown || m_status == NetworkStatus::Offline )
			status = NetworkStatus::Offline;
		else
			status = m_status;
		
		return status;
	}
}

void Network::registerUsage( const QCString appId, const QString host )
{
	NetworkUsageStruct nus;
	nus.appId = appId;
	nus.host = host;
	NetworkUsageList::iterator end = m_usage.end();
	for ( NetworkUsageList::iterator it = m_usage.begin(); it != end; ++it )
	{
		if ( (*it).appId == appId && (*it).host == host )
			return;
	}
	kdDebug() << k_funcinfo << "registering " << appId << " as using network " << m_name << " for " << host << endl;
	m_usage.append( nus );
}

void Network::unregisterUsage( const QCString appId, const QString host )
{
	NetworkUsageList::iterator end = m_usage.end();
	for ( NetworkUsageList::iterator it = m_usage.begin(); it != end; ++it )
	{
		if ( (*it).appId == appId && (*it).host == host )
		{
			kdDebug() << k_funcinfo << "unregistering " << appId << "'s usage of " << m_name << " for " << host << endl;
			m_usage.remove( it );
			break;
		}
	}
}

void Network::setStatus( NetworkStatus::EnumStatus status )
{
	m_status = status;
}

void Network::removeAllUsage()
{
	m_usage.clear();
}