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

#include <KFileDialog>
#include <KCmdLineOptions>
#include <KApplication>
#include <KUrl>

#include <QDebug>
#include <QFileDialog>

int main( int argc, char** argv )
{
    KCmdLineArgs::init(argc, argv, "scriptsieveparsing", 0, ki18n("ScriptSieveParsingTest_Gui"),
                       "1.0", ki18n("Test for script sieve parsing"));

    KCmdLineOptions option;
    option.add("+[url]", ki18n("URL of a sieve script to be opened"));
    KCmdLineArgs::addCmdLineOptions(option);


    KApplication app;

    QByteArray script;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    QString fileName;
    if (args->count()) {
        fileName = args->url(0).path();
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

    KSieve::Parser parser( script.begin(),
                           script.begin() + script.length() );
    KSieveUi::XMLPrintingScriptBuilder psb;
    parser.setScriptBuilder( &psb );
    if ( parser.parse() )
        qDebug() << "ok";
    else
        qDebug() << "bad";
    KSieveUi::ParsingResultDialog dlg;
    dlg.setResultParsing(psb.toDom().toString());

    dlg.show();
    app.exec();
    return 0;
}
