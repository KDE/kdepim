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
#include <qstring.h>

// Local includes
#include "EmpathMessageListWidget.h"
#include "EmpathMessageListItem.h"
#include "EmpathUIUtils.h"

EmpathMessageListItem::EmpathMessageListItem(
		EmpathMessageListWidget * parent,
		const EmpathIndexRecord & msgDesc)
	:	
		QListViewItem(parent),
		id_			(msgDesc.id()),
		messageID_	(msgDesc.messageID()),
		parentID_	(msgDesc.parentID()),
		subject_	(msgDesc.subject()),
		sender_		(msgDesc.sender()),
		date_		(msgDesc.date()),
		status_		(msgDesc.status()),
		size_		(msgDesc.size())
{
	empathDebug("ctor");
	niceDate_ = msgDesc.niceDate(true);
	_init();
}

EmpathMessageListItem::EmpathMessageListItem(
		EmpathMessageListItem * parent,
		const EmpathIndexRecord & msgDesc)
	:	
		QListViewItem(parent),
		id_			(msgDesc.id()),
		messageID_	(msgDesc.messageID()),
		parentID_	(msgDesc.parentID()),
		subject_	(msgDesc.subject()),
		sender_		(msgDesc.sender()),
		date_		(msgDesc.date()),
		status_		(msgDesc.status()),
		size_		(msgDesc.size())
{
	empathDebug("ctor");
	niceDate_ = msgDesc.niceDate(true);
	_init();
}

EmpathMessageListItem::~EmpathMessageListItem()
{
	empathDebug("dtor");
}

	void
EmpathMessageListItem::_init()
{	
	setText(0, subject_);
	setText(2, niceDate_);
	
	QString sizeStr;
	
	if (size_ < 1024)
		sizeStr.sprintf("%8i bytes", size_);
	else
		if (size_ < 1048576)
			sizeStr.sprintf("%8.2f Kb", size_ / 1024.0);
	else
		sizeStr.sprintf("%8.2f Mb", size_ / 1048576.0);

	setText(4, sizeStr);
	
	if (sender_.phrase().isEmpty())
		setText(1, sender_.asString());
		
	else {

		QString s = sender_.phrase();
		if (s.left(1)	== "\"") s.remove(0, 1);
		if (s.right(1)	== "\"") s.remove(s.length(), 1);
		setText(1, s);
	}
}

	void
EmpathMessageListItem::setup()
{
	empathDebug("setup() called");
	
	widthChanged();
	int ph = pixmap(0) ? pixmap(0)->height() : 0;
	int th = QFontMetrics(empathGeneralFont()).height();
	setHeight(QMAX(ph, th));
}

	QString
EmpathMessageListItem::key(int column, bool b) const
{
	static QString s;
	
	switch (column) {
		
		case 0:
			s = text(0);
			break;
			
		case 1:
			s = text(1);
			break;
			
		case 2:
			if (b)
				s.sprintf("%016ul", date_.asUnixTime());
			else
				s.sprintf("%016l", -date_.asUnixTime());
			break;
			
		case 3:
			s = text(3);
			break;
		
		case 4:
			if (b)
				s.sprintf("%08ul", size_);
			else
				s.sprintf("%08l", -size_);
			break;
			
		default:
			break;
	}
	
	return s;
}

