// conduitApp.cc
//
// Copyright (C) 1998-2000 Dan Pilone
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
// This is conduitApp.cc for KDE 2 / KPilot 4.
//
//
//

static const char *id=
	"$Id$";

#include "options.h"

#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stream.h>
#include <kwin.h>
#include <klocale.h>
#include <kglobal.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapp.h>
#include <kdebug.h>

#include "conduitApp.h"


// In KDE2 we use command line options in KDE2 style (!)
// as opposed to the GNU style long options used in
// KDE1. This increases conformity.
//
// Help and debug options are standard in KDE2.
//
static KCmdLineOptions conduitoptions[] =
{
	{ "setup", I18N_NOOP("Start up configuration dialog " 
		"for this conduit"), 0L },
	{ "info", I18N_NOOP("Print which databases are associated "
		"with this conduit"), 0L },
	{ "backup", I18N_NOOP("Backup the databases associated "
		"with this conduit"), 0L },
	{ "hotsync", I18N_NOOP("HotSync the databases associated "
		"with this conduit"), 0L },
	{ "debug <level>", I18N_NOOP("Set debugging level"), "0" },
	{ 0,0,0 }
} ;


// Constructors
//
//
// The constructor for KDE2. In KDE2 a ConduitApp
// HAS a KApplication, it's not a KIND OF KApplication.
//
//
ConduitApp::ConduitApp(
	int& argc, 
	char** argv, 
	const char * rAppName,
	const char *conduitName,
	const char *version) :
	fAbout(0L),
	fApp(0L),
	fCmd(false),
	fMode(BaseConduit::None),
	fArgc(argc),
	fArgv(argv)
{
	fAbout=new KAboutData(rAppName,
		conduitName,
		version,
		I18N_NOOP("Pilot HotSync software for KDE 2"),
		(int) KAboutData::License_GPL,
		"Copyright (c) 1998-2000 Dan Pilone",
		(const char *)0L,
		"http://www.slac.com/pilone/kpilot_home/");

	fAbout->addAuthor("Dan Pilone",
		I18N_NOOP("Project Leader"),
		"pilone@slac.com",
		"http://www.slac.com/pilone/kpilot_home/");
}



void ConduitApp::addAuthor(const char *name,
	const char *task,
	const char *email)
{
	fAbout->addAuthor(name,task,email);
}








// Next are helper functions, which vary considerably
// from KDE1 to KDE2 but have the same purpose: deal
// with the options that can be passed to the conduit.
//
// Options are added by addOptions(); the actual handling
// of options is done much later by exec()
//
//
void ConduitApp::addOptions(KCmdLineOptions *p)
{
	if (!fCmd)
	{
		KCmdLineArgs::init(fArgc,fArgv,fAbout);
		KCmdLineArgs::addCmdLineOptions(conduitoptions);
		fCmd=true;
	}
	if (p)
	{
		KCmdLineArgs::addCmdLineOptions(p);
	}
}

KCmdLineArgs *ConduitApp::getOptions()
{
	KCmdLineArgs *p;

	if (!fCmd)
	{
		addOptions(0L);
	}
	KApplication::addCmdLineOptions();

	p=KCmdLineArgs::parsedArgs();

	debug_level = atoi(p->getOption("debug"));

	return p;
#ifdef DEBUG
	/* NOTREACHED */
	(void) id;
#endif
}


// In KDE1, setConduit() performs some actions based
// on the mode (already determined by handleOptions()).
// In KDE2, this is left to exec(). Therefore, much
// of setConduit() is commented out in KDE2.
//
void
ConduitApp::setConduit(BaseConduit* conduit)
{
	FUNCTIONSETUP;

	fConduit = conduit;
}

// exec() actually runs the conduit. This isn't strictly
// true since under KDE1 setConduit() does a lot of work
// which in KDE2 is transferred here.
//
//
#define CheckArg(s,m)	if (args->isSet(s)) \
		{ if (fMode==BaseConduit::None) \
		{ fMode=BaseConduit::m; } else \
		{ cerr << fname << ": More than one mode given (mode now " \
			<< (int)fMode << ')' << endl; \
			fMode=BaseConduit::Error; } }

BaseConduit::eConduitMode ConduitApp::getMode()
{
	FUNCTIONSETUP;

	if (fMode!=BaseConduit::None) return fMode;

	KCmdLineArgs *args=getOptions();
	CheckArg("info",DBInfo);
	CheckArg("setup",Setup);
	CheckArg("hotsync",HotSync);
	CheckArg("backup",Backup);

	if (fMode==BaseConduit::None)
	{
		cerr << fname 
			<< ": You must specify a mode for the conduit."
			<< endl;
		fMode=BaseConduit::Error;
	}
	else
	{
		if (debug_level & SYNC_TEDIOUS)
		{
			kdDebug() << fname 
				<< ": Set mode to "
				<< (int) fMode
				<< endl;
		}
	}

	return fMode;
}

int ConduitApp::exec()
{
	FUNCTIONSETUP;

	QWidget *widget = 0L;
	QPixmap *icon= 0L;

	(void) getMode();

	// Note that both styles and GUI are only in effect
	// if we are going to do the setup dialog.
	//
	fApp=new KApplication(fMode==BaseConduit::Setup,
		fMode==BaseConduit::Setup);

	switch(fMode)
	{
	case BaseConduit::DBInfo : cout << fConduit->dbInfo(); break;
	case BaseConduit::HotSync : fConduit->doSync(); break;
	case BaseConduit::Backup : fConduit->doBackup(); break;
	case BaseConduit::Setup :
		widget = fConduit->aboutAndSetup();
		icon= fConduit->icon();

		fApp->setMainWidget(widget);
		KWin::setIcons(widget->winId(),*icon,*icon);

		widget->show();

		return fApp->exec();
	case BaseConduit::Error :
		cerr << fname << ": ConduitApp is in Error state."
			<< endl;
		break;
	default :
		cerr << fname << ": ConduitApp has state " 
			<< (int) fMode  << endl 
			<< fname << ": where it is strange to call me."
			<< endl;
	}

	// Info, HotSync and Backup can't really fail?
	//
	//
	return 0;
}


// $Log$
// Revision 1.10  2000/11/14 06:32:26  adridg
// Ditched KDE1 stuff
//
// Revision 1.9  2000/10/29 22:11:06  adridg
// Added debug-merge feature to conduits
//
