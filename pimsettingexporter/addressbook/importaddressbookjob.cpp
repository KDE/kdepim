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

#include "importaddressbookjob.h"
#include "archivestorage.h"

#include <KTempDir>
#include <KStandardDirs>
#include <KLocale>

#include <QFile>


ImportAddressbookJob::ImportAddressbookJob(QWidget *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep)
    : AbstractImportExportJob(parent, archiveStorage, typeSelected, numberOfStep)
{
    initializeImportJob();
}

ImportAddressbookJob::~ImportAddressbookJob()
{

}

void ImportAddressbookJob::start()
{
    mArchiveDirectory = archive()->directory();
    if (mTypeSelected & Utils::Resources)
        restoreResources();
    if (mTypeSelected & Utils::Config)
        restoreConfig();
}

void ImportAddressbookJob::restoreResources()
{
    //TODO
    Q_EMIT info(i18n("Restore resources..."));
    Q_EMIT info(i18n("Resources restored."));
}

void ImportAddressbookJob::restoreConfig()
{
    const QString kaddressbookStr(QLatin1String("kaddressbookrc"));
    const KArchiveEntry* kaddressbookrcentry  = mArchiveDirectory->entry(Utils::configsPath() + kaddressbookStr);
    if (kaddressbookrcentry && kaddressbookrcentry->isFile()) {
        const KArchiveFile* kaddressbookrcFile = static_cast<const KArchiveFile*>(kaddressbookrcentry);
        const QString kaddressbookrc = KStandardDirs::locateLocal( "config",  kaddressbookStr);
        if (QFile(kaddressbookrc).exists()) {
            if (overwriteConfigMessageBox(kaddressbookStr)) {
                importkalarmConfig(kaddressbookrcFile, kaddressbookrc, kaddressbookStr, Utils::configsPath());
            }
        } else {
            importkalarmConfig(kaddressbookrcFile, kaddressbookrc, kaddressbookStr, Utils::configsPath());
        }
    }
    Q_EMIT info(i18n("Config restored."));
}

void ImportAddressbookJob::importkalarmConfig(const KArchiveFile* file, const QString &config, const QString &filename,const QString &prefix)
{
    copyToFile(file, config, filename, prefix);
    KSharedConfig::Ptr kaddressbookConfig = KSharedConfig::openConfig(config);

    //TODO adapt collection
    kaddressbookConfig->sync();
}


QString ImportAddressbookJob::componentName() const
{
    return QLatin1String("KAddressBook");
}

#include "importaddressbookjob.moc"
