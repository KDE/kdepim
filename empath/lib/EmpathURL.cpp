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
	_assemble();
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
	_stripSlashes(mailboxName_);
	_stripSlashes(folderPath_);
	_simplifySlashes(folderPath_);
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
	empathDebug("_parse() called");
	
	if (strRep_.left(9) != "empath://" || strRep_.length() < 10) {
		empathDebug("URL is invalid");
		isValid_ = false;
		return;
	}

	int endOfMailboxName = strRep_.find('/', 9);
	
	if (endOfMailboxName == -1) {
		
		// Now assume that we only got a mailbox, no folderPath.
		mailboxName_ = strRep_.right(strRep_.length() - 9);
	
	} else {

		mailboxName_ = strRep_.mid(9, endOfMailboxName - 9);
	}
	
	// Otherwise, there's a folderPath, or just a '/' at the end.

	if (strRep_.length() == (unsigned)endOfMailboxName) {
	
		// There's no folderPath, just a mailbox.
		folderPath_ = "";
	
	} else {
	
		folderPath_ = strRep_.right(strRep_.length() - endOfMailboxName);
	}
	
	_stripSlashes(mailboxName_);
	_stripSlashes(folderPath_);
	_simplifySlashes(folderPath_);
	
	empathDebug("mailbox name is " + mailboxName_);
	empathDebug("folder path is " + folderPath_);
	_assemble();
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
	_stripSlashes(mailboxName_);
	_stripSlashes(folderPath_);
	_simplifySlashes(folderPath_);
	_assemble();
	return *this;
}

	EmpathURL &
EmpathURL::operator = (const QString & url)
{
	empathDebug("operator = \"" + url + "\"");
	strRep_ = url;
	_parse();
	return *this;
}

	bool
EmpathURL::operator == (const EmpathURL & b)
{
	empathDebug("operator ==");
	return (
			mailboxName_	== b.mailboxName_	&&
			folderPath_		== b.folderPath_	&&
			messageID_		== b.messageID_);
}

	bool
EmpathURL::operator == (const QString & s)
{
	empathDebug("operator ==");
	EmpathURL url(s);
	
	return (*this == s);
}


	void
EmpathURL::setMailboxName(const QString & mailboxName)
{
	empathDebug("setMailboxName(" + mailboxName + ") called");
	mailboxName_ = mailboxName;
	_assemble();
}

	void
EmpathURL::setFolderPath(const QString & folderPath)
{
	empathDebug("setFolderPath(" + folderPath + ") called");
	folderPath_ = folderPath;
	_assemble();
}

	QString
EmpathURL::asString() const
{
	empathDebug("asString() called");
	if (mailboxName_ == "local" && folderPath_ == "orphaned")
		return i18n("<No folder selected>");
	return strRep_;
}

	void
EmpathURL::_assemble()
{
	empathDebug("_assemble() called");
	_stripSlashes(mailboxName_);
	_stripSlashes(folderPath_);
	strRep_ = "empath://" + mailboxName_ + "/" + folderPath_;
}

	void
EmpathURL::_stripSlashes(QString & s)
{
	empathDebug("_stripSlashes(" + s + ") called");
	if (s.isEmpty()) return;
	while (s[0] == '/')			s = s.right(s.length() - 1);
	while (s.right(1) == "/")	s = s.left(s.length() - 1);
}

	void
EmpathURL::_simplifySlashes(QString & s)
{
	empathDebug("_simplifySlashes(" + s + ") called");
	s.replace(QRegExp("//"), "/");
}

	QStringList
EmpathURL::folderPathList()
{
	empathDebug("folderPathList() called");
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

