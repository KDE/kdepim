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
#include <qfont.h>
#include <qstring.h>
#include <qfontmetrics.h>

// Local includes
#include "EmpathFolderListItem.h"
#include "EmpathFolder.h"
#include "EmpathMailbox.h"
#include "EmpathDefines.h"
#include "EmpathUIUtils.h"

EmpathFolderListItem::EmpathFolderListItem(
		QListViewItem * parent,
		const EmpathFolder & _folder)
	:
		QListViewItem(parent),
		type_(Folder),
		folder_((EmpathFolder *)&_folder),
		mailbox_(0)
{
	empathDebug("ctor with folder");
	
	setText(0, _folder.name());
	empathDebug("1");
	setText(1, QString().setNum(_folder.unreadMessageCount()));
	empathDebug("2");
	setText(2, QString().setNum(_folder.messageCount()));
	empathDebug("3");
	
	if (_folder.pixmap().isNull()) {
		empathDebug("folder's pixmap is 0");
	}
	empathDebug("4");
	setPixmap(0, _folder.pixmap());
	empathDebug("5");
	connect(folder_, SIGNAL(countUpdated(int, int)),
		this, SLOT(s_setCount(int, int)));
	empathDebug("6");
}
	
EmpathFolderListItem::EmpathFolderListItem(
		QListView * parent,
		const EmpathMailbox & _mailbox)
	:
		QListViewItem(parent),
		type_(Mailbox),
		folder_(0),
		mailbox_((EmpathMailbox *)&_mailbox)
{
	empathDebug("ctor with mailbox");

	setText(0, _mailbox.name());
	empathDebug("1");
	setText(1, QString().setNum(_mailbox.unreadMessageCount()));
	empathDebug("2");
	setText(2, QString().setNum(_mailbox.messageCount()));
	empathDebug("3");
	setPixmap(0, _mailbox.pixmap());
	connect(mailbox_, SIGNAL(countUpdated(int, int)),
		this, SLOT(s_setCount(int, int)));
}

EmpathFolderListItem::~EmpathFolderListItem()
{
	empathDebug("dtor");
}

#if QT_VERSION >= 200
	QString
#else
	const char *
#endif
EmpathFolderListItem::key(int column, bool) const
{
	QString tmpString;
	if (type_ == Folder)
		tmpString.sprintf("%08i", folder_->id());
	else
		tmpString.sprintf("%08i", mailbox_->id());
	return tmpString;
}

	void
EmpathFolderListItem::setup()
{
	empathDebug("setup() called");
	
	widthChanged();
	if (pixmap(0)->isNull()) return;
	int ph = pixmap(0)->height();
	int th = QFontMetrics(empathGeneralFont()).height();
	setHeight(QMAX(ph, th) + 8);
}

	EmpathFolder &
EmpathFolderListItem::folder() const
{
	return *folder_;
}

	EmpathMailbox &
EmpathFolderListItem::mailbox() const
{
	return *mailbox_;
}

	void
EmpathFolderListItem::s_setCount(int unread, int read)
{
	setText(1, QString().setNum(unread));
	setText(2, QString().setNum(read));
}

