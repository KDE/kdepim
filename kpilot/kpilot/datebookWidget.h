#ifndef _KPILOT_DATEBOOKWIDGET_H
#define _KPILOT_DATEBOOKWIDGET_H
/* datebookWidget.h		KPilot
**
** Copyright (C) 2003 by Dan Pilone.
**	Authored by Adriaan de Groot
**
** This is the viewer widget for viewing datebook entries in
** a marginally useful form.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "pilotComponent.h"

class KDatePicker;
class QPushButton;
class KListView;

class DatebookWidget : public PilotComponent
{
Q_OBJECT

public:
	DatebookWidget(QWidget* parent, const QString& dbpath);
	virtual ~DatebookWidget();

	// Pilot component methods
	/* virtual */ void initialize();

protected slots:
	void slotDayChanged();
	void slotAddEvent();
	void slotEditEvent();
	void slotDeleteEvent();

private:
	KDatePicker*fDatePicker;
	QPushButton*fAddButton;
	QPushButton*fEditButton;
	QPushButton*fDeleteButton;
	KListView*fEventList;
};


#endif
