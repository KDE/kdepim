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

#include <kconfig.h>
#include <kapp.h>

#include "EmpathConfig.h"
#include "EmpathMailSender.h"
#include "EmpathFolder.h"
#include "EmpathURL.h"
#include "EmpathIndex.h"
#include "Empath.h"

EmpathMailSender::EmpathMailSender()
	:	QObject()
{
	empathDebug("ctor");
}

EmpathMailSender::~EmpathMailSender()
{
	empathDebug("dtor");
}

	bool
EmpathMailSender::sendQueued()
{
	KConfig * c(kapp->getConfig());
	c->setGroup(EmpathConfig::GROUP_SENDING);
	
	EmpathURL queueURL(c->readEntry(EmpathConfig::KEY_QUEUE_FOLDER));
	
	EmpathFolder *queueFolder(empath->folder(queueURL));
	
	if (queueFolder == 0) {
		empathDebug("Couldn't send messages - couldn't find queue folder !");
		return false;
	}
	
	QList<EmpathURL> messageList;
	
	EmpathURL url;
	
	bool status = true;

	EmpathIndexIterator it(queueFolder->messageList());
	
	for (; it.current(); ++it) {
		url = queueFolder->url();
		url.setMessageID(it.current()->id());
		RMessage * m(empath->message(url));
		if (m == 0) return false;
		RMessage message(*m);
		if (!sendOne(message)) status = false;
	}
	
	return status;
}

