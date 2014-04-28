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
#include <QDebug>

#include "libksieve/ksieveui/autocreatescripts/autocreatescriptdialog.h"
#include "pimcommon/sievehighlighter/sievesyntaxhighlighterutil.h"

int main (int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, "autocreatescripts_gui", 0, ki18n("AutoCreateScripttest_Gui"),
                       "1.0", ki18n("Test for autocreate script dialog"));
    KApplication app;
    KSieveUi::AutoCreateScriptDialog *dialog = new KSieveUi::AutoCreateScriptDialog;
    QStringList capabilities = PimCommon::SieveSyntaxHighlighterUtil::fullCapabilities();
    //Add all capabilities for testing
    dialog->setSieveCapabilities(capabilities);
    if (dialog->exec() ) {
        QString requires;
        const QString script = dialog->script(requires);
        qDebug()<<" generated script :\n"<<requires<<"\n"<<script;
    }
    delete dialog;
    return 0;
}

