/* conduitApp.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This provides a base class for KApplications that also
** happen to be conduits.
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
static const char *conduitapp_id=
	"$Id$";

#include "options.h"

#include <unistd.h>
#include <stdlib.h>
#include <iostream.h>
#include <kwin.h>
#include <klocale.h>
#include <kglobal.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapp.h>
#include <kdebug.h>
#include <dcopclient.h>

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
	{ "local",I18N_NOOP("HotSync the database to the local database file instead of the palm pilot") ,0L },
	{ "test",I18N_NOOP("Test this conduit (possibly unimplemented)")
		,0L },
#ifdef DEBUG
	{ "debug <level>", I18N_NOOP("Set debugging level"), "0" },
#endif
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

	/* NOTREACHED */
	(void) conduitapp_id;
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

#ifdef DEBUG
	debug_level = atoi(p->getOption("debug"));
#endif

	return p;
#ifdef DEBUG
	/* NOTREACHED */
	(void) conduitapp_id;
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

// setup a conduit's DCOP stuff.
//
//
bool
ConduitApp::setupDCOP()
{
	FUNCTIONSETUP;

	// Can only happen if we call setupDCOP from somewhere
	// before exec(). That would be very strange ....
	//
	if (!fApp)
	{
		kdError() << __FUNCTION__
			<< "No KApplication object!"
			<< endl;
		return false;
	}

	DCOPClient *p = fApp->dcopClient();
	if (!p)
	{
		kdError() << __FUNCTION__
			<< "Couldn't get DCOP connection."
			<< endl;
		return false;
	}

	if (!p->attach())
	{
		// The DCOP docs say that KApplication
		// has already displayed a warning message.
		// So don't print one here.
		
		return false;
	}

	// Conduit's never serve requests so there's
	// no need to register a name.

	return true;
}

// exec() actually runs the conduit. This isn't strictly
// true since under KDE1 setConduit() does a lot of work
// which in KDE2 is transferred here.
//
//
#define CheckArg(s,m)	if (args->isSet(s)) \
		{ if (fMode==BaseConduit::None) \
		{ fMode=BaseConduit::m; } else \
		{ kdError() << __FUNCTION__ \
			<< ": More than one mode given (mode now " \
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
	CheckArg("test",Test);
	CheckArg("local",LocalDB);

	if (fMode==BaseConduit::None)
	{
		kdError() << __FUNCTION__ 
			<< ": You must specify a mode for the conduit."
			<< endl;
		fMode=BaseConduit::Error;
	}
	else
	{
#ifdef DEBUG
		if (debug_level & SYNC_TEDIOUS)
		{
			kdDebug() << fname 
				<< ": Set mode to "
				<< (int) fMode
				<< endl;
		}
#endif
	}

	return fMode;
}

int ConduitApp::exec(bool withDCOP,bool withGUI)
{
	FUNCTIONSETUP;

	QWidget *widget = 0L;

	(void) getMode();

	if (fMode == BaseConduit::Setup)
	{
		withGUI=true;
	}

	// Note that both styles and GUI are only in effect
	// if we are going to do the setup dialog.
	//
	fApp=new KApplication(withGUI,withGUI);

	if (withDCOP) 
	{
		if (!setupDCOP())
		{
			return 1;
		}
	}

	// init the conduit after DCOP is setup
	fConduit->init();

	switch(fMode)
	{
	case BaseConduit::DBInfo : cout << fConduit->dbInfo(); break;
	case BaseConduit::LocalDB : 
	case BaseConduit::HotSync : fConduit->doSync(); break;
	case BaseConduit::Backup : fConduit->doBackup(); break;
	case BaseConduit::Test : 
#ifdef DEBUG
		debug_level=-1; 
#endif
		fConduit->doTest(); 
		break;
	case BaseConduit::Setup :
		{
		QPixmap icon = fConduit->icon();
		widget = fConduit->aboutAndSetup();

		fApp->setMainWidget(widget);
		KWin::setIcons(widget->winId(),icon,icon);

		widget->show();

		return fApp->exec();
		}
	case BaseConduit::Error :
		kdError() << __FUNCTION__ << ": ConduitApp is in Error state."
			<< endl;
		break;
	default :
		kdWarning() << __FUNCTION__ << ": ConduitApp has state " 
			<< (int) fMode 
			<< ": where it is strange to call me."
			<< endl;
	}

	// Info, HotSync and Backup can't really fail?
	//
	//
	return 0;
}


// $Log$
// Revision 1.20  2001/03/27 11:10:39  leitner
// ported to Tru64 unix: changed all stream.h to iostream.h, needed some
// #ifdef DEBUG because qstringExpand etc. were not defined.
//
// Revision 1.19  2001/02/26 22:13:07  adridg
// Removed useless getopt.h; fixes compile prob on Solaris
//
// Revision 1.18  2001/02/08 08:13:44  habenich
// exchanged the common identifier "id" with source unique <sourcename>_id for --enable-final build
//
// Revision 1.17  2001/02/05 20:55:07  adridg
// Fixed copyright headers for source releases. No code changed
//
// Revision 1.16  2000/12/31 16:44:00  adridg
// Patched up the debugging stuff again
//
// Revision 1.15  2000/12/22 07:47:04  adridg
// Added DCOP support to conduitApp. Breaks binary compatibility.
//
// Revision 1.14  2000/12/21 00:42:50  adridg
// Mostly debugging changes -- added EFUNCTIONSETUP and more #ifdefs. KPilot should now compile -DNDEBUG or with DEBUG undefined
//
// Revision 1.13  2000/12/06 22:22:51  adridg
// Debug updates
//
// Revision 1.12  2000/11/20 00:24:27  adridg
// Added --test
//
// Revision 1.11  2000/11/17 08:31:59  adridg
// Minor changes
//
// Revision 1.10  2000/11/14 06:32:26  adridg
// Ditched KDE1 stuff
//
// Revision 1.9  2000/10/29 22:11:06  adridg
// Added debug-merge feature to conduits
//
