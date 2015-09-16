/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include <QPointer>
#include "../src/ksieveui/scriptsparsing/xmlprintingscriptbuilder.h"
#include "../src/ksieveui/scriptsparsing/parsingresultdialog.h"

#include <ksieve/parser.h>
using KSieve::Parser;

#include <ksieve/error.h>
#include <ksieve/scriptbuilder.h>
#include <QFileDialog>
#include <QApplication>
#include <KAboutData>
#include <KLocalizedString>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "../autocreatescriptdialog.h"
#include "../sievescriptparsingerrordialog.h"
#include "PimCommon/SieveSyntaxHighlighterUtil"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    KAboutData aboutData(QStringLiteral("parsingscript_gui"), i18n("ParsingScriptTest_Gui"), QStringLiteral("1.0"));
    aboutData.setShortDescription(i18n("Test for parsing script dialog"));
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("+[url]"), i18n("URL of a sieve script to be opened")));

    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    QByteArray script;

    QString fileName;
    if (parser.positionalArguments().count()) {
        fileName = parser.positionalArguments().at(0);
    } else {
        fileName = QFileDialog::getOpenFileName(Q_NULLPTR, QString(), QString(), i18n("Sieve File (*.siv)"));
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

    KSieve::Parser sieveParser(script.begin(),
                               script.begin() + script.length());
    KSieveUi::XMLPrintingScriptBuilder psb;
    sieveParser.setScriptBuilder(&psb);
    if (sieveParser.parse()) {
        qDebug() << "ok";
    } else {
        qDebug() << "bad";
    }
    KSieveUi::ParsingResultDialog dlg;
    dlg.setResultParsing(psb.toDom().toString());
    dlg.show();

    KSieveUi::AutoCreateScriptDialog *dialog = new KSieveUi::AutoCreateScriptDialog;
    PimCommon::SieveSyntaxHighlighterUtil sieveHighlighterutil;
    const QStringList capabilities = sieveHighlighterutil.fullCapabilities();
    //Add all capabilities for testing
    dialog->setSieveCapabilities(capabilities);
    QString error;
    dialog->loadScript(psb.toDom(), error);
    if (!error.isEmpty()) {
        QPointer<KSieveUi::SieveScriptParsingErrorDialog> dlg = new KSieveUi::SieveScriptParsingErrorDialog;
        dlg->setError(QString::fromLatin1(script), error);
        dlg->exec();
        delete dlg;
    }

    dialog->show();
    app.exec();
    delete dialog;
    return 0;
}

