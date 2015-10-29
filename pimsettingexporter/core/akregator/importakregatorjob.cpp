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

#include "importakregatorjob.h"
#include "archivestorage.h"

#include "PimCommon/CreateResource"

#include <KArchive>
#include <KLocalizedString>
#include <KZip>
#include <QTimer>

#include <QStandardPaths>

ImportAkregatorJob::ImportAkregatorJob(QObject *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep)
    : AbstractImportExportJob(parent, archiveStorage, typeSelected, numberOfStep)
{
    initializeImportJob();
}

ImportAkregatorJob::~ImportAkregatorJob()
{
}

void ImportAkregatorJob::start()
{
    Q_EMIT title(i18n("Start import Akregator settings..."));
    mArchiveDirectory = archive()->directory();
    initializeListStep();
    createProgressDialog(i18n("Import Akregator settings"));
    QTimer::singleShot(0, this, &ImportAkregatorJob::slotNextStep);
}

void ImportAkregatorJob::slotNextStep()
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

void ImportAkregatorJob::restoreConfig()
{
    const QString akregatorStr(QStringLiteral("akregatorrc"));
    increaseProgressDialog();
    restoreConfigFile(akregatorStr);
    Q_EMIT info(i18n("Config restored."));
    QTimer::singleShot(0, this, &ImportAkregatorJob::slotNextStep);
}

void ImportAkregatorJob::restoreData()
{
    increaseProgressDialog();
    const KArchiveEntry *akregatorEntry  = mArchiveDirectory->entry(Utils::dataPath() + QLatin1String("akregator/"));
    if (akregatorEntry && akregatorEntry->isDirectory()) {
        const QString akregatorPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + QLatin1String("akregator/");
        overwriteDirectory(akregatorPath, akregatorEntry);
    }
    Q_EMIT info(i18n("Data restored."));
    QTimer::singleShot(0, this, &ImportAkregatorJob::slotNextStep);
}

