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

EmpathTaskWidget::EmpathTaskWidget(QWidget * parent, const char * name)
	:	QWidget(parent, name)
{
	empathDebug("ctor");
	
	tickLabel_ = new QLabel(this, "tickLabel");
	CHECK_PTR(tickLabel_);
	
	tickLabel_->setPixmap(empathIcon("tick.xpm"));
	
	stopButton_ = new QPushButton(this, "stopButton");
	CHECK_PTR(stopButton_);
	
	stopButton_->setPixmap(empathIcon("stop.xpm"));
	
	hide();
}

EmpathTaskWidget::~EmpathTaskWidget()
{
	empathDebug("dtor");
}

	void
EmpathTaskWidget::manage(QWidget * w)
{
	statusWidget_ = w;
	resizeEvent((QResizeEvent *)0);
	QObject::connect(statusWidget_, SIGNAL(done()),
		this, SLOT(s_done()));
	show();
}

	void
EmpathTaskWidget::resizeEvent(QResizeEvent * e)
{
	statusWidget_->move(0,0);
	statusWidget_->resize(width() - 100, height());
	tickLabel_->move(width() - 50, (height() / 2) - (tickLabel_->height() / 2));
	stopButton_->move(width() - 50, (height() / 2) - (stopButton_->height() / 2));
}

	void
EmpathTaskWidget::s_done()
{
	stopButton_->setEnabled(false);
	stopButton_->hide();
}

