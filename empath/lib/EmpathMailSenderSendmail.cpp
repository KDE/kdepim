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
#include "EmpathMailSenderSendmail.h"
#include "EmpathConfig.h"
#include "Empath.h"

EmpathMailSenderSendmail::EmpathMailSenderSendmail()
	:	error_(false)
{
	QObject::connect (&sendmailProcess_, SIGNAL(processExited(KProcess *)),
			this, SLOT(sendmailExited(KProcess *)));

	QObject::connect (&sendmailProcess_,
			SIGNAL(receivedStderr(KProcess *, char *, int)),
			this,
			SLOT(sendmailReceivedStderr(KProcess *, char *, int)));

	QObject::connect (&sendmailProcess_, SIGNAL(wroteStdin(KProcess *)),
			this, SLOT(wroteStdin(KProcess *)));
}

EmpathMailSenderSendmail::~EmpathMailSenderSendmail()
{
}

	void
EmpathMailSenderSendmail::setSendmailLocation(const QString & location)
{
	sendmailLocation_ = location;
}

	bool
EmpathMailSenderSendmail::sendOne(const RMessage & message)
{
	empathDebug("sendOne() called");

	error_ = false;

	empathDebug("Message text:");
	messageAsString_ = message.asString();
	empathDebug(messageAsString_);

	sendmailProcess_.clearArguments();

	KConfig * c = kapp->getConfig();
	c->setGroup(GROUP_SENDING);

	QString sendmailLocation = c->readEntry(KEY_SENDMAIL_LOCATION);
	
	if (sendmailLocation.isEmpty()) {
		empathDebug("No location configured for sendmail");
		return false;
	}

	empathDebug("sendmail location is" + sendmailLocation);

	sendmailProcess_ << QString(c->readEntry(KEY_SENDMAIL_LOCATION));
	sendmailProcess_ << "-t";
	sendmailProcess_ << "-oem";
	sendmailProcess_ << "-oi";

//  Not necessary with -t flag to sendmail
//	sendmailProcess_ << message.recipientListAsPlainString();

	empathDebug("Starting sendmail process");
	if (!sendmailProcess_.start(KProcess::NotifyOnExit, KProcess::All)) {
		empathDebug("Couldn't start sendmail process");
		return false;
	}

	empathDebug("Starting piping message to sendmail process");
	
	// Start at first byte of message
	messagePos_ = 0;
	wroteStdin(&sendmailProcess_);

	while (!written_ && !error_) {
		kapp->processEvents();
	}

	if (error_) {
		empathDebug("Error: " + errorStr_);
		return false;
	}

	return true;
}

	bool
EmpathMailSenderSendmail::send(EmpathMessageList & messageList)
{
	bool status = true;
	
	EmpathMessageListIterator it(messageList);
	
	for (; it.current(); ++it)
		if (!sendOne(*(it.current()))) status = false;
	
	return status;
}

	void
EmpathMailSenderSendmail::wroteStdin(KProcess * p)
{
	empathDebug("wroteStdin() called");

	if (messagePos_ >=  messageAsString_.length()) {
		empathDebug("messagePos has reached message length");
		sendmailProcess_.closeStdin();
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

	// Remember the current pos in the message string
	messagePos_ += blockSize;
	
	sendmailProcess_.writeStdin((char *)s.data(), s.length());
}

	void
EmpathMailSenderSendmail::sendmailExited(KProcess * p)
{
	empathDebug("Sendmail exited");

	error_ = (	!sendmailProcess_.normalExit() ||
				sendmailProcess_.exitStatus() != 0);
	
	if (error_) errorStr_ = "sendmail exited abnormally";
	
	written_ = !error_;
	
	messageAsString_ = "";
}

	void
EmpathMailSenderSendmail::sendmailReceivedStderr(
		KProcess *, char * buf, int buflen)
{
	QString eatBuf;
	
	eatBuf = buf + '\n';
	
	// eat
	empathDebug("Process send stderr:");
	empathDebug(eatBuf);
}

	void
EmpathMailSenderSendmail::saveConfig()
{
	KConfig * c = kapp->getConfig();
	c->setGroup(GROUP_SENDING);
	c->writeEntry(KEY_SENDMAIL_LOCATION, sendmailLocation_);
}

	void
EmpathMailSenderSendmail::readConfig()
{
	KConfig * c = kapp->getConfig();
	c->setGroup(GROUP_SENDING);
	sendmailLocation_ =
		c->readEntry(KEY_SENDMAIL_LOCATION, "/usr/lib/sendmail");
}

