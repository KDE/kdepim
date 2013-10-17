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
    Q_EMIT title(i18n("Start import knotes settings..."));
    mArchiveDirectory = archive()->directory();
    if (mTypeSelected & Utils::Config)
        restoreConfig();
    if (mTypeSelected & Utils::Data)
        restoreData();
}

void ImportAkregatorJob::restoreConfig()
{
    const KArchiveEntry *knotesEntry  = mArchiveDirectory->entry(Utils::configsPath() + QLatin1String( "knotesrc" ) );
    if (knotesEntry && knotesEntry->isFile()) {
        const KArchiveEntry *entry = static_cast<const KArchiveEntry*>(knotesEntry);
        copyToDirectory(entry, KGlobal::dirs()->saveLocation( "config", QLatin1String( "knotesrc" )));
    }

    Q_EMIT info(i18n("Config restored."));
}

void ImportAkregatorJob::restoreData()
{
    const KArchiveEntry *notesEntry  = mArchiveDirectory->entry(Utils::dataPath() + QLatin1String( "knotes/" ) );
    if (notesEntry && notesEntry->isDirectory()) {
        //TODO 4.12 verify if notes already exists.
        const QString notesPath = KGlobal::dirs()->saveLocation("data", QLatin1String("knotes/"));
        const KArchiveDirectory *notesDir = static_cast<const KArchiveDirectory*>(notesEntry);
        notesDir->copyTo(notesPath);
    }
    Q_EMIT info(i18n("Data restored."));
}

#include "importakregatorjob.moc"
