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
