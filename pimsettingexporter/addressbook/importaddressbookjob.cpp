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
#include <KZip>

#include <QFile>
#include <QDir>


static const QString storeAddressbook = QLatin1String("backupaddressbook/");

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
    Q_EMIT title(i18n("Start import kaddressbook settings..."));
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
    restoreResourceFile(QString::fromLatin1("akonadi_vcard_resource"), Utils::addressbookPath(), QDir::homePath() + QLatin1String("/.kde/share/apps/kabc/"));

    if (!mListResourceFile.isEmpty()) {
        QDir dir(mTempDirName);
        dir.mkdir(Utils::addressbookPath());
        const QString copyToDirName(mTempDirName + QLatin1Char('/') + Utils::addressbookPath());

        for (int i = 0; i < mListResourceFile.size(); ++i) {
            resourceFiles value = mListResourceFile.at(i);
            QMap<QString, QVariant> settings;
            if (value.akonadiConfigFile.contains(QLatin1String("akonadi_vcarddir_resource_")) ||
                    value.akonadiConfigFile.contains(QLatin1String("akonadi_contacts_resource_")) ) {
                const KArchiveEntry* fileResouceEntry = mArchiveDirectory->entry(value.akonadiConfigFile);
                if (fileResouceEntry && fileResouceEntry->isFile()) {
                    const KArchiveFile* file = static_cast<const KArchiveFile*>(fileResouceEntry);
                    file->copyTo(copyToDirName);
                    QString resourceName(file->name());

                    QString filename(file->name());
                    //TODO adapt filename otherwise it will use all the time the same filename.
                    KSharedConfig::Ptr resourceConfig = KSharedConfig::openConfig(copyToDirName + QLatin1Char('/') + resourceName);

                    //TODO fix default path
                    const KUrl newUrl = Utils::adaptResourcePath(resourceConfig, QDir::homePath() + QLatin1String("/.local/share/contacts/"));
                    const QString dataFile = value.akonadiResources;
                    const KArchiveEntry* dataResouceEntry = mArchiveDirectory->entry(dataFile);
                    if (dataResouceEntry->isFile()) {
                        const KArchiveFile* file = static_cast<const KArchiveFile*>(dataResouceEntry);
                        //TODO  adapt directory name too
                        extractZipFile(file, copyToDirName, newUrl.path());
                    }
                    settings.insert(QLatin1String("Path"), newUrl.path());

                    const QString agentConfigFile = value.akonadiAgentConfigFile;
                    if (!agentConfigFile.isEmpty()) {
                        const KArchiveEntry *akonadiAgentConfigEntry = mArchiveDirectory->entry(agentConfigFile);
                        if (akonadiAgentConfigEntry->isFile()) {
                            const KArchiveFile* file = static_cast<const KArchiveFile*>(akonadiAgentConfigEntry);
                            file->copyTo(copyToDirName);
                            resourceName = file->name();
                            KSharedConfig::Ptr akonadiAgentConfig = KSharedConfig::openConfig(copyToDirName + QLatin1Char('/') + resourceName);
                            filename = Utils::akonadiAgentName(akonadiAgentConfig);
                        }
                    }
                    QString instanceType;
                    if (value.akonadiConfigFile.contains(QLatin1String("akonadi_vcarddir_resource_")))
                        instanceType = QLatin1String("akonadi_vcarddir_resource");
                    else if (value.akonadiConfigFile.contains(QLatin1String("akonadi_contacts_resource_")))
                        instanceType = QLatin1String("akonadi_contacts_resource");
                    else
                        qDebug()<<" not supported"<<value.akonadiConfigFile;

                    const QString newResource = mCreateResource->createResource( instanceType, filename, settings, true );
                    infoAboutNewResource(newResource);
                    qDebug()<<" newResource"<<newResource;
                }
            }
        }
    }

    Q_EMIT info(i18n("Resources restored."));
}

void ImportAddressbookJob::addSpecificResourceSettings(KSharedConfig::Ptr resourceConfig, const QString &resourceName, QMap<QString, QVariant> &settings)
{
    if (resourceName == QLatin1String("akonadi_vcard_resource")) {
        KConfigGroup general = resourceConfig->group(QLatin1String("General"));
        if (general.hasKey(QLatin1String("DisplayName"))) {
            settings.insert(QLatin1String("DisplayName"), general.readEntry(QLatin1String("DisplayName")));
        }
        if (general.hasKey(QLatin1String("ReadOnly"))) {
            settings.insert(QLatin1String("ReadOnly"), general.readEntry(QLatin1String("ReadOnly"), false));
        }
        if (general.hasKey(QLatin1String("MonitorFile"))) {
            settings.insert(QLatin1String("MonitorFile"), general.readEntry(QLatin1String("MonitorFile"), true));
        }
    }
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
            const KArchiveDirectory *resourceDir = static_cast<const KArchiveDirectory*>(entry);
            const QStringList lst = resourceDir->entries();

            if (lst.count() >= 2) {
                const QString archPath(prefix + QLatin1Char('/') + entryName + QLatin1Char('/'));
                resourceFiles files;
                Q_FOREACH(const QString &name, lst) {
                    if (name.endsWith(QLatin1String("rc")) && (name.contains(QLatin1String("akonadi_vcarddir_resource_")) ||
                                                               name.contains(QLatin1String("akonadi_vcard_resource_")) ||
                                                               name.contains(QLatin1String("akonadi_contacts_resource_")))) {
                        files.akonadiConfigFile = archPath + name;
                    } else if (name.startsWith(Utils::prefixAkonadiConfigFile())) {
                        files.akonadiAgentConfigFile = archPath + name;
                    } else {
                        files.akonadiResources = archPath + name;
                    }
                }
                files.debug();
                mListResourceFile.append(files);
            } else {
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


