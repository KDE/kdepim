/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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

#include "importalarmjob.h"
#include "archivestorage.h"

#include "PimCommon/CreateResource"

#include <KArchive>
#include <KLocalizedString>
#include <KConfigGroup>
#include <KZip>
#include <KArchiveEntry>

#include <QDir>
#include <QFile>
#include <QStandardPaths>

namespace
{
inline const QString storeAlarm()
{
    return QStringLiteral("backupalarm/");
}
}

ImportAlarmJob::ImportAlarmJob(QObject *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep)
    : AbstractImportExportJob(parent, archiveStorage, typeSelected, numberOfStep)
{
    initializeImportJob();
}

ImportAlarmJob::~ImportAlarmJob()
{

}

void ImportAlarmJob::start()
{
    Q_EMIT title(i18n("Start import KAlarm settings..."));
    mArchiveDirectory = archive()->directory();
    searchAllFiles(mArchiveDirectory, QString());
    initializeListStep();
    nextStep();
}

void ImportAlarmJob::nextStep()
{
    ++mIndex;
    if (mIndex < mListStep.count()) {
        const Utils::StoredType type = mListStep.at(mIndex);
        if (type == Utils::Resources) {
            restoreResources();
        } else if (type == Utils::Config) {
            restoreConfig();
        }
    } else {
        Q_EMIT jobFinished();
    }
}

void ImportAlarmJob::restoreResources()
{
    Q_EMIT info(i18n("Restore resources..."));
    QStringList listResource;
    listResource << restoreResourceFile(QStringLiteral("akonadi_kalarm_resource"), Utils::alarmPath(), storeAlarm(), false);
    if (!mListResourceFile.isEmpty()) {
        QDir dir(mTempDirName);
        dir.mkdir(Utils::alarmPath());
        const QString copyToDirName(mTempDirName + QLatin1Char('/') + Utils::alarmPath());

        const int numberOfResourceFile = mListResourceFile.size();
        for (int i = 0; i < numberOfResourceFile; ++i) {
            resourceFiles value = mListResourceFile.at(i);
            QMap<QString, QVariant> settings;
            if (value.akonadiConfigFile.contains(QStringLiteral("akonadi_kalarm_dir_resource_"))) {
                const KArchiveEntry *fileResouceEntry = mArchiveDirectory->entry(value.akonadiConfigFile);
                if (fileResouceEntry && fileResouceEntry->isFile()) {
                    const KArchiveFile *file = static_cast<const KArchiveFile *>(fileResouceEntry);
                    file->copyTo(copyToDirName);
                    QString resourceName(file->name());

                    QString filename(file->name());
                    //TODO adapt filename otherwise it will use all the time the same filename.
                    qCDebug(PIMSETTINGEXPORTERCORE_LOG) << " filename :" << filename;

                    KSharedConfig::Ptr resourceConfig = KSharedConfig::openConfig(copyToDirName + QLatin1Char('/') + resourceName);

                    const QString newUrl = Utils::adaptResourcePath(resourceConfig, storeAlarm());
                    const QString dataFile = value.akonadiResources;
                    const KArchiveEntry *dataResouceEntry = mArchiveDirectory->entry(dataFile);
                    if (dataResouceEntry->isFile()) {
                        const KArchiveFile *file = static_cast<const KArchiveFile *>(dataResouceEntry);
                        //TODO  adapt directory name too
                        extractZipFile(file, copyToDirName, newUrl);
                    }
                    settings.insert(QStringLiteral("Path"), newUrl);

                    const QString agentConfigFile = value.akonadiAgentConfigFile;
                    if (!agentConfigFile.isEmpty()) {
                        const KArchiveEntry *akonadiAgentConfigEntry = mArchiveDirectory->entry(agentConfigFile);
                        if (akonadiAgentConfigEntry->isFile()) {
                            const KArchiveFile *file = static_cast<const KArchiveFile *>(akonadiAgentConfigEntry);
                            file->copyTo(copyToDirName);
                            resourceName = file->name();
                            filename = Utils::akonadiAgentName(copyToDirName + QLatin1Char('/') + resourceName);
                        }
                    }

                    const QString newResource = mCreateResource->createResource(QStringLiteral("akonadi_kalarm_dir_resource"), filename, settings, true);
                    infoAboutNewResource(newResource);
                    listResource << newResource;
                    qCDebug(PIMSETTINGEXPORTERCORE_LOG) << " newResource" << newResource;
                }
            }
        }
    }
    //It's maildir support. Need to add support
    startSynchronizeResources(listResource);
}

