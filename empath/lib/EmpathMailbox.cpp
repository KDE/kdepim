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

// Local includes
#include "EmpathUtilities.h"
#include "EmpathFolder.h"
#include "EmpathMailbox.h"
#include "EmpathURL.h"
#include "Empath.h"

EmpathMailbox::EmpathMailbox(const QString & name)
	:	name_(name)
{
	empathDebug("ctor");
	timer_ = new QTimer(this, "mailboxTimer");
	CHECK_PTR(timer_);
//	pixmap_ = empathIcon("mailbox.xpm");
	folderList_.setAutoDelete(true);
	QObject::connect(this, SIGNAL(updateFolderLists()),
		empath, SLOT(s_updateFolderLists()));
}

EmpathMailbox::~EmpathMailbox()
{
	empathDebug("dtor");
}

	void
EmpathMailbox::setID(Q_UINT32 id)
{
	id_ = id;
	canonName_ = "Mailbox_" + QString().setNum(id);
}

	bool
EmpathMailbox::newMailReady() const
{
	return (newMessagesCount_ != 0);
}

	Q_UINT32
EmpathMailbox::newMails() const
{
	return newMessagesCount_;
}

	void
EmpathMailbox::setCheckMail(bool yn)
{
	empathDebug(QString("Setting check mail to ") + (yn ? "true" : "false"));
	checkMail_ = yn;
	if (checkMail_) {
		empathDebug("Switching on timer");
		timer_->stop();
		timer_->start(checkMailInterval_ * 60000);
	}
	else {
		empathDebug("Switching off timer");
		timer_->stop();
	}
}

	void
EmpathMailbox::setCheckMailInterval(Q_UINT32 checkMailInterval)
{
	empathDebug("Setting timer interval to  " +
			QString().setNum(checkMailInterval));
	checkMailInterval_ = checkMailInterval;
	if (checkMail_) {
		timer_->stop();
		timer_->start(checkMailInterval_ * 60000);
	}
}

	bool
EmpathMailbox::checkMail() const
{
	return checkMail_;
}

	Q_UINT32
EmpathMailbox::checkMailInterval() const
{
	return checkMailInterval_;
}

	Q_UINT32
EmpathMailbox::id() const
{
	return id_;
}

	void
EmpathMailbox::setName(const QString & name) 
{
	name_ = name.data();
	empathDebug("Mailbox: setting name to " + name_);
}

	QString
EmpathMailbox::location() const
{
	return location_;
}
	
	AccountType
EmpathMailbox::type() const
{
	return type_;
}

	bool
EmpathMailbox::usesTimer() const
{
	return checkMail_;
}

	Q_UINT32
EmpathMailbox::timerInterval() const
{
	return checkMailInterval_;
}

	QString
EmpathMailbox::path() const
{
	return EmpathURL(name_).asString();
}

	const EmpathFolderList &
EmpathMailbox::folderList() const
{
	return folderList_;
}

	Q_UINT32
EmpathMailbox::messageCount() const
{
	empathDebug("messageCount() called");

	Q_UINT32 c = 0;
	
	EmpathFolderListIterator it(folderList_);
	
	for (; it.current(); ++it)
		c += it.current()->messageCount();

	return c;
}

	Q_UINT32
EmpathMailbox::unreadMessageCount() const
{
	empathDebug("unreadMessageCount() called");

	Q_UINT32 c = 0;
	
	EmpathFolderListIterator it(folderList_);
	
	for (; it.current(); ++it)
		c += it.current()->messageCount();

	return c;
}

	const QPixmap &
EmpathMailbox::pixmap() const
{
	return pixmap_;
}

	void
EmpathMailbox::update(EmpathFolder * f)
{
	empathDebug("update(" + f->name() + ") called");
	emit(countUpdated((int)unreadMessageCount(), (int)messageCount()));
}

	void
EmpathMailbox::s_countUpdated(EmpathFolder * f, int unread, int read)
{
	emit(countUpdated((int)unreadMessageCount(), (int)messageCount()));
}

	const EmpathFolder *
EmpathMailbox::folder(const QString & folderPath)
{
	empathDebug("folder(" + folderPath + ") called");
	EmpathFolderListIterator it(folderList_);
	
	for (; it.current(); ++it)
		if (it.current()->url().folderPath() == folderPath)
			return it.current();
	
	return 0;
}

