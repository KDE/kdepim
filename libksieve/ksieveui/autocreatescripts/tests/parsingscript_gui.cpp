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

#include <qdebug.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <KFileDialog>
#include <QDebug>
#include <QPointer>
#include <QUrl>
#include "xmlprintingscriptbuilder.h"
#include "parsingresultdialog.h"

//#include <config-libksieve.h> // SIZEOF_UNSIGNED_LONG
#include <ksieve/parser.h>
using KSieve::Parser;

#include <ksieve/error.h>
#include <ksieve/scriptbuilder.h>
#include <QFileDialog>


#include "libksieve/ksieveui/autocreatescripts/autocreatescriptdialog.h"
#include "libksieve/ksieveui/autocreatescripts/sievescriptparsingerrordialog.h"
#include "pimcommon/sievehighlighter/sievesyntaxhighlighterutil.h"

int main (int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, "parsingscript_gui", 0, ki18n("ParsingScriptTest_Gui"), "1.0", ki18n("Test for parsing script dialog"));

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

    KSieveUi::AutoCreateScriptDialog *dialog = new KSieveUi::AutoCreateScriptDialog;
    QStringList capabilities = PimCommon::SieveSyntaxHighlighterUtil::fullCapabilities();
    //Add all capabilities for testing
    dialog->setSieveCapabilities(capabilities);
    QString error;
    dialog->loadScript(psb.toDom(), error);
    if (!error.isEmpty()) {
        QPointer<SieveScriptParsingErrorDialog> dlg = new SieveScriptParsingErrorDialog;
        dlg->setError(QString::fromLatin1(script), error);
        dlg->exec();
        delete dlg;
    }

    dialog->show();
    app.exec();
    delete dialog;
    return 0;
}

