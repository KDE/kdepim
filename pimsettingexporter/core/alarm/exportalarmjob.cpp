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

#include "exportalarmjob.h"

#include "Libkdepim/KCursorSaver"

#include <AkonadiCore/AgentManager>

#include <KLocalizedString>

#include <QTemporaryFile>
#include <QTimer>
#include <KConfigGroup>
#include <KZip>

#include <QWidget>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <exportresourcearchivejob.h>

ExportAlarmJob::ExportAlarmJob(QObject *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep)
    : AbstractImportExportJob(parent, archiveStorage, typeSelected, numberOfStep),
      mIndexIdentifier(0)
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
        QTimer::singleShot(0, this, SLOT(slotCheckBackupResource()));
    } else if (mTypeSelected & Utils::Config) {
        QTimer::singleShot(0, this, SLOT(slotCheckBackupConfig()));
    } else {
        Q_EMIT jobFinished();
    }
}

void ExportAlarmJob::slotCheckBackupResource()
{
    showInfo(i18n("Backing up resources..."));
    //TODO verify it.
    KPIM::KCursorSaver busy(KPIM::KBusyPtr::busy());
    increaseProgressDialog();
    QTimer::singleShot(0, this, SLOT(slotWriteNextArchiveResource()));
}

void ExportAlarmJob::slotCheckBackupConfig()
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

void ExportAlarmJob::slotAlarmJobTerminated()
{
    if (wasCanceled()) {
        Q_EMIT jobFinished();
        return;
    }
    mIndexIdentifier++;
    QTimer::singleShot(0, this, SLOT(slotWriteNextArchiveResource()));
}

void ExportAlarmJob::slotWriteNextArchiveResource()
{
    Akonadi::AgentManager *manager = Akonadi::AgentManager::self();
    const Akonadi::AgentInstance::List list = manager->instances();
    if (mIndexIdentifier < list.count()) {
        Akonadi::AgentInstance agent = list.at(mIndexIdentifier);
        const QString identifier = agent.identifier();
        if (identifier.contains(QStringLiteral("akonadi_kalarm_dir_resource_"))) {
            const QString archivePath = Utils::alarmPath() + identifier + QDir::separator();

            QString url = Utils::resourcePath(agent);
            if (!mAgentPaths.contains(url)) {
                mAgentPaths << url;
                if (!url.isEmpty()) {
                    ExportResourceArchiveJob *resourceJob = new ExportResourceArchiveJob(this);
                    resourceJob->setArchivePath(archivePath);
                    resourceJob->setUrl(url);
                    resourceJob->setIdentifier(identifier);
                    resourceJob->setArchive(archive());
                    resourceJob->setArchiveName(QStringLiteral("alarm.zip"));
                    connect(resourceJob, &ExportResourceArchiveJob::error, this, &ExportAlarmJob::error);
                    connect(resourceJob, &ExportResourceArchiveJob::info, this, &ExportAlarmJob::info);
                    connect(resourceJob, &ExportResourceArchiveJob::terminated, this, &ExportAlarmJob::slotAlarmJobTerminated);
                    resourceJob->start();
                }
            } else {
                QTimer::singleShot(0, this, SLOT(slotAlarmJobTerminated()));
            }
        } else if (identifier.contains(QStringLiteral("akonadi_kalarm_resource_"))) {
            backupResourceFile(agent, Utils::alarmPath());
            QTimer::singleShot(0, this, SLOT(slotAlarmJobTerminated()));
        } else {
            QTimer::singleShot(0, this, SLOT(slotAlarmJobTerminated()));
        }
    } else {
        Q_EMIT info(i18n("Resources backup done."));
        QTimer::singleShot(0, this, SLOT(slotCheckBackupConfig()));
    }
}


void ExportAlarmJob::backupConfig()
{
    showInfo(i18n("Backing up config..."));
    KPIM::KCursorSaver busy(KPIM::KBusyPtr::busy());
    const QString kalarmStr(QStringLiteral("kalarmrc"));
    const QString kalarmrc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + kalarmStr;
    if (QFile(kalarmrc).exists()) {
        KSharedConfigPtr kalarm = KSharedConfig::openConfig(kalarmrc);

        QTemporaryFile tmp;
        tmp.open();

        KConfig *kalarmConfig = kalarm->copyTo(tmp.fileName());

        const QString collectionsStr(QStringLiteral("Collections"));
        if (kalarmConfig->hasGroup(collectionsStr)) {
            KConfigGroup group = kalarmConfig->group(collectionsStr);
            const QString selectionKey(QStringLiteral("FavoriteCollectionIds"));
            Utils::convertCollectionIdsToRealPath(group, selectionKey);
        }

        kalarmConfig->sync();
        backupFile(tmp.fileName(), Utils::configsPath(), kalarmStr);
        delete kalarmConfig;
    }

    Q_EMIT info(i18n("Config backup done."));
}

