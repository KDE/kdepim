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
#include <qpixmap.h>

// Local includes
#include "EmpathUIUtils.h"
#include "EmpathDefines.h"
#include "EmpathTaskWidget.h"

EmpathTaskWidget * EmpathTaskWidget::taskWidget_ = 0;

	void
EmpathTaskWidget::addTask(EmpathTask * t)
{
	if (taskWidget_ == 0) {
		taskWidget_ = new EmpathTaskWidget();
		CHECK_PTR(taskWidget_);
	}
	
	taskWidget_->_addTask(t);
}

EmpathTaskWidget::EmpathTaskWidget()
	:	QWidget()
{
	empathDebug("ctor");
}

EmpathTaskWidget::~EmpathTaskWidget()
{
	empathDebug("dtor");
}

	void
EmpathTaskWidget::resizeEvent(QResizeEvent * e)
{
}

	void
EmpathTaskWidget::_addTask(EmpathTask * t)
{
	empathDebug("_addTask(" + t->name() + ") called");
}