void ImportAlarmJob::addSpecificResourceSettings(KSharedConfig::Ptr resourceConfig, const QString &resourceName, QMap<QString, QVariant> &settings)
{

}

void ImportAlarmJob::searchAllFiles(const KArchiveDirectory *dir, const QString &prefix)
{
    Q_FOREACH (const QString &entryName, dir->entries()) {
        const KArchiveEntry *entry = dir->entry(entryName);
        if (entry && entry->isDirectory()) {
            const QString newPrefix = (prefix.isEmpty() ? prefix : prefix + QLatin1Char('/')) + entryName;
            if (entryName == QLatin1String("alarm")) {
                storeAlarmArchiveResource(static_cast<const KArchiveDirectory *>(entry), entryName);
            } else {
                searchAllFiles(static_cast<const KArchiveDirectory *>(entry), newPrefix);
            }
        }
    }
}

void ImportAlarmJob::storeAlarmArchiveResource(const KArchiveDirectory *dir, const QString &prefix)
{
    Q_FOREACH (const QString &entryName, dir->entries()) {
        const KArchiveEntry *entry = dir->entry(entryName);
        if (entry && entry->isDirectory()) {
            const KArchiveDirectory *resourceDir = static_cast<const KArchiveDirectory *>(entry);
            const QStringList lst = resourceDir->entries();

            if (lst.count() >= 2) {
                const QString archPath(prefix + QLatin1Char('/') + entryName + QLatin1Char('/'));
                resourceFiles files;
                Q_FOREACH (const QString &name, lst) {
                    if (name.endsWith(QLatin1String("rc")) && (name.contains(QStringLiteral("akonadi_kalarm_resource_"))
                            || name.contains(QStringLiteral("akonadi_kalarm_dir_resource_")))) {
                        files.akonadiConfigFile = archPath + name;
                    } else if (name.startsWith(Utils::prefixAkonadiConfigFile())) {
                        files.akonadiAgentConfigFile = archPath + name;
                    } else {
                        files.akonadiResources = archPath + name;
                    }
                }
                files.debug();
                mListResourceFile.append(files);
            } else {
                qCDebug(PIMSETTINGEXPORTERCORE_LOG) << " Problem in archive. number of file " << lst.count();
            }
        }
    }
}

void ImportAlarmJob::restoreConfig()
{
    const QString kalarmStr(QStringLiteral("kalarmrc"));
    const KArchiveEntry *kalarmrcentry  = mArchiveDirectory->entry(Utils::configsPath() + kalarmStr);
    if (kalarmrcentry && kalarmrcentry->isFile()) {
        const KArchiveFile *kalarmrcFile = static_cast<const KArchiveFile *>(kalarmrcentry);
        const QString kalarmrc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + kalarmStr;
        if (QFile(kalarmrc).exists()) {
            if (overwriteConfigMessageBox(kalarmStr)) {
                importkalarmConfig(kalarmrcFile, kalarmrc, kalarmStr, Utils::configsPath());
            }
        } else {
            importkalarmConfig(kalarmrcFile, kalarmrc, kalarmStr, Utils::configsPath());
        }
    }
    Q_EMIT info(i18n("Config restored."));
    nextStep();
}

void ImportAlarmJob::importkalarmConfig(const KArchiveFile *kalarmFile, const QString &kalarmrc, const QString &filename, const QString &prefix)
{
    copyToFile(kalarmFile, kalarmrc, filename, prefix);
    KSharedConfig::Ptr kalarmConfig = KSharedConfig::openConfig(kalarmrc);

    const QString collectionsStr(QStringLiteral("Collections"));
    if (kalarmConfig->hasGroup(collectionsStr)) {
        KConfigGroup group = kalarmConfig->group(collectionsStr);
        const QString selectionKey(QStringLiteral("FavoriteCollectionIds"));
        convertRealPathToCollectionList(group, selectionKey, false);
    }

    kalarmConfig->sync();
}

