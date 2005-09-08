//
//  Copyright (C) 2003 - 2004 Tobias Koenig <tokoe@kde.org>
//  Copyright (C) 2005 Kevin Krammer <kevin.krammer@gmx.at>
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
//  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
//

// standard includes
#include <iostream>
#include <sstream>

// Qt includes
#include <qfileinfo.h>

// KDE includes
#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>

// local includes
#include "formatfactory.h"
#include "inputformat.h"
#include "kabcclient.h"
#include "outputformat.h"

static const char version[] = "0.8.1";

static KCmdLineOptions cmdLineOptions[] =
{
    {"A", 0, 0},
    {"add", I18N_NOOP("Add input data as new addressbook entries"), 0},
    {"R", 0, 0},
    {"remove", I18N_NOOP("Remove entries matching the input data"), 0},
    {"M", 0, 0},
    {"merge", I18N_NOOP("Merge input data into the addressbook"), 0},
    {"S", 0, 0},
    {"search", I18N_NOOP("Search for entries matching the input data"), 0},
    {"L", 0, 0},
    {"list", I18N_NOOP("List all entries in address book"), 0},
    {"nosave",
     I18N_NOOP("Do not save changes to the addressbook on add/remove operations"), 0}, 
    {"if", 0, 0},
    {"input-format <format>", I18N_NOOP("How to interpret the input data."), "search"},
    {"if-opts", 0, 0},
    {"input-format-options <options>", I18N_NOOP("Input options for the selected format"), 0},
    {"of", 0, 0},
    {"output-format <format>", I18N_NOOP("How to present the output data."), "vcard"},
    {"of-opts", 0, 0},
    {"output-format-options <options>", I18N_NOOP("Output options for the selected format"), 0},
    {"ic", 0, 0},
    {"input-codec <textcodec>", I18N_NOOP("How to convert the input text."), "local"},
    {"oc", 0, 0},
    {"output-codec <textcodec>", I18N_NOOP("How to convert the output text."), "local"},
    {"match-case",
     I18N_NOOP("Match key fields case sensitive. UID is always matched case sensitive"), 0},
    {"+[input data]", I18N_NOOP("Input to use instead of reading stdin"), 0},
    KCmdLineLastOption
};
    
bool checkForFormatHelp(KCmdLineArgs* args, FormatFactory* factory);
bool checkForCodecHelp(KCmdLineArgs* args);

int handleKABC2Mutt(int argc, char** argv);

void avoidQPixmapWarning(QtMsgType type, const char *msg);

