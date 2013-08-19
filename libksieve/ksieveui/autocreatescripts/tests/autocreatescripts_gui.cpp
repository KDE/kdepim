/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include <kdebug.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <QDebug>

#include "libksieve/ksieveui/autocreatescripts/autocreatescriptdialog.h"

int main (int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, "autocreatescripts_gui", 0, ki18n("AutoCreateScripttest_Gui"),
                       "1.0", ki18n("Test for autocreate script dialog"));
    KApplication app;
    KSieveUi::AutoCreateScriptDialog *dialog = new KSieveUi::AutoCreateScriptDialog;
    QStringList capabilities;
    //Add all capabilities for testing
    capabilities
            <<QLatin1String("body")
            <<QLatin1String("envelope")
            <<QLatin1String("redirect")
            <<QLatin1String("fileinto")
            <<QLatin1String("editheader")
            <<QLatin1String("reject")
            <<QLatin1String("imapflags")
            <<QLatin1String("enotify")
            <<QLatin1String("date")
            <<QLatin1String("copy")
            <<QLatin1String("mailbox")
            <<QLatin1String("spamtest")
            <<QLatin1String("spamtestplus")
            <<QLatin1String("virustest")
            <<QLatin1String("vacation")
            <<QLatin1String("vacation-seconds")
            <<QLatin1String("ihave")
            <<QLatin1String("subaddress")
            <<QLatin1String("environment")
            <<QLatin1String("imap4flags")
            <<QLatin1String("enclose")
            <<QLatin1String("replace")
            <<QLatin1String("include")
            <<QLatin1String("extracttext")
            <<QLatin1String("metadata")
            <<QLatin1String("convert")
            <<QLatin1String("foreverypart")
            <<QLatin1String("variables")
            <<QLatin1String("servermetadata")
            <<QLatin1String("comparator-i;ascii-numeric");
    dialog->setSieveCapabilities(capabilities);
    if (dialog->exec() ) {
        QString requires;
        const QString script = dialog->script(requires);
        qDebug()<<" generated script :\n"<<requires<<"\n"<<script;
    }
    delete dialog;
    return 0;
}

