#ifndef KDED_NETWORKSTATUS_SERVICEIFACE_H
#define KDED_NETWORKSTATUS_SERVICEIFACE_H

#include "networkstatuscommon.h"

#include <dcopobject.h>

class ServiceIface : virtual public DCOPObject
{
K_DCOP
k_dcop:
	/** Change the status for the given network */
	virtual void setNetworkStatus( const QString & networkName, int status ) = 0;
	/** Register or update the properties for a network 
	NB Check that people don't use this to change status */
	virtual void registerNetwork( const QString & networkName, NetworkStatus::Properties properties ) = 0;
	/**
	 * Indicate that this service is no longer administering the named network
	 * TODO: Work out the implications to clients of unregistering a network 
	 * - maybe this method needs more parameters to give them a clue. 
	 */
	virtual void unregisterNetwork( const QString & networkName ) = 0;
	/**
	 * Tell the daemon that the service would like to shut down this network connection,
	 * and to notify clients using it so they can stop using it in a controlled manner
	 */
	virtual void requestShutdown( const QString & networkName ) = 0;
};

#endif
