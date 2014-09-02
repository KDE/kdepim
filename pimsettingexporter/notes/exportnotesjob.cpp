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

#include "exportnotesjob.h"

#include "messageviewer/utils/kcursorsaver.h"

#include <AkonadiCore/AgentManager>

#include <KLocalizedString>
#include <KZip>
#include <QTemporaryFile>
#include <KConfigGroup>

#include <QWidget>
#include <QDir>
#include <QStandardPaths>

ExportNotesJob::ExportNotesJob(QWidget *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep)
    : AbstractImportExportJob(parent, archiveStorage, typeSelected, numberOfStep)
{
}

ExportNotesJob::~ExportNotesJob()
{

}

void ExportNotesJob::start()
{
    Q_EMIT title(i18n("Start export KNotes settings..."));
    mArchiveDirectory = archive()->directory();
    if (mTypeSelected & Utils::Config) {
        backupConfig();
        increaseProgressDialog();
        if (wasCanceled()) {
            Q_EMIT jobFinished();
            return;
        }
    }
    if (mTypeSelected & Utils::Data) {
        backupData();
        increaseProgressDialog();
        if (wasCanceled()) {
            Q_EMIT jobFinished();
            return;
        }
    }
    Q_EMIT jobFinished();
}

void ExportNotesJob::backupConfig()
{
    showInfo(i18n("Backing up config..."));
    MessageViewer::KCursorSaver busy(MessageViewer::KBusyPtr::busy());
    const QString knotesStr(QLatin1String("knotesrc"));
    const QString knotesrc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + knotesStr;
    backupFile(knotesrc, Utils::configsPath(), knotesStr);

    const QString globalNoteSettingsStr(QLatin1String("globalnotesettings"));
    const QString globalNoteSettingsrc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + globalNoteSettingsStr;

    if (QFile(globalNoteSettingsrc).exists()) {
        KSharedConfigPtr globalnotesettingsrc = KSharedConfig::openConfig(globalNoteSettingsrc);

        QTemporaryFile tmp;
        tmp.open();

        KConfig *knoteConfig = globalnotesettingsrc->copyTo(tmp.fileName());
        const QString selectFolderNoteStr(QLatin1String("SelectNoteFolder"));
        if (knoteConfig->hasGroup(selectFolderNoteStr)) {
            KConfigGroup selectFolderNoteGroup = knoteConfig->group(selectFolderNoteStr);

            const QString selectFolderNoteGroupStr(QLatin1String("DefaultFolder"));
            Utils::convertCollectionIdsToRealPath(selectFolderNoteGroup, selectFolderNoteGroupStr);
        }
        knoteConfig->sync();
        backupFile(tmp.fileName(), Utils::configsPath(), globalNoteSettingsStr);
        delete knoteConfig;
    }
    Q_EMIT info(i18n("Config backup done."));
}

void ExportNotesJob::backupData()
{
    showInfo(i18n("Backing up data..."));
    MessageViewer::KCursorSaver busy(MessageViewer::KBusyPtr::busy());

#if 0  //Code for knote <knote-akonadi
    const QString icsfileStr = QLatin1String("notes.ics");
    const QString icsfile = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/knotes/") + icsfileStr ;

    backupFile(icsfile, Utils::dataPath() +  QLatin1String("/knotes/"), icsfileStr);

    const QString notesDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/knotes/notes/") ;
    QDir notesDirectory(notesDir);
    if (notesDirectory.exists()) {
        const bool notesDirAdded = archive()->addLocalDirectory(notesDir, Utils::dataPath() +  QLatin1String("/knotes/notes/"));
        if (!notesDirAdded) {
            Q_EMIT error(i18n("\"%1\" directory cannot be added to backup file.", notesDir));
        }
    }
#endif
    const QString notesThemeDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/knotes/print/") ;

    Akonadi::AgentManager *manager = Akonadi::AgentManager::self();
    const Akonadi::AgentInstance::List list = manager->instances();
    foreach (const Akonadi::AgentInstance &agent, list) {
        const QString identifier = agent.identifier();
        if (identifier.contains(QLatin1String("akonadi_akonotes_resource_"))) {
            const QString archivePath = Utils::notePath() + identifier + QDir::separator();
            KUrl url = Utils::resourcePath(agent);
            if (!url.isEmpty()) {
                const bool fileAdded = backupFullDirectory(url, archivePath, QLatin1String("notes.zip"));
                if (fileAdded) {
                    const QString errorStr = Utils::storeResources(archive(), identifier, archivePath);
                    if (!errorStr.isEmpty()) {
                        Q_EMIT error(errorStr);
                    }
                    url = Utils::akonadiAgentConfigPath(identifier);
                    if (!url.isEmpty()) {
                        const QString filename = url.fileName();
                        const bool fileAdded  = archive()->addLocalFile(url.path(), archivePath + filename);
                        if (fileAdded) {
                            Q_EMIT info(i18n("\"%1\" was backuped.", filename));
                        } else {
                            Q_EMIT error(i18n("\"%1\" file cannot be added to backup file.", filename));
                        }
                    }
                }
            }
        }
    }

    QDir notesThemeDirectory(notesThemeDir);
    if (notesThemeDirectory.exists()) {
        const bool notesDirAdded = archive()->addLocalDirectory(notesThemeDir, Utils::dataPath() +  QLatin1String("/knotes/print"));
        if (!notesDirAdded) {
            Q_EMIT error(i18n("\"%1\" directory cannot be added to backup file.", notesThemeDir));
        }
    }
    Q_EMIT info(i18n("Data backup done."));
}

