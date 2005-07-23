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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

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
