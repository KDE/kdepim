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

#include "exportalarmjob.h"

#include "messageviewer/utils/kcursorsaver.h"

#include <AkonadiCore/AgentManager>

#include <KLocalizedString>

#include <QTemporaryFile>
#include <KConfigGroup>
#include <KZip>

#include <QWidget>
#include <QFile>
#include <QDir>
#include <QStandardPaths>

ExportAlarmJob::ExportAlarmJob(QWidget *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep)
    : AbstractImportExportJob(parent, archiveStorage, typeSelected, numberOfStep)
{
}

ExportAlarmJob::~ExportAlarmJob()
{

}

void ExportAlarmJob::start()
{
    Q_EMIT title(i18n("Start export KAlarm settings..."));
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

void ExportAlarmJob::backupResources()
{
    showInfo(i18n("Backing up resources..."));
    MessageViewer::KCursorSaver busy(MessageViewer::KBusyPtr::busy());

    Akonadi::AgentManager *manager = Akonadi::AgentManager::self();
    const Akonadi::AgentInstance::List list = manager->instances();
    Q_FOREACH (const Akonadi::AgentInstance &agent, list) {
        const QString identifier = agent.identifier();
        if (identifier.contains(QLatin1String("akonadi_kalarm_resource_"))) {
            backupResourceFile(agent, Utils::alarmPath());
        } else if (identifier.contains(QLatin1String("akonadi_kalarm_dir_resource_"))) {
            const QString archivePath = Utils::alarmPath() + identifier + QDir::separator();

            QUrl url = Utils::resourcePath(agent);
            if (!url.isEmpty()) {
                const bool fileAdded = backupFullDirectory(url, archivePath, QLatin1String("alarm.zip"));
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
                            Q_EMIT info(i18n("\"%1\" was backed up.", filename));
                        } else {
                            Q_EMIT error(i18n("\"%1\" file cannot be added to backup file.", filename));
                        }
                    }
                }
            }
        }
    }

    Q_EMIT info(i18n("Resources backup done."));
}

void ExportAlarmJob::backupConfig()
{
    showInfo(i18n("Backing up config..."));
    MessageViewer::KCursorSaver busy(MessageViewer::KBusyPtr::busy());
    const QString kalarmStr(QLatin1String("kalarmrc"));
    const QString kalarmrc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + kalarmStr;
    if (QFile(kalarmrc).exists()) {
        KSharedConfigPtr kalarm = KSharedConfig::openConfig(kalarmrc);

        QTemporaryFile tmp;
        tmp.open();

        KConfig *kalarmConfig = kalarm->copyTo(tmp.fileName());

        const QString collectionsStr(QLatin1String("Collections"));
        if (kalarmConfig->hasGroup(collectionsStr)) {
            KConfigGroup group = kalarmConfig->group(collectionsStr);
            const QString selectionKey(QLatin1String("FavoriteCollectionIds"));
            Utils::convertCollectionIdsToRealPath(group, selectionKey);
        }

        kalarmConfig->sync();
        backupFile(tmp.fileName(), Utils::configsPath(), kalarmStr);
        delete kalarmConfig;
    }

    Q_EMIT info(i18n("Config backup done."));

}

