/*
  Copyright (c) 2015-2016 Montel Laurent <montel@kde.org>

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

#include "blogilomigrateapplication.h"

#include <Kdelibs4ConfigMigrator>

BlogiloMigrateApplication::BlogiloMigrateApplication()
{
    initializeMigrator();
}

void BlogiloMigrateApplication::migrate()
{
    Kdelibs4ConfigMigrator migrate(QStringLiteral("blogilo"));
    migrate.setConfigFiles(QStringList() << QStringLiteral("blogilorc"));
    migrate.setUiFiles(QStringList() << QStringLiteral("blogiloui.rc"));
    migrate.migrate();
    // Migrate folders and files.
    if (mMigrator.checkIfNecessary()) {
        mMigrator.start();
    }

}

void BlogiloMigrateApplication::initializeMigrator()
{
    const int currentVersion = 2;
    mMigrator.setApplicationName(QStringLiteral("blogilo"));
    mMigrator.setConfigFileName(QStringLiteral("blogilorc"));
    mMigrator.setCurrentConfigVersion(currentVersion);

    // To migrate we need a version < currentVersion
    const int initialVersion = currentVersion + 1;

    // Database
    PimCommon::MigrateFileInfo migrateInfoDatabase;
    migrateInfoDatabase.setFolder(false);
    migrateInfoDatabase.setType(QStringLiteral("data"));
    migrateInfoDatabase.setPath(QStringLiteral("blogilo/blogilo.db"));
    migrateInfoDatabase.setVersion(initialVersion);
    mMigrator.insertMigrateInfo(migrateInfoDatabase);

}

