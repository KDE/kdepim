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

#ifndef KDED_NETWORKSTATUS_H
#define KDED_NETWORKSTATUS_H

#include "networkstatuscommon.h"

#include <kdedmodule.h>

#include "clientiface.h"
#include "network.h"
#include "serviceiface.h"

struct NetworkStatusStruct;

class NetworkStatusModule : virtual public KDEDModule/*, public ClientIface, ServiceIface*/ // <-spot the multiple inheritance pb
{
Q_OBJECT
K_DCOP
public:
	NetworkStatusModule( const QCString & obj );
	~NetworkStatusModule();
k_dcop:
	// Client interface
	QStringList networks();
	int status( const QString & host );
	int request( const QString & host, bool userInitiated );
	void relinquish( const QString & host );
	bool reportFailure( const QString & host );
// 	QString statusAsString();
	// Service interface
	void setNetworkStatus( const QString & networkName, int status );
	void registerNetwork( const QString & networkName, NetworkStatus::Properties properties );
	void unregisterNetwork( const QString & networkName );
	void requestShutdown( const QString & networkName );
k_dcop_signals:
	/**
	 * A status change occurred for the network(s) used to connect to the given host.
	 * @param host The host which the application has indicated it is using
	 * @param status The new status of the network used to reach host.
	 */
		void statusChange( QString host, int status );
	/**
	 * The network would like to shut down - any clients using this host are to finish using it immediately and call 
	 * relinquish() when done.
	 * @param host The host, registered as in use by applications, which is about to be disconnected.
	 */
	void shutdownRequested( QString host );
protected slots:
	void registeredToDCOP( const QCString& appId );
	void unregisteredFromDCOP( const QCString& appId );

protected:
	// QStringList networkUsers( const QString & networkName );
	Network * networkForHost( const QString & host );

private:
	class Private;
	Private *d;
};
#endif
