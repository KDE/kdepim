#include "networkstatus.h"

#include <qdict.h>
#include <qtimer.h>
#include <qvaluelist.h>

#include <dcopclient.h>
#include <kapplication.h>
#include <kdebug.h>

#include "clientifaceimpl.h"
#include "serviceifaceimpl.h"
#include "network.h"

extern "C" {
	KDE_EXPORT KDEDModule* create_networkstatus( const QCString& obj )
	{
		return new NetworkStatusModule( obj );
	}
};

// INTERNALLY USED STRUCTS AND TYPEDEFS

//typedef QDict< Network > NetworkList;
typedef QValueList< Network * > NetworkList;

class NetworkStatusModule::Private
{
public:
	NetworkList networks;
/*	ClientIface * clientIface;
	ServiceIface * serviceIface;*/
};

// CTORS/DTORS

NetworkStatusModule::NetworkStatusModule( const QCString & obj ) : KDEDModule( obj )
{
	d = new Private;
/*	d->clientIface = new ClientIfaceImpl( this );
	d->serviceIface = new ServiceIfaceImpl( this );*/
	connect( kapp->dcopClient(), SIGNAL( applicationRemoved( const QCString& ) ) , this, SLOT( unregisteredFromDCOP( const QCString& ) ) );
	connect( kapp->dcopClient(), SIGNAL( applicationRegistered( const QCString& ) ) , this, SLOT( registeredToDCOP( const QCString& ) ) );
}

NetworkStatusModule::~NetworkStatusModule()
{
/*	delete d->clientIface;
	delete d->serviceIface;*/
	delete d;
}

// CLIENT INTERFACE

QStringList NetworkStatusModule::networks()
{
	kdDebug() << k_funcinfo << " contains " << d->networks.count() << " networks" << endl;
	QStringList networks;
	NetworkList::iterator end = d->networks.end();
	NetworkList::iterator it = d->networks.begin();
	for ( ; it != end; ++it )
		networks.append( (*it)->name() );
	return networks;
}

int NetworkStatusModule::status( const QString & host )
{
	Network * p = networkForHost( host );
	if ( !p )
	{
		kdDebug() << k_funcinfo << " no networks have status for host '" << host << "'" << endl;
		return (int)NetworkStatus::NoNetworks;
	}
	else
	{	
		kdDebug() << k_funcinfo << " got status for host '" << host << "' : " << (int)(p->status()) << endl;
		return (int)(p->status());
	}
}

int NetworkStatusModule::request( const QString & host, bool userInitiated )
{
	// identify most suitable network for host
	Network * p = networkForHost( host );
	if ( !p )
		return NetworkStatus::Unavailable;
	
	NetworkStatus::EnumStatus status = p->status();
	QCString appId = kapp->dcopClient()->senderId();
	if ( status == NetworkStatus::Online )
	{
		p->registerUsage( appId, host );
		return NetworkStatus::Connected;
	}
	// if online
	//   register usage
	//   return Available
	else if ( status == NetworkStatus::Establishing )
	{
		p->registerUsage( appId, host );
		return NetworkStatus::RequestAccepted;
	}
	// if establishing
	//   register usage
	//   return Accepted
	else if ( status == NetworkStatus::Offline || status == NetworkStatus::ShuttingDown )
	{
		// TODO: check on demand policy
		
		p->registerUsage( appId, host );
		return NetworkStatus::RequestAccepted;
	}
	// if offline or ShuttingDown
	//   check ODP::
	//   always or Permanent: register, return accepted
	//   user: check userInitiated, register, return Accepted or UserRefused
	//   never: return UserRefused
	else if ( status == NetworkStatus::OfflineFailed )
	{
		// TODO: check user's preference for dealing with failed networks
		p->registerUsage( appId, host );
		return NetworkStatus::RequestAccepted;
	}
	// if OfflineFailed
	//   check user's preference
	else if ( status == NetworkStatus::OfflineDisconnected )
	{
		return NetworkStatus::Unavailable;
	}
	else
		return NetworkStatus::Unavailable;
	// if OfflineDisconnected or NoNetworks
	//   return Unavailable
}

