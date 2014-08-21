#include "mailreader.h"



#include <KLocalizedString>
#include <QApplication>
#include <KAboutData>
#include <QCommandLineParser>
#include <QCommandLineOption>

static const char description[] =
    I18N_NOOP("A KDE 4 Application");

int main(int argc, char **argv)
{
    KLocalizedString::setApplicationDomain("mailreader"); 
    KAboutData about(QStringLiteral("mailreader"), i18n("mailreader"), QStringLiteral("0.1"), i18n(description),
                     KAboutLicense::GPL, i18n("(C) 2007 Andras Mantia"));
    about.addAuthor( i18n("Andras Mantia"), QString(), QStringLiteral("amantia@kde.org") );

    QCommandLineParser parser;
    QApplication app(argc, argv);
    parser.addVersionOption();
    parser.addHelpOption();
    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("+[URL]"), i18n( "Document to open" )));

    mailreader *widget = new mailreader;

    // see if we are starting with session management
    if (app.isSessionRestored())
    {
        RESTORE(mailreader);
    }
    else
    {
        // no session.. just start up normally
        if (parser.positionalArguments().count() == 0)
        {
            //mailreader *widget = new mailreader;
            widget->show();
        }
        else
        {
            int i = 0;
            for (; i < parser.positionalArguments().count(); i++)
            {
                //mailreader *widget = new mailreader;
                widget->show();
            }
        }
        
    }

    return app.exec();
}
