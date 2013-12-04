/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>
  
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

#include "exportjotjob.h"

#include "messageviewer/utils/kcursorsaver.h"

#include <Akonadi/AgentManager>

#include <KLocale>
#include <KStandardDirs>
#include <KTemporaryFile>
#include <KConfigGroup>
#include <KZip>

#include <QWidget>
#include <QFile>
#include <QDir>

ExportJotJob::ExportJotJob(QWidget *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage,int numberOfStep)
    : AbstractImportExportJob(parent, archiveStorage, typeSelected, numberOfStep)
{
}

ExportJotJob::~ExportJotJob()
{

}

void ExportJotJob::start()
{
    Q_EMIT title(i18n("Start export KJots settings..."));
    mArchiveDirectory = archive()->directory();
    if (mTypeSelected & Utils::Resources) {
        backupResources();
        increaseProgressDialog();
        if (wasCanceled()) {
            Q_EMIT jobFinished();
            return;
        }
    }
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

void ExportJotJob::backupResources()
{
    showInfo(i18n("Backing up resources..."));
    MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );

    Akonadi::AgentManager *manager = Akonadi::AgentManager::self();
    const Akonadi::AgentInstance::List list = manager->instances();
    foreach( const Akonadi::AgentInstance &agent, list ) {
        const QString identifier = agent.identifier();
        if (identifier.contains(QLatin1String("akonadi_akonotes_resource_"))) {
            const QString archivePath = Utils::jotPath() + identifier + QDir::separator();
            KUrl url = Utils::resourcePath(agent);
            if (!url.isEmpty()) {
                const bool fileAdded = backupFullDirectory(url, archivePath, QLatin1String("job.zip"));
                if (fileAdded) {
                    const QString errorStr = Utils::storeResources(archive(), identifier, archivePath);
                    if (!errorStr.isEmpty())
                        Q_EMIT error(errorStr);
                    url = Utils::akonadiAgentConfigPath(identifier);
                    if (!url.isEmpty()) {
                        const QString filename = url.fileName();
                        const bool fileAdded  = archive()->addLocalFile(url.path(), archivePath + filename);
                        if (fileAdded)
                            Q_EMIT info(i18n("\"%1\" was backuped.",filename));
                        else
                            Q_EMIT error(i18n("\"%1\" file cannot be added to backup file.",filename));
                    }
                }
            }
        }
    }

    Q_EMIT info(i18n("Resources backup done."));
}

void ExportJotJob::backupConfig()
{
    showInfo(i18n("Backing up config..."));
    MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
    const QString kjotStr(QLatin1String("kjotrc"));
    const QString kjotrc = KStandardDirs::locateLocal( "config", kjotStr);
    if (QFile(kjotrc).exists()) {
        KSharedConfigPtr kjot = KSharedConfig::openConfig(kjotrc);

        KTemporaryFile tmp;
        tmp.open();

        KConfig *kjotConfig = kjot->copyTo( tmp.fileName() );

        const QString collectionsStr(QLatin1String("TreeState"));
        if (kjotConfig->hasGroup(collectionsStr)) {
            KConfigGroup group = kjotConfig->group(collectionsStr);
            const QString selectionKey(QLatin1String("Expansion"));
            Utils::convertCollectionIdsToRealPath(group, selectionKey);
        }

        kjotConfig->sync();
        backupFile(tmp.fileName(), Utils::configsPath(), kjotStr);
        delete kjotConfig;
    }


    Q_EMIT info(i18n("Config backup done."));

}

