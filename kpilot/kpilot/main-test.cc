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

#include <kapplication.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kservice.h>
#include <kservicetype.h>
#include <kuserprofile.h>

#include <pi-version.h>

#include "logWidget.h"
#include "kpilotConfig.h"

#include "syncStack.h"

static KCmdLineOptions kpilotoptions[] = {
	{"port <device>",
		I18N_NOOP("Path to Pilot device node"),
		"/dev/pilot"},
	{"l",0,0},
	{"list", I18N_NOOP("List DBs (default)"), 0},
	{"b",0,0},
	{"backup", I18N_NOOP("Backup instead of list DBs"), 0},
	{"r",0,0},
	{"restore", I18N_NOOP("Restore Pilot from backup"), 0},
	{"L",0,0},
	{ "conduit-list", I18N_NOOP("List available conduits"), 0},
	{"E",0,0},
	{ "conduit-exec <filename>",
		I18N_NOOP("Run conduit from desktop file <filename>"),
		0 },
	{ "T",0,0},
	{ "notest",
		I18N_NOOP("*Really* run the conduit, not in test mode."),
		0 } ,
	{0, 0, 0}
};


static LogWidget *logWidget = 0L;

void createLogWidget()
{
	LogWidget *w = new LogWidget(0L);

	w->resize(300, 300);
	w->show();
	w->setShowTime(true);
	kapp->setMainWidget(w);
	logWidget = w;
}

static KPilotDeviceLink *deviceLink = 0L;

void createLink()
{
	FUNCTIONSETUP;

	deviceLink = KPilotDeviceLink::init(0, "deviceLink");
}

static SyncStack *syncStack = 0L;

void connectStack()
{
	FUNCTIONSETUP;

	QObject::connect(syncStack, SIGNAL(logError(const QString &)),
		logWidget, SLOT(addError(const QString &)));
	QObject::connect(syncStack, SIGNAL(logMessage(const QString &)),
		logWidget, SLOT(addMessage(const QString &)));
	QObject::connect(syncStack,SIGNAL(logProgress(const QString &,int)),
		logWidget, SLOT(addProgress(const QString &,int)));

	QObject::connect(deviceLink, SIGNAL(deviceReady()), syncStack, SLOT(exec()));
	QObject::connect(syncStack, SIGNAL(syncDone(SyncAction *)),
		logWidget, SLOT(syncDone()));
	QObject::connect(syncStack, SIGNAL(syncDone(SyncAction *)),
		deviceLink, SLOT(close()));
}

void createConnection(KCmdLineArgs *p)
{
	FUNCTIONSETUP;

	QString devicePath = p->getOption("port");

	if (devicePath.isEmpty())
	{
		devicePath = "/dev/pilot";
	}

	KPilotDeviceLink::DeviceType deviceType =
		KPilotDeviceLink::OldStyleUSB;

	deviceLink->reset(deviceType, devicePath);
}

int syncTest(KCmdLineArgs *p)
{
	FUNCTIONSETUP;

	createLogWidget();
	createLink();

	syncStack = new SyncStack(deviceLink,&KPilotConfig::getConfig());

	if (p->isSet("backup"))
	{
		syncStack->prepare(SyncStack::Backup | SyncStack::WithUserCheck);
	}
	else if (p->isSet("restore"))
	{
		syncStack->prepareRestore();
	}
	else
	{
		syncStack->prepare(SyncStack::Test);
	}

	connectStack();
	createConnection(p);
	return kapp->exec();
}

int execConduit(KCmdLineArgs *p)
{
	FUNCTIONSETUP;

	// get --exec-conduit value
	QString s = p->getOption("conduit-exec");
	if (s.isEmpty()) return 1;
	QStringList l;
	l.append(s);

	createLogWidget();
	createLink();

	syncStack = new SyncStack(deviceLink,&KPilotConfig::getConfig(),l);

	if (p->isSet("test"))
	{
		syncStack->prepare(SyncStack::HotSyncMode| SyncStack::TestMode);
	}
	else
	{
		syncStack->prepareSync();
	}

	connectStack();
	createConnection(p);

	return kapp->exec();
}


int listConduits(KCmdLineArgs *p)
{
	FUNCTIONSETUP;

	KServiceTypeProfile::OfferList offers =
		KServiceTypeProfile::offers("KPilotConduit");

	// Now actually fill the two list boxes, just make
	// sure that nothing gets listed in both.
	//
	//
	QValueListIterator < KServiceOffer > availList(offers.begin());
	while (availList != offers.end())
	{
		KSharedPtr < KService > o = (*availList).service();

		cout << o->desktopEntryName() << endl;
		cout << "\t" << o->name()  << endl;
		if (!o->library().isEmpty())
		{
			cout << "\tIn "
				<< o->library()
				<< endl;
		}

		++availList;
	}

	return 0;
}

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


	KApplication a;
#ifdef DEBUG
	debug_level = -1;
#endif

	if (p->isSet("backup") || p->isSet("restore") || p->isSet("list"))
	{
		return syncTest(p);
	}

	if (p->isSet("conduit-list"))
	{
		return listConduits(p);
	}

	if (p->isSet("conduit-exec"))
	{
		return execConduit(p);
	}

	// The default is supposed to be "list"
	return syncTest(p);
	/* NOTREACHED */
	(void) test_id;
}


// $Log$
// Revision 1.16  2002/05/14 22:57:40  adridg
// Merge from _BRANCH
//
// Revision 1.15.2.2  2002/04/13 11:54:43  adridg
// Handle --test mode for conduits properly
//
// Revision 1.15.2.1  2002/04/04 20:28:28  adridg
// Fixing undefined-symbol crash in vcal. Fixed FD leak. Compile fixes
// when using PILOT_VERSION. kpilotTest defaults to list, like the options
// promise. Always do old-style USB sync (also works with serial devices)
// and runs conduits only for HotSync. KPilot now as it should have been
// for the 3.0 release.
//
// Revision 1.15  2002/02/02 11:46:02  adridg
// Abstracting away pilot-link stuff
//
// Revision 1.14  2002/01/31 13:20:09  hollomon
//
// fix some compile/configure errors unde KDE 3.
// The configure.in.in fix was suggested by Waldo Bastian.
//
// Revision 1.13  2002/01/22 22:11:41  danimo
// "link" clashes with a function from unistd/stdlib
//
// Revision 1.12  2001/12/29 15:36:57  adridg
// Sanity checking and simplification
//
// Revision 1.11  2001/12/02 22:05:46  adridg
// Minor tweaks for conduit exec()
//
// Revision 1.10  2001/10/10 22:20:52  adridg
// Added --notest, --exec-conduit
//
// Revision 1.9  2001/10/08 22:20:18  adridg
// Changeover to libkpilot, prepare for lib-based conduits
//
// Revision 1.8  2001/09/29 16:26:18  adridg
// The big layout change
//
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
