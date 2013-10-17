/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>
  
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

#include "pimcommon/util/createresource.h"

#include <KTempDir>
#include <KStandardDirs>
#include <KArchive>
#include <KLocale>
#include <KConfigGroup>
#include <KZip>

#include <QFile>
#include <QDir>

ImportAkregatorJob::ImportAkregatorJob(QWidget *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep)
    : AbstractImportExportJob(parent, archiveStorage, typeSelected, numberOfStep)
{
    initializeImportJob();
}

ImportAkregatorJob::~ImportAkregatorJob()
{
}

void ImportAkregatorJob::start()
{
    Q_EMIT title(i18n("Start import akregator settings..."));
    mArchiveDirectory = archive()->directory();
    if (mTypeSelected & Utils::Config)
        restoreConfig();
    if (mTypeSelected & Utils::Data)
        restoreData();
}

void ImportAkregatorJob::restoreConfig()
{
    const QString akregatorStr(QLatin1String("akregatorrc"));
    restoreConfigFile(akregatorStr);
    Q_EMIT info(i18n("Config restored."));
}

void ImportAkregatorJob::restoreData()
{
    const KArchiveEntry *akregatorEntry  = mArchiveDirectory->entry(Utils::dataPath() + QLatin1String( "akregator/" ) );
    if (akregatorEntry && akregatorEntry->isDirectory()) {
        //TODO 4.12 verify if akregator already exists.
        const QString akregatorPath = KGlobal::dirs()->saveLocation("data", QLatin1String("akregator/"));
        const KArchiveDirectory *akregatorDir = static_cast<const KArchiveDirectory*>(akregatorEntry);
        akregatorDir->copyTo(akregatorPath);
    }
    Q_EMIT info(i18n("Data restored."));
}

#include "importakregatorjob.moc"
