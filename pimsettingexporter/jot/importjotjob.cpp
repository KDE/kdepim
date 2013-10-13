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

#include "importjotjob.h"
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

static const QString storeJot = QLatin1String("backupjot/");

ImportJotJob::ImportJotJob(QWidget *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep)
    : AbstractImportExportJob(parent, archiveStorage, typeSelected, numberOfStep)
{
    initializeImportJob();
}

ImportJotJob::~ImportJotJob()
{
}

void ImportJotJob::start()
{
    Q_EMIT title(i18n("Start import kjots settings..."));
    mArchiveDirectory = archive()->directory();
    searchAllFiles(mArchiveDirectory ,QString());
    if (mTypeSelected & Utils::Resources)
        restoreResources();
    if (mTypeSelected & Utils::Config)
        restoreConfig();
}

void ImportJotJob::restoreResources()
{
    Q_EMIT info(i18n("Restore resources..."));
    //It's maildir support. Need to add support
    restoreResourceFile(QString::fromLatin1("akonadi_akonotes_resource"), Utils::jotPath(), storeJot);
    Q_EMIT info(i18n("Resources restored."));
}

void ImportJotJob::addSpecificResourceSettings(KSharedConfig::Ptr resourceConfig, const QString &resourceName, QMap<QString, QVariant> &settings)
{

}

void ImportJotJob::searchAllFiles(const KArchiveDirectory *dir,const QString &prefix)
{
    Q_FOREACH(const QString& entryName, dir->entries()) {
        const KArchiveEntry *entry = dir->entry(entryName);
        if (entry && entry->isDirectory()) {
            const QString newPrefix = (prefix.isEmpty() ? prefix : prefix + QLatin1Char('/')) + entryName;
            if (entryName == QLatin1String("jot")) {
                storeJotArchiveResource(static_cast<const KArchiveDirectory*>(entry),entryName);
            } else {
                searchAllFiles(static_cast<const KArchiveDirectory*>(entry), newPrefix);
            }
        }
    }
}

void ImportJotJob::storeJotArchiveResource(const KArchiveDirectory *dir, const QString &prefix)
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
                    if (name.endsWith(QLatin1String("rc")) && (name.contains(QLatin1String("akonadi_akonotes_resource")))) {
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


void ImportJotJob::restoreConfig()
{
    const QString jotStr(QLatin1String("jotrc"));
    const KArchiveEntry* jotrcentry  = mArchiveDirectory->entry(Utils::configsPath() + jotStr);
    if (jotrcentry && jotrcentry->isFile()) {
        const KArchiveFile* jotrcFile = static_cast<const KArchiveFile*>(jotrcentry);
        const QString jotrc = KStandardDirs::locateLocal( "config",  jotStr);
        if (QFile(jotrc).exists()) {
            if (overwriteConfigMessageBox(jotStr)) {
                importjotConfig(jotrcFile, jotrc, jotStr, Utils::configsPath());
            }
        } else {
            importjotConfig(jotrcFile,jotrc,jotStr,Utils::configsPath());
        }
    }
    Q_EMIT info(i18n("Config restored."));
}

void ImportJotJob::importjotConfig(const KArchiveFile* jotFile, const QString &jotrc, const QString &filename,const QString &prefix)
{
    copyToFile(jotFile, jotrc, filename, prefix);
    KSharedConfig::Ptr kjotConfig = KSharedConfig::openConfig(jotrc);

    const QString collectionsStr(QLatin1String("TreeState"));
    if (kjotConfig->hasGroup(collectionsStr)) {
        KConfigGroup group = kjotConfig->group(collectionsStr);
        const QString selectionKey(QLatin1String("Expansion"));
        Utils::convertCollectionIdsToRealPath(group, selectionKey);
    }

    kjotConfig->sync();
}

#include "importjotjob.moc"
