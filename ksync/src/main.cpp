
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kglobal.h>

#include "ksync.h"


static const char *description =
	I18N_NOOP("KSync");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE


static KCmdLineOptions options[] =
{
  { "+[File]", I18N_NOOP("file to open"), 0 },
  KCmdLineLastOption
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};

int main(int argc, char *argv[])
{

	KAboutData aboutData( "ksync", I18N_NOOP("KSync"),
                          "0.1", description, KAboutData::License_GPL,
                          "(c) 2001, Cornelius Schumacher", 0, 0, "schumacher@kde.org");
	aboutData.addAuthor("Cornelius Schumacher",0, "schumacher@kde.org");
	KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

    KApplication app;
    KGlobal::locale()->insertCatalogue("libksync");

    if (app.isRestored())
    {
        RESTORE(KSync);
    }
    else
    {
        KSync *ksync = new KSync();
        ksync->show();

        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

        if (args->count())
        {
            ksync->openDocumentFile(args->arg(0));
        }
        else
        {
            ksync->openDocumentFile();
        }
        args->clear();
    }

    return app.exec();
}
