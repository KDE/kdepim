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
#include "Empath.h"
#include "EmpathFolderList.h"
#include "EmpathFolder.h"
#include "EmpathDefines.h"
#include "EmpathMailbox.h"
#include "EmpathIndex.h"
#include "EmpathUtilities.h"

uID EmpathFolder::ID = 0;

EmpathFolder::EmpathFolder()
	:	QObject(),
		messageCount_(0),
		unreadMessageCount_(0)
{
	empathDebug("default ctor !");
	id_ = ID++;
	pixmapName_ = "mini-folder-grey.xpm";
}

EmpathFolder::EmpathFolder(const EmpathURL & url)
	:	QObject(),
		messageCount_(0),
		unreadMessageCount_(0),
		url_(url)
{
	empathDebug("ctor with url == \"" + url_.asString() + "\"");
	messageList_.setFolder(this);
	id_ = ID++;
	QObject::connect(this, SIGNAL(countUpdated(int, int)),
		empath->mailbox(url_), SLOT(s_countUpdated(int, int)));
	pixmapName_ = "mini-folder-grey.xpm";
}

	bool
EmpathFolder::operator == (const EmpathFolder & other) const
{
	empathDebug("operator == () called");
	return id_ == other.id_;
}

EmpathFolder::~EmpathFolder()
{
	empathDebug("dtor");
}

	void
EmpathFolder::setPixmap(const QString & p)
{
	pixmapName_ = p;
}

	bool
EmpathFolder::removeMessage(const EmpathURL & id)
{
	EmpathMailbox * m = empath->mailbox(url_);
	return (m != 0 && m->removeMessage(id));
}

	const EmpathIndexRecord *
EmpathFolder::messageDescription(const RMessageID & id) const
{
	empathDebug("messageWithID(" + id.asString() + ") called");
	return messageList_.messageDescription(id);
}

	RMessage *
EmpathFolder::message(const EmpathURL & url)
{
	EmpathMailbox * m = empath->mailbox(url_);
	if (m == 0) return 0;
	return m->message(url);
}

	void
EmpathFolder::update()
{
	empathDebug("update() called");
	EmpathMailbox * m = empath->mailbox(url_);
	if (m == 0) return;
	empathDebug("mailbox name = " + m->name());
	m->syncIndex(url_);
	emit(countUpdated(messageList_.countUnread(), messageList_.count()));
}

	bool
EmpathFolder::writeMessage(const RMessage & message)
{
	EmpathMailbox * m = empath->mailbox(url_);
	return (m != 0 && m->writeMessage(url_, message));
}

	EmpathFolder *
EmpathFolder::parent() const
{
	empathDebug("parent() called");
	QString f = url_.folderPath();
	QString m = url_.mailboxName();
	empathDebug("My folder path is \"" + f + "\"");
	if (f.right(1) == "/")
		f = f.remove(f.length(), 1);
	if (!f.contains("/")) return 0;
	f = f.left(f.length() - f.findRev("/") + 1);
	f += "/";
	empathDebug("Parent folder path is \"" + f + "\"");
	EmpathURL u(m, f, QString::null);
	return empath->folder(u);
}

