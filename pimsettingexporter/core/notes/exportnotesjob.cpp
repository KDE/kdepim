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

#include "exportnotesjob.h"

#include "Libkdepim/KCursorSaver"

#include <AkonadiCore/AgentManager>

#include <KLocalizedString>
#include <KZip>
#include <QTemporaryFile>
#include <KConfigGroup>

#include <QWidget>
#include <QDir>
#include <QTimer>
#include <QStandardPaths>
#include <exportresourcearchivejob.h>

ExportNotesJob::ExportNotesJob(QObject *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep)
    : AbstractImportExportJob(parent, archiveStorage, typeSelected, numberOfStep),
      mIndexIdentifier(0)
{
}

ExportNotesJob::~ExportNotesJob()
{

}

void ExportNotesJob::start()
{
    Q_EMIT title(i18n("Start export KNotes settings..."));
    createProgressDialog(i18n("Export KNotes settings"));
    if (mTypeSelected & Utils::Data) {
        QTimer::singleShot(0, this, SLOT(slotCheckBackupResource()));
    } else if (mTypeSelected & Utils::Config) {
        QTimer::singleShot(0, this, SLOT(slotCheckBackupConfig()));
    } else {
        Q_EMIT jobFinished();
    }
}

void ExportNotesJob::backupTheme()
{
    const QString notesThemeDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/knotes/print/");
    QDir notesThemeDirectory(notesThemeDir);
    if (notesThemeDirectory.exists()) {
        const bool notesDirAdded = archive()->addLocalDirectory(notesThemeDir, Utils::dataPath() +  QLatin1String("/knotes/print"));
        if (!notesDirAdded) {
            Q_EMIT error(i18n("\"%1\" directory cannot be added to backup file.", notesThemeDir));
        }
    }
}

void ExportNotesJob::slotCheckBackupResource()
{
    setProgressDialogLabel(i18n("Backing up resources..."));
    //TODO verify it.
    KPIM::KCursorSaver busy(KPIM::KBusyPtr::busy());
    increaseProgressDialog();
    backupTheme();

    QTimer::singleShot(0, this, SLOT(slotWriteNextArchiveResource()));
}

void ExportNotesJob::slotCheckBackupConfig()
{
    if (mTypeSelected & Utils::Config) {
        backupConfig();
        increaseProgressDialog();
        if (wasCanceled()) {
            Q_EMIT jobFinished();
            return;
        }
    }
    Q_EMIT jobFinished();
}

void ExportNotesJob::slotAddressbookJobTerminated()
{
    if (wasCanceled()) {
        Q_EMIT jobFinished();
        return;
    }
    mIndexIdentifier++;
    QTimer::singleShot(0, this, SLOT(slotWriteNextArchiveResource()));
}

void ExportNotesJob::slotWriteNextArchiveResource()
{
    Akonadi::AgentManager *manager = Akonadi::AgentManager::self();
    const Akonadi::AgentInstance::List list = manager->instances();
    if (mIndexIdentifier < list.count()) {
        Akonadi::AgentInstance agent = list.at(mIndexIdentifier);
        const QString identifier = agent.identifier();
        if (identifier.contains(QStringLiteral("akonadi_akonotes_resource_"))) {
            const QString archivePath = Utils::notePath() + identifier + QDir::separator();

            QString url = Utils::resourcePath(agent);
            if (!mAgentPaths.contains(url)) {
                mAgentPaths << url;
                if (!url.isEmpty()) {
                    ExportResourceArchiveJob *resourceJob = new ExportResourceArchiveJob(this);
                    resourceJob->setArchivePath(archivePath);
                    resourceJob->setUrl(url);
                    resourceJob->setIdentifier(identifier);
                    resourceJob->setArchive(archive());
                    resourceJob->setArchiveName(QStringLiteral("notes.zip"));
                    connect(resourceJob, &ExportResourceArchiveJob::error, this, &ExportNotesJob::error);
                    connect(resourceJob, &ExportResourceArchiveJob::info, this, &ExportNotesJob::info);
                    connect(resourceJob, &ExportResourceArchiveJob::terminated, this, &ExportNotesJob::slotAddressbookJobTerminated);
                    resourceJob->start();
                }
            } else {
                QTimer::singleShot(0, this, SLOT(slotAddressbookJobTerminated()));
            }
        } else {
            QTimer::singleShot(0, this, SLOT(slotAddressbookJobTerminated()));
        }
    } else {
        Q_EMIT info(i18n("Resources backup done."));
        QTimer::singleShot(0, this, SLOT(slotCheckBackupConfig()));
    }
}

void ExportNotesJob::backupConfig()
{
    setProgressDialogLabel(i18n("Backing up config..."));
    KPIM::KCursorSaver busy(KPIM::KBusyPtr::busy());
    const QString knotesStr(QStringLiteral("knotesrc"));
    const QString knotesrc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + knotesStr;
    backupFile(knotesrc, Utils::configsPath(), knotesStr);

    const QString globalNoteSettingsStr(QStringLiteral("globalnotesettings"));
    const QString globalNoteSettingsrc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + globalNoteSettingsStr;

    if (QFile(globalNoteSettingsrc).exists()) {
        KSharedConfigPtr globalnotesettingsrc = KSharedConfig::openConfig(globalNoteSettingsrc);

        QTemporaryFile tmp;
        tmp.open();

        KConfig *knoteConfig = globalnotesettingsrc->copyTo(tmp.fileName());
        const QString selectFolderNoteStr(QStringLiteral("SelectNoteFolder"));
        if (knoteConfig->hasGroup(selectFolderNoteStr)) {
            KConfigGroup selectFolderNoteGroup = knoteConfig->group(selectFolderNoteStr);

            const QString selectFolderNoteGroupStr(QStringLiteral("DefaultFolder"));
            Utils::convertCollectionIdsToRealPath(selectFolderNoteGroup, selectFolderNoteGroupStr);
        }
        knoteConfig->sync();
        backupFile(tmp.fileName(), Utils::configsPath(), globalNoteSettingsStr);
        delete knoteConfig;
    }
    Q_EMIT info(i18n("Config backup done."));
}
