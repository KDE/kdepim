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
# pragma implementation "EmpathMainWidget.h"
#endif

// Qt includes
#include <qheader.h>

// KDE includes
#include <kconfig.h>
#include <kglobal.h>

// Local includes
#include "EmpathConfig.h"
#include "EmpathMainWidget.h"
#include "EmpathLeftSideWidget.h"
#include "EmpathMessageViewWidget.h"
#include "EmpathMessageListWidget.h"

EmpathMainWidget::EmpathMainWidget(QWidget * parent, const char * name)
	: QWidget(parent, name)
{
	empathDebug("ctor");
	
	hSplit = new KNewPanner(this, "hSplit", KNewPanner::Vertical);
	
	CHECK_PTR(hSplit);
	
	vSplit = new KNewPanner(hSplit, "vSplit", KNewPanner::Horizontal);
	CHECK_PTR(vSplit);
		
	messageListWidget_ =
		new EmpathMessageListWidget(vSplit, "messageListWidget");
	CHECK_PTR(messageListWidget_);

	messageViewWidget_ =
		new EmpathMessageViewWidget(EmpathURL(),
				vSplit, "messageViewWidget");
	CHECK_PTR(messageViewWidget_);

	leftSideWidget_ =
		new EmpathLeftSideWidget(messageListWidget_, hSplit, "leftSideWidget");
	CHECK_PTR(leftSideWidget_);

	empathDebug("Updating message list widget");
	messageListWidget_->update();
	
	QObject::connect(messageListWidget_, SIGNAL(changeView(const EmpathURL &)),
			this, SLOT(s_displayMessage(const EmpathURL &)));
	
	messageListWidget_->setSignalUpdates(true);
	
	vSplit->activate(messageListWidget_, messageViewWidget_);
	hSplit->activate(leftSideWidget_, vSplit);
	
	KConfig * c = KGlobal::config();
	c->setGroup(EmpathConfig::GROUP_DISPLAY);
	
	vSplit->setSeparatorPos(
		c->readNumEntry(EmpathConfig::KEY_MAIN_WIDGET_V_SEP, 30));

	hSplit->setSeparatorPos(
		c->readNumEntry(EmpathConfig::KEY_MAIN_WIDGET_H_SEP, 50));
}

EmpathMainWidget::~EmpathMainWidget()
{
	empathDebug("dtor");
	
	KConfig * c = KGlobal::config();
	c->setGroup(EmpathConfig::GROUP_DISPLAY);
	
	c->writeEntry(
		EmpathConfig::KEY_MAIN_WIDGET_V_SEP, vSplit->separatorPos());
	c->writeEntry(
		EmpathConfig::KEY_MAIN_WIDGET_H_SEP, hSplit->separatorPos());
	c->sync();
}

	void
EmpathMainWidget::resizeEvent(QResizeEvent * e)
{
	resize(e->size());
	hSplit->resize(e->size());
}

	EmpathMessageListWidget *
EmpathMainWidget::messageListWidget()
{
	return messageListWidget_;
}

	EmpathMessageViewWidget *
EmpathMainWidget::messageViewWidget()
{
	return messageViewWidget_;
}

	void
EmpathMainWidget::s_displayMessage(const EmpathURL & url)
{
	messageViewWidget_->s_setMessage(url);
	messageViewWidget_->go();
}

