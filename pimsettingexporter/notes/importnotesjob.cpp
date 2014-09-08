/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "pimcommon/util/createresource.h"

#include <KArchive>
#include <KLocalizedString>

#include <KZip>
#include <KConfigGroup>

#include <QFile>
#include <QStandardPaths>
#include <QDir>

static const QString storeNote = QLatin1String("backupnote/");

ImportNotesJob::ImportNotesJob(QWidget *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep)
    : AbstractImportExportJob(parent, archiveStorage, typeSelected, numberOfStep)
{
    initializeImportJob();
}

ImportNotesJob::~ImportNotesJob()
{
}

void ImportNotesJob::start()
{
    Q_EMIT title(i18n("Start import KNotes settings..."));
    mArchiveDirectory = archive()->directory();
    if (mTypeSelected & Utils::Config) {
        restoreConfig();
    }
    if (mTypeSelected & Utils::Data) {
        restoreData();
    }
    Q_EMIT jobFinished();
}

void ImportNotesJob::restoreConfig()
{
    const QString knotesStr(QLatin1String("knotesrc"));
    restoreConfigFile(knotesStr);
    if (archiveVersion() <= 1) {
        const QString globalNoteSettingsStr(QLatin1String("globalnotesettings"));
        restoreConfigFile(globalNoteSettingsStr);
    } else {
        const QString globalNoteStr(QLatin1String("globalnotesettings"));
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
}

void ImportNotesJob::restoreData()
{
    if (archiveVersion() <= 1) {
        //Knote < knote-akonadi
        const KArchiveEntry *notesEntry  = mArchiveDirectory->entry(Utils::dataPath() + QLatin1String("knotes/"));
        if (notesEntry && notesEntry->isDirectory()) {
            const QString notesPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + QLatin1String("knotes/");
            overwriteDirectory(notesPath, notesEntry);
        }
    } else {
        restoreResources();
    }
    Q_EMIT info(i18n("Data restored."));
}

void ImportNotesJob::restoreResources()
{
    Q_EMIT info(i18n("Restore resources..."));
    QStringList listResource;
    if (!mListResourceFile.isEmpty()) {
        QDir dir(mTempDirName);
        dir.mkdir(Utils::jotPath());
        const QString copyToDirName(mTempDirName + QLatin1Char('/') + Utils::notePath());

        for (int i = 0; i < mListResourceFile.size(); ++i) {
            resourceFiles value = mListResourceFile.at(i);
            QMap<QString, QVariant> settings;
            if (value.akonadiConfigFile.contains(QLatin1String("akonadi_akonotes_resource_"))) {
                const KArchiveEntry *fileResouceEntry = mArchiveDirectory->entry(value.akonadiConfigFile);
                if (fileResouceEntry && fileResouceEntry->isFile()) {
                    const KArchiveFile *file = static_cast<const KArchiveFile *>(fileResouceEntry);
                    file->copyTo(copyToDirName);
                    QString resourceName(file->name());

                    QString filename(file->name());
                    //TODO adapt filename otherwise it will use all the time the same filename.
                    qDebug() << " filename :" << filename;

                    KSharedConfig::Ptr resourceConfig = KSharedConfig::openConfig(copyToDirName + QLatin1Char('/') + resourceName);

                    const QUrl newUrl = Utils::adaptResourcePath(resourceConfig, storeNote);

                    const QString dataFile = value.akonadiResources;
                    const KArchiveEntry *dataResouceEntry = mArchiveDirectory->entry(dataFile);
                    if (dataResouceEntry->isFile()) {
                        const KArchiveFile *file = static_cast<const KArchiveFile *>(dataResouceEntry);
                        //TODO  adapt directory name too
                        extractZipFile(file, copyToDirName, newUrl.path());
                    }
                    settings.insert(QLatin1String("Path"), newUrl.path());

                    const QString agentConfigFile = value.akonadiAgentConfigFile;
                    if (!agentConfigFile.isEmpty()) {
                        const KArchiveEntry *akonadiAgentConfigEntry = mArchiveDirectory->entry(agentConfigFile);
                        if (akonadiAgentConfigEntry->isFile()) {
                            const KArchiveFile *file = static_cast<const KArchiveFile *>(akonadiAgentConfigEntry);
                            file->copyTo(copyToDirName);
                            resourceName = file->name();
                            KSharedConfig::Ptr akonadiAgentConfig = KSharedConfig::openConfig(copyToDirName + QLatin1Char('/') + resourceName);
                            filename = Utils::akonadiAgentName(akonadiAgentConfig);
                        }
                    }

                    const QString newResource = mCreateResource->createResource(QString::fromLatin1("akonadi_akonotes_resource"), filename, settings, true);
                    infoAboutNewResource(newResource);
                    qDebug() << " newResource" << newResource;
                    listResource << newResource;
                }
            }
        }
    }
    //It's maildir support. Need to add support
    startSynchronizeResources(listResource);
}

void ImportNotesJob::importKNoteGlobalSettings(const KArchiveFile *kmailsnippet, const QString &kmail2rc, const QString &filename, const QString &prefix)
{
    copyToFile(kmailsnippet, kmail2rc, filename, prefix);
    KSharedConfig::Ptr kmailConfig = KSharedConfig::openConfig(kmail2rc);

    const QString composerStr(QLatin1String("SelectNoteFolder"));
    if (kmailConfig->hasGroup(composerStr)) {
        KConfigGroup composerGroup = kmailConfig->group(composerStr);
        const QString previousStr(QLatin1String("DefaultFolder"));
        convertRealPathToCollection(composerGroup, previousStr);
    }
    kmailConfig->sync();
}
