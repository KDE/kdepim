#include <iostream.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "conduitApp.moc"
#include "kpilot.h"

// The id string is available in all modules so 
// you can find out what revision your binaries are.
//
//
static char *id="$Id$";

// A conduit gets its debug_level from here --
// let's hope it doesn't come from somewhere else as well.
//
//
int debug_level=0;

static struct option longOptions[]=
{
	{ "setup",0,0L,'s' },
	{ "info",0,0L,'i' },
	{ "backup",0,0L,'b' },
	{ "debug",1,0L,'d' },
	{ "hotsync",0,0L,'h' },	// Included for orthogonality
	{ 0L,0,0L,0 }
} ;


void ConduitApp::usage()
{
	int i;

	cerr << 
	"Accepted options are:\n";

	for (i=0; longOptions[i].name; i++)
	{
		cerr << "\t--" << longOptions[i].name;
		if (longOptions[i].val)
		{
			cerr << " (-" << (char)longOptions[i].val << ')' ;
		}
		cerr << '\n';
	}

}

BaseConduit::eConduitMode ConduitApp::handleOptions(int& argc,char **argv)
{
	FUNCTIONSETUP;

	int c,li;
	BaseConduit::eConduitMode rc=BaseConduit::HotSync;

	while ((c=getopt_long(argc,argv,"sibd:vh",longOptions,&li))>0)
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
		default : rc=BaseConduit::Error;
		}
	}

	return rc;
}

ConduitApp::ConduitApp(int& argc, char** argv)
  : KApplication(argc, argv), fConduit(0L)
{
	fMode=handleOptions(argc,argv);
	if (fMode==BaseConduit::Error) usage();
}

ConduitApp::ConduitApp(int& argc, char** argv, const QString& rAppName)
  : KApplication(argc, argv, rAppName), fConduit(0L)
{
	fMode=handleOptions(argc,argv);
	if (fMode==BaseConduit::Error) usage();
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
if (debug_level>1)
{
		cerr << fname << ": Running setup widget.\n";
}
		QWidget* widget = fConduit->aboutAndSetup();
		KApplication::setMainWidget(widget);
		widget->show();
		return KApplication::exec();
	}
	return 0;
}
