/* main-test.cc                         KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This is the main program for kpilotTest, which shows a SyncLog and
** exercises the KPilotDeviceLink class. It's intended to test if the
** Palm hardware and the KPilot software are functioning correctly to
** some extent.
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
** Bug reports and questions can be sent to kde-pim@kde.org.
*/
static const char *test_id =
	"$Id$";

#include "options.h"

#include <stdlib.h>
#include <time.h>

#include <iostream.h>

#include <kapp.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include "logWidget.h"
#include "kpilotConfig.h"

#include "hotSync.h"
#include "interactiveSync.h"


static KCmdLineOptions kpilotoptions[] = {
	{"port <device>",
		I18N_NOOP("Path to Pilot device node"),
		"/dev/pilot"},
	{"test", I18N_NOOP("List DBs (default)"), 0},
	{"backup", I18N_NOOP("Backup instead of list DBs"), 0},
	{"restore", I18N_NOOP("Restore Pilot from backup"), 0},
	{0, 0, 0}
};

int main(int argc, char **argv)
{
	FUNCTIONSETUP;
	KAboutData about("kpilotTest",
		I18N_NOOP("KPilotTest"),
		KPILOT_VERSION,
		"KPilot Tester",
		KAboutData::License_GPL, "(C) 2001, Adriaan de Groot");
	about.addAuthor("Adriaan de Groot",
		I18N_NOOP("KPilot Maintainer"),
		"groot@kde.org", "http://www.cs.kun.nl/~adridg/kpilot/");

	KCmdLineArgs::init(argc, argv, &about);
#ifdef DEBUG
	KCmdLineArgs::addCmdLineOptions(debug_options, "debug", "debug");
#endif
	KCmdLineArgs::addCmdLineOptions(kpilotoptions, "kpilottest", 0L,
		"debug");
	KApplication::addCmdLineOptions();

	KCmdLineArgs *p = KCmdLineArgs::parsedArgs();

	QString devicePath = p->getOption("port");

	if (devicePath.isEmpty())
	{
		devicePath = "/dev/pilot";
	}

	KPilotDeviceLink::DeviceType deviceType =
		KPilotDeviceLink::OldStyleUSB;

	KApplication a;

	KPilotConfig::getDebugLevel(p);


	LogWidget *w = new LogWidget(0L);

	w->resize(300, 300);
	w->show();
	w->setShowTime(true);
	a.setMainWidget(w);

	KPilotDeviceLink *t = KPilotDeviceLink::init(0, "deviceLink");
	SyncAction *head = 0L;
	SyncAction *tail = 0L;

	if (p->isSet("backup"))
	{
		head = tail = new BackupAction(t);
	}
	else if (p->isSet("restore"))
	{
		head = new CheckUser(t, w);
		SyncAction *l = new RestoreAction(t, w);

		tail = new CleanupAction(t);

		QObject::connect(head, SIGNAL(syncDone(SyncAction *)),
			l, SLOT(exec()));
		QObject::connect(l, SIGNAL(syncDone(SyncAction *)),
			tail, SLOT(exec()));
	}
	else
	{
		head = tail = new TestLink(t);
	}

	QObject::connect(t, SIGNAL(logError(const QString &)),
		w, SLOT(addError(const QString &)));
	QObject::connect(t, SIGNAL(logMessage(const QString &)),
		w, SLOT(addMessage(const QString &)));
	QObject::connect(t, SIGNAL(deviceReady()), head, SLOT(exec()));
	QObject::connect(tail, SIGNAL(syncDone(SyncAction *)),
		w, SLOT(syncDone()));
	QObject::connect(tail, SIGNAL(syncDone(SyncAction *)),
		t, SLOT(close()));

	t->reset(deviceType, devicePath);

	return a.exec();

	/* NOTREACHED */
	(void) test_id;
}

// $Log$
// Revision 1.7  2001/09/24 22:22:49  adridg
// Modified to handle new interactive SyncActions
//
// Revision 1.6  2001/09/24 10:43:19  cschumac
// Compile fixes.
//
// Revision 1.5  2001/09/23 21:42:35  adridg
// Factored out debugging options
//
// Revision 1.4  2001/09/23 18:28:52  adridg
// Adjusted tests to new .ui and config
//
// Revision 1.3  2001/09/16 13:37:48  adridg
// Large-scale restructuring
//
// Revision 1.2  2001/09/06 22:05:00  adridg
// Enforce singleton-ness
//
// Revision 1.1  2001/09/05 21:53:51  adridg
// Major cleanup and architectural changes. New applications kpilotTest
// and kpilotConfig are not installed by default but can be used to test
// the codebase. Note that nothing else will actually compile right now.
//
