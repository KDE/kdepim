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
#include <qregexp.h>

// KDE includes
#include <klocale.h>

// Local includes
#include "EmpathURL.h"

EmpathURL::EmpathURL()
	:	mailboxName_(""),
		folderPath_(""),
		messageID_(""),
		strRep_("")
{
	empathDebug("ctor");
}

EmpathURL::EmpathURL(
		const QString & mailboxName,
		const QString & folderPath,
		const QString & messageID)
	:
		mailboxName_(mailboxName),
		folderPath_(folderPath),
		messageID_(messageID),
		strRep_("")
{
	empathDebug("ctor with \"" + QString(mailboxName) +
		"\", \"" + QString(folderPath) + "\"");
	_assemble();
}

EmpathURL::EmpathURL(const QString & fullPath)
	:	strRep_(fullPath)
{
	empathDebug("ctor with \"" + QString(fullPath) + "\"");
	_parse();
	_assemble();
}


EmpathURL::EmpathURL(const EmpathURL & url)
	:	mailboxName_(url.mailboxName_),
		folderPath_(url.folderPath_),
		messageID_(url.messageID_)
{
	empathDebug("copy ctor");
	_assemble();
}

	void
EmpathURL::_parse()
{
	if (strRep_.left(9) != "empath://") {
		isValid_ = false;
		return;
	}
	
	QString s = strRep_.right(strRep_.length() - 9);
	s.replace(QRegExp("//"), "/");
	
	unsigned int slashes = s.contains('/');
	
	// Case 1: No slashes, therefore it's just got a mailbox name.
	if (slashes == 0) {
		mailboxName_ = s;
		folderPath_	= "";
		messageID_ = "";
		return;
	}
	
	// Case 2: Only one slash. Just a mailbox name again,
	// ignore the trailing slash.
	if (slashes == 1) {
		mailboxName_ = s.left(s.length() - 1);
		folderPath_	= "";
		messageID_ = "";
		return;
	}
	
	// Case 3: Not a mailbox (because the above didn't match), but has a
	// trailing slash. Therefore it's a mailbox + a folder path.
	if (s.right(1) == '/') {
		unsigned int i = s.find('/');
		mailboxName_ = s.left(i);
		folderPath_ = s.right(s.length() - i);
		messageID_ = "";
		return;
	}
	
	// Case 4: We have a mailbox, a folder path, and a message id.
	unsigned int i = s.find('/');
	mailboxName_ = s.left(i);
	unsigned int j = s.findRev('/');
	folderPath_ = s.mid(i + 1, j);
	messageID_ = s.right(s.length() - j);
}

EmpathURL::~EmpathURL()
{
	empathDebug("dtor");
}

	EmpathURL &
EmpathURL::operator = (const EmpathURL & url)
{
	empathDebug("operator = ");
	mailboxName_	= url.mailboxName_;
	folderPath_		= url.folderPath_;
	_assemble();
	return *this;
}

	EmpathURL &
EmpathURL::operator = (const QString & url)
{
	empathDebug("operator = \"" + url + "\"");
	strRep_ = url;
	_parse();
	_assemble();
	return *this;
}

	bool
EmpathURL::operator == (const EmpathURL & b) const
{
	empathDebug("operator ==");
	return (
			mailboxName_	== b.mailboxName_	&&
			folderPath_		== b.folderPath_	&&
			messageID_		== b.messageID_);
}

	bool
EmpathURL::operator == (const QString & s) const
{
	empathDebug("operator ==");
	EmpathURL url(s);
	
	return (*this == s);
}


	void
EmpathURL::setMailboxName(const QString & mailboxName)
{
	mailboxName_ = mailboxName;
	_assemble();
}

	void
EmpathURL::setFolderPath(const QString & folderPath)
{
	folderPath_ = folderPath;
	_assemble();
}

	void
EmpathURL::setMessageID(const QString & messageID)
{
	messageID_ = messageID;
	_assemble();
}

	void
EmpathURL::_assemble()
{
	QString s = mailboxName_ + "/" + folderPath_ + "/" + messageID_;
	s.replace(QRegExp("//"), "/");
	strRep_ = "empath://" + s;
}

	QStringList
EmpathURL::folderPathList()
{
	QStringList sl_f;
	unsigned int startPos = 0;
	int i = folderPath_.find('/');
	
	while (i != -1) {
		sl_f.append(folderPath_.mid(startPos, i));
		startPos = i;
		i = folderPath_.find('/', startPos + 1);
	}
	
	if (startPos < folderPath_.length()) {
		if (folderPath_[startPos] == '/') ++startPos;
		sl_f.append(folderPath_.mid(startPos, folderPath_.length()));
	}
	
	return sl_f;
}

	EmpathURL
EmpathURL::withoutMessageID() const
{
	EmpathURL url(mailboxName_, folderPath_, "");
	return url;
}

