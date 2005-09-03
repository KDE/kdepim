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

#ifndef NETWORKSTATUS_NETWORK_H
#define NETWORKSTATUS_NETWORK_H

#include <qstringlist.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <Q3CString>
#include <ksharedptr.h>
#include "networkstatuscommon.h"

struct NetworkUsageStruct
{
	Q3CString appId;
	QString host;
};

typedef Q3ValueList< NetworkUsageStruct > NetworkUsageList;

class Network
{
public:
	Network( const QString name, NetworkStatus::Properties properties );
	Network( const Network & other );
	NetworkStatus::EnumStatus reachabilityFor( const QString & host );
	void registerUsage( const Q3CString appId, const QString host );
	void unregisterUsage( const Q3CString appId, const QString host );
	void setStatus( NetworkStatus::EnumStatus status );

	void removeAllUsage();

	NetworkStatus::EnumStatus status() { return m_status; }
	QString name() { return m_name; }
	bool internet() { return m_internet; }
	QStringList netmasks() { return m_netmasks; }
	Q3CString service() { return m_service; }
	NetworkStatus::EnumOnDemandPolicy onDemandPolicy() { return m_onDemandPolicy; }
	NetworkUsageList usage() { return m_usage; }
	
private:
	NetworkStatus::EnumStatus m_status;
	QString m_name;
	bool m_internet;
	QStringList m_netmasks;
	Q3CString m_service;
	NetworkStatus::EnumOnDemandPolicy m_onDemandPolicy;
	NetworkUsageList m_usage;
};

#endif
