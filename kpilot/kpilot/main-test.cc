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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org.
*/
static const char *test_id =
	"$Id$";

#include "options.h"

#include <stdlib.h>
#include <time.h>

#include <qpushbutton.h>
#include <qhbox.h>

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
static QPushButton *resetButton = 0L;

void createLogWidget()
{
	LogWidget *w = new LogWidget(0L);

	w->resize(300, 300);
	w->show();
	w->setShowTime(true);
	kapp->setMainWidget(w);
	logWidget = w;

	resetButton = new QPushButton(i18n("Reset"),w->buttonBox());
}

static KPilotDeviceLink *deviceLink = 0L;

void createLink()
{
	FUNCTIONSETUP;

	deviceLink = KPilotDeviceLink::init(0, "deviceLink");

	QObject::connect(deviceLink, SIGNAL(logError(const QString &)),
		logWidget, SLOT(addError(const QString &)));
	QObject::connect(deviceLink, SIGNAL(logMessage(const QString &)),
		logWidget, SLOT(addMessage(const QString &)));
	QObject::connect(deviceLink,SIGNAL(logProgress(const QString &,int)),
		logWidget, SLOT(addProgress(const QString &,int)));
}

static ActionQueue *syncStack = 0L;

void connectStack()
{
	FUNCTIONSETUP;

	QObject::connect(syncStack, SIGNAL(logError(const QString &)),
		logWidget, SLOT(addError(const QString &)));
	QObject::connect(syncStack, SIGNAL(logMessage(const QString &)),
		logWidget, SLOT(addMessage(const QString &)));
	QObject::connect(syncStack,SIGNAL(logProgress(const QString &,int)),
		logWidget, SLOT(addProgress(const QString &,int)));

	QObject::connect(deviceLink, SIGNAL(deviceReady()), syncStack, SLOT(execConduit()));

	QObject::connect(syncStack, SIGNAL(syncDone(SyncAction *)),
		logWidget, SLOT(syncDone()));
	QObject::connect(syncStack, SIGNAL(syncDone(SyncAction *)),
		deviceLink, SLOT(close()));

	QObject::connect(resetButton,SIGNAL(clicked()),deviceLink,SLOT(reset()));
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

	syncStack = new ActionQueue(deviceLink,&KPilotConfig::getConfig());

	if (p->isSet("backup"))
	{
		syncStack->prepare(ActionQueue::Backup | ActionQueue::WithUserCheck);
	}
	else if (p->isSet("restore"))
	{
		syncStack->prepareRestore();
	}
	else
	{
		syncStack->prepare(ActionQueue::Test);
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

	syncStack = new ActionQueue(deviceLink,&KPilotConfig::getConfig(),l);

	if (p->isSet("test"))
	{
		syncStack->prepare(ActionQueue::HotSyncMode| ActionQueue::TestMode);
	}
	else
	{
		syncStack->prepareSync();
	}

	connectStack();
	createConnection(p);

	return kapp->exec();
}


int listConduits(KCmdLineArgs *)
{
	FUNCTIONSETUP;

	KServiceTypeProfile::OfferList offers =
		KServiceTypeProfile::offers(CSL1("KPilotConduit"));

	// Now actually fill the two list boxes, just make
	// sure that nothing gets listed in both.
	//
	//
	QValueListIterator < KServiceOffer > availList(offers.begin());
	while (availList != offers.end())
	{
		KSharedPtr < KService > o = (*availList).service();

		cout << o->desktopEntryName().latin1() << endl;
		cout << "\t" << o->name().latin1()  << endl;
		if (!o->library().isEmpty())
		{
			cout << "\tIn "
				<< o->library().latin1()
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


