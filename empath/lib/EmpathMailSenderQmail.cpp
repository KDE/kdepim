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

// Qt includes
#include <qcstring.h>

// KDE includes
#include <kconfig.h>
#include <kapp.h>

// Local includes
#include "EmpathMailSenderQmail.h"
#include "EmpathConfig.h"
#include "Empath.h"

EmpathMailSenderQmail::EmpathMailSenderQmail()
	:	EmpathMailSender(),
		error_(false)
{
	QObject::connect (&qmailProcess_, SIGNAL(processExited(KProcess *)),
			this, SLOT(qmailExited(KProcess *)));

	QObject::connect (&qmailProcess_,
			SIGNAL(receivedStderr(KProcess *, char *, int)),
			this,
			SLOT(qmailReceivedStderr(KProcess *, char *, int)));

	QObject::connect (&qmailProcess_, SIGNAL(wroteStdin(KProcess *)),
			this, SLOT(wroteStdin(KProcess *)));
}

EmpathMailSenderQmail::~EmpathMailSenderQmail()
{
}

	void
EmpathMailSenderQmail::setQmailLocation(const QString & location)
{
	qmailLocation_ = location;
}

	bool
EmpathMailSenderQmail::sendOne(RMessage & message)
{
	empathDebug("sendOne() called");
	
	error_ = false;

	empathDebug("Message text:");
	messageAsString_ = message.asString();
	empathDebug(messageAsString_);
	
	qmailProcess_.clearArguments();

	KConfig * c = kapp->getConfig();
	c->setGroup(EmpathConfig::GROUP_SENDING);
	
	empathDebug("qmail location is" +
		QString(c->readEntry(EmpathConfig::KEY_QMAIL_LOCATION)));

	qmailProcess_ << c->readEntry(EmpathConfig::KEY_QMAIL_LOCATION);

	empathDebug("Starting qmail process");
	if (!qmailProcess_.start(KProcess::NotifyOnExit, KProcess::All)) {
		empathDebug("Couldn't start qmail process");
		return false;
	}
	
	empathDebug("Starting piping message to qmail process");

	messagePos_ = 0;
	wroteStdin(&qmailProcess_);

	while (!written_ && !error_) {
		kapp->processEvents();	
	}

	if (error_) {
		empathDebug("Error !" + errorStr_);
		return false;
	}

	return true;
}

	void
EmpathMailSenderQmail::wroteStdin(KProcess * p)
{
	empathDebug("wroteStdin() called");

	if (messagePos_ >=  messageAsString_.length()) {
		empathDebug("messagePos has reached message length");
		qmailProcess_.closeStdin();
		written_ = true;
		return;
	}

	empathDebug("message pos = " + QString().setNum(messagePos_));

	int blockSize =
		messagePos_ + 1024 > messageAsString_.length() ?
			messageAsString_.length() - messagePos_ : 1024;

	QCString s = messageAsString_.mid(messagePos_, blockSize);

	kapp->processEvents();
	
	empathDebug("Writing \"" + s + "\" to process");
	
	messagePos_ += blockSize;

	qmailProcess_.writeStdin((char *)s.data(), s.length());
}

	void
EmpathMailSenderQmail::qmailExited(KProcess * p)
{
	empathDebug("qmail exited");
	
	error_ = (!qmailProcess_.normalExit() || qmailProcess_.exitStatus() != 0);
	
	if (error_) errorStr_ = "qmail exited abnormally";
	
	written_ = !error_;
	
	messageAsString_ = "";
}

	void
EmpathMailSenderQmail::qmailReceivedStderr(KProcess *, char * buf, int buflen)
{
	QString eatBuf;
	
	eatBuf = buf + '\n';
	
	// eat
	empathDebug("Process send stderr:");
	empathDebug(eatBuf);
}

	void
EmpathMailSenderQmail::saveConfig()
{
	KConfig * c = kapp->getConfig();
	c->setGroup(EmpathConfig::GROUP_SENDING);
	c->writeEntry(EmpathConfig::KEY_QMAIL_LOCATION, qmailLocation_);
}

	void
EmpathMailSenderQmail::readConfig()
{
	KConfig * c = kapp->getConfig();
	c->setGroup(EmpathConfig::GROUP_SENDING);
	qmailLocation_ =
		c->readEntry(EmpathConfig::KEY_QMAIL_LOCATION, "/var/qmail/bin/qmail-inject");
}

