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

#include "importblogilojob.h"
#include "archivestorage.h"

#include "pimcommon/util/createresource.h"

#include <KArchive>
#include <KLocalizedString>
#include <KGlobal>
#include <KStandardDirs>
#include <KZip>

ImportBlogiloJob::ImportBlogiloJob(QWidget *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep)
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
    mArchiveDirectory = archive()->directory();
    if (mTypeSelected & Utils::Config)
        restoreConfig();
    if (mTypeSelected & Utils::Data)
        restoreData();
    Q_EMIT jobFinished();
}

void ImportBlogiloJob::restoreConfig()
{
    const QString blogiloStr(QLatin1String("blogilorc"));
    restoreConfigFile(blogiloStr);
    Q_EMIT info(i18n("Config restored."));
}

void ImportBlogiloJob::restoreData()
{
    const KArchiveEntry *blogiloEntry  = mArchiveDirectory->entry(Utils::dataPath() + QLatin1String( "blogilo/" ) );
    if (blogiloEntry && blogiloEntry->isDirectory()) {
        const QString blogiloPath = KGlobal::dirs()->saveLocation("data", QLatin1String("blogilo/"));
        overwriteDirectory(blogiloPath, blogiloEntry);
    }
    Q_EMIT info(i18n("Data restored."));
}

