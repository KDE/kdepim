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


#include <kaboutdata.h>
#include <klocale.h>
#include <QApplication>
#include <KFileDialog>
#include "mboxmainwindow.h"

#include <KLocalizedString>
#include <QFileDialog>
#include <qcommandlineparser.h>
#include <qcommandlineoption.h>
#include "kdepim-version.h"

int main(int argc, char *argv[])
{
    KLocalizedString::setApplicationDomain("mboximporter");

    QApplication app(argc, argv);
    KAboutData aboutData( QStringLiteral("mboximporter"),
                          i18n("MBox importer tool"),
                          QLatin1String(KDEPIM_VERSION),
                          i18n("Messageviewer Header Theme Editor"),
                          KAboutLicense::GPL_V2,
                          i18n("Copyright © 2013 MBoxImporter authors"));
    aboutData.addAuthor(i18n("Laurent Montel"), i18n("Maintainer"), QStringLiteral("montel@kde.org"));

    aboutData.setProgramIconName(QLatin1String("kmail"));
    aboutData.setOrganizationDomain(QByteArray("kde.org"));
    aboutData.setProductName(QByteArray("mboximporter"));

    KAboutData::setApplicationData(aboutData);

    QString fileName;
    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);

    parser.addPositionalArgument(QStringLiteral("url"), i18n("URL of mbox to be imported"), QStringLiteral("[url]"));

    parser.process(app);
    aboutData.processCommandLine(&parser);

    const QStringList &args = parser.positionalArguments();
    if (!args.isEmpty()) {
        fileName = args.first();
    } else {
        fileName = QFileDialog::getOpenFileName(); 
    }
    if (fileName.isEmpty()) {
        return 0;
    }
    MBoxMainWindow *w = new MBoxMainWindow(fileName);
    w->show();
    int ret = app.exec();
    delete w;
    return ret;
}