using std::cout;
using std::endl;

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
    KAboutData aboutData("kabcclient", I18N_NOOP("KABC client"), version,
                         I18N_NOOP("KDE addressbook commandline client"),
                         KAboutData::License_GPL_V2);

    aboutData.addAuthor("Kevin Krammer", I18N_NOOP("Primary Author"), "kevin.krammer@gmx.at");

    QString commandName = QFileInfo(QFile::decodeName(argv[0])).fileName();
    if (commandName == "kabc2mutt")
    {
        return handleKABC2Mutt(argc, argv);
    }
    
    KCmdLineArgs::addCmdLineOptions(cmdLineOptions);
    KCmdLineArgs::init(argc, argv, &aboutData);
    
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
    
    qInstallMsgHandler(avoidQPixmapWarning);
    KApplication::disableAutoDcopRegistration();
    bool gui = args->getOption("input-format") == "dialog";
    KApplication app(gui, gui);
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
        std::cerr << i18n("No operation specified, assuming --search").local8Bit() << endl;
    }
    
    KABCClient client(operation, &formatFactory);

    if (!client.setInputFormat(args->getOption("input-format")))
    {
        const QString error = i18n("Invalid input format \"%0\". See --input-format help");
        KCmdLineArgs::usage(error.arg(args->getOption("input-format")));
        return 1;
    }

    if (args->isSet("input-format-options"))
    {
        if (!client.setInputOptions(args->getOption("input-format-options")))
        {
            const QString error = i18n("Invalid options for input format \"%0\". "
                                      "See --input-format-options help");
            KCmdLineArgs::usage(error.arg(args->getOption("input-format")));
        }
    }
    
    if (!client.setOutputFormat(args->getOption("output-format")))
    {
        const QString error = i18n("Invalid output format \"%0\". See --output-format help");
        KCmdLineArgs::usage(error.arg(args->getOption("output-format")));
        return 1;
    }

    if (args->isSet("output-format-options"))
    {
        if (!client.setOutputOptions(args->getOption("output-format-options")))
        {
            const QString error = i18n("Invalid options for output format \"%0\". "
                                       "See --output-format-options help");
            KCmdLineArgs::usage(error.arg(args->getOption("output-format")));
        }
    }

    QCString codecName = args->getOption("input-codec");
    if (!args->isSet("input-codec") && args->getOption("input-format") == QCString("vcard"))
        codecName = "UTF8";
        
    if (!client.setInputCodec(codecName))
    {
        const QString error = i18n("Invalid input codec \"%0\"");
        KCmdLineArgs::usage(error.arg(codecName));
        return 1;
    }
    
    codecName = args->getOption("output-codec");
    if (!args->isSet("output-codec") && args->getOption("output-format") == QCString("vcard"))
        codecName = "UTF8";
        
    if (!client.setOutputCodec(codecName))
    {
        const QString error = i18n("Invalid output codec \"%0\"");
        KCmdLineArgs::usage(error.arg(codecName));
        return 1;
    }
    
    if (args->isSet("match-case"))
    {
        client.setMatchCaseSensitive(true);
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
            sstream << args->arg(i) << endl;
        }

        client.setInputStream(&sstream);
    }
    else
    {
        client.setInputStream(&std::cin);
    }
    
    if (!client.initOperation())
    {
        cout << i18n("Unable to perform requested operation").local8Bit() << endl;
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
        cout << i18n("The following input formats are available:").local8Bit() << endl;
        
        QCStringList formats = factory->inputFormatList();
        QCStringList::const_iterator it    = formats.begin();
        QCStringList::const_iterator endIt = formats.end();
        for (; it != endIt; ++it)
        {
            InputFormat* format = factory->inputFormat(*it);
            if (format != 0)
            {
                cout << *it << ( (*it).length() >= 8 ? "\t" : "\t\t");
                
                QString description = format->description();
                if (description.isEmpty()) description = i18n("No description available");
                
                cout << description.local8Bit() << endl;
                delete format;
            }
        }
    }
    else if (args->isSet("input-format-options") &&
             args->getOption("input-format-options") == "help")
    {
        formatHelpRequested = true;
        
        InputFormat* format = factory->inputFormat(args->getOption("input-format"));
        if (format == 0)
        {
            const QString error = i18n("Invalid input format \"%0\". See --input-format help");
            KCmdLineArgs::usage(error.arg(args->getOption("input-format")));
            exit(1);
        }

        QString usage = format->optionUsage();

        const QString message =
            (usage.isEmpty() 
                ? i18n("No options available for input format %1")
                : i18n("The following options are available for input format %1:"));
        
        cout << endl;
        cout << message.arg(args->getOption("input-format")).local8Bit() << endl;
        if (!usage.isEmpty()) cout << usage.local8Bit() << endl;
        
        delete format;
    }
    
    if (args->isSet("output-format") && args->getOption("output-format") == "help")
    {
        formatHelpRequested = true;
    
        cout << endl;        
        cout << i18n("The following output formats are available:").local8Bit() << endl;
        
        QCStringList formats = factory->outputFormatList();
        QCStringList::const_iterator it    = formats.begin();
        QCStringList::const_iterator endIt = formats.end();
        for (; it != endIt; ++it)
        {
            OutputFormat* format = factory->outputFormat(*it);
            if (format != 0)
            {
                cout << *it << ( (*it).length() >= 8 ? "\t" : "\t\t");
                
                QString description = format->description();
                if (description.isEmpty()) description = i18n("No description available");
                
                cout << description.local8Bit() << endl;
                delete format;
            }
        }
    }
    else if (args->isSet("output-format-options") &&
             args->getOption("output-format-options") == "help")
    {
        formatHelpRequested = true;
        
        OutputFormat* format = factory->outputFormat(args->getOption("output-format"));
        if (format == 0)
        {
            const QString error = i18n("Invalid output format \"%0\". See --output-format help");
            KCmdLineArgs::usage(error.arg(args->getOption("output-format")));
            exit(1);
        }

        QString usage = format->optionUsage();

        const QString message =
            (usage.isEmpty() 
                ? i18n("No options available for output format %1")
                : i18n("The following options are available for output format %1:"));
        
        cout << endl;
        cout << message.arg(args->getOption("output-format")).local8Bit() << endl;
        if (!usage.isEmpty()) cout << usage.local8Bit() << endl;
        
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
                     "universal internal format").local8Bit() << endl;
        cout << i18n("Default input encoding is 'local' unless input format is 'vcard', "
                     "in which case the default encoding will be 'utf8'.").local8Bit() << endl;
    }
    
    if (args->isSet("output-codec") && args->getOption("output-codec") == "help")
    {
        codecHelpRequested = true;
    
        cout << i18n("The output codec transforms the output text data from the "
                     "internal format to an 8-bit text format").local8Bit() << endl;
        cout << i18n("Default output encoding is 'local' unless output format is 'vcard', "
                     "in which case the default encoding will be 'utf8'.").local8Bit() << endl;
    }

    if (codecHelpRequested)
    {
        cout << i18n("Built-in codecs are UTF8 and LOCAL, respectively using "
                     "the 8-bit unicode format or your local encoding").local8Bit()
             << endl;

        cout << i18n("Other codecs can be specified by their ISO code, for "
                     "example 'ISO 8859-15' for western european languages, "
                     "including the Euro sign").local8Bit() << endl;
    }
    
    return codecHelpRequested;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static KCmdLineOptions kabc2muttCmdLineOptions[] =
{
    {"query <substring>",
     I18N_NOOP("Only show contacts where name or address matches <substring>"), 0},
    {"format <format>",
     I18N_NOOP("Default format is 'alias'. 'query' returns email<tab>name<tab>, "
               "as needed by mutt's query_command"), "alias"},
    {"alternate-key-format",
     I18N_NOOP("Default key format is 'JohDoe', this option turns it into 'jdoe'"), 0},
    {"ignore-case", I18N_NOOP("Make queries case insensitive"), 0},
    {"all-addresses", I18N_NOOP("Return all mail addresses, not just the preferred one"), 0},
    KCmdLineLastOption
};