void NetworkStatusModule::relinquish( const QString & host )
{
	QCString appId = kapp->dcopClient()->senderId();
	// find network currently used by app for host...
	NetworkList::iterator end = d->networks.end();
	NetworkList::iterator it = d->networks.begin();
	for ( ; it != end; ++it )
	{
		Network * net = *it;
		NetworkUsageList usage = net->usage();
		NetworkUsageList::iterator end2 = usage.end();
		for ( NetworkUsageList::iterator usageIt = usage.begin(); usageIt != end2; ++usageIt )
		{
			if ( (*usageIt).appId == appId && (*usageIt).host == host )
			{
				// remove host usage record
				usage.remove( usageIt );
				// if requested shutdown flagged for network
				//  check if all hosts have relinquished
				//   call confirmShutDown on Service
				//checkShutdownOk();
			}
		}
	}
}

bool NetworkStatusModule::reportFailure( const QString & host )
{
	// find network for host
	// check IP record.  remove IP usage record.  if other IP exists, return true.
	Q_UNUSED( host );
	kdDebug() << k_funcinfo << "NOT IMPLEMENTED" << endl;
	return false;
}

// PROTECTED UTILITY FUNCTIONS
/*
 * Determine the network to use for the supplied host
 */
Network * NetworkStatusModule::networkForHost( const QString & host )
{
	// return a null pointer if no networks are registered
	if ( d->networks.isEmpty() )
		return 0;
	
	NetworkList::iterator it = d->networks.begin();
	Network * bestNetwork = *(it++);
	NetworkList::iterator end = d->networks.end();
 	for ( ; it != end; ++it )
	{
		if ( (*it)->reachabilityFor( host ) > bestNetwork->reachabilityFor( host ) )
		{
			bestNetwork = (*it);
		}
	}
	return bestNetwork;
}


void NetworkStatusModule::registeredToDCOP( const QCString & appId )
{
}

void NetworkStatusModule::unregisteredFromDCOP( const QCString & appId )
{
	// unregister any networks owned by a service that has just unregistered
	NetworkList::iterator it = d->networks.begin();
	NetworkList::iterator end = d->networks.end();
	for ( ; it != end; ++it )
	{
		if ( (*it)->service() == appId)
		{
			kdDebug() << k_funcinfo << "removing '" << (*it)->name() << "', registered by " << appId << endl;
			d->networks.remove( it );
			break;
		}
	}
}

// SERVICE INTERFACE //
void NetworkStatusModule::setNetworkStatus( const QString & networkName, int st )
{
	kdDebug() << k_funcinfo << endl;
	NetworkStatus::EnumStatus status = (NetworkStatus::EnumStatus)st;
	Network * net = 0;
	NetworkList::iterator it = d->networks.begin();
	NetworkList::iterator end = d->networks.end();
	for ( ; it != end; ++it )
	{
		if ( (*it)->name() == networkName )
		{
			net = (*it);
			break;
		}
	}
	if ( net )
	{
		if ( net->status() == status )
			return;

		// update the status of the network
		net->setStatus( status );

		// notify for each host in use on that network
		NetworkUsageList usage = net->usage();
		NetworkUsageList::iterator end = usage.end();
		QStringList notified;
		for ( NetworkUsageList::iterator it = usage.begin(); it != end; ++it )
		{
			// only notify once per host
			if ( !notified.contains( (*it).host ) )
			{
				kdDebug() << "notifying statusChange of " << networkName << " to " << (int)status << 
						" because " << (*it).appId << " is using " << (*it).host << endl;
				/*d->clientIface->*/statusChange( (*it).host, (int)status );
				notified.append( (*it).host );
			}
		}

		// if we are now anything but Establishing or Online, reset the usage records for that network
		if ( !( net->status() == NetworkStatus::Establishing || net->status() == NetworkStatus::Establishing ) )
			net->removeAllUsage();
	}
	else
		kdDebug() << k_funcinfo << "No network found by this name" << endl;
}

void NetworkStatusModule::registerNetwork( const QString & networkName, const NetworkStatus::Properties properties )
{
	kdDebug() << k_funcinfo << "registering '" << networkName << "', with status " << properties.status << endl;
	// TODO: check for re-registration, checking appid matches
	
	d->networks.append( new Network( networkName, properties ) );
}

void NetworkStatusModule::unregisterNetwork( const QString & networkName )
{
	// TODO: check appid
	//d->networks.remove( networkName );
}

void NetworkStatusModule::requestShutdown( const QString & networkName )
{
	Q_UNUSED( networkName );
	kdDebug() << k_funcinfo << "NOT IMPLEMENTED" << endl;
}

#include "networkstatus.moc"
