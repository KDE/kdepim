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

#include <QDebug>
#include <QApplication>
#include <KAboutData>
#include <KLocalizedString>
#include <QCommandLineParser>

#include "libksieve/ksieveui/autocreatescripts/autocreatescriptdialog.h"
#include "pimcommon/sievehighlighter/sievesyntaxhighlighterutil.h"

int main(int argc, char **argv)
{
    KAboutData aboutData(QStringLiteral("autocreatescripts_gui"), i18n("AutoCreateScripttest_Gui"), QStringLiteral("1.0"));
    aboutData.setShortDescription(i18n("Test for autocreate script dialog"));
    QApplication app(argc, argv);
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);
    KSieveUi::AutoCreateScriptDialog *dialog = new KSieveUi::AutoCreateScriptDialog;
    QStringList capabilities = PimCommon::SieveSyntaxHighlighterUtil::fullCapabilities();
    //Add all capabilities for testing
    dialog->setSieveCapabilities(capabilities);
    if (dialog->exec()) {
        QString requires;
        const QString script = dialog->script(requires);
        qDebug() << " generated script :\n" << requires << "\n" << script;
    }
    delete dialog;
    return 0;
}

