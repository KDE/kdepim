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

#include "exportakregatorjob.h"

#include "messageviewer/utils/kcursorsaver.h"

#include <AkonadiCore/AgentManager>

#include <KLocalizedString>
#include <KStandardDirs>
#include <KZip>

#include <QWidget>
#include <QDir>

ExportAkregatorJob::ExportAkregatorJob(QWidget *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage,int numberOfStep)
    : AbstractImportExportJob(parent, archiveStorage, typeSelected, numberOfStep)
{
}

ExportAkregatorJob::~ExportAkregatorJob()
{

}

void ExportAkregatorJob::start()
{
    Q_EMIT title(i18n("Start export Akregator settings..."));
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


void ExportAkregatorJob::backupConfig()
{
    showInfo(i18n("Backing up config..."));
    MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
    const QString akregatorStr(QLatin1String("akregatorrc"));
    const QString akregatorsrc = KStandardDirs::locateLocal( "config", akregatorStr);
    backupFile(akregatorsrc, Utils::configsPath(), akregatorStr);
    Q_EMIT info(i18n("Config backup done."));
}

void ExportAkregatorJob::backupData()
{
    showInfo(i18n("Backing up data..."));
    MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
    const QString akregatorDir = KStandardDirs::locateLocal( "data", QLatin1String( "akregator" ) );
    QDir akregatorDirectory( akregatorDir );
    if (akregatorDirectory.exists()) {
        const bool akregatorDirAdded = archive()->addLocalDirectory(akregatorDir, Utils::dataPath() +  QLatin1String( "/akregator" ));
        if (!akregatorDirAdded) {
            Q_EMIT error(i18n("\"%1\" directory cannot be added to backup file.", akregatorDir));
        }
    }
    Q_EMIT info(i18n("Data backup done."));
}

