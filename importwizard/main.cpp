/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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
#include <KDBusService>
#include <KLocalizedString>
#include <Kdelibs4ConfigMigrator>
#include <QApplication>

#include "importwizard.h"

#include "kdepim-version.h"

#include <stdio.h>

int main(int argc, char *argv[])
{
    Kdelibs4ConfigMigrator migrate(QLatin1String("importwizard"));
    migrate.setConfigFiles(QStringList() << QLatin1String("importwizardrc"));
    migrate.migrate();

    KLocalizedString::setApplicationDomain("importwizard");
    //FIXME: "wizards" are "assistents" in new KDE slang
    KAboutData aboutData(QStringLiteral("importwizard"),
                         i18n("PIM Import Tool"),
                         QLatin1String(KDEPIM_VERSION),
                         i18n("PIM Import Tool"),
                         KAboutLicense::GPL_V2,
                         i18n("Copyright © 2012-2013-2014 importwizard authors"));

    aboutData.addAuthor(i18n("Laurent Montel"), i18n("Maintainer"), QStringLiteral("montel@kde.org"));
    aboutData.setProgramIconName(QLatin1String("kontact-import-wizard"));
    aboutData.setOrganizationDomain(QByteArray("kde.org"));
    aboutData.setProductName(QByteArray("importwizard"));

    KAboutData::setApplicationData(aboutData);

    QApplication app(argc, argv);

    KDBusService service();

    ImportWizard *wizard = new ImportWizard();
    wizard->show();
    int ret = app.exec();
    delete wizard;
    return ret;
}
