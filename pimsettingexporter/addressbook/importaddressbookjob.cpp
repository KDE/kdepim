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

#include "pimcommon/util/createresource.h"

#include <KTempDir>
#include <KStandardDirs>
#include <KLocale>
#include <KConfigGroup>

#include <QFile>
#include <QDir>


static const QString storeAddressbook = QLatin1String("backupcalendar/");

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
    searchAllFiles(mArchiveDirectory, QString());
    if (mTypeSelected & Utils::Resources)
        restoreResources();
    if (mTypeSelected & Utils::Config)
        restoreConfig();
}

void ImportAddressbookJob::restoreResources()
{
    Q_EMIT info(i18n("Restore resources..."));
    if (!mHashAddressBookArchive.isEmpty()) {
        QHashIterator<QString, QString> i(mHashAddressBookArchive);
        QDir dir(mTempDirName);
        dir.mkdir(Utils::mailsPath());
        const QString copyToDirName(mTempDirName + QLatin1Char('/') + Utils::addressbookPath());
        while (i.hasNext()) {
            i.next();
            qDebug() << i.key() << ": " << i.value() << endl;
            QMap<QString, QVariant> settings;
            //FIXME
            if (i.key().contains(QLatin1String("akonadi_vcarddir_resource_"))) {
                const KArchiveEntry* fileResouceEntry = mArchiveDirectory->entry(i.key());
                if (fileResouceEntry && fileResouceEntry->isFile()) {
                    const KArchiveFile* file = static_cast<const KArchiveFile*>(fileResouceEntry);
                    file->copyTo(copyToDirName);
                    const QString resourceName(file->name());
                    const QString filename(file->name());

                    KSharedConfig::Ptr resourceConfig = KSharedConfig::openConfig(copyToDirName + QLatin1Char('/') + resourceName);

                    KUrl newUrl = Utils::adaptResourcePath(resourceConfig, storeAddressbook);

                    const QString dataFile = i.value();
                    const KArchiveEntry* dataResouceEntry = mArchiveDirectory->entry(dataFile);
                    if (dataResouceEntry->isFile()) {
                        const KArchiveFile* file = static_cast<const KArchiveFile*>(dataResouceEntry);
                        file->copyTo(newUrl.path());
                    }
                    settings.insert(QLatin1String("Path"),newUrl.path());
                    const QString newResource = mCreateResource->createResource( QString::fromLatin1("akonadi_vcarddir_resource"), filename, settings );

                    qDebug()<<" newResource"<<newResource;
                    const QString mailFile = i.value();
                    //TODO import it.

                }
                //TODO unzip it
            } else if ( i.key().contains(QLatin1String("akonadi_vcard_resource_"))) {
                const KArchiveEntry* fileResouceEntry = mArchiveDirectory->entry(i.key());
                if (fileResouceEntry && fileResouceEntry->isFile()) {
                    const KArchiveFile* file = static_cast<const KArchiveFile*>(fileResouceEntry);
                    file->copyTo(copyToDirName);
                    const QString resourceName(file->name());
                    const QString filename(file->name());

                    KSharedConfig::Ptr resourceConfig = KSharedConfig::openConfig(copyToDirName + QLatin1Char('/') + resourceName);

                    KUrl newUrl = Utils::adaptResourcePath(resourceConfig, storeAddressbook);

                    const QString dataFile = i.value();
                    const KArchiveEntry* dataResouceEntry = mArchiveDirectory->entry(dataFile);
                    if (dataResouceEntry->isFile()) {
                        const KArchiveFile* file = static_cast<const KArchiveFile*>(dataResouceEntry);
                        file->copyTo(newUrl.path());
                    }
                    settings.insert(QLatin1String("Path"),newUrl.path());
                    const QString newResource = mCreateResource->createResource( QString::fromLatin1("akonadi_vcard_resource"), filename, settings );
                    qDebug()<<" newResource"<<newResource;
                    //TODO restore it.

                    const KArchiveEntry* fileDataEntry = mArchiveDirectory->entry(i.value());
                    if (fileDataEntry && fileDataEntry->isFile()) {
                        const KArchiveFile* fileData = static_cast<const KArchiveFile*>(fileDataEntry);
                        //
                    }
                }
                //TODO
            }

        }
    }
    Q_EMIT info(i18n("Resources restored."));
}

