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

#include "exportcalendarjob.h"

#include <AkonadiCore/AgentManager>

#include <KLocalizedString>

#include <QTemporaryFile>
#include <KConfigGroup>
#include <KZip>

#include <QTimer>
#include <QFile>
#include <QDir>

#include <QStandardPaths>
#include <exportresourcearchivejob.h>

ExportCalendarJob::ExportCalendarJob(QObject *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep)
    : AbstractImportExportJob(parent, archiveStorage, typeSelected, numberOfStep),
      mIndexIdentifier(0)
{
}

ExportCalendarJob::~ExportCalendarJob()
{

}

void ExportCalendarJob::start()
{
    Q_EMIT title(i18n("Start export KOrganizer settings..."));
    createProgressDialog(i18n("Export KOrganizer settings"));
    if (mTypeSelected & Utils::Resources) {
        QTimer::singleShot(0, this, SLOT(slotCheckBackupResource()));
    } else if (mTypeSelected & Utils::Config) {
        QTimer::singleShot(0, this, SLOT(slotCheckBackupConfig()));
    } else {
        Q_EMIT jobFinished();
    }
}

void ExportCalendarJob::slotCheckBackupResource()
{
    setProgressDialogLabel(i18n("Backing up resources..."));
    increaseProgressDialog();
    QTimer::singleShot(0, this, SLOT(slotWriteNextArchiveResource()));
}

void ExportCalendarJob::slotCheckBackupConfig()
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

void ExportCalendarJob::slotCalendarJobTerminated()
{
    if (wasCanceled()) {
        Q_EMIT jobFinished();
        return;
    }
    mIndexIdentifier++;
    QTimer::singleShot(0, this, SLOT(slotWriteNextArchiveResource()));
}

void ExportCalendarJob::slotWriteNextArchiveResource()
{
    Akonadi::AgentManager *manager = Akonadi::AgentManager::self();
    const Akonadi::AgentInstance::List list = manager->instances();
    if (mIndexIdentifier < list.count()) {
        Akonadi::AgentInstance agent = list.at(mIndexIdentifier);
        const QString identifier = agent.identifier();
        if (identifier.contains(QStringLiteral("akonadi_icaldir_resource_"))) {
            const QString archivePath = Utils::calendarPath() + identifier + QDir::separator();

            QString url = Utils::resourcePath(agent);
            if (!mAgentPaths.contains(url)) {
                mAgentPaths << url;
                if (!url.isEmpty()) {
                    ExportResourceArchiveJob *resourceJob = new ExportResourceArchiveJob(this);
                    resourceJob->setArchivePath(archivePath);
                    resourceJob->setUrl(url);
                    resourceJob->setIdentifier(identifier);
                    resourceJob->setArchive(archive());
                    resourceJob->setArchiveName(QStringLiteral("calendar.zip"));
                    connect(resourceJob, &ExportResourceArchiveJob::error, this, &ExportCalendarJob::error);
                    connect(resourceJob, &ExportResourceArchiveJob::info, this, &ExportCalendarJob::info);
                    connect(resourceJob, &ExportResourceArchiveJob::terminated, this, &ExportCalendarJob::slotCalendarJobTerminated);
                    resourceJob->start();
                }
            } else {
                QTimer::singleShot(0, this, SLOT(slotCalendarJobTerminated()));
            }
        } else if (identifier.contains(QStringLiteral("akonadi_ical_resource_"))) {
            backupResourceFile(agent, Utils::calendarPath());
            QTimer::singleShot(0, this, SLOT(slotCalendarJobTerminated()));
        } else {
            QTimer::singleShot(0, this, SLOT(slotCalendarJobTerminated()));
        }
    } else {
        Q_EMIT info(i18n("Resources backup done."));
        QTimer::singleShot(0, this, SLOT(slotCheckBackupConfig()));
    }
}

void ExportCalendarJob::backupConfig()
{
    setProgressDialogLabel(i18n("Backing up config..."));

    const QString korganizerStr(QStringLiteral("korganizerrc"));
    const QString korganizerrc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + korganizerStr;
    if (QFile(korganizerrc).exists()) {
        KSharedConfigPtr korganizer = KSharedConfig::openConfig(korganizerrc);

        QTemporaryFile tmp;
        tmp.open();

        KConfig *korganizerConfig = korganizer->copyTo(tmp.fileName());

        const QString globalCollectionsStr(QStringLiteral("GlobalCollectionSelection"));
        if (korganizerConfig->hasGroup(globalCollectionsStr)) {
            KConfigGroup group = korganizerConfig->group(globalCollectionsStr);
            const QString selectionKey(QStringLiteral("Selection"));
            Utils::convertCollectionListToRealPath(group, selectionKey);
        }

        korganizerConfig->sync();
        backupFile(tmp.fileName(), Utils::configsPath(), korganizerStr);
        delete korganizerConfig;
    }

    backupConfigFile(QStringLiteral("calendar_printing.rc"));
    backupConfigFile(QStringLiteral("korgacrc"));

    const QString freebusyurlsStr(QStringLiteral("korganizer/freebusyurls"));
    const QString freebusyurls = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + freebusyurlsStr;
    if (QFile(freebusyurls).exists()) {
        backupFile(freebusyurls, Utils::dataPath(), freebusyurlsStr);
    }

    const QString templateDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/korganizer/templates/");
    QDir templateDirectory(templateDir);
    if (templateDirectory.exists()) {
        const bool templateDirAdded = archive()->addLocalDirectory(templateDir, Utils::dataPath() +  QLatin1String("/korganizer/templates/"));
        if (!templateDirAdded) {
            Q_EMIT error(i18n("\"%1\" directory cannot be added to backup file.", templateDir));
        }
    }
    backupUiRcFile(QStringLiteral("korganizerui.rc"), QStringLiteral("korganizer"));
    backupUiRcFile(QStringLiteral("korganizer_part.rc"), QStringLiteral("korganizer"));
    Q_EMIT info(i18n("Config backup done."));
}

