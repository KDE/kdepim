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
