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
#include "RMM_Token.h"

EmpathURL::EmpathURL()
	:	mailboxName_	(QString::null),
		folderPath_		(QString::null),
		messageID_		(QString::null),
		strRep_			(QString::null)
{
	empathDebug("ctor");
}

EmpathURL::EmpathURL(
		const QString & mailboxName,
		const QString & folderPath,
		const QString & messageID)
	:
		mailboxName_	(mailboxName),
		folderPath_		(folderPath),
		messageID_		(messageID),
		strRep_			(QString::null)
{
	empathDebug("ctor with \"" +
		mailboxName_ + "\", \"" + folderPath_ + "\", \"" + messageID_ + "\"");
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
	
	strRep_.remove(0, 9);
	bool hadTrailingSlash(strRep_.at(0) == '/');
	_cleanUp(strRep_);
	
	int x = strRep_.contains('/') ? 0 : 1;

	if (strRep_.contains('/') == 0) {
		// No slashes, therefore it's just got a mailbox name.
		mailboxName_	= strRep_;
		folderPath_		= QString::null;
		messageID_		= QString::null;
		return;
	}
			
	if (!hadTrailingSlash) {
		// Not a mailbox (because the above didn't match), but has a
		// trailing slash. Therefore it's a mailbox + a folder path.
		unsigned int i	= strRep_.find('/');
		mailboxName_	= strRep_.left(i);
		folderPath_		= strRep_.right(strRep_.length() - i - 1);
		messageID_		= QString::null;
		return;
	}
	
	// Well, it's got a mailbox and a folder path, and the above didn't match,
	// so it's also got a message id.
	unsigned int i	= strRep_.find('/');
	mailboxName_	= strRep_.left(i);
	unsigned int j	= strRep_.findRev('/');
	folderPath_		= strRep_.mid(i + 1, j);
	messageID_		= strRep_.right(strRep_.length() - j);
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
	messageID_		= url.messageID_;
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
	
	return (*this == url);
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
	empathDebug("Assembled to \"" + strRep_ + "\"");
}

	QStrList
EmpathURL::folderPathList()
{
	QStrList sl_f;
	RTokenise(folderPath_.ascii(), "/", sl_f);
	return sl_f;
}

	EmpathURL
EmpathURL::withoutMessageID() const
{
	EmpathURL url(mailboxName_, folderPath_, QString::null);
	return url;
}

	void
EmpathURL::_cleanUp(QString & s)
{
	while (s.find("//") != -1)
	s.replace(QRegExp("//"), "/");
	if (s.at(0) == '/') s.remove(0, 1);
	if (s.at(s.length() - 1) == '/') s.remove(s.length() - 1, 1);
}

	QString
EmpathURL::mailboxName() const
{
	return _stripSlashes(mailboxName_);
}

	QString
EmpathURL::folderPath() const
{
	return _stripSlashes(folderPath_);
}

	QString
EmpathURL::messageID() const
{
	return _stripSlashes(messageID_);
}

	QString
EmpathURL::_stripSlashes(const QString & s) const
{
	QString stripped(s);
	while (stripped.at(0) == '/')
		stripped.remove(0, 1);
	
	while (stripped.at(s.length()) == '/')
		stripped.remove(s.length() - 1, 1);
	
	return stripped;
}

