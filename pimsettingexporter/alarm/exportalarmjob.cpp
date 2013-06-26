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

#include <Akonadi/AgentManager>

#include <KLocale>
#include <KStandardDirs>
#include <KTemporaryFile>
#include <KConfigGroup>

#include <QWidget>
#include <QFile>
#include <QDir>

ExportAlarmJob::ExportAlarmJob(QWidget *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage,int numberOfStep)
    : AbstractImportExportJob(parent, archiveStorage, typeSelected, numberOfStep)
{
}

ExportAlarmJob::~ExportAlarmJob()
{

}

void ExportAlarmJob::start()
{
    mArchiveDirectory = archive()->directory();
    if (mTypeSelected & Utils::Resources) {
        backupResources();
        increaseProgressDialog();
        if (wasCanceled()) {
            return;
        }
    }
    if (mTypeSelected & Utils::Config) {
        backupConfig();
        increaseProgressDialog();
        if (wasCanceled()) {
            return;
        }
    }
}


void ExportAlarmJob::backupResources()
{
    showInfo(i18n("Backing up resources..."));
    MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );

    Akonadi::AgentManager *manager = Akonadi::AgentManager::self();
    const Akonadi::AgentInstance::List list = manager->instances();
    foreach( const Akonadi::AgentInstance &agent, list ) {
        const QString identifier = agent.identifier();
        if (identifier.contains(QLatin1String("akonadi_kalarm_resource_"))) {
            backupResourceFile(agent, Utils::alarmPath());
        }
    }

    Q_EMIT info(i18n("Resources backup done."));
}

void ExportAlarmJob::backupConfig()
{
    showInfo(i18n("Backing up config..."));
    MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
    const QString kalarmStr(QLatin1String("kalarmrc"));
    const QString kalarmrc = KStandardDirs::locateLocal( "config", kalarmStr);
    if (QFile(kalarmrc).exists()) {
        KSharedConfigPtr kalarm = KSharedConfig::openConfig(kalarmrc);

        KTemporaryFile tmp;
        tmp.open();

        KConfig *kalarmConfig = kalarm->copyTo( tmp.fileName() );

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

#include "exportalarmjob.moc"
