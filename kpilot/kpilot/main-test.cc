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
** Bug reports and questions can be sent to groot@kde.org
*/
static const char *test_id="$Id$";

#include "options.h"

#include <stdlib.h>

#include <iostream.h>

#include <kapp.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include "logWidget.h"
#include "kpilotConfig.h"

#include "main-test.moc"

KPilotTestLink *KPilotTestLink::fTestLink = 0L;

KPilotTestLink::KPilotTestLink(DeviceType t,const QString &p) :
	KPilotDeviceLink()
{
	FUNCTIONSETUP;

	ASSERT(fTestLink==0L);

	fTestLink = this;

	connect(this,SIGNAL(deviceReady()),
		this,SLOT(enumerateDatabases()));
	reset(t,p);
}

KPilotTestLink *KPilotTestLink::getTestLink(DeviceType t,const QString &p)
{
	FUNCTIONSETUP;

	if (fTestLink) return fTestLink;

	fTestLink = new KPilotTestLink(t,p);
	return fTestLink;
}

void KPilotTestLink::enumerateDatabases()
{
	FUNCTIONSETUP;

	int i;
	int dbindex=0;
	int count=0;
	struct DBInfo db;

	while ((i=dlp_ReadDBList(pilotSocket(),0,dlpDBListRAM,
		dbindex,&db)) > 0)
	{
		count++;
		dbindex=db.index+1;

		DEBUGKPILOT << fname
			<< ": Read database "
			<< db.name
			<< endl;

		emit logMessage(i18n("Syncing database %1...").arg(db.name));

		kapp->processEvents();
	}

	emit logMessage(i18n("HotSync finished."));
	KPilotDeviceLink::close();
}

static KCmdLineOptions kpilotoptions[] =
{
	{ "port <device>", I18N_NOOP("Path to Pilot device node"), 0},
#ifdef DEBUG
	{ "debug <level>", I18N_NOOP("Set debugging level"), "0" },
#endif
	{ 0,0,0}
} ;

int main(int argc, char **argv)
{
	KAboutData about("kpilotTest",
		I18N_NOOP("KPilotTest"),
		KPILOT_VERSION,
		"KPilot Tester",
		KAboutData::License_GPL,
		"(C) 2001, Adriaan de Groot");
	about.addAuthor("Adriaan de Groot",
		I18N_NOOP("KPilot Maintainer"),
		"groot@kde.org",
		"http://www.cs.kun.nl/~adridg/kpilot/");
	
	KCmdLineArgs::init(argc,argv,&about);
	KCmdLineArgs::addCmdLineOptions(kpilotoptions);
	KApplication::addCmdLineOptions();

	KCmdLineArgs *p = KCmdLineArgs::parsedArgs();

	QString devicePath = p->getOption("port");
	if (devicePath.isEmpty())
	{
		devicePath = "/dev/pilot";
	}

	KPilotDeviceLink::DeviceType deviceType = 
		KPilotDeviceLink::OldStyleUSB;

	KPilotConfig::getDebugLevel(p);

	KApplication  a;

	LogWidget *w = new LogWidget(0L);
	w->resize(300,300);
	w->show();
	w->setShowTime(true);
	a.setMainWidget(w);

	KPilotTestLink *t = KPilotTestLink::getTestLink(deviceType,devicePath);

	QObject::connect(t,SIGNAL(logError(const QString &)),
		w,SLOT(addError(const QString &)));
	QObject::connect(t,SIGNAL(logMessage(const QString &)),
		w,SLOT(addMessage(const QString &)));

	return a.exec();

	/* NOTREACHED */
	(void) test_id;
}

// $Log$
// Revision 1.2  2001/09/06 22:05:00  adridg
// Enforce singleton-ness
//
// Revision 1.1  2001/09/05 21:53:51  adridg
// Major cleanup and architectural changes. New applications kpilotTest
// and kpilotConfig are not installed by default but can be used to test
// the codebase. Note that nothing else will actually compile right now.
//
