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
		EmpathIndexRecord * msgDesc)
	:	
		QListViewItem(parent),
		msgDesc_(msgDesc)
{
	empathDebug("ctor");
	
	setText(0, msgDesc_->subject());
	setText(2, msgDesc_->niceDate(true)); // XXX HARDCODED true
	
	int sz = msgDesc_->size();
	QString sizeStr;
	
	if (sz < 1024)
		sizeStr.sprintf("%8i bytes", sz);
	else if (sz < 1048576)
		sizeStr.sprintf("%8.2f Kb", sz / 1024.0);
	else
		sizeStr.sprintf("%8.2f Mb", sz / 1048576.0);

	setText(4, sizeStr);
	
	if (!msgDesc_->sender().phrase().isEmpty()) {
		
		QString s = msgDesc_->sender().phrase();
		if (s.at(0) == '"') s.remove(0, 1);
		if (s.at(s.length()) == '"') s.remove(s.length(), 1);
		setText(1, s);
		
	} else
		setText(1, msgDesc_->sender().asString());
}

EmpathMessageListItem::EmpathMessageListItem(
		EmpathMessageListItem * parent,
		EmpathIndexRecord * msgDesc)
	:	
		QListViewItem(parent),
		msgDesc_(msgDesc)
{
	empathDebug("ctor");
	
	setText(0, msgDesc_->subject());
	setText(2, msgDesc_->niceDate(true)); // XXX HARDCODED true
	
	int sz = msgDesc_->size();
	QString sizeStr;
	
	if (sz < 1024)
		sizeStr.sprintf("%8i bytes", sz);
	else if (sz < 1048576)
		sizeStr.sprintf("%8.2f Kb", sz / 1024.0);
	else
		sizeStr.sprintf("%8.2f Mb", sz / 1048576.0);

	setText(4, sizeStr);
	
	if (!msgDesc_->sender().phrase().isEmpty()) {
		
		QString s = msgDesc_->sender().phrase();
		if (s.at(0) == '"') s.remove(0, 1);
		if (s.at(s.length()) == '"') s.remove(s.length(), 1);
		setText(1, s);
		
	} else
		setText(1, msgDesc_->sender().asString());
}

EmpathMessageListItem::~EmpathMessageListItem()
{
	empathDebug("dtor");
}

	QString
EmpathMessageListItem::size() const
{
	QString s;
	s.setNum(msgDesc_->size());
	return s;
}

	void
EmpathMessageListItem::setup()
{
	empathDebug("setup() called");
	
	widthChanged();
	int ph = pixmap(0) ? pixmap(0)->height() : 0;
	int th = QFontMetrics(empathGeneralFont()).height();
	setHeight(QMAX(ph, th) + 4);
}

#if QT_VERSION >= 200
	QString
#else
	const char *
#endif
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
				s.sprintf("%016ul", msgDesc_->date().asUnixTime());
			else
				s.sprintf("%016l", -msgDesc_->date().asUnixTime());
			break;
			
		case 3:
			s = text(3);
			break;
		
		case 4:
			if (b)
				s.sprintf("%08ul", msgDesc_->size());
			else
				s.sprintf("%08l", -msgDesc_->size());
			break;
			
		default:
			break;
	}
	
	return s;
}

