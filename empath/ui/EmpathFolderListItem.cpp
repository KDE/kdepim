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
#include <qpixmap.h>

// Local includes
#include "EmpathFolderListItem.h"
#include "EmpathFolder.h"
#include "EmpathMailbox.h"
#include "EmpathDefines.h"
#include "EmpathUIUtils.h"
#include "Empath.h"

EmpathFolderListItem::EmpathFolderListItem(
		QListView * parent,
		const EmpathURL & url)
	:
		QListViewItem(parent),
		url_(url)
{
	empathDebug("ctor with mailbox");

	EmpathMailbox * m(empath->mailbox(url_));
	
	if (m == 0) {
		empathDebug("Can't find the mailbox !!!!");
		return;
	}
	
	setText(0, m->name());
	setText(1, QString().setNum(m->unreadMessageCount()));
	setText(2, QString().setNum(m->messageCount()));
	setPixmap(0, empathIcon(m->pixmapName()));
	connect(m, SIGNAL(countUpdated(int, int)),
		this, SLOT(s_setCount(int, int)));
}

EmpathFolderListItem::EmpathFolderListItem(
		QListViewItem * parent,
		const EmpathURL & url)
	:
		QListViewItem(parent),
		url_(url)
{
	empathDebug("ctor with folder \"" + url_.folderPath() + "\"");

	EmpathFolder * f(empath->folder(url_));
	
	if (f == 0) {
		empathDebug("Can't find the folder !!!!");
		return;
	}

	QString s = url_.folderPath();
	if (s.right(1) == "/")
		s = s.remove(s.length(), 1);
	s = s.right(s.length() - s.findRev("/") - 1);
	
	setText(0, s);
	setText(1, QString().setNum(f->unreadMessageCount()));
	setText(2, QString().setNum(f->messageCount()));
	setPixmap(0, empathIcon(f->pixmapName()));
	connect(f, SIGNAL(countUpdated(int, int)),
		this, SLOT(s_setCount(int, int)));
}
	
EmpathFolderListItem::~EmpathFolderListItem()
{
	empathDebug("dtor");
}

	QString
EmpathFolderListItem::key(int column, bool) const
{
	QString tmpString;
	
	if (url_.hasFolder()) {
	
		EmpathFolder * f(empath->folder(url_));
		
		if (f != 0)
			tmpString.sprintf("%08i", f->id());
	
	} else {
	
		EmpathMailbox * m(empath->mailbox(url_));
		
		if (m != 0)
			tmpString.sprintf("%08i", m->id());
	}

	return tmpString;
}

	void
EmpathFolderListItem::setup()
{
	empathDebug("setup() called");
	
	widthChanged();
	
	int th = QFontMetrics(empathGeneralFont()).height();
	
	if (!pixmap(0))
		setHeight(th);
	else 
		setHeight(QMAX(pixmap(0)->height(), th));
}

	void
EmpathFolderListItem::s_setCount(int unread, int read)
{
	setText(1, QString().setNum(unread));
	setText(2, QString().setNum(read));
}

