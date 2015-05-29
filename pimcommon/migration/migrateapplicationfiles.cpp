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

#include "migrateapplicationfiles.h"
#include "pimcommon_debug.h"
#include <kdelibs4migration.h>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QDir>

using namespace PimCommon;

MigrateApplicationFiles::MigrateApplicationFiles(QObject *parent)
    : QObject(parent),
      mMigrateApplicationVersion(1),
      mCurrentConfigVersion(0)
{

}

MigrateApplicationFiles::~MigrateApplicationFiles()
{

}

void MigrateApplicationFiles::finished()
{
    writeConfig();
    Q_EMIT migrateDone();
}

bool MigrateApplicationFiles::start()
{
    if (mApplicationName.isEmpty()) {
        qCDebug(PIMCOMMON_LOG) << "Missing application name";
    }
    // Testing for kdehome
    if (!mMigration.kdeHomeFound()) {
        finished();
        return false;
    }

    if (mMigrateInfoList.isEmpty()) {
        finished();
        return false;
    }

    if (mConfigFileName.isEmpty()) {
        qCDebug(PIMCOMMON_LOG) << " config file name not defined.";
        finished();
        return false;
    }
    return migrateConfig();
}

bool MigrateApplicationFiles::migrateConfig()
{
    Q_FOREACH(const MigrateFileInfo &info, mMigrateInfoList) {
        if ((info.version() == -1) || (info.version() > mCurrentConfigVersion)) {
            if (info.folder()) {
                migrateFolder(info);
            } else {
                migrateFile(info);
            }
        }
    }
    finished();
    return true;
}

QString MigrateApplicationFiles::applicationName() const
{
    return mApplicationName;
}

void MigrateApplicationFiles::setApplicationName(const QString &applicationName)
{
    mApplicationName = applicationName;
}

int MigrateApplicationFiles::currentConfigVersion() const
{
    return mCurrentConfigVersion;
}

void MigrateApplicationFiles::setCurrentConfigVersion(int currentConfigVersion)
{
    mCurrentConfigVersion = currentConfigVersion;
}


QString MigrateApplicationFiles::configFileName() const
{
    return mConfigFileName;
}

void MigrateApplicationFiles::setConfigFileName(const QString &configFileName)
{
    mConfigFileName = configFileName;
}

void MigrateApplicationFiles::writeConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig(mConfigFileName, KConfig::SimpleConfig);
    KConfigGroup grp = config->group(QStringLiteral("Migratekde4"));
    grp.writeEntry(QStringLiteral("Version"), mMigrateApplicationVersion);
    grp.sync();
}

void MigrateApplicationFiles::migrateFolder(const MigrateFileInfo &info)
{
}

void MigrateApplicationFiles::migrateFile(const MigrateFileInfo &info)
{
    if (info.filePattern().isEmpty()) {
        QString originalPath;
        QString newPath;
        if (info.type() == QStringLiteral("data")) {
            originalPath = mMigration.locateLocal("data", info.path());
            newPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + info.path();
            QFileInfo fileInfo(newPath);
            QDir().mkpath(fileInfo.absolutePath());
        } else {
            qCDebug(PIMCOMMON_LOG) << "Type not supported: " << info.type();
        }
        if (!originalPath.isEmpty()) {
            QFile newFile(newPath);
            if (!newFile.exists()) {
                QFile copyFile(originalPath);
                copyFile.copy(newPath);
            }
        }
    } else {

    }
}

int MigrateApplicationFiles::version() const
{
    return mMigrateApplicationVersion;
}

void MigrateApplicationFiles::setVersion(int version)
{
    mMigrateApplicationVersion = version;
}

bool MigrateApplicationFiles::checkIfNecessary()
{
    if (mConfigFileName.isEmpty()) {
        qCDebug(PIMCOMMON_LOG) << " config file name not defined.";
        return false;
    }
    KSharedConfig::Ptr config = KSharedConfig::openConfig(mConfigFileName, KConfig::SimpleConfig);
    if (config->hasGroup(QStringLiteral("Migratekde4"))) {
        KConfigGroup grp = config->group(QStringLiteral("Migratekde4"));
        mCurrentConfigVersion = grp.readEntry(QStringLiteral("Version"), 0);
        if (mCurrentConfigVersion < mMigrateApplicationVersion) {
            return true;
        } else {
            return false;
        }
    }
    return true;
}

void MigrateApplicationFiles::insertMigrateInfo(const MigrateFileInfo &info)
{
    if (info.isValid()) {
        mMigrateInfoList.append(info);
    }
}

