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
#include <kapp.h>

// Local includes
#include "Empath.h"
#include "EmpathUI.h"
#include "EmpathMainWindow.h"
#include "EmpathComposeWindow.h"
#include "RMM_Message.h"
#include "EmpathConfig.h"
#include "EmpathTipOfTheDay.h"

EmpathUI::EmpathUI()
	: QObject()
{
	empathDebug("ctor");
	
	connect(empath,	SIGNAL(newComposer(ComposeType, const EmpathURL &)),
			this,	SLOT(s_newComposer(ComposeType, const EmpathURL &)));
	
	EmpathMainWindow * mainWindow = new EmpathMainWindow("mainWindow");
	kapp->setMainWidget(mainWindow);
}

EmpathUI::~EmpathUI()
{
	empathDebug("dtor");
}

	void	
EmpathUI::s_newComposer(ComposeType t, const EmpathURL & m)
{
	empathDebug("s_newComposer(" +
		QString().setNum(t) + ", " + m.asString() + ") called");

	EmpathComposeWindow * c = new EmpathComposeWindow(t, m);
	CHECK_PTR(c);
	c->show();
}

	void
EmpathUI::_showTipOfTheDay() const
{
	KConfig * c = kapp->getConfig();

	c->setGroup(EmpathConfig::GROUP_GENERAL);
	
	if (!c->readBoolEntry(EmpathConfig::KEY_TIP_OF_THE_DAY_AT_STARTUP, false)) return;

	EmpathTipOfTheDay totd(
			(QWidget *)0L,
			"tipWidget",
			kapp->kde_datadir() + "/empath/tips/EmpathTips",
			QString::null,
			0);
	totd.exec();
}

