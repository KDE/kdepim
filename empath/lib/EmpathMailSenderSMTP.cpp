/*
	Empath - Mailer for KDE
	
	Copyright (C) 1998 Rik Hemsley rik@kde.org
	
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
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// KDE includes
#include <kconfig.h>
#include <kapp.h>

// Local includes
#include "EmpathMailSenderSMTP.h"
#include "EmpathConfig.h"
#include "Empath.h"

EmpathMailSenderSMTP::EmpathMailSenderSMTP()
	:	EmpathMailSender()
{
}

EmpathMailSenderSMTP::~EmpathMailSenderSMTP()
{
}

	void
EmpathMailSenderSMTP::setServer(const QString & name, const Q_UINT32 port)
{
	serverName_ = name;
	serverPort_ = port;
}

	bool
EmpathMailSenderSMTP::sendOne(RMessage & message)
{
	return false;
}

	void
EmpathMailSenderSMTP::saveConfig()
{
	KConfig * c = kapp->getConfig();
	c->setGroup(EmpathConfig::GROUP_SENDING);
	c->writeEntry(EmpathConfig::KEY_SMTP_SERVER_LOCATION, serverName_);
	c->writeEntry(EmpathConfig::KEY_SMTP_SERVER_PORT, serverPort_);
}

	void
EmpathMailSenderSMTP::readConfig()
{
	KConfig * c = kapp->getConfig();
	c->setGroup(EmpathConfig::GROUP_SENDING);
	serverName_ = c->readEntry(EmpathConfig::KEY_SMTP_SERVER_LOCATION, "localhost");
	serverPort_ = c->readUnsignedNumEntry(EmpathConfig::KEY_SMTP_SERVER_PORT, 25);
}

