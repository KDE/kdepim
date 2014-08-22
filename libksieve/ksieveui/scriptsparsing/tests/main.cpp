/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "xmlprintingscriptbuilder.h"
#include "parsingresultdialog.h"

//#include <config-libksieve.h> // SIZEOF_UNSIGNED_LONG
#include <ksieve/parser.h>
using KSieve::Parser;

#include <ksieve/error.h>
#include <ksieve/scriptbuilder.h>


#include <QDebug>
#include <QFileDialog>
#include <QApplication>
#include <KAboutData>
#include <KLocalizedString>
#include <QCommandLineParser>
#include <QCommandLineOption>

int main( int argc, char** argv )
{
    KAboutData aboutData( QLatin1String("scriptsieveparsing"), i18n("ScriptSieveParsingTest_Gui"), QLatin1String("1.0"));
    aboutData.setShortDescription(i18n("Test for script sieve parsing"));
    QApplication app(argc, argv);
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("+[url]"), i18n("URL of a sieve script to be opened")));

    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);



    QByteArray script;

    QString fileName;
    if (parser.positionalArguments().count()) {
        fileName = parser.positionalArguments().at(0);
    } else {
        fileName = QFileDialog::getOpenFileName(0, QString(), QString(), QLatin1String("*.siv"));
    }
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            script = file.readAll();
        }
    } else {
        return 0;
    }
    //qDebug() << "scriptUtf8 = \"" + script +"\"";

    KSieve::Parser sieveParser( script.begin(),
                           script.begin() + script.length() );
    KSieveUi::XMLPrintingScriptBuilder psb;
    sieveParser.setScriptBuilder( &psb );
    if ( sieveParser.parse() )
        qDebug() << "ok";
    else
        qDebug() << "bad";
    KSieveUi::ParsingResultDialog dlg;
    dlg.setResultParsing(psb.toDom().toString());

    dlg.show();
    app.exec();
    return 0;
}
