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

#ifdef __GNUG__
# pragma implementation "EmpathMailSender.h"
#endif

// System includes
#include <unistd.h>

// Qt includes
#include <qfile.h>
#include <qfileinfo.h>

// KDE includes
#include <kglobal.h>
#include <kconfig.h>
#include <kmsgbox.h>
#include <klocale.h>

// Local includes
#include "EmpathConfig.h"
#include "EmpathMailSender.h"
#include "EmpathFolder.h"
#include "EmpathURL.h"
#include "EmpathIndex.h"
#include "EmpathTask.h"
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
EmpathMailSender::send(RMM::RMessage & message)
{
	EmpathTask * t(empath->addTask("Sending message"));
	empath->s_infoMessage(i18n("Sending message"));

	KConfig * c(KGlobal::config());
	c->setGroup(EmpathConfig::GROUP_SENDING);
	
	EmpathURL queueURL(c->readEntry(EmpathConfig::KEY_QUEUE_FOLDER));
	EmpathURL sentURL(c->readEntry(EmpathConfig::KEY_SENT_FOLDER));
	
	EmpathFolder * queueFolder(empath->folder(queueURL));
	EmpathFolder * sentFolder(empath->folder(sentURL));
	
	if (queueFolder == 0) {
		empathDebug("Couldn't queue message - couldn't find queue folder !");
		empathDebug("Queue folder was specified as: \"" + queueURL.asString() + "\"");
		KMsgBox::message(0, "Empath",
			i18n("Couldn't queue message ! Writing backup"),
			KMsgBox::EXCLAMATION, i18n("OK"));
		emergencyBackup(message);
		t->done();
		empath->s_infoMessage(i18n("Unable to send message"));
		return false;
	}
	
	QString id(queueFolder->writeMessage(message));
	
	if (id.isNull()) {
		empathDebug("Couldn't queue message - folder won't accept !");
		KMsgBox::message(0, "Empath",
			i18n("Couldn't queue message ! Writing backup"),
			KMsgBox::EXCLAMATION, i18n("OK"));
		emergencyBackup(message);
		t->done();
		empath->s_infoMessage(i18n("Unable to send message"));
		return false;
	}

	bool sentOK = sendOne(message);
	
	if (!sentOK) {
		empathDebug("Couldn't send message !");
		emergencyBackup(message);
		t->done();
		empath->s_infoMessage(i18n("Unable to send message"));
		return false;
	}
	
	id = sentFolder->writeMessage(message);

	if (id.isNull()) {
		empathDebug("Couldn't write message to sent folder !");
		emergencyBackup(message);
		t->done();
		empath->s_infoMessage(i18n("Unable to send message"));
		return false;
	}

	EmpathURL q(queueURL);
	q.setMessageID(id);
	
	if (!queueFolder->removeMessage(q)) {
		empathDebug("Couldn't remove message from queue folder !");
		emergencyBackup(message);
		t->done();
		empath->s_infoMessage(i18n("Unable to send message"));
		return false;
	}
	
	t->done();
	empath->s_infoMessage(i18n("Message sent successfully"));

	return true;
}

	void
EmpathMailSender::sendQueued()
{
	KConfig * c(KGlobal::config());
	c->setGroup(EmpathConfig::GROUP_SENDING);
	
	EmpathURL queueURL(c->readEntry(EmpathConfig::KEY_QUEUE_FOLDER));
	EmpathURL sentURL(c->readEntry(EmpathConfig::KEY_SENT_FOLDER));
	
	EmpathFolder * queueFolder(empath->folder(queueURL));
	EmpathFolder * sentFolder(empath->folder(sentURL));
	
	if (queueFolder == 0) {
		empathDebug("Couldn't send messages - couldn't find queue folder !");
		return;
	}
	
	if (sentFolder == 0) {
		empathDebug("Couldn't send messages - couldn't find sent folder !");
		return;
	}
	
	QList<EmpathURL> messageList;
	
	EmpathURL url;
	
	bool status = true;
	
	EmpathTask * t(empath->addTask("Sending messages"));
	t->setMax(queueFolder->messageList().count());

	EmpathIndexIterator it(queueFolder->messageList());
	
	for (; it.current(); ++it) {
		
		url = queueFolder->url();
		url.setMessageID(it.current()->id());
		
		RMM::RMessage * m(empath->message(url));
		
		if (m == 0) {
			empathDebug("Can't find message \"" + url.asString() + "\"");
			t->done();
			return;
		}
		
		RMM::RMessage message(*m);
		if (!sendOne(message))
			status = false;
		else
			if (!sentFolder->writeMessage(message).isNull())
				queueFolder->removeMessage(url);
		t->doneOne();
	}
	
	t->done();
	
	if (!status) {
		empathDebug("Error sending messages");
	}
}

	void
