/*  This file is part of kdepim.

    Copyright (C) 2005,2007 Will Stephenson <wstephenson@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef _TEST_NETWORKSTATUS_SERVICE_H
#define _TEST_NETWORKSTATUS_SERVICE_H

#include <KMainWindow>

#include <networkstatuscommon.h>
#include "ui_testserviceview.h"

class OrgKdeSolidNetworkingServiceInterface;

class TestService : public KMainWindow {
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
	void changeComboActivated( int index );
    void serviceOwnerChanged( const QString& service,const QString& oldOwner, const QString& newOwner );

	void changeButtonClicked();

	void slotStatusChange();
private:
    void registerService();
    static QColor toQColor( NetworkStatus::Status );
    OrgKdeSolidNetworkingServiceInterface * m_service;
    NetworkStatus::Status m_status;
    NetworkStatus::Status m_nextStatus;
    Ui_TestServiceView ui;
    QWidget * m_view;
};

#endif
