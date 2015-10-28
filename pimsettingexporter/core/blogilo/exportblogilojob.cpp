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

#include "exportblogilojob.h"

#include <AkonadiCore/AgentManager>

#include <KLocalizedString>

#include <KZip>

#include <QStandardPaths>
#include <QTimer>

ExportBlogiloJob::ExportBlogiloJob(QObject *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep)
    : AbstractImportExportJob(parent, archiveStorage, typeSelected, numberOfStep)
{
}

ExportBlogiloJob::~ExportBlogiloJob()
{

}

void ExportBlogiloJob::start()
{
    Q_EMIT title(i18n("Start export Blogilo settings..."));
    createProgressDialog(i18n("Export Blogilo settings"));
    if (mTypeSelected & Utils::Config) {
        QTimer::singleShot(0, this, SLOT(slotCheckBackupConfig()));
    } else if (mTypeSelected & Utils::Data) {
        QTimer::singleShot(0, this, SLOT(slotCheckBackupData()));
    } else {
        Q_EMIT jobFinished();
    }
}

void ExportBlogiloJob::slotCheckBackupConfig()
{
    setProgressDialogLabel(i18n("Backing up config..."));

    const QString blogiloStr(QStringLiteral("blogilorc"));
    const QString blogilorc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + blogiloStr;
    backupFile(blogilorc, Utils::configsPath(), blogiloStr);

    Q_EMIT info(i18n("Config backup done."));
    QTimer::singleShot(0, this, SLOT(slotCheckBackupData()));
}

void ExportBlogiloJob::slotCheckBackupData()
{
    if (mTypeSelected & Utils::Data) {
        setProgressDialogLabel(i18n("Backing up data..."));

        const QString dbfileStr = QStringLiteral("blogilo.db");
        const QString dbfile = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/blogilo/") + dbfileStr;

        backupFile(dbfile, Utils::dataPath() +  QLatin1String("/blogilo/"), dbfileStr);

        Q_EMIT info(i18n("Data backup done."));
    }
    Q_EMIT jobFinished();
}