EmpathMailSender::queue(RMM::RMessage & message)
{
	KConfig * c(KGlobal::config());
	c->setGroup(EmpathConfig::GROUP_SENDING);
	
	EmpathURL queueURL(c->readEntry(EmpathConfig::KEY_QUEUE_FOLDER));
	
	EmpathFolder * queueFolder(empath->folder(queueURL));
	
	if (queueFolder == 0) {
		empathDebug("Couldn't queue message - couldn't find queue folder !");
		KMsgBox::message(0, "Empath",
			i18n("Couldn't queue message ! Writing backup"),
			KMsgBox::EXCLAMATION, i18n("OK"));
		emergencyBackup(message);
		empath->s_infoMessage(i18n("Unable to queue message"));
		return;
	}
	
	if (!queueFolder->writeMessage(message)) {
		empathDebug("Couldn't queue message - folder won't accept !");
		KMsgBox::message(0, "Empath",
			i18n("Couldn't queue message ! Writing backup"),
			KMsgBox::EXCLAMATION, i18n("OK"));
		emergencyBackup(message);
		empath->s_infoMessage(i18n("Unable to queue message"));
		return;
	}
	
	empath->s_infoMessage(i18n("Message queued for later delivery"));
}

	void
EmpathMailSender::emergencyBackup(RMM::RMessage & message)
{
	empathDebug("Couldn't queue message ! Writing to emergency backup");

	QCString tempComposeFilename = "/tmp/ORPHANED_EMPATH_MESSAGE_XXXXXX";
	char * tn = new char[strlen(tempComposeFilename)];
	strcpy(tn, tempComposeFilename);

	empathDebug("tempName = \"" + QCString(tn) + "\"");
	empathDebug("running mkstemp");

	int fd = mkstemp(tn);
	QCString tempName(tn);
	delete [] tn;

	empathDebug("Opening file " + QString(tempName));
	QFile f;

	if (!f.open(IO_WriteOnly, fd)) {
		empathDebug("Couldn't open the temporary file " + tempName);
		empathDebug("EMERGENCY BACKUP COULD NOT BE WRITTEN !");
		empathDebug("PLEASE CONTACT PROGRAM MAINTAINER !");
		KMsgBox::message(0, "Empath",
			i18n("Couldn't write the backup file ! Message has been LOST !"),
			KMsgBox::EXCLAMATION, i18n("OK"));
		empath->s_infoMessage(i18n("Couldn't write backup file !"));
		return;
	}

	QString fileName = QString(tempName);

	QCString text(message.asString());
	f.writeBlock(text.data(), text.length());

	f.flush();
	f.close();
	
	if (f.status() != IO_Ok) {
		empathDebug("Couldn't successfully close the file.");
		empathDebug("EMERGENCY BACKUP COULD NOT BE VERIFIED !");
		empathDebug("PLEASE CONTACT PROGRAM MAINTAINER !");
		KMsgBox::message(0, "Empath",
		i18n("Couldn't write the backup file ! Message may have been LOST !"),
			KMsgBox::EXCLAMATION, i18n("OK"));
		empath->s_infoMessage(i18n("Couldn't write backup file !"));
		return;
	}
	
	KMsgBox::message(0, "Empath",
		i18n("Message backup written to") + " " +
		QString::fromLatin1(tempComposeFilename),
		KMsgBox::INFORMATION, i18n("OK"));
	
	empath->s_infoMessage(
		i18n("Message backup written to:") + " " +
		QString::fromLatin1(tempComposeFilename));
}

