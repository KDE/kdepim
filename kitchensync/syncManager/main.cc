/* main.cc                      KitchenSync
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

#include "config.h"
#include "lib/debug.h"

#include <kaboutdata.h>
#include <kuniqueapp.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kprogress.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
 
#include "configdialog.h"
#include "systray.h"
#include "syncManager.h"

int main(int argc, char **argv)
{
	KAboutData about("kitchenSync",
		I18N_NOOP("KitchenSync"),
		"0.1",
		I18N_NOOP("KitchenSync System Tray Application"),
		KAboutData::License_GPL,
		"(C) 2001 Adriaan de Groot");
	KCmdLineArgs::init(argc,argv,&about);
	KCmdLineArgs::addCmdLineOptions(debug_options);
	KUniqueApplication::addCmdLineOptions();

	KCmdLineArgs *p = KCmdLineArgs::parsedArgs();

	debug_level = p->isSet("debug");

	KUniqueApplication a(true,true);

	Config c;

	SyncManager *sm = new SyncManager();

	SystrayWindow *sw = new SystrayWindow(&c,sm);
	sw->showTray();

	QObject::connect(sm,SIGNAL(syncStarted(const QString &)),
		sw,SLOT(startSync(const QString &)));
	QObject::connect(sm,SIGNAL(syncEnded(const QString &)),
		sw,SLOT(endSync(const QString &)));


	return a.exec();
}

// $Log:$
