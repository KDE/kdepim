#ifndef NETWORKSTATUS_PROVIDERIFACE_H
#define NETWORKSTATUS_PROVIDERIFACE_H

#include <dcopobject.h>
class ProviderIface : virtual public DCOPObject
{
K_DCOP
k_dcop:
	/** @return NetworkStatus::EnumOnlineStatus */
	virtual int status( const QString & network ) = 0;
	/** @return NetworkStatus::EnumRequestResult */
	virtual int establish( const QString & network ) = 0;
	/** @return NetworkStatus::EnumRequestResult */
	virtual int shutdown( const QString & network ) = 0;
	/** fake a failure - go directly to failed */
	virtual void simulateFailure() = 0;
	/** fake a network disconnect - go directly to offlinedisconnected */
	virtual void simulateDisconnect() = 0;
};

#endif
