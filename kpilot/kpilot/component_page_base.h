#ifndef COMPONENT_PAGE_BASE_H
#define COMPONENT_PAGE_BASE_H
/* component_page_base.h			KPilot
**
** Copyright (C) 2007 Bertjan Broeksema <b.broeksema@kdemail.net>
**
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <QtGui/QWidget>

class ComponentPageBase : public QWidget
{

public:
	ComponentPageBase( QWidget *parent ) : QWidget( parent ) {};
	
	~ComponentPageBase() {};
	
public slots:
	/**
	 * Called when the component is shown in kpilot. It should load everything it
	 * needs and populate the widgets.
	 */
	virtual void showPage() = 0;
	
	/**
	 * Called when the component is hidden in kpilot. It shoul clean up to save
	 * memory.
	 */
	virtual void hidePage() = 0;
};

#endif
