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
    qCDebug(PIMCOMMON_LOG) << "Start migration...";
    Q_FOREACH (const MigrateFileInfo &info, mMigrateInfoList) {
        if ((info.version() == -1) || (info.version() > mCurrentConfigVersion)) {
            if (info.folder()) {
                migrateFolder(info);
            } else {
                migrateFile(info);
            }
        }
    }
    qCDebug(PIMCOMMON_LOG) << "Migration finished.";
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
    QString originalPath;
    QString newPath;
    if (info.type() == QLatin1String("data")) {
        originalPath = mMigration.locateLocal("data", info.path());
        newPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + info.path();
        QFileInfo fileInfo(newPath);
        QDir().mkpath(fileInfo.absolutePath());
    } else {
        qCDebug(PIMCOMMON_LOG) << "Type not supported: " << info.type();
    }
    if (!originalPath.isEmpty()) {
        copyRecursively(originalPath, newPath);
    }
}

bool MigrateApplicationFiles::copyRecursively(const QString &srcFilePath, const QString &tgtFilePath)
{
    QFileInfo srcFileInfo(srcFilePath);
    if (srcFileInfo.isDir()) {
        QDir targetDir(tgtFilePath);
        targetDir.cdUp();
        if (!targetDir.mkpath(QFileInfo(tgtFilePath).path())) {
            return false;
        }
        QDir sourceDir(srcFilePath);
        const QStringList fileNames = sourceDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        Q_FOREACH (const QString &fileName, fileNames) {
            const QString newSrcFilePath = srcFilePath + QLatin1Char('/') + fileName;
            const QString newTgtFilePath = tgtFilePath + QLatin1Char('/') + fileName;
            if (!copyRecursively(newSrcFilePath, newTgtFilePath)) {
                return false;
            }
        }
    } else {
        if (!QDir().mkpath(QFileInfo(tgtFilePath).absolutePath())) {
            qCDebug(PIMCOMMON_LOG) << "Can not create path " << srcFileInfo.absolutePath();
            return false;
        }
        if (!QFile(tgtFilePath).exists() && !QFile::copy(srcFilePath, tgtFilePath)) {
            qCDebug(PIMCOMMON_LOG) << " can't copy" << srcFilePath << " tgtFilePath" << tgtFilePath;
            return false;
        }
    }
    return true;
}

void MigrateApplicationFiles::migrateFile(const MigrateFileInfo &info)
{
    QString originalPath;
    QString newPath;
    if (info.type() == QLatin1String("data")) {
        originalPath = mMigration.locateLocal("data", info.path());
        newPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + info.path();
        QFileInfo fileInfo(newPath);
        QDir().mkpath(fileInfo.absolutePath());
    } else {
        qCDebug(PIMCOMMON_LOG) << "Type not supported: " << info.type();
    }

    if (!originalPath.isEmpty()) {
        if (info.filePatterns().isEmpty()) {
            QFile newFile(newPath);
            if (!newFile.exists()) {
                QFile copyFile(originalPath);
                if (!copyFile.copy(newPath)) {
                    qCDebug(PIMCOMMON_LOG) << "impossible to copy " << originalPath << " to " << newPath;
                }
            }
        } else {
            QDir sourceDir(originalPath);
            const QStringList fileNames = sourceDir.entryList(info.filePatterns(), QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
            Q_FOREACH (const QString &file, fileNames) {
                QFile copyFile(originalPath + QLatin1Char('/') + file);
                if (!copyFile.copy(newPath + QLatin1Char('/') + file)) {
                    qCDebug(PIMCOMMON_LOG) << "impossible to copy " << copyFile.fileName() << " to " << newPath;
                }
            }
        }
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

