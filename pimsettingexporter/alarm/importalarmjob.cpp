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

#include "pimcommon/util/createresource.h"

#include <KTempDir>
#include <KStandardDirs>
#include <KArchive>
#include <KLocale>
#include <KConfigGroup>
#include <KZip>

#include <QDir>
#include <QFile>

static const QString storeAlarm = QLatin1String("backupalarm/");

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
    searchAllFiles(mArchiveDirectory ,QString());
    if (mTypeSelected & Utils::Resources)
        restoreResources();
    if (mTypeSelected & Utils::Config)
        restoreConfig();
}

void ImportAlarmJob::restoreResources()
{
    //TODO we don't have several resource.
    //Need to overwrite it.
#if 0 //PORT ME
    Q_EMIT info(i18n("Restore resources..."));
    if (!mListResourceFile.isEmpty()) {
        QHashIterator<QString, QString> i(mListResourceFile);
        QDir dir(mTempDirName);
        dir.mkdir(Utils::alarmPath());
        const QString copyToDirName(mTempDirName + QLatin1Char('/') + Utils::alarmPath());

        while (i.hasNext()) {
            i.next();
            qDebug() << i.key() << ": " << i.value() << endl;
            QMap<QString, QVariant> settings;
            //FIXME
            if (i.key().contains(QLatin1String("akonadi_kalarm_resource_"))) {
                const KArchiveEntry* fileResouceEntry = mArchiveDirectory->entry(i.key());
                if (fileResouceEntry && fileResouceEntry->isFile()) {
                    const KArchiveFile* file = static_cast<const KArchiveFile*>(fileResouceEntry);
                    file->copyTo(copyToDirName);
                    const QString resourceName(file->name());
                    const QString filename(file->name());

                    KSharedConfig::Ptr resourceConfig = KSharedConfig::openConfig(copyToDirName + QLatin1Char('/') + resourceName);

                    KUrl newUrl = Utils::adaptResourcePath(resourceConfig, storeAlarm);

                    const QString dataFile = i.value();
                    const KArchiveEntry* dataResouceEntry = mArchiveDirectory->entry(dataFile);
                    if (dataResouceEntry->isFile()) {
                        const KArchiveFile* file = static_cast<const KArchiveFile*>(dataResouceEntry);
                        file->copyTo(newUrl.path());
                    }
                    settings.insert(QLatin1String("Path"),newUrl.path());
                    const QString newResource = mCreateResource->createResource( QString::fromLatin1("akonadi_kalarm_resource"), filename, settings );
                    qDebug()<<" newResource"<<newResource;
                }
                //TODO
            }

        }
    }

    Q_EMIT info(i18n("Resources restored."));
#endif
}

void ImportAlarmJob::addSpecificResourceSettings(KSharedConfig::Ptr resourceConfig, const QString &resourceName, QMap<QString, QVariant> &settings)
{

}

void ImportAlarmJob::searchAllFiles(const KArchiveDirectory *dir,const QString &prefix)
{
    Q_FOREACH(const QString& entryName, dir->entries()) {
        const KArchiveEntry *entry = dir->entry(entryName);
        if (entry && entry->isDirectory()) {
            const QString newPrefix = (prefix.isEmpty() ? prefix : prefix + QLatin1Char('/')) + entryName;
            if (entryName == QLatin1String("alarm")) {
                storeAlarmArchiveResource(static_cast<const KArchiveDirectory*>(entry),entryName);
            } else {
                searchAllFiles(static_cast<const KArchiveDirectory*>(entry), newPrefix);
            }
        }
    }
}

void ImportAlarmJob::storeAlarmArchiveResource(const KArchiveDirectory *dir, const QString &prefix)
{
    Q_FOREACH(const QString& entryName, dir->entries()) {
        const KArchiveEntry *entry = dir->entry(entryName);
        if (entry && entry->isDirectory()) {
            const KArchiveDirectory *resourceDir = static_cast<const KArchiveDirectory*>(entry);
            const QStringList lst = resourceDir->entries();

            if (lst.count() >= 2) {
                const QString archPath(prefix + QLatin1Char('/') + entryName + QLatin1Char('/'));
                resourceFiles files;
                Q_FOREACH(const QString &name, lst) {
                    if (name.endsWith(QLatin1String("rc")) && (name.contains(QLatin1String("akonadi_alarm_resource_")))) {
                        files.akonadiConfigFile = archPath + name;
                    } else if (name.startsWith(Utils::prefixAkonadiConfigFile())) {
                        files.akonadiAgentConfigFile = archPath + name;
                    } else {
                        files.akonadiResources = archPath + name;
                    }
                }
                mListResourceFile.append(files);
            } else {
                kDebug()<<" Problem in archive. number of file "<<lst.count();
            }
        }
    }
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

#include "importalarmjob.moc"
