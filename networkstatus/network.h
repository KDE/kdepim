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

#ifndef NETWORKSTATUS_NETWORK_H
#define NETWORKSTATUS_NETWORK_H

#include <qstringlist.h>
#include <ksharedptr.h>
#include "networkstatuscommon.h"

struct NetworkUsageStruct
{
	QCString appId;
	QString host;
};

typedef QValueList< NetworkUsageStruct > NetworkUsageList;

class Network
{
public:
	Network( const QString name, NetworkStatus::Properties properties );
	Network( const Network & other );
	NetworkStatus::EnumStatus reachabilityFor( const QString & host );
	void registerUsage( const QCString appId, const QString host );
	void unregisterUsage( const QCString appId, const QString host );
	void setStatus( NetworkStatus::EnumStatus status );

	void removeAllUsage();

	NetworkStatus::EnumStatus status() { return m_status; }
	QString name() { return m_name; }
	bool internet() { return m_internet; }
	QStringList netmasks() { return m_netmasks; }
	QCString service() { return m_service; }
	NetworkStatus::EnumOnDemandPolicy onDemandPolicy() { return m_onDemandPolicy; }
	NetworkUsageList usage() { return m_usage; }
	
private:
	NetworkStatus::EnumStatus m_status;
	QString m_name;
	bool m_internet;
	QStringList m_netmasks;
	QCString m_service;
	NetworkStatus::EnumOnDemandPolicy m_onDemandPolicy;
	NetworkUsageList m_usage;
};

#endif
