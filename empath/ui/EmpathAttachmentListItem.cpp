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

#ifdef __GNUG__
# pragma implementation "EmpathAttachmentListItem.h"
#endif

#ifdef __GNUG__
# pragma implementation ""
#endif

// Qt includes
#include <qpixmap.h>

// KDE includes
#include <kglobal.h>

// Local includes
#include "EmpathAttachmentListItem.h"
#include "EmpathUIUtils.h"
#include "EmpathDefines.h"

EmpathAttachmentListItem::EmpathAttachmentListItem(
		QListView * parent,
		const EmpathAttachmentSpec & s)
	:	QListViewItem(parent),
		spec_(s)
{
	empathDebug("ctor");
	
	setText(0, spec_.filename());
}

EmpathAttachmentListItem::~EmpathAttachmentListItem()	
{	
	empathDebug("dtor");
}
		
	void
EmpathAttachmentListItem::setup()	
{	
	widthChanged();
	
	int th = QFontMetrics(KGlobal::generalFont()).height();
	
	if (!pixmap(0))
		setHeight(th);
	else 
		setHeight(QMAX(pixmap(0)->height(), th));

}
		
	QString
EmpathAttachmentListItem::key(int, bool) const
{
	return text(0);
}

	void
EmpathAttachmentListItem::setSpec(const EmpathAttachmentSpec & s)
{
	spec_ = s;
	setText(0, spec_.filename());
}

