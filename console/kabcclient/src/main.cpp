//
//  Copyright (C) 2003 - 2004 Tobias Koenig <tokoe@kde.org>
//  Copyright (C) 2005 - 2006 Kevin Krammer <kevin.krammer@gmx.at>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

// local includes
#include "formatfactory.h"
#include "inputformat.h"
#include "kabcclient.h"
#include "outputformat.h"

// standard includes
#include <iostream>
#include <sstream>
#include <stdlib.h>

// Qt includes
#include <QtCore/QFileInfo>

// KDE includes
#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocalizedstring.h>
static const char version[] = "0.8.1";

bool checkForFormatHelp(KCmdLineArgs* args, FormatFactory* factory);
bool checkForCodecHelp(KCmdLineArgs* args);

int handleKABC2Mutt(int argc, char** argv);

void avoidQPixmapWarning(QtMsgType type, const char *msg);

QString optionAsString(KCmdLineArgs* args, const char* option);

using std::cout;
using std::endl;

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
    QString commandName = QFileInfo(QFile::decodeName(argv[0])).fileName();
    if (commandName == "kabc2mutt")
    {
        return handleKABC2Mutt(argc, argv);
    }

    KAboutData aboutData("kabcclient", "kabcclient", ki18n("KABC client"), version,
                         ki18n("KDE address book command-line client"),
                         KAboutData::License_GPL_V2);

    aboutData.addAuthor(ki18n("Kevin Krammer"), ki18n("Primary Author"), "kevin.krammer@gmx.at");

    KCmdLineOptions cmdLineOptions;

    cmdLineOptions.add("A");

    cmdLineOptions.add("add", ki18n("Add input data as new address book entries"));

    cmdLineOptions.add("R");

    cmdLineOptions.add("remove", ki18n("Remove entries matching the input data"));

    cmdLineOptions.add("M");

    cmdLineOptions.add("merge", ki18n("Merge input data into the address book"));

    cmdLineOptions.add("S");

    cmdLineOptions.add("search", ki18n("Search for entries matching the input data"));

    cmdLineOptions.add("L");

    cmdLineOptions.add("list", ki18n("List all entries in address book"));

    cmdLineOptions.add("nosave", ki18n("Do not save changes to the address book on add/remove operations"));

    cmdLineOptions.add("if");

    cmdLineOptions.add("input-format <format>", ki18n("How to interpret the input data."), "search");

    cmdLineOptions.add("if-opts");

    cmdLineOptions.add("input-format-options <options>", ki18n("Input options for the selected format"));

    cmdLineOptions.add("of");

    cmdLineOptions.add("output-format <format>", ki18n("How to present the output data."), "vcard");

    cmdLineOptions.add("of-opts");

    cmdLineOptions.add("output-format-options <options>", ki18n("Output options for the selected format"));

    cmdLineOptions.add("ic");

    cmdLineOptions.add("input-codec <textcodec>", ki18n("How to convert the input text."), "local");

    cmdLineOptions.add("oc");

    cmdLineOptions.add("output-codec <textcodec>", ki18n("How to convert the output text."), "local");

    cmdLineOptions.add("match-case", ki18n("Match key fields case sensitive. UID is always matched case sensitive"));

    cmdLineOptions.add(ki18n("+[input data]").toString().toLocal8Bit(), ki18n("Input to use instead of reading stdin"));

    KCmdLineArgs::addCmdLineOptions(cmdLineOptions);
    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

    qInstallMsgHandler(avoidQPixmapWarning);
    KApplication app;
    qInstallMsgHandler(0);

    FormatFactory formatFactory;

    if (checkForFormatHelp(args, &formatFactory)) return 0;

    if (checkForCodecHelp(args)) return 0;

    KABCClient::Operation operation = KABCClient::Search;
    if (args->isSet("search"))
    {
        operation = KABCClient::Search;
    }
    else if (args->isSet("list"))
    {
        operation = KABCClient::List;
    }
    else if (args->isSet("add"))
    {
        operation = KABCClient::Add;
    }
    else if (args->isSet("merge"))
    {
        operation = KABCClient::Merge;
    }
    else if (args->isSet("remove"))
    {
        operation = KABCClient::Remove;
    }
    else
    {
        std::cerr << i18n("No operation specified, assuming --search").toLocal8Bit().data() << endl;
    }

    KABCClient client(operation, &formatFactory);

    if (!client.setInputFormat(args->getOption("input-format").toLocal8Bit()))
    {
        const QString error = i18n("Invalid input format \"%1\". See --input-format help",
                                   optionAsString(args, "input-format"));
        KCmdLineArgs::usageError(error);
        return 1;
    }

    if (args->isSet("input-format-options"))
    {
        if (!client.setInputOptions(args->getOption("input-format-options").toLocal8Bit()))
        {
            const QString error = i18n("Invalid options for input format \"%1\". "
                                      "See --input-format-options help",
                                      optionAsString(args, "input-format"));
            KCmdLineArgs::usageError(error);
        }
    }

    if (!client.setOutputFormat(args->getOption("output-format").toLocal8Bit()))
    {
        const QString error = i18n("Invalid output format \"%1\". See --output-format help",
                                   optionAsString(args, "output-format"));
        KCmdLineArgs::usageError(error);
        return 1;
    }

    if (args->isSet("output-format-options"))
    {
        if (!client.setOutputOptions(args->getOption("output-format-options").toLocal8Bit()))
        {
            const QString error = i18n("Invalid options for output format \"%1\". "
                                       "See --output-format-options help",
                                       optionAsString(args, "output-format"));
            KCmdLineArgs::usageError(error);
        }
    }

    QString codecName = args->getOption("input-codec");
    if (!args->isSet("input-codec") && args->getOption("input-format") == QString("vcard"))
        codecName = "UTF8";

    if (!client.setInputCodec(codecName.toLocal8Bit()))
    {
        const QString error = i18n("Invalid input codec \"%1\"", codecName);
        KCmdLineArgs::usageError(error);
        return 1;
    }

    codecName = args->getOption("output-codec");
    if (!args->isSet("output-codec") && args->getOption("output-format") == QString("vcard"))
        codecName = "UTF8";

    if (!client.setOutputCodec(codecName.toLocal8Bit()))
    {
        const QString error = i18n("Invalid output codec \"%1\"", codecName);
        KCmdLineArgs::usageError(error);
        return 1;
    }

    if (args->isSet("match-case"))
    {
        client.setMatchCaseSensitivity(Qt::CaseSensitive);
    }

    if (!args->isSet("save"))
    {
        client.setAllowSaving(false);
    }

    std::stringstream sstream;
    if (args->count() > 0)
    {
        for (int i = 0; i < args->count(); ++i)
        {
            sstream << args->arg(i).toLocal8Bit().data() << endl;
        }

        client.setInputStream(&sstream);
    }
    else
    {
        client.setInputStream(&std::cin);
    }

    if (!client.initOperation())
    {
        cout << i18n("Unable to perform requested operation").toLocal8Bit().data() << endl;
        return 1;
    }

    return app.exec();
}

