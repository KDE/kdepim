/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  based on code from Sune Vuorela <sune@vuorela.dk> (Rawatar source code)

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

#include "kaddressbookmigrateapplication.h"

#include <Kdelibs4ConfigMigrator>

KAddressBookMigrateApplication::KAddressBookMigrateApplication()
{
    initializeMigrator();
}

void KAddressBookMigrateApplication::migrate()
{
    // Migrate to xdg.
    Kdelibs4ConfigMigrator migrate(QLatin1String("kaddressbook"));
    migrate.setConfigFiles(QStringList() << QStringLiteral("kaddressbookrc"));
    migrate.setUiFiles(QStringList() << QStringLiteral("kaddressbookui.rc"));
    migrate.migrate();

    // Migrate folders and files.
    if (mMigrator.checkIfNecessary()) {
        mMigrator.start();
    }
}

void KAddressBookMigrateApplication::initializeMigrator()
{
    const int currentVersion = 1;
    mMigrator.setApplicationName(QStringLiteral("kaddressbook"));
    mMigrator.setConfigFileName(QStringLiteral("kaddressbookrc"));
    mMigrator.setCurrentConfigVersion(currentVersion);

    // printing
    PimCommon::MigrateFileInfo migrateInfoPrinting;
    migrateInfoPrinting.setFolder(true);
    migrateInfoPrinting.setType(QStringLiteral("apps"));
    migrateInfoPrinting.setPath(QStringLiteral("kaddressbook/printing"));
    mMigrator.insertMigrateInfo(migrateInfoPrinting);

    // viewertemplates
    PimCommon::MigrateFileInfo migrateInfoViewerTemplates;
    migrateInfoViewerTemplates.setFolder(true);
    migrateInfoViewerTemplates.setType(QStringLiteral("apps"));
    migrateInfoViewerTemplates.setPath(QStringLiteral("kaddressbook/viewertemplates"));
    mMigrator.insertMigrateInfo(migrateInfoViewerTemplates);

    // viewertemplates
    PimCommon::MigrateFileInfo migrateInfoCsvTemplates;
    migrateInfoCsvTemplates.setFolder(true);
    migrateInfoCsvTemplates.setType(QStringLiteral("apps"));
    migrateInfoCsvTemplates.setPath(QStringLiteral("kaddressbook/csv-templates"));
    mMigrator.insertMigrateInfo(migrateInfoCsvTemplates);

    //TODO add folder to migrate
}

