/*
    This file is part of kdepim.

    Copyright (c) 2005 Will Stephenson <lists@stevello.free-online.co.uk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

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
