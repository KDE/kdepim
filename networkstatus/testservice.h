#ifndef _TEST_NETWORKSTATUS_SERVICE_H
#define _TEST_NETWORKSTATUS_SERVICE_H

#include "networkstatuscommon.h"
#include "provideriface.h"

class ServiceIface_stub;

class TestService : virtual public QObject, ProviderIface
{
Q_OBJECT
public:
	TestService();
	virtual ~TestService();
	int status( const QString & network );
	int establish( const QString & network );
	int shutdown( const QString & network );
	void simulateFailure();
	void simulateDisconnect();
protected slots:
	void slotStatusChange();
private:
	ServiceIface_stub * m_service;
	NetworkStatus::EnumStatus m_status;
	NetworkStatus::EnumStatus m_nextStatus;
};

#endif
