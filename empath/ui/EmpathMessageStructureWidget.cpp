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

// KDE includes
#include <klocale.h>
#include <kapp.h>

// Local includes
#include "EmpathMessageStructureWidget.h"
#include "EmpathMessageStructureItem.h"
#include "EmpathUIUtils.h"
#include <RMM_Enum.h>

EmpathMessageStructureWidget::EmpathMessageStructureWidget
	(QWidget * parent, const char * name)
	:	QListView(parent, name)
{
	empathDebug("ctor");
	setCaption(i18n("Message Structure - ") + kapp->getCaption());
	
	addColumn(i18n("Type"));
	addColumn(i18n("Subtype"));
	addColumn(i18n("Size"));
	
	setAllColumnsShowFocus(true);
	setRootIsDecorated(true);
	setSorting(-1); // Don't sort this.

	QObject::connect(this, SIGNAL(currentChanged(QListViewItem *)),
			this, SLOT(s_currentChanged(QListViewItem *)));
	
	QObject::connect(
		this,
		SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int)),
		this,
		SLOT(s_rightButtonPressed(QListViewItem *, const QPoint &, int)));
	
}

	void
EmpathMessageStructureWidget::setMessage(RBodyPart & m)
{
	clear();
	
	EmpathMessageStructureItem * i = new EmpathMessageStructureItem(this, m);
	CHECK_PTR(i);
	
	QListIterator<RBodyPart> it(m.body());
	
	for (; it.current(); ++it) {
		
		EmpathMessageStructureItem * j =
			new EmpathMessageStructureItem(i, *(it.current()));
		CHECK_PTR(j);

		_addChildren(it.current(), j);
	}
}

	void
EmpathMessageStructureWidget::_addChildren(RBodyPart * p, QListViewItem * i)
{
	QListIterator<RBodyPart> it(p->body());
	
	for (; it.current(); ++it) {

		EmpathMessageStructureItem * j =
			new EmpathMessageStructureItem((EmpathMessageStructureItem *)i, *p);

		CHECK_PTR(j);

		_addChildren(it.current(), j);
	}
}

	void
EmpathMessageStructureWidget::s_currentChanged(QListViewItem * item)
{
	empathDebug("s_currentChanged() called");
	EmpathMessageStructureItem * i = (EmpathMessageStructureItem *)item;
	emit(partChanged(i->part()));
}

