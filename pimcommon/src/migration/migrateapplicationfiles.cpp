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

class PimCommon::MigrateApplicationFilesPrivate
{
public:
    MigrateApplicationFilesPrivate()
        : mMigrateApplicationVersion(1),
          mCurrentConfigVersion(0)
    {

    }

    QVector<MigrateFileInfo> mMigrateInfoList;
    QString mConfigFileName;
    QString mApplicationName;
    Kdelibs4Migration mMigration;
    int mMigrateApplicationVersion;
    int mCurrentConfigVersion;
};

MigrateApplicationFiles::MigrateApplicationFiles(QObject *parent)
    : QObject(parent),
      d(new PimCommon::MigrateApplicationFilesPrivate)
{

}

MigrateApplicationFiles::~MigrateApplicationFiles()
{
    delete d;
}

void MigrateApplicationFiles::finished()
{
    writeConfig();
    Q_EMIT migrateDone();
}

bool MigrateApplicationFiles::start()
{
    if (d->mApplicationName.isEmpty()) {
        qCDebug(PIMCOMMON_LOG) << "Missing application name";
    }
    // Testing for kdehome
    if (!d->mMigration.kdeHomeFound()) {
        finished();
        return false;
    }

    if (d->mMigrateInfoList.isEmpty()) {
        finished();
        return false;
    }

    if (d->mConfigFileName.isEmpty()) {
        qCDebug(PIMCOMMON_LOG) << " config file name not defined.";
        finished();
        return false;
    }
    return migrateConfig();
}

bool MigrateApplicationFiles::migrateConfig()
{
    qCDebug(PIMCOMMON_LOG) << "Start migration...";
    Q_FOREACH (const MigrateFileInfo &info, d->mMigrateInfoList) {
        if ((info.version() == -1) || (info.version() > d->mCurrentConfigVersion)) {
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
    return d->mApplicationName;
}

void MigrateApplicationFiles::setApplicationName(const QString &applicationName)
{
    d->mApplicationName = applicationName;
}

int MigrateApplicationFiles::currentConfigVersion() const
{
    return d->mCurrentConfigVersion;
}

void MigrateApplicationFiles::setCurrentConfigVersion(int currentConfigVersion)
{
    d->mCurrentConfigVersion = currentConfigVersion;
}

QString MigrateApplicationFiles::configFileName() const
{
    return d->mConfigFileName;
}

void MigrateApplicationFiles::setConfigFileName(const QString &configFileName)
{
    d->mConfigFileName = configFileName;
}

void MigrateApplicationFiles::writeConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig(d->mConfigFileName, KConfig::SimpleConfig);
    KConfigGroup grp = config->group(QStringLiteral("Migratekde4"));
    grp.writeEntry(QStringLiteral("Version"), d->mMigrateApplicationVersion);
    grp.sync();
}

void MigrateApplicationFiles::migrateFolder(const MigrateFileInfo &info)
{
    QString originalPath;
    QString newPath;
    if (info.type() == QLatin1String("data")) {
        originalPath = d->mMigration.locateLocal("data", info.path());
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
        originalPath = d->mMigration.locateLocal("data", info.path());
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
    return d->mMigrateApplicationVersion;
}

void MigrateApplicationFiles::setVersion(int version)
{
    d->mMigrateApplicationVersion = version;
}

bool MigrateApplicationFiles::checkIfNecessary()
{
    if (d->mConfigFileName.isEmpty()) {
        qCDebug(PIMCOMMON_LOG) << " config file name not defined.";
        return false;
    }
    KSharedConfig::Ptr config = KSharedConfig::openConfig(d->mConfigFileName, KConfig::SimpleConfig);
    if (config->hasGroup(QStringLiteral("Migratekde4"))) {
        KConfigGroup grp = config->group(QStringLiteral("Migratekde4"));
        d->mCurrentConfigVersion = grp.readEntry(QStringLiteral("Version"), 0);
        if (d->mCurrentConfigVersion < d->mMigrateApplicationVersion) {
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
        d->mMigrateInfoList.append(info);
    }
}