///////////////////////////////////////////////////////////////////////////////

bool checkForFormatHelp(KCmdLineArgs* args, FormatFactory* factory)
{
    bool formatHelpRequested = false;

    if (args->isSet("input-format") && args->getOption("input-format") == "help")
    {
        formatHelpRequested = true;

        cout << endl;
        cout << i18n("The following input formats are available:").toLocal8Bit().data() << endl;

        QByteArrayList formats = factory->inputFormatList();
        QByteArrayList::const_iterator it    = formats.constBegin();
        QByteArrayList::const_iterator endIt = formats.constEnd();
        for (; it != endIt; ++it)
        {
            InputFormat* format = factory->inputFormat(*it);
            if (format != 0)
            {
                cout << it->data() << ( (*it).length() >= 8 ? "\t" : "\t\t");

                QString description = format->description();
                if (description.isEmpty()) description = i18n("No description available");

                cout << description.toLocal8Bit().data() << endl;
                delete format;
            }
        }
    }
    else if (args->isSet("input-format-options") &&
             args->getOption("input-format-options") == "help")
    {
        formatHelpRequested = true;

        InputFormat* format = factory->inputFormat(args->getOption("input-format").toLocal8Bit());
        if (format == 0)
        {
            const QString error = i18n("Invalid input format \"%1\". See --input-format help",
                                       optionAsString(args, "input-format"));
            KCmdLineArgs::usageError(error);
            exit(1);
        }

        QString usage = format->optionUsage();

        const QString tmparg = optionAsString(args, "input-format");
        const QString message =
            (usage.isEmpty()
                ? i18n("No options available for input format %1", tmparg)
                : i18n("The following options are available for input format %1:", tmparg));

        cout << endl;
        cout << message.toLocal8Bit().data() << endl;
        if (!usage.isEmpty()) cout << usage.toLocal8Bit().data() << endl;

        delete format;
    }

    if (args->isSet("output-format") && args->getOption("output-format") == "help")
    {
        formatHelpRequested = true;

        cout << endl;
        cout << i18n("The following output formats are available:").toLocal8Bit().data() << endl;

        QByteArrayList formats = factory->outputFormatList();
        QByteArrayList::const_iterator it    = formats.constBegin();
        QByteArrayList::const_iterator endIt = formats.constEnd();
        for (; it != endIt; ++it)
        {
            OutputFormat* format = factory->outputFormat(*it);
            if (format != 0)
            {
                cout << it->data() << ( (*it).length() >= 8 ? "\t" : "\t\t");

                QString description = format->description();
                if (description.isEmpty()) description = i18n("No description available");

                cout << description.toLocal8Bit().data() << endl;
                delete format;
            }
        }
    }
    else if (args->isSet("output-format-options") &&
             args->getOption("output-format-options") == "help")
    {
        formatHelpRequested = true;

        OutputFormat* format = factory->outputFormat(args->getOption("output-format").toLocal8Bit());
        if (format == 0)
        {
            const QString error = i18n("Invalid output format \"%1\". See --output-format help",
                                       optionAsString(args, "output-format"));
            KCmdLineArgs::usageError(error);
            exit(1);
        }

        QString usage = format->optionUsage();

        const QString tmparg = optionAsString(args, "output-format");
        const QString message =
            (usage.isEmpty()
                ? i18n("No options available for output format %1", tmparg)
                : i18n("The following options are available for output format %1:", tmparg));

        cout << endl;
        cout << message.toLocal8Bit().data() << endl;
        if (!usage.isEmpty()) cout << usage.toLocal8Bit().data() << endl;

        delete format;
    }

    return formatHelpRequested;
}

