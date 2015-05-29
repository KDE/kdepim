/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "migrateapplicationfilestest.h"
#include "../migrateapplicationfiles.h"
#include <qtest.h>
#include <QStandardPaths>
#include <QDebug>
#include <QDir>
#include <QTemporaryDir>

using namespace PimCommon;
MigrateApplicationFilesTest::MigrateApplicationFilesTest(QObject *parent)
    : QObject(parent)
{

}

MigrateApplicationFilesTest::~MigrateApplicationFilesTest()
{

}

void MigrateApplicationFilesTest::initTestCase()
{
    //qDebug() << " QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)" <<QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    QStandardPaths::setTestModeEnabled(true);
    const QString applicationHome = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    //qDebug() << "application data" << applicationHome;
    QDir(applicationHome).removeRecursively();
}

void MigrateApplicationFilesTest::shouldHaveDefaultValue()
{
    MigrateApplicationFiles migrate;
    QVERIFY(!migrate.start());
    QVERIFY(migrate.configFileName().isEmpty());
    QVERIFY(migrate.applicationName().isEmpty());
}

void MigrateApplicationFilesTest::shouldVerifyIfCheckIsNecessary()
{
    MigrateApplicationFiles migrate;
    //Invalid before config file is not set.
    QVERIFY(!migrate.checkIfNecessary());
    migrate.setConfigFileName(QStringLiteral("foorc"));
    // If config file doesn't exist we need to check migrate
    QVERIFY(migrate.checkIfNecessary());
}

void MigrateApplicationFilesTest::shouldNotMigrateIfKdehomeDoNotExist()
{
    qputenv("KDEHOME", "");
    MigrateApplicationFiles migrate;
    migrate.setConfigFileName(QStringLiteral("foorc"));

    QCOMPARE(migrate.start(), false);
}

void MigrateApplicationFilesTest::shouldMigrateIfKde4HomeDirExist()
{
    QTemporaryDir kdehomeDir;
    QVERIFY(kdehomeDir.isValid());
    const QString kdehome = kdehomeDir.path();
    qputenv("KDEHOME", QFile::encodeName(kdehome));
    MigrateApplicationFiles migrate;
    QCOMPARE(migrate.start(), false);

    migrate.setConfigFileName(QStringLiteral("foorc"));

    MigrateFileInfo info;
    info.setPath(QStringLiteral("foo/foo"));
    info.setType(QStringLiteral("data"));

    migrate.insertMigrateInfo(info);
    QCOMPARE(migrate.start(), true);
}

void MigrateApplicationFilesTest::shouldMigrateFolders()
{
    QTemporaryDir kdehomeDir;
    QVERIFY(kdehomeDir.isValid());
    const QString kdehome = kdehomeDir.path();
    qputenv("KDEHOME", QFile::encodeName(kdehome));

    //Generate kde4 apps dir
    const QString appsPath = kdehome + QLatin1Char('/') + QLatin1String("share/apps/foo/");
    QDir().mkpath(appsPath);
    QVERIFY(QDir(appsPath).exists());

    //TODO
}

void MigrateApplicationFilesTest::shouldMigrateFiles()
{
    QTemporaryDir kdehomeDir;
    QVERIFY(kdehomeDir.isValid());
    const QString kdehome = kdehomeDir.path();
    qputenv("KDEHOME", QFile::encodeName(kdehome));

    //Generate kde4 apps dir
    const QString appsPath = kdehome + QLatin1Char('/') + QLatin1String("share/apps/foo/");
    QDir().mkpath(appsPath);
    QVERIFY(QDir(appsPath).exists());

    const QString xdgDatahome = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/foo/");

    QStringList files;
    files << QStringLiteral("file1") << QStringLiteral("file2");
    Q_FOREACH (const QString &file, files) {
        QFile fooFile(QLatin1String(MIGRATION_DATA_DIR) + QLatin1Char('/') + file);
        QVERIFY(fooFile.exists());
        const QString storedConfigFilePath = appsPath + QLatin1Char('/') + file;
        //qDebug()<<" storedConfigFilePath"<<storedConfigFilePath;
        QVERIFY(QFile::copy(fooFile.fileName(), storedConfigFilePath));

        const QString xdgFile = xdgDatahome + file;
        QVERIFY(!QFile::exists(xdgFile));
    }
    MigrateApplicationFiles migrate;
    migrate.setConfigFileName(QStringLiteral("foorc"));

    MigrateFileInfo info;
    info.setPath(QStringLiteral("foo/file1"));
    info.setType(QStringLiteral("data"));
    migrate.insertMigrateInfo(info);

    MigrateFileInfo info2;
    info2.setPath(QStringLiteral("foo/file2"));
    info2.setType(QStringLiteral("data"));
    migrate.insertMigrateInfo(info2);

    QVERIFY(migrate.checkIfNecessary());
    QVERIFY(migrate.start());
    Q_FOREACH (const QString &file, files) {
        qDebug()<<" "<<QFile(xdgDatahome + file).fileName();
        QVERIFY(QFile(xdgDatahome + file).exists());
    }

    //TODO
}

void MigrateApplicationFilesTest::shouldMigrateFilesWithPattern()
{
    QTemporaryDir kdehomeDir;
    QVERIFY(kdehomeDir.isValid());
    const QString kdehome = kdehomeDir.path();
    qputenv("KDEHOME", QFile::encodeName(kdehome));

    //Generate kde4 apps dir
    const QString appsPath = kdehome + QLatin1Char('/') + QLatin1String("share/apps/foo/");
    QDir().mkpath(appsPath);
    QVERIFY(QDir(appsPath).exists());
    //TODO
}

void MigrateApplicationFilesTest::cleanup()
{
    QStandardPaths::setTestModeEnabled(true);
    const QString applicationHome = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    QDir applicationDir(applicationHome);
    applicationDir.removeRecursively();
    const QString configHome = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    QDir configDir(configHome);
    configDir.removeRecursively();
}

QTEST_MAIN(MigrateApplicationFilesTest)
