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

// KDE includes
#include <kapp.h>

// Local includes
#include "EmpathUIUtils.h"
#include "EmpathFilterListItem.h"
#include "EmpathFilter.h"
#include "EmpathDefines.h"

EmpathFilterListItem::EmpathFilterListItem(
		QListView * parent,
		EmpathFilter * _filter)
	:
		QListViewItem(parent, _filter->name()),
		filter_(_filter)
{
	empathDebug("ctor");
	setPixmap(0, empathIcon("filter.png"));
}

EmpathFilterListItem::~EmpathFilterListItem()
{
}

	QString
EmpathFilterListItem::key(int column, bool) const
{
	QString tmpString;
	tmpString.sprintf("%08x", filter_->priority());
	return tmpString;
}

	void
EmpathFilterListItem::setup()
{
	empathDebug("setup() called");
	
	widthChanged();
	int ph = pixmap(0)->height();
	int th = QFontMetrics(kapp->generalFont()).height();
	setHeight((ph > th ? ph : th) + 8);
}

	EmpathFilter *
EmpathFilterListItem::filter() const
{
	return filter_;
}