int handleKABC2Mutt(int argc, char** argv)
{
    KAboutData aboutData("kabc2mutt", I18N_NOOP("kabc2mutt"), version,
                         I18N_NOOP("kabc - mutt converter"),
                         KAboutData::License_GPL_V2);

    aboutData.addAuthor("Tobias König",  I18N_NOOP("Primary Author"),
                        "tokoe@kde.org");
    aboutData.addAuthor("Kevin Krammer", I18N_NOOP("Contributor"), "kevin.krammer@gmx.at");
    
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

    QString options = QString::fromLocal8Bit(args->getOption("format"));
    
    if (args->isSet("alternate-key-format"))
    {
        options.append(",altkeys");
    }
    if (args->isSet("all-addresses"))
    {
        options.append(",allemails");
    }
    
    client.setMatchCaseSensitive(!args->isSet("ignore-case"));
        
    client.setOutputOptions(options.local8Bit());

    std::stringstream sstream;
    if (operation == KABCClient::Search)
    {
        sstream << args->getOption("query") << endl;
        client.setInputStream(&sstream);
    }
    
    qInstallMsgHandler(avoidQPixmapWarning);
    KApplication::disableAutoDcopRegistration();
    KApplication app(false, false);
    qInstallMsgHandler(0);
    
    if (!client.initOperation())
    {
        cout << i18n("Unable to perform requested operation").local8Bit() << endl;
        return 1;
    }

    // mutt wants a line of text before the results
    cout << i18n("Searching KDE addressbook").local8Bit() << endl;
    
    int result =  app.exec();

    // in case of no match mutt wants a line of text saying so
    if (result == 2) // Operation Search returns 2 on no match
        cout << i18n("No matches in KDE addressbook").local8Bit() << endl;
    
    return result;
}

///////////////////////////////////////////////////////////////////////////////

void avoidQPixmapWarning(QtMsgType type, const char *msg)
{
    Q_UNUSED(type)
    Q_UNUSED(msg)
}

// End of file
