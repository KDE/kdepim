/*
 * Copyright (C) 2003 Helge Deller <deller@kde.org>
 */

#include "kmobile.h"
#include <kuniqueapplication.h>
#include <dcopclient.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

static const char *description =
    I18N_NOOP("KDE Mobile Devices Manager");

static const char *version = "0.1";

static KCmdLineOptions options[] =
{
//    { "+[URL]", I18N_NOOP( "Document to open." ), 0 },
    { 0, 0, 0 }
};

int main(int argc, char **argv)
{
    KAboutData about("kmobile", I18N_NOOP("kmobile"), version, description,
                     KAboutData::License_GPL, "(C) 2003 Helge Deller", 0, 0, "deller@kde.org");
    about.addAuthor( "Helge Deller", 0, "deller@kde.org" );
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions(options);
    KUniqueApplication app;

    // register ourselves as a dcop client
    app.dcopClient()->registerAs(app.name(), false);

    // see if we are starting with session management
    if (app.isRestored())
    {
        RESTORE(KMobile);
    }
    else
    {
        KMobile *widget = new KMobile;
        widget->show();
    }

    return app.exec();
}
