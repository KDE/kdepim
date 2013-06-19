/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>
  
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

#include "importalarmjob.h"
#include "archivestorage.h"

#include <KTempDir>
#include <KStandardDirs>
#include <KArchive>
#include <KLocale>
#include <KConfigGroup>

#include <QFile>

ImportAlarmJob::ImportAlarmJob(QWidget *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep)
    : AbstractImportExportJob(parent, archiveStorage, typeSelected, numberOfStep)
{
    initializeImportJob();
}

ImportAlarmJob::~ImportAlarmJob()
{

}


void ImportAlarmJob::start()
{
    mArchiveDirectory = archive()->directory();
    if (mTypeSelected & Utils::Resources)
        restoreResources();
    if (mTypeSelected & Utils::Config)
        restoreConfig();
}

void ImportAlarmJob::restoreResources()
{
    Q_EMIT info(i18n("Restore resources..."));
    Q_EMIT info(i18n("Resources restored."));

    //TODO
}

void ImportAlarmJob::restoreConfig()
{
    const QString kalarmStr(QLatin1String("kalarmrc"));
    const KArchiveEntry* kalarmrcentry  = mArchiveDirectory->entry(Utils::configsPath() + kalarmStr);
    if (kalarmrcentry && kalarmrcentry->isFile()) {
        const KArchiveFile* kalarmrcFile = static_cast<const KArchiveFile*>(kalarmrcentry);
        const QString kalarmrc = KStandardDirs::locateLocal( "config",  kalarmStr);
        if (QFile(kalarmrc).exists()) {
            if (overwriteConfigMessageBox(kalarmStr)) {
                importkalarmConfig(kalarmrcFile, kalarmrc, kalarmStr, Utils::configsPath());
            }
        } else {
            importkalarmConfig(kalarmrcFile,kalarmrc,kalarmStr,Utils::configsPath());
        }
    }
    Q_EMIT info(i18n("Config restored."));
}

void ImportAlarmJob::importkalarmConfig(const KArchiveFile* kalarmFile, const QString &kalarmrc, const QString &filename,const QString &prefix)
{
    copyToFile(kalarmFile, kalarmrc, filename, prefix);
    KSharedConfig::Ptr kalarmConfig = KSharedConfig::openConfig(kalarmrc);

    const QString collectionsStr(QLatin1String("Collections"));
    if (kalarmConfig->hasGroup(collectionsStr)) {
        KConfigGroup group = kalarmConfig->group(collectionsStr);
        const QString selectionKey(QLatin1String("FavoriteCollectionIds"));
        convertRealPathToCollectionList(group, selectionKey, false);
    }

    kalarmConfig->sync();
}

QString ImportAlarmJob::componentName() const
{
    return QLatin1String("KAlarm");
}

#include "importalarmjob.moc"
