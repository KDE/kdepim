#include "abbrowser.h"
#include <kapp.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>


static const char* description=I18N_NOOP("The KDE Address Book Browser");
static const char* VERSION="0.0.1";

int main(int argc, char *argv[])
{
	KAboutData aboutData( "abbrowser", I18N_NOOP("abBrowser"),
		VERSION, description, KAboutData::License_GPL,
		"(c) 1999, Don Sanders");
	aboutData.addAuthor("Don Sanders",0, "dsanders@kde.org");
	KCmdLineArgs::init( argc, argv, &aboutData );
	
	KApplication app;

	// All session management is handled in the RESTORE macro
	if (app.isRestored())
	{
		RESTORE(Pab)
	}
	else
	{
		Pab *widget = new Pab;
		widget->show();
	}

	return app.exec();
}
