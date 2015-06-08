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
#include <QDebug>

KAddressBookMigrateApplication::KAddressBookMigrateApplication()
{
    initializeMigrator();
}

void KAddressBookMigrateApplication::migrate()
{
    // Migrate to xdg.
    Kdelibs4ConfigMigrator migrate(QStringLiteral("kaddressbook"));
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
    const int currentVersion = 2;
    mMigrator.setApplicationName(QStringLiteral("kaddressbook"));
    mMigrator.setConfigFileName(QStringLiteral("kaddressbookrc"));

    // To migrate we need a version > currentVersion
    const int initialVersion = currentVersion + 1;

    // printing
    PimCommon::MigrateFileInfo migrateInfoPrinting;
    migrateInfoPrinting.setFolder(true);
    migrateInfoPrinting.setType(QStringLiteral("data"));
    migrateInfoPrinting.setPath(QStringLiteral("kaddressbook/printing"));
    migrateInfoPrinting.setVersion(initialVersion);
    mMigrator.insertMigrateInfo(migrateInfoPrinting);

    // viewertemplates
    PimCommon::MigrateFileInfo migrateInfoViewerTemplates;
    migrateInfoViewerTemplates.setFolder(true);
    migrateInfoViewerTemplates.setType(QStringLiteral("data"));
    migrateInfoViewerTemplates.setPath(QStringLiteral("kaddressbook/viewertemplates"));
    migrateInfoViewerTemplates.setVersion(initialVersion);
    mMigrator.insertMigrateInfo(migrateInfoViewerTemplates);

    // viewertemplates
    PimCommon::MigrateFileInfo migrateInfoCsvTemplates;
    migrateInfoCsvTemplates.setFolder(true);
    migrateInfoCsvTemplates.setType(QStringLiteral("data"));
    migrateInfoCsvTemplates.setPath(QStringLiteral("kaddressbook/csv-templates"));
    migrateInfoCsvTemplates.setVersion(initialVersion);
    mMigrator.insertMigrateInfo(migrateInfoCsvTemplates);

    mMigrator.setCurrentConfigVersion(currentVersion);

    //TODO add folder to migrate
}

