#ifndef NETWORKSTATUS_SERVICEIFACEIMPL_H
#define NETWORKSTATUS_SERVICEIFACEIMPL_H

#include "networkstatus.h"
#include "serviceiface.h"

/**
 * Glue class linking DCOP skeleton to daemon
 */
class ServiceIfaceImpl : virtual public ServiceIface
{
public:
	ServiceIfaceImpl( NetworkStatusModule * module );
	void setStatus( QString networkName, int status );
	void registerNetwork( QString networkName, NetworkStatus::Properties properties );
	void unregisterNetwork( QString networkName );
	void requestShutdown( QString networkName );
private:
	NetworkStatusModule * m_module;
};

#endif
