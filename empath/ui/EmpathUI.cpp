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
# pragma implementation "EmpathUI.h"
#endif

// Qt includes
#include <qmessagebox.h>
#include <qstring.h>

// KDE includes
#include <kglobal.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kapp.h>
#include <klocale.h>

// Local includes
#include "Empath.h"
#include "EmpathUI.h"
#include "EmpathMainWindow.h"
#include "EmpathComposeWindow.h"
#include "EmpathDisplaySettingsDialog.h"
#include "EmpathIdentitySettingsDialog.h"
#include "EmpathComposeSettingsDialog.h"
#include "EmpathSendingSettingsDialog.h"
#include "EmpathAccountsSettingsDialog.h"
#include "EmpathFilterManagerDialog.h"
#include "RMM_Message.h"
#include "EmpathConfig.h"
#include "EmpathTipOfTheDay.h"
#include "EmpathTask.h"
#include "EmpathTaskWidget.h"

EmpathUI::EmpathUI()
	: QObject()
{
	empathDebug("ctor");
	
	KConfig * c(KGlobal::config());
	c->setGroup(EmpathConfig::GROUP_DISPLAY);
	
	QString iconSetPath(c->readEntry(EmpathConfig::KEY_ICON_SET, "standard"));

	KGlobal::iconLoader()->insertDirectory(0,
		kapp->kde_datadir() + "/empath/pics/" + iconSetPath);
	KGlobal::iconLoader()->insertDirectory(0,
		kapp->kde_datadir() + "/empath/pics/mime");
	
	QObject::connect(
		empath,	SIGNAL(newComposer(Empath::ComposeType, const EmpathURL &)),
		this,	SLOT(s_newComposer(Empath::ComposeType, const EmpathURL &)));
	
	QObject::connect(
		empath,	SIGNAL(newComposer(const QString &)),
		this,	SLOT(s_newComposer(const QString &)));
	
	QObject::connect(
		empath,	SIGNAL(setupDisplay()),
		this,	SLOT(s_setupDisplay()));
	
	QObject::connect(
		empath,	SIGNAL(setupIdentity()),
		this,	SLOT(s_setupIdentity()));
	
	QObject::connect(
		empath,	SIGNAL(setupSending()),
		this,	SLOT(s_setupSending()));
	
	QObject::connect(
		empath,	SIGNAL(setupComposing()),
		this,	SLOT(s_setupComposing()));
	
	QObject::connect(
		empath,	SIGNAL(setupAccounts()),
		this,	SLOT(s_setupAccounts()));
	
	QObject::connect(
		empath,	SIGNAL(setupFilters()),
		this,	SLOT(s_setupFilters()));
	
	QObject::connect(
		empath,	SIGNAL(about()),
		this,	SLOT(s_about()));
		
	QObject::connect(
		empath,	SIGNAL(bugReport()),
		this,	SLOT(s_bugReport()));
	
	EmpathMainWindow * mainWindow = new EmpathMainWindow("mainWindow");
	kapp->setMainWidget(mainWindow);
}

EmpathUI::~EmpathUI()
{
	empathDebug("dtor");
}

	void	
EmpathUI::s_newComposer(Empath::ComposeType t, const EmpathURL & m)
{
	EmpathComposeWindow * c = new EmpathComposeWindow(t, m);
	CHECK_PTR(c);
	c->show();
}

	void	
EmpathUI::s_newComposer(const QString & recipient)
{
	EmpathComposeWindow * c = new EmpathComposeWindow(recipient);
	CHECK_PTR(c);
	c->show();
}

	void
EmpathUI::_showTipOfTheDay() const
{
	KConfig * c = KGlobal::config();

	c->setGroup(EmpathConfig::GROUP_GENERAL);
	
	if (!c->readBoolEntry(EmpathConfig::KEY_TIP_OF_THE_DAY_AT_STARTUP, false))
		return;

	EmpathTipOfTheDay totd(
			(QWidget *)0L,
			"tipWidget",
			kapp->kde_datadir() + "/empath/tips/EmpathTips",
			QString::null,
			0);
	totd.exec();
}

	void
EmpathUI::s_setupDisplay()
{
	EmpathDisplaySettingsDialog::create();
}
	void
EmpathUI::s_setupIdentity()
{
	EmpathIdentitySettingsDialog::create();
}

	void
EmpathUI::s_setupSending()
{
	EmpathSendingSettingsDialog::create();
}

	void
EmpathUI::s_setupComposing()
{
	EmpathComposeSettingsDialog::create();
}

	void
EmpathUI::s_setupAccounts()
{
	EmpathAccountsSettingsDialog::create();
}

	void
EmpathUI::s_setupFilters()
{
	EmpathFilterManagerDialog::create();
}

	void
EmpathUI::s_about()
{
	static const QString EmpathAbout = QString::fromLatin1(
		"<h3>Empath</h3>"
		"<p>Version %1</p>"
		"<p>A sophisticated mail client for <b>KDE</b></p>"
		"<p>Maintainer contact address: <b>&lt;rik@kde.org&gt;</b></p>"
		"<hr>"
		"<p>Compiled with KDE libraries version: %2</p>"
		"<p>Compiled with Qt library version: %3</p>"
		"<p>Empath is licensed under the <em>GNU Public License</em></p>");

	QString kdeVersion		= QString::fromLatin1(KDE_VERSION_STRING);
	QString qtVersion		= QString::fromLatin1(QT_VERSION_STR);

	QMessageBox::about(0, i18n("About Empath"),
		i18n(EmpathAbout.ascii())
		.arg(EMPATH_VERSION_STRING)
		.arg(kdeVersion)
		.arg(qtVersion));
}

	void
EmpathUI::s_bugReport()
{
	EmpathComposeWindow * c = new EmpathComposeWindow();
	CHECK_PTR(c);
	c->bugReport();
}

