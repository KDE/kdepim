/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include "importnotesjob.h"
#include "archivestorage.h"

#include "PimCommon/CreateResource"

#include <KArchive>
#include <KLocalizedString>

#include <KZip>
#include <KConfigGroup>

#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QTimer>

namespace
{
inline const QString backupnote()
{
    return QStringLiteral("backupnote/");
}
}

ImportNotesJob::ImportNotesJob(QObject *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep)
    : AbstractImportExportJob(parent, archiveStorage, typeSelected, numberOfStep)
{
    initializeImportJob();
}

ImportNotesJob::~ImportNotesJob()
{
}

void ImportNotesJob::slotNextStep()
{
    ++mIndex;
    if (mIndex < mListStep.count()) {
        const Utils::StoredType type = mListStep.at(mIndex);
        if (type == Utils::Resources) {
            restoreResources();
        } else if (type == Utils::Config) {
            restoreConfig();
        } else if (type == Utils::Data) {
            restoreData();
        } else {
            qCDebug(PIMSETTINGEXPORTERCORE_LOG) << Q_FUNC_INFO << " not supported type "<< type;
            slotNextStep();
        }
    } else {
        Q_EMIT jobFinished();
    }
}


void ImportNotesJob::start()
{
    Q_EMIT title(i18n("Start import KNotes settings..."));
    mArchiveDirectory = archive()->directory();
    // FIXME search archive ? searchAllFiles(mArchiveDirectory, QString());
    createProgressDialog(i18n("Import KNotes settings"));
    initializeListStep();
    QTimer::singleShot(0, this, &ImportNotesJob::slotNextStep);
}

void ImportNotesJob::restoreConfig()
{
    increaseProgressDialog();
    setProgressDialogLabel(i18n("Restore configs..."));
    const QString knotesStr(QStringLiteral("knotesrc"));
    restoreConfigFile(knotesStr);
    if (archiveVersion() <= 1) {
        const QString globalNoteSettingsStr(QStringLiteral("globalnotesettings"));
        restoreConfigFile(globalNoteSettingsStr);
    } else {
        const QString globalNoteStr(QStringLiteral("globalnotesettings"));
        const KArchiveEntry *globalNotecentry  = mArchiveDirectory->entry(Utils::configsPath() + globalNoteStr);
        if (globalNotecentry && globalNotecentry->isFile()) {
            const KArchiveFile *globalNotecentryrc = static_cast<const KArchiveFile *>(globalNotecentry);
            const QString globalNoterc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + globalNoteStr;
            if (QFile(globalNoterc).exists()) {
                if (overwriteConfigMessageBox(globalNoteStr)) {
                    importKNoteGlobalSettings(globalNotecentryrc, globalNoterc, globalNoteStr, Utils::configsPath());
                }
            } else {
                importKNoteGlobalSettings(globalNotecentryrc, globalNoterc, globalNoteStr, Utils::configsPath());
            }
        }

    }

    Q_EMIT info(i18n("Config restored."));
    QTimer::singleShot(0, this, &ImportNotesJob::slotNextStep);
}

void ImportNotesJob::restoreData()
{
    increaseProgressDialog();
    setProgressDialogLabel(i18n("Restore data..."));
    if (archiveVersion() <= 1) {
        //Knote < knote-akonadi
        const KArchiveEntry *notesEntry  = mArchiveDirectory->entry(Utils::dataPath() + QLatin1String("knotes/"));
        if (notesEntry && notesEntry->isDirectory()) {
            const QString notesPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + QLatin1String("knotes/");
            overwriteDirectory(notesPath, notesEntry);
        }
        QTimer::singleShot(0, this, &ImportNotesJob::slotNextStep);
    } else {
        restoreResources();
    }
    Q_EMIT info(i18n("Data restored."));
}

void ImportNotesJob::restoreResources()
{
    Q_EMIT info(i18n("Restore resources..."));
    setProgressDialogLabel(i18n("Restore resouces..."));
    QStringList listResource;
    if (!mListResourceFile.isEmpty()) {
        QDir dir(mTempDirName);
        dir.mkdir(Utils::jotPath());
        const QString copyToDirName(mTempDirName + QLatin1Char('/') + Utils::notePath());
        const int numberOfResourceFile = mListResourceFile.size();
        for (int i = 0; i < numberOfResourceFile; ++i) {
            resourceFiles value = mListResourceFile.at(i);
            QMap<QString, QVariant> settings;
            if (value.akonadiConfigFile.contains(QStringLiteral("akonadi_akonotes_resource_"))) {
                const KArchiveEntry *fileResouceEntry = mArchiveDirectory->entry(value.akonadiConfigFile);
                if (fileResouceEntry && fileResouceEntry->isFile()) {
                    const KArchiveFile *file = static_cast<const KArchiveFile *>(fileResouceEntry);
                    file->copyTo(copyToDirName);
                    QString resourceName(file->name());

                    QString filename(file->name());
                    //TODO adapt filename otherwise it will use all the time the same filename.
                    qCDebug(PIMSETTINGEXPORTERCORE_LOG) << " filename :" << filename;

                    KSharedConfig::Ptr resourceConfig = KSharedConfig::openConfig(copyToDirName + QLatin1Char('/') + resourceName);

                    const QString newUrl = Utils::adaptResourcePath(resourceConfig, backupnote());

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

                    const QString newResource = mCreateResource->createResource(QStringLiteral("akonadi_akonotes_resource"), filename, settings, true);
                    infoAboutNewResource(newResource);
                    qCDebug(PIMSETTINGEXPORTERCORE_LOG) << " newResource" << newResource;
                    listResource << newResource;
                }
            }
        }
    }
    //It's maildir support. Need to add support
    startSynchronizeResources(listResource);
}

void ImportNotesJob::importKNoteGlobalSettings(const KArchiveFile *archive, const QString &configrc, const QString &filename, const QString &prefix)
{
    copyToFile(archive, configrc, filename, prefix);
    KSharedConfig::Ptr kmailConfig = KSharedConfig::openConfig(configrc);

    const QString composerStr(QStringLiteral("SelectNoteFolder"));
    if (kmailConfig->hasGroup(composerStr)) {
        KConfigGroup composerGroup = kmailConfig->group(composerStr);
        const QString previousStr(QStringLiteral("DefaultFolder"));
        convertRealPathToCollection(composerGroup, previousStr);
    }
    kmailConfig->sync();
}
