#include "clientifaceimpl.h"

ClientIfaceImpl::ClientIfaceImpl( NetworkStatusModule * module ) : m_module ( module )
{
}

int ClientIfaceImpl::status( QString host )
{
	return m_module->status( host );
}

int ClientIfaceImpl::request( QString host, bool userInitiated )
{
	return m_module->request( host, userInitiated );
}

void ClientIfaceImpl::relinquish( QString host )
{
	m_module->relinquish( host );
}

bool ClientIfaceImpl::reportFailure( QString host )
{
	return m_module->reportFailure( host );
}

// QString ClientIfaceImpl::statusAsString()
// {
// 	
// }
