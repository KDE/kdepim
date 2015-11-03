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

#include "importblogilojob.h"
#include "archivestorage.h"

#include "PimCommon/CreateResource"

#include <KArchive>
#include <KLocalizedString>

#include <KZip>
#include <QStandardPaths>
#include <QTimer>

ImportBlogiloJob::ImportBlogiloJob(QObject *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep)
    : AbstractImportExportJob(parent, archiveStorage, typeSelected, numberOfStep)
{
    initializeImportJob();
}

ImportBlogiloJob::~ImportBlogiloJob()
{
}

void ImportBlogiloJob::start()
{
    Q_EMIT title(i18n("Start import Blogilo settings..."));
    createProgressDialog(i18n("Import Blogilo settings"));
    mArchiveDirectory = archive()->directory();
    initializeListStep();
    QTimer::singleShot(0, this, &ImportBlogiloJob::slotNextStep);
}

void ImportBlogiloJob::slotNextStep()
{
    ++mIndex;
    if (mIndex < mListStep.count()) {
        const Utils::StoredType type = mListStep.at(mIndex);
        if (type == Utils::Config) {
            restoreConfig();
        } else if (type == Utils::Data) {
            restoreData();
        } else {
            qCDebug(PIMSETTINGEXPORTERCORE_LOG) << Q_FUNC_INFO << " not supported type "<< type;
            slotNextStep();
        }
    } else {
        Q_EMIT jobFinished();
    }
}

void ImportBlogiloJob::restoreConfig()
{
    increaseProgressDialog();
    setProgressDialogLabel(i18n("Restore configs..."));
    const QString blogiloStr(QStringLiteral("blogilorc"));
    restoreConfigFile(blogiloStr);
    restoreUiRcFile(QStringLiteral("blogiloui.rc"), QStringLiteral("blogilo"));
    Q_EMIT info(i18n("Config restored."));
    QTimer::singleShot(0, this, &ImportBlogiloJob::slotNextStep);
}

void ImportBlogiloJob::restoreData()
{
    increaseProgressDialog();
    setProgressDialogLabel(i18n("Restore data..."));
    const KArchiveEntry *blogiloEntry  = mArchiveDirectory->entry(Utils::dataPath() + QLatin1String("blogilo/"));
    if (blogiloEntry && blogiloEntry->isDirectory()) {
        const QString blogiloPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + QLatin1String("blogilo/");
        overwriteDirectory(blogiloPath, blogiloEntry);
    }
    Q_EMIT info(i18n("Data restored."));
    QTimer::singleShot(0, this, &ImportBlogiloJob::slotNextStep);
}

