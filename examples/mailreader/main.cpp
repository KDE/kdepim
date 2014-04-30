#include "mailreader.h"
#include <kapplication.h>
#include <k4aboutdata.h>
#include <kcmdlineargs.h>
#include <KLocale>

static const char description[] =
    I18N_NOOP("A KDE 4 Application");

static const char version[] = "0.1";

int main(int argc, char **argv)
{
    K4AboutData about("mailreader", 0, ki18n("mailreader"), version, ki18n(description),
                     K4AboutData::License_GPL, ki18n("(C) 2007 Andras Mantia"), KLocalizedString(), 0, "amantia@kde.org");
    about.addAuthor( ki18n("Andras Mantia"), KLocalizedString(), "amantia@kde.org" );
    KCmdLineArgs::init(argc, argv, &about);

    KCmdLineOptions options;
    options.add("+[URL]", ki18n( "Document to open" ));
    KCmdLineArgs::addCmdLineOptions(options);
    KApplication app;

    mailreader *widget = new mailreader;

    // see if we are starting with session management
    if (app.isSessionRestored())
    {
        RESTORE(mailreader);
    }
    else
    {
        // no session.. just start up normally
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
        if (args->count() == 0)
        {
            //mailreader *widget = new mailreader;
            widget->show();
        }
        else
        {
            int i = 0;
            for (; i < args->count(); i++)
            {
                //mailreader *widget = new mailreader;
                widget->show();
            }
        }
        args->clear();
    }

    return app.exec();
}
