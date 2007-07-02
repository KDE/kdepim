/*
 * (C) 1999, 2000 Sirtaj Singh Kang <taj@kde.org>
 * (C) 2000 Rik Hemsley <rik@kde.org>
 */

#include"kornapp.h"
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <klocale.h>

#include"kornshell.h"

static const char description[] = I18N_NOOP("KDE mail checker");
static const char version[] = "0.3";


int main(int argc, char *argv[])
{
	KAboutData aboutData(argv[0], 0, ki18n("Korn"),
		version, ki18n(description), KAboutData::License_GPL,
		ki18n("(c) 1999-2004, The Korn Developers"));
	aboutData.addAuthor(ki18n("Sirtaj Singh Kang"),KLocalizedString(), "taj@kde.org");
	aboutData.addAuthor(ki18n("Cristian Tibirna"),KLocalizedString(), "tibirna@kde.org");
	aboutData.addAuthor(ki18n("Kurt Granroth"),KLocalizedString(), "granroth@kde.org");
	aboutData.addAuthor(ki18n("Rik Hemsley"),KLocalizedString(), "rik@kde.org");
	aboutData.addAuthor(ki18n("Fixes by Jörg Habenicht"),KLocalizedString(), "j.habenicht@europemail.com");
	aboutData.addAuthor(ki18n("Preview by Heiner Eichmann"),KLocalizedString(), "h.eichmann@gmx.de");
	aboutData.addAuthor(ki18n("Mart Kelder"),KLocalizedString(),"mart.kde@hccnet.nl");
	
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

	//if( korn->init() ) {
			korn->show();
			app->exec();
	//}

	delete korn;
	delete app;

	return 0;
}
