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

#include "importnotesjob.h"
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

ImportNotesJob::ImportNotesJob(QWidget *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep)
    : AbstractImportExportJob(parent, archiveStorage, typeSelected, numberOfStep)
{
    initializeImportJob();
}

ImportNotesJob::~ImportNotesJob()
{
}

void ImportNotesJob::start()
{
    Q_EMIT title(i18n("Start import knotes settings..."));
    mArchiveDirectory = archive()->directory();
    if (mTypeSelected & Utils::Config)
        restoreConfig();
    if (mTypeSelected & Utils::Data)
        restoreData();
}

void ImportNotesJob::restoreConfig()
{
    const KArchiveEntry *knotesEntry  = mArchiveDirectory->entry(Utils::configsPath() + QLatin1String( "knotesrc" ) );
    if (knotesEntry && knotesEntry->isFile()) {
        const KArchiveEntry *entry = static_cast<const KArchiveEntry*>(knotesEntry);
        copyToDirectory(entry, KGlobal::dirs()->saveLocation( "config", QLatin1String( "knotesrc" )));
    }

    Q_EMIT info(i18n("Config restored."));
}

void ImportNotesJob::restoreData()
{
    //TODO
}

#include "importnotesjob.moc"
