/* systray.cc                   KitchenSync
**
** Copyright (C) 2001 by Adriaan de Groot
**
** This program is part of KitchenSync, the KDE handheld-device
** synchronisation framework.
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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
*/
 
/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/

#include <config.h>
#include "../lib/debug.h"

#include <kglobal.h>
#include <kiconloader.h>
#include <kwin.h>
#include <kapp.h>
#include <kpopupmenu.h>
#include <kconfig.h>
#include <kservicetype.h>

#include "configdialog.h"
#include "syncManager.h"
#include "logWindow.h"

#include "systray.moc"



SystrayWindow::SystrayWindow(Config *c,SyncManager *m) :
	KSystemTray(0,"kitchensync"),
	fConfig(c),
	fManager(m),
	fLog(0L),
	fConfigDialog(0L)
{
	FUNCTIONSETUP;

	KGlobal::iconLoader()->addAppDir("kitchensync");

	fIdleIcon = KGlobal::iconLoader()->loadIcon("hotsync",
		KIcon::Toolbar,
		0,KIcon::DefaultState,
		0, true);

	fBusyIcon = KGlobal::iconLoader()->loadIcon("busysync",
		KIcon::Toolbar,
		0,KIcon::DefaultState,
		0, true);

	setIcon(Idle);

	KPopupMenu *menu = contextMenu();

	fToggleStatusWindowId = menu->insertItem(
		i18n("&Show status window"),
		this,SLOT(toggleStatusWindow()));
	int id = menu->insertItem(
		SmallIcon("configure"),
		i18n("&Configure..."));

	addDaemons();

	menu->connectItem(id,this,SLOT(showConfigDialog()));

	if (c->logWindowShown())
	{
		createLogWindow();
	}
}

void SystrayWindow::addDaemons()
{
	FUNCTIONSETUP;

	KService::List daemons = KServiceType::offers("SyncDaemon");

	DEBUGMANAGER << "There are " << daemons.count() << " daemons." << endl;

	KService::List::ConstIterator i;

	for (i=daemons.begin(); i!=daemons.end(); ++i)
	{
		DEBUGMANAGER << (*i)->name() << endl;
	}
}

void SystrayWindow::setIcon(Icon i)
{
	FUNCTIONSETUP;

	switch(i)
	{
	case Busy :
		setPixmap(fBusyIcon);
		break;
	case Idle :
		setPixmap(fIdleIcon);
		break;
	}
}

void SystrayWindow::showTray()
{
	FUNCTIONSETUP;

	KWin::setSystemTrayWindowFor(winId(),0);
	setGeometry(-100,-100,42,42);
	show();
}


/* virtual */ void SystrayWindow::closeEvent(QCloseEvent *)
{
	FUNCTIONSETUP;

	if (fLog) fLog->saveYourself(fConfig);

	kapp->quit();
}

void SystrayWindow::toggleStatusWindow()
{
	FUNCTIONSETUP;

	KPopupMenu *menu=contextMenu();
	bool b=!menu->isItemChecked(fToggleStatusWindowId);

	menu->setItemChecked(fToggleStatusWindowId,b);

	if (b)
	{
		if (fLog) fLog->show();
		else createLogWindow();
	}
	else
	{
		if (fLog) fLog->hide();
	}
}

void SystrayWindow::statusWindowHidden()
{
	FUNCTIONSETUP;

	contextMenu()->setItemChecked(fToggleStatusWindowId,false);
}

void SystrayWindow::statusWindowRevealed()
{
	FUNCTIONSETUP;

	contextMenu()->setItemChecked(fToggleStatusWindowId,true);
}

void SystrayWindow::createLogWindow()
{
	FUNCTIONSETUP;

	fLog = new LogWindow(0L,fConfig);

	QObject::connect(fManager,SIGNAL(syncProgress(int)),
		fLog,SLOT(setValue(int)));
	QObject::connect(fManager,SIGNAL(syncMessage(const QString &)),
		fLog,SLOT(setMessage(const QString &)));
	QObject::connect(fManager,SIGNAL(syncLog(const QString &)),
		fLog,SLOT(appendLog(const QString &)));
	QObject::connect(this,SIGNAL(configChanged(Config *)),
		fLog,SLOT(configChanged(Config *)));

	fLog->show();
	statusWindowRevealed();
}

void SystrayWindow::showConfigDialog()
{
	fConfigDialog = new ConfigDialog(0L,fConfig);

	fConfigDialog->show();
	QObject::connect(fConfigDialog,SIGNAL(okClicked()),
		this,SLOT(configChanged()));
	QObject::connect(fConfigDialog,SIGNAL(cancelClicked()),
		this,SLOT(configClosed()));
	QObject::connect(fConfigDialog,SIGNAL(closeClicked()),
		this,SLOT(configClosed()));
}

void SystrayWindow::configChanged()
{
	FUNCTIONSETUP;

	fConfigDialog->save(fConfig);

	emit configChanged(fConfig);

	configClosed();
}

void SystrayWindow::configClosed()
{
	FUNCTIONSETUP;

	fConfigDialog->delayedDestruct();
	fConfigDialog=0L;
}

void SystrayWindow::startSync(const QString &s)
{
	setIcon(Busy);
	if (fLog)
	{
		fLog->setCaption(s);
		fLog->setMessage(i18n("%1 connected...").arg(s));
	}
}

void SystrayWindow::endSync(const QString &s)
{
	setIcon(Idle);
	if (fLog)
	{
		fLog->setCaption(QString::null);
		fLog->setMessage(i18n("%1 done.").arg(s));
	}
}

// $Log$
// Revision 1.1.1.1  2001/06/21 19:50:19  adridg
// KitchenSync is the next-gen KDE-PIM Handheld Device Synchronization
// Framework, which aims to integrate all the Handheld sync tools in 
// KDE, such as KPilot and Kandy. (This is the *real* import).
//
