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

#include <qtimer.h>
#include <dcopclient.h>
#include <kapplication.h>
#include "provideriface.h"

#include "testservice.h"
#include "serviceiface_stub.h"

TestService::TestService() : QObject(), DCOPObject("ProviderIface")
{
	kapp->dcopClient()->registerAs("testservice" );
	m_service = new ServiceIface_stub( "kded", "networkstatus" );
	m_status = NetworkStatus::Offline;
	NetworkStatus::Properties nsp;
	nsp.internet = true;
	nsp.name = "test_net";
	nsp.onDemandPolicy = NetworkStatus::All;
	nsp.service = kapp->dcopClient()->appId();
	nsp.status = m_status;
	m_service->registerNetwork( "test_net", nsp );
}

TestService::~TestService()
{
	delete m_service;
}

int TestService::status( const QString & network )
{
	Q_UNUSED( network );
	return (int)m_status;
}

int TestService::establish( const QString & network )
{
	Q_UNUSED( network );
	m_status = NetworkStatus::Establishing;
	m_service->setNetworkStatus( "test_net", (int)m_status );
	m_nextStatus = NetworkStatus::Online;
	QTimer::singleShot( 5000, this, SLOT( slotStatusChange() ) );
	return (int)NetworkStatus::RequestAccepted;
}

int TestService::shutdown( const QString & network )
{
	Q_UNUSED( network );
	m_status = NetworkStatus::ShuttingDown;
	m_service->setNetworkStatus( "test_net", (int)m_status );
	m_nextStatus = NetworkStatus::Offline;
	QTimer::singleShot( 5000, this, SLOT( slotStatusChange() ) );
	return (int)NetworkStatus::RequestAccepted;
}

void TestService::simulateFailure()
{
	m_status = NetworkStatus::OfflineFailed;
	m_service->setNetworkStatus( "test_net", (int)m_status );
}

void TestService::simulateDisconnect()
{
	m_status = NetworkStatus::OfflineDisconnected;
	m_service->setNetworkStatus( "test_net", (int)m_status );
}

void TestService::slotStatusChange()
{
	m_status = m_nextStatus;
	m_service->setNetworkStatus( "test_net", (int)m_status );
}

int main( int argc, char** argv )
{
	KApplication app(argc, argv, "testdcop");
	TestService * test = new TestService;
	Q_UNUSED( test );
	return app.exec();
}

#include "testservice.moc"