void ImportAddressbookJob::searchAllFiles(const KArchiveDirectory *dir, const QString &prefix)
{
    Q_FOREACH(const QString& entryName, dir->entries()) {
        const KArchiveEntry *entry = dir->entry(entryName);
        if (entry && entry->isDirectory()) {
            const QString newPrefix = (prefix.isEmpty() ? prefix : prefix + QLatin1Char('/')) + entryName;
            if (entryName == QLatin1String("addressbook")) {
                storeAddressBookArchiveResource(static_cast<const KArchiveDirectory*>(entry),entryName);
            } else {
                searchAllFiles(static_cast<const KArchiveDirectory*>(entry), newPrefix);
            }
        }
    }
}

void ImportAddressbookJob::storeAddressBookArchiveResource(const KArchiveDirectory *dir, const QString &prefix)
{
    Q_FOREACH(const QString& entryName, dir->entries()) {
        const KArchiveEntry *entry = dir->entry(entryName);
        if (entry && entry->isDirectory()) {
            const KArchiveDirectory*resourceDir = static_cast<const KArchiveDirectory*>(entry);
            const QStringList lst = resourceDir->entries();
            if (lst.count() == 2) {
                const QString archPath(prefix + QLatin1Char('/') + entryName + QLatin1Char('/'));
                const QString name(lst.at(0));
                if (name.endsWith(QLatin1String("rc"))&&(name.contains(QLatin1String("akonadi_vcarddir_resource_")) ||
                                                         name.contains(QLatin1String("akonadi_vcard_resource_")))) {
                    mHashAddressBookArchive.insert(archPath + name,archPath +lst.at(1));
                } else {
                    mHashAddressBookArchive.insert(archPath +lst.at(1),archPath + name);
                }
            } else {
                kDebug()<<" lst.at(0)"<<lst.at(0);
                kDebug()<<" Problem in archive. number of file "<<lst.count();
            }
        }
    }
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
                importkaddressBookConfig(kaddressbookrcFile, kaddressbookrc, kaddressbookStr, Utils::configsPath());
            }
        } else {
            importkaddressBookConfig(kaddressbookrcFile, kaddressbookrc, kaddressbookStr, Utils::configsPath());
        }
    }
    Q_EMIT info(i18n("Config restored."));
}

void ImportAddressbookJob::importkaddressBookConfig(const KArchiveFile* file, const QString &config, const QString &filename,const QString &prefix)
{
    copyToFile(file, config, filename, prefix);
    KSharedConfig::Ptr kaddressBookConfig = KSharedConfig::openConfig(config);


    const QString collectionViewCheckStateStr(QLatin1String("CollectionViewCheckState"));
    if (kaddressBookConfig->hasGroup(collectionViewCheckStateStr)) {
        KConfigGroup group = kaddressBookConfig->group(collectionViewCheckStateStr);
        const QString selectionKey(QLatin1String("Selection"));
        convertRealPathToCollectionList(group, selectionKey, true);
    }

    const QString collectionViewStateStr(QLatin1String("CollectionViewState"));
    if (kaddressBookConfig->hasGroup(collectionViewStateStr)) {
        KConfigGroup group = kaddressBookConfig->group(collectionViewStateStr);
        QString currentKey(QLatin1String("Current"));
        convertRealPathToCollection(group, currentKey, true);

        currentKey = QLatin1String("Expansion");
        convertRealPathToCollection(group, currentKey, true);

        currentKey = QLatin1String("Selection");
        convertRealPathToCollection(group, currentKey, true);
    }

    kaddressBookConfig->sync();
}


#include "importaddressbookjob.moc"
