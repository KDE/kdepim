/*
 * (C) 1999, 2000 Sirtaj Singh Kang <taj@kde.org>
 * (C) 2000 Rik Hemsley <rik@kde.org>
 */

#include"kornapp.h"
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include"shell.h"

static const char * description=I18N_NOOP("The KDE Mail Checker");
static const char * version="0.2";


int main(int argc, char *argv[])
{
	KAboutData aboutData(argv[0], I18N_NOOP("Korn"),
		version, description, KAboutData::License_GPL,
		I18N_NOOP("(c) 1999-2000, The Korn Developers"));
	aboutData.addAuthor("Sirtaj Singh Kang",0, "taj@kde.org");
	aboutData.addAuthor("Cristian Tibirna",0, "tibirna@kde.org");
	aboutData.addAuthor("Kurt Granroth",0, "granroth@kde.org");
	aboutData.addAuthor("Rik Hemsley",0, "rik@kde.org");
	aboutData.addAuthor("Fixes by Jörg Habenicht",0, "j.habenicht@europemail.com");
	
	KCmdLineArgs::init( argc, argv, &aboutData );
	KUniqueApplication::addCmdLineOptions();
	
	if( !KUniqueApplication::start()  ) {
		// Already running. Should pop up the preferences dialog
		return 0;
	}

	KornApp *app = new KornApp();
	KornShell *korn = new KornShell( 0 );
	app->setShell( korn );

	//app->enableSessionManagement( true );

	if( korn->init() ) {
			korn->show();
			app->exec();
	}

	delete korn;
	delete app;

	return 0;
}