///////////////////////////////////////////////////////////////////////////////

bool checkForCodecHelp(KCmdLineArgs* args)
{
    bool codecHelpRequested = false;

    if (args->isSet("input-codec") && args->getOption("input-codec") == "help")
    {
        codecHelpRequested = true;

        cout << i18n("The input codec transforms the input text data into an "
                     "universal internal format").toLocal8Bit().data() << endl;
        cout << i18n("Default input encoding is 'local' unless input format is 'vcard', "
                     "in which case the default encoding will be 'utf8'.").toLocal8Bit().data() << endl;
    }

    if (args->isSet("output-codec") && args->getOption("output-codec") == "help")
    {
        codecHelpRequested = true;

        cout << i18n("The output codec transforms the output text data from the "
                     "internal format to an 8-bit text format").toLocal8Bit().data() << endl;
        cout << i18n("Default output encoding is 'local' unless output format is 'vcard', "
                     "in which case the default encoding will be 'utf8'.").toLocal8Bit().data() << endl;
    }

    if (codecHelpRequested)
    {
        cout << i18n("Built-in codecs are UTF8 and LOCAL, respectively using "
                     "the 8-bit unicode format or your local encoding").toLocal8Bit().data()
             << endl;

        cout << i18n("Other codecs can be specified by their ISO code, for "
                     "example 'ISO 8859-15' for western european languages, "
                     "including the Euro sign").toLocal8Bit().data() << endl;
    }

    return codecHelpRequested;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int handleKABC2Mutt(int argc, char** argv)
{
    KAboutData aboutData("kabc2mutt", "kabcclient", ki18n("kabc2mutt"), version,
                         ki18n("kabc - mutt converter"),
                         KAboutData::License_GPL_V2);

    aboutData.addAuthor(ki18n("Tobias König"),  ki18n("Primary Author"),
                        "tokoe@kde.org");
    aboutData.addAuthor(ki18n("Kevin Krammer"), ki18n("Contributor"), "kevin.krammer@gmx.at");


    KCmdLineOptions kabc2muttCmdLineOptions;

    kabc2muttCmdLineOptions.add("query <substring>", ki18n("Only show contacts where name or address matches <placeholder>substring</placeholder>"));

    kabc2muttCmdLineOptions.add("format <format>", ki18n("Default format is 'alias'. 'query' returns email[tab]name[tab], "
               "as needed by mutt's query_command"), "alias");

    kabc2muttCmdLineOptions.add("alternate-key-format", ki18n("Default key format is 'JohDoe', this option turns it into 'jdoe'"));

    kabc2muttCmdLineOptions.add("ignore-case", ki18n("Make queries case insensitive"));

    kabc2muttCmdLineOptions.add("all-addresses", ki18n("Return all mail addresses, not just the preferred one"));

    KCmdLineArgs::addCmdLineOptions(kabc2muttCmdLineOptions);
    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

    KABCClient::Operation operation = KABCClient::List;
    if (args->isSet("query"))
    {
        operation = KABCClient::Search;
    }

    FormatFactory formatFactory;
    KABCClient client(operation, &formatFactory);

    client.setInputFormat("email");
    client.setOutputFormat("mutt");
    client.setInputCodec("UTF-8");
    QString options = args->getOption("format");

    if (args->isSet("alternate-key-format"))
    {
        options.append(",altkeys");
    }
    if (args->isSet("all-addresses"))
    {
        options.append(",allemails");
    }

    if (!args->isSet("ignore-case"))
    {
        client.setMatchCaseSensitivity(Qt::CaseSensitive);
    }

    client.setOutputOptions(options.toLocal8Bit().data());

    std::stringstream sstream;
    if (operation == KABCClient::Search)
    {
        sstream << args->getOption("query").toLocal8Bit().data() << endl;
        client.setInputStream(&sstream);
    }

    qInstallMsgHandler(avoidQPixmapWarning);
    //KApplication::disableAutoDcopRegistration();
    KApplication app(false);
    qInstallMsgHandler(0);

    if (!client.initOperation())
    {
        cout << i18n("Unable to perform requested operation").toLocal8Bit().data() << endl;
        return 1;
    }

    // mutt wants a line of text before the results
    cout << i18n("Searching KDE address book").toLocal8Bit().data() << endl;

    int result =  app.exec();

    // in case of no match mutt wants a line of text saying so
    if (result == 2) // Operation Search returns 2 on no match
        cout << i18n("No matches in KDE address book").toLocal8Bit().data() << endl;

    return result;
}

///////////////////////////////////////////////////////////////////////////////

void avoidQPixmapWarning(QtMsgType type, const char *msg)
{
    Q_UNUSED(type)
    Q_UNUSED(msg)
}

///////////////////////////////////////////////////////////////////////////////

QString optionAsString(KCmdLineArgs* args, const char* option)
{
    if (args == 0 || option == 0) return QString();

    return args->getOption(option);
}

// End of file
