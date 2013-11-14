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

#include "exportcalendarjob.h"

#include "messageviewer/utils/kcursorsaver.h"

#include <Akonadi/AgentManager>

#include <KLocale>
#include <KStandardDirs>
#include <KTemporaryFile>
#include <KConfigGroup>
#include <KZip>

#include <QFile>
#include <QDir>
#include <QWidget>


ExportCalendarJob::ExportCalendarJob(QWidget *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage,int numberOfStep)
    :AbstractImportExportJob(parent, archiveStorage, typeSelected, numberOfStep)
{
}

ExportCalendarJob::~ExportCalendarJob()
{

}

void ExportCalendarJob::start()
{
    Q_EMIT title(i18n("Start export korganizer settings..."));
    mArchiveDirectory = archive()->directory();
    createProgressDialog();

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


void ExportCalendarJob::backupResources()
{
    showInfo(i18n("Backing up resources..."));
    MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );

    Akonadi::AgentManager *manager = Akonadi::AgentManager::self();
    const Akonadi::AgentInstance::List list = manager->instances();
    foreach( const Akonadi::AgentInstance &agent, list ) {
        const QString identifier = agent.identifier();
        if (identifier.contains(QLatin1String("akonadi_icaldir_resource_"))) {
            const QString archivePath = Utils::calendarPath() + identifier + QDir::separator();

            KUrl url = Utils::resourcePath(agent);
            if (!mAgentPaths.contains(url.path())) {
                if (!url.isEmpty()) {
                    mAgentPaths << url.path();
                    const bool fileAdded = backupFullDirectory(url, archivePath, QLatin1String("calendar.zip"));
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
        } else if (identifier.contains(QLatin1String("akonadi_ical_resource_"))) {
            backupResourceFile(agent, Utils::calendarPath());
        }
    }

    Q_EMIT info(i18n("Resources backup done."));
}

void ExportCalendarJob::backupConfig()
{
    showInfo(i18n("Backing up config..."));
    MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );

    const QString korganizerStr(QLatin1String("korganizerrc"));
    const QString korganizerrc = KStandardDirs::locateLocal( "config", korganizerStr);
    if (QFile(korganizerrc).exists()) {
        KSharedConfigPtr korganizer = KSharedConfig::openConfig(korganizerrc);

        KTemporaryFile tmp;
        tmp.open();

        KConfig *korganizerConfig = korganizer->copyTo( tmp.fileName() );


        const QString globalCollectionsStr(QLatin1String("GlobalCollectionSelection"));
        if (korganizerConfig->hasGroup(globalCollectionsStr)) {
            KConfigGroup group = korganizerConfig->group(globalCollectionsStr);
            const QString selectionKey(QLatin1String("Selection"));
            Utils::convertCollectionListToRealPath(group, selectionKey);
        }

        korganizerConfig->sync();
        backupFile(tmp.fileName(), Utils::configsPath(), korganizerStr);
        delete korganizerConfig;
    }

    backupConfigFile(QLatin1String("calendar_printing.rc"));
    backupConfigFile(QLatin1String("korgacrc"));

    const QString freebusyurlsStr(QLatin1String("korganizer/freebusyurls"));
    const QString freebusyurls = KStandardDirs::locateLocal( "data", freebusyurlsStr );
    if (QFile(freebusyurls).exists()) {
        backupFile(freebusyurls, Utils::dataPath(), freebusyurlsStr);
    }

    const QString templateDir = KStandardDirs::locateLocal( "data", QLatin1String( "korganizer/templates/" ) );
    QDir templateDirectory( templateDir );
    if (templateDirectory.exists()) {
        const bool templateDirAdded = archive()->addLocalDirectory(templateDir, Utils::dataPath() +  QLatin1String( "/korganizer/templates/" ));
        if (!templateDirAdded) {
            Q_EMIT error(i18n("\"%1\" directory cannot be added to backup file.", templateDir));
        }

    }


    Q_EMIT info(i18n("Config backup done."));
}

