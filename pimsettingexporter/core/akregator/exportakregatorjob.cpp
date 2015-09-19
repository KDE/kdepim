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

#include "exportakregatorjob.h"

#include "Libkdepim/KCursorSaver"

#include <AkonadiCore/AgentManager>

#include <KLocalizedString>
#include <KZip>

#include <QWidget>
#include <QDir>
#include <QStandardPaths>

ExportAkregatorJob::ExportAkregatorJob(QObject *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep)
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
    KPIM::KCursorSaver busy(KPIM::KBusyPtr::busy());
    const QString akregatorStr(QStringLiteral("akregatorrc"));
    const QString akregatorsrc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + akregatorStr;
    backupFile(akregatorsrc, Utils::configsPath(), akregatorStr);
    Q_EMIT info(i18n("Config backup done."));
}

void ExportAkregatorJob::backupData()
{
    showInfo(i18n("Backing up data..."));
    KPIM::KCursorSaver busy(KPIM::KBusyPtr::busy());
    const QString akregatorDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/akregator") ;
    QDir akregatorDirectory(akregatorDir);
    if (akregatorDirectory.exists()) {
        const bool akregatorDirAdded = archive()->addLocalDirectory(akregatorDir, Utils::dataPath() +  QLatin1String("/akregator"));
        if (!akregatorDirAdded) {
            Q_EMIT error(i18n("\"%1\" directory cannot be added to backup file.", akregatorDir));
        }
    }
    Q_EMIT info(i18n("Data backup done."));
}

