#include "serviceifaceimpl.h"

ServiceIfaceImpl::ServiceIfaceImpl( NetworkStatusModule * module ) : m_module ( module )
{
}

void ServiceIfaceImpl::setStatus( QString networkName, int status )
{
	m_module->setStatus( networkName, (NetworkStatus::EnumStatus)status );
}

void ServiceIfaceImpl::registerNetwork( QString networkName, NetworkStatus::Properties properties )
{
	m_module->registerNetwork( networkName, properties );
}

void ServiceIfaceImpl::unregisterNetwork( QString networkName )
{
	m_module->unregisterNetwork( networkName );
}

void ServiceIfaceImpl::requestShutdown( QString networkName )
{
	m_module->requestShutdown( networkName );
}
