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
