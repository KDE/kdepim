#ifndef NETWORKSTATUS_CLIENTIFACEIMPL_H
#define NETWORKSTATUS_CLIENTIFACEIMPL_H

#include <qstring.h>

#include "clientiface.h"
#include "networkstatus.h"

class ClientIfaceImpl : virtual public ClientIface
{
public:
	ClientIfaceImpl( NetworkStatusModule * module );
	int status( QString host );
	int request( QString host, bool userInitiated );
	void relinquish( QString host );
	bool reportFailure( QString host );
/*	QString statusAsString();*/
private:
	NetworkStatusModule * m_module;
};

#endif
