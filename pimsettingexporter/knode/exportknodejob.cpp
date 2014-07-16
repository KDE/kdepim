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

#include "exportknodejob.h"

#include "messageviewer/utils/kcursorsaver.h"

#include <Akonadi/AgentManager>

#include <KLocalizedString>
#include <KStandardDirs>
#include <KZip>

#include <QWidget>
#include <QDir>

ExportKnodeJob::ExportKnodeJob(QWidget *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage,int numberOfStep)
    : AbstractImportExportJob(parent, archiveStorage, typeSelected, numberOfStep)
{
}

ExportKnodeJob::~ExportKnodeJob()
{

}

void ExportKnodeJob::start()
{
    Q_EMIT title(i18n("Start export KNode settings..."));
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


void ExportKnodeJob::backupConfig()
{
    showInfo(i18n("Backing up config..."));
    MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
    const QString knodeStr(QLatin1String("knoderc"));
    const QString knoderc = KStandardDirs::locateLocal( "config", knodeStr);
    backupFile(knoderc, Utils::configsPath(), knodeStr);

    Q_EMIT info(i18n("Config backup done."));
}

void ExportKnodeJob::backupData()
{
    showInfo(i18n("Backing up data..."));
    MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
    const QString nodeDir = KStandardDirs::locateLocal( "data", QLatin1String( "knode/" ) );
    QDir nodeDirectory( nodeDir );
    if (nodeDirectory.exists()) {
        const bool nodeDirAdded = archive()->addLocalDirectory(nodeDir, Utils::dataPath() +  QLatin1String( "/knode/" ));
        if (!nodeDirAdded) {
            Q_EMIT error(i18n("\"%1\" directory cannot be added to backup file.", nodeDir));
        }
    }
    Q_EMIT info(i18n("Data backup done."));
}

