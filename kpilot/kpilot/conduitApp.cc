// conduitApp.cc
//
// Copyright (C) 1998,1999 Dan Pilone
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
// This is conduitApp.cc for KDE 2 / KPilot 4.
//
// TODO:
//	Move from getopt() to KCmdLineArgs()
//
//

static char *id="$Id$";

#include "options.h"

#ifdef KDE2
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stream.h>
#include <kwin.h>

#include "conduitApp.moc"
#else
#include <iostream.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <kwm.h>
#include "conduitApp.moc"
#include "kpilot.h"
#endif


static struct option longOptions[]=
{
	{ "setup",0,0L,'s' },
	{ "info",0,0L,'i' },
	{ "backup",0,0L,'b' },
	{ "debug",1,0L,'d' },
	{ "hotsync",0,0L,'h' },	// Included for orthogonality
	{ "help",0,0L,1 },
	{ 0L,0,0L,0 }
} ;


BaseConduit::eConduitMode ConduitApp::handleOptions(const char *banner,
	int& argc,char **argv)
{
	FUNCTIONSETUP;

	int c,li;
	BaseConduit::eConduitMode rc=BaseConduit::HotSync;

	while ((c=getopt_long(argc,argv,"sibd:vh?",longOptions,&li))>0)
	{
		switch(c)
		{
		case 's' : rc=BaseConduit::Setup; break;
		case 'i' : rc=BaseConduit::DBInfo; break;
		case 'b' : rc=BaseConduit::Backup; break;
		case 'h' : rc=BaseConduit::HotSync; break;
		case 'd' : debug_level=atoi(optarg); 
			if (debug_level)
			{
				cerr << fname << ": Debug level set to "
					<< debug_level << endl;
			}
			break;
		case 1 : usage(banner,longOptions); exit(0);
		default : usage(banner,longOptions); exit(1);
		}
	}

	return rc;
}


ConduitApp::ConduitApp(
	int& argc, 
	char** argv, 
#ifdef KDE2
	const QCString &rAppName,
#else
	const QString& rAppName,
#endif
	const char *banner)
  : KApplication(argc, argv, rAppName), fConduit(0L)
{
	fMode=handleOptions(banner,argc,argv);
	if (fMode==BaseConduit::Error) usage(banner,longOptions);
}

void
ConduitApp::setConduit(BaseConduit* conduit)
{
	FUNCTIONSETUP;

	if (fMode==BaseConduit::Error)
	{
		cerr << fname << ": ConduitApp has state \"Error\".\n";
		return;
	}

	fConduit = conduit;

	switch(fMode)
	{
	case BaseConduit::DBInfo : cout << conduit->dbInfo(); break;
	case BaseConduit::HotSync : conduit->doSync(); break;
	case BaseConduit::Backup : conduit->doBackup(); break;
	case BaseConduit::Setup : break;
	default :
		cerr << fname << ": ConduitApp has state " 
			<< (int) fMode  << endl 
			<< fname << ": where it is strange to call me."
			<< endl;
	}
}

int
ConduitApp::exec()
{
	FUNCTIONSETUP;

	if(fMode == BaseConduit::Setup)
	{
		if (debug_level & UI_MAJOR)
		{
				cerr << fname 
					<< ": Running setup widget"
					<< endl ;
		}
		QWidget* widget = fConduit->aboutAndSetup();
		QPixmap *icon= fConduit->icon();

		KApplication::setMainWidget(widget);
#ifdef KDE2
		KWin::setIcons(widget->winId(),*icon,*icon);
#else
		KWM::setIcon(widget->winId(),*icon);
#endif

		widget->show();
		return KApplication::exec();
	}
	return 0;
}
