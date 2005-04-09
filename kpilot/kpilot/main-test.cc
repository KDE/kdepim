/* KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2001,2002,2003,2004 by Adriaan de Groot
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
#include <iostream>

#include <qpushbutton.h>
#include <qhbox.h>
#include <qtimer.h>

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
#include "hotSync.h"
#include "interactiveSync.h"

static KCmdLineOptions kpilotoptions[] = {
	{"p",0,0},
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
	{ "F",0,0},
	{ "test-local",
		I18N_NOOP("Run the conduit in file-test mode."),
		0 } ,
	{ "HHtoPC",
		I18N_NOOP("Copy Pilot to Desktop."),
		0 } ,
	{ "PCtoHH",
		I18N_NOOP("Copy Desktop to Pilot."),
		0 } ,
	{ "test-timeout",
		I18N_NOOP("Run conduit specially designed to timeout."),
		0 } ,
	{ "test-usercheck",
		I18N_NOOP("Run conduit just for user check."),
		0 } ,
#ifdef DEBUG
	{"debug <level>", I18N_NOOP("Set debugging level"), "0"},
#endif
	KCmdLineLastOption
};


static LogWidget *logWidget = 0L;
static QPushButton *resetButton = 0L;



/**
*** Conduits - sync actions - for testing specific scenarios.
**/

class TimeoutAction : public SyncAction
{
public:
	TimeoutAction(KPilotDeviceLink *p) ;
protected:
	virtual bool exec();
} ;

TimeoutAction::TimeoutAction(KPilotDeviceLink *p) :
	SyncAction(p)
{
	FUNCTIONSETUP;
}

bool TimeoutAction::exec()
{
	FUNCTIONSETUP;

	for (int i = 0; i<3; i++)
	{
		logMessage( CSL1("Hup two %1").arg(i) );
		fHandle->tickle();
		qApp->processEvents();
		sleep(1);
	}

	logMessage( CSL1("Now sleeping 65") );
	qApp->processEvents();
	sleep(65);
	return delayDone();
}







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

	deviceLink = new KPilotDeviceLink(0, "deviceLink");

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

	QObject::connect(deviceLink, SIGNAL(deviceReady(KPilotDeviceLink*)), syncStack, SLOT(execConduit()));

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

	deviceLink->reset(devicePath);
}

int syncTest(KCmdLineArgs *p)
{
	FUNCTIONSETUP;

	createLogWidget();
	createLink();

	syncStack = new ActionQueue(deviceLink);

	if (p->isSet("backup"))
	{
		syncStack->queueInit();
		syncStack->addAction(new BackupAction(deviceLink,true));
	}
	else if (p->isSet("restore"))
	{
		syncStack->queueInit(0);
		syncStack->addAction(new RestoreAction(deviceLink));
	}
	else if (p->isSet("test-timeout"))
	{
		syncStack->queueInit();
		syncStack->addAction( new TimeoutAction(deviceLink) );
		syncStack->addAction( new TimeoutAction(deviceLink) );
	}
	else
	{
		syncStack->queueInit(p->isSet("test-usercheck") /* whether to run usercheck */);
		syncStack->addAction(new TestLink(deviceLink));
	}
	syncStack->queueCleanup();

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

	SyncAction::SyncMode syncMode = SyncAction::eHotSync;
	if (p->isSet("test")) syncMode = SyncAction::eTest;
	if (p->isSet("HHtoPC")) syncMode = SyncAction::eCopyHHToPC;
	if (p->isSet("PCtoHH")) syncMode = SyncAction::eCopyPCToHH;

	syncStack = new ActionQueue(deviceLink);
	syncStack->queueInit();
	syncStack->queueConduits(l,syncMode,false);
	syncStack->queueCleanup();

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

		std::cout << o->desktopEntryName().latin1() << std::endl;
		std::cout << "\t" << o->name().latin1()  << std::endl;
		if (!o->library().isEmpty())
		{
			std::cout << "\tIn "
				<< o->library().latin1()
				<< std::endl;
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
		KAboutData::License_GPL, "(C) 2001-2004, Adriaan de Groot");
	about.addAuthor("Adriaan de Groot",
		I18N_NOOP("KPilot Maintainer"),
		"groot@kde.org", "http://www.cs.kun.nl/~adridg/kpilot/");

	KCmdLineArgs::init(argc, argv, &about);
	KCmdLineArgs::addCmdLineOptions(kpilotoptions, "kpilottest");
	KApplication::addCmdLineOptions();

	KCmdLineArgs *p = KCmdLineArgs::parsedArgs();


	KApplication a;
#ifdef DEBUG
	KPilotConfig::getDebugLevel(p);
#endif

	if ( p->isSet("backup") ||
		p->isSet("restore") ||
		p->isSet("list") ||
		p->isSet("test-timeout") ||
		p->isSet("test-usercheck") )
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


