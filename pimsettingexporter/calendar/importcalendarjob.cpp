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

#include "importcalendarjob.h"
#include "archivestorage.h"

#include "pimcommon/util/createresource.h"

#include <KStandardDirs>
#include <KLocalizedString>
#include <KConfigGroup>
#include <KArchiveDirectory>
#include <KArchiveEntry>
#include <KArchive>
#include <KZip>

#include <QDebug>

#include <QFile>
#include <QDir>
#include <QStandardPaths>

static const QString storeCalendar = QLatin1String("backupcalendar/");

ImportCalendarJob::ImportCalendarJob(QWidget *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep)
    : AbstractImportExportJob(parent, archiveStorage, typeSelected, numberOfStep)
{
    initializeImportJob();
}

ImportCalendarJob::~ImportCalendarJob()
{
}

void ImportCalendarJob::start()
{
    Q_EMIT title(i18n("Start import KOrganizer settings..."));
    mArchiveDirectory = archive()->directory();
    searchAllFiles(mArchiveDirectory ,QString());
    initializeListStep();
    nextStep();
}

void ImportCalendarJob::nextStep()
{
    ++mIndex;
    if (mIndex < mListStep.count()) {
        Utils::StoredType type = mListStep.at(mIndex);
        if (type == Utils::Resources)
            restoreResources();
        if (type == Utils::Config)
            restoreConfig();
    } else {
        Q_EMIT jobFinished();
    }
}

void ImportCalendarJob::restoreResources()
{
    Q_EMIT info(i18n("Restore resources..."));
    QStringList listResource;
    listResource << restoreResourceFile(QString::fromLatin1("akonadi_ical_resource"), Utils::calendarPath(), storeCalendar);

    if (!mListResourceFile.isEmpty()) {
        QDir dir(mTempDirName);
        dir.mkdir(Utils::addressbookPath());
        const QString copyToDirName(mTempDirName + QLatin1Char('/') + Utils::calendarPath());

        for (int i = 0; i < mListResourceFile.size(); ++i) {
            resourceFiles value = mListResourceFile.at(i);
            QMap<QString, QVariant> settings;
            if (value.akonadiConfigFile.contains(QLatin1String("akonadi_icaldir_resource_"))) {
                const KArchiveEntry* fileResouceEntry = mArchiveDirectory->entry(value.akonadiConfigFile);
                if (fileResouceEntry && fileResouceEntry->isFile()) {
                    const KArchiveFile* file = static_cast<const KArchiveFile*>(fileResouceEntry);
                    file->copyTo(copyToDirName);
                    QString resourceName(file->name());

                    QString filename(file->name());
                    //TODO adapt filename otherwise it will use all the time the same filename.
                    qDebug()<<" filename :"<<filename;

                    KSharedConfig::Ptr resourceConfig = KSharedConfig::openConfig(copyToDirName + QLatin1Char('/') + resourceName);

                    const KUrl newUrl = Utils::adaptResourcePath(resourceConfig, storeCalendar);

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

                    const QString newResource = mCreateResource->createResource( QString::fromLatin1("akonadi_icaldir_resource"), filename, settings, true );
                    infoAboutNewResource(newResource);
                    listResource << newResource;
                    qDebug()<<" newResource"<<newResource;
                }
            }
        }
    }
    //It's maildir support. Need to add support
    startSynchronizeResources(listResource);
}

void ImportCalendarJob::addSpecificResourceSettings(KSharedConfig::Ptr resourceConfig, const QString &resourceName, QMap<QString, QVariant> &settings)
{
    if (resourceName == QLatin1String("akonadi_ical_resource")) {
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

void ImportCalendarJob::searchAllFiles(const KArchiveDirectory *dir,const QString &prefix)
{
    Q_FOREACH(const QString& entryName, dir->entries()) {
        const KArchiveEntry *entry = dir->entry(entryName);
        if (entry && entry->isDirectory()) {
            const QString newPrefix = (prefix.isEmpty() ? prefix : prefix + QLatin1Char('/')) + entryName;
            if (entryName == QLatin1String("calendar")) {
                storeCalendarArchiveResource(static_cast<const KArchiveDirectory*>(entry),entryName);
            } else {
                searchAllFiles(static_cast<const KArchiveDirectory*>(entry), newPrefix);
            }
        }
    }
}

void ImportCalendarJob::storeCalendarArchiveResource(const KArchiveDirectory *dir, const QString &prefix)
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
                    if (name.endsWith(QLatin1String("rc")) && (name.contains(QLatin1String("akonadi_ical_resource_"))
                                                               || name.contains(QLatin1String("akonadi_icaldir_resource_")))) {
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
                qDebug()<<" Problem in archive. number of file "<<lst.count();
            }
        }
    }
}


void ImportCalendarJob::restoreConfig()
{
    const QString korganizerPrinterrcStr(QLatin1String("calendar_printing.rc"));
    const QString oldKorganizerPrintrrcStr(QLatin1String("korganizer_printing.rc"));
    const KArchiveEntry* oldKorganizerPrinterEntry  = mArchiveDirectory->entry(Utils::configsPath() + oldKorganizerPrintrrcStr);
    if (oldKorganizerPrinterEntry && oldKorganizerPrinterEntry->isFile()) {
        const KArchiveFile* korganizerFile = static_cast<const KArchiveFile*>(oldKorganizerPrinterEntry);
        const QString oldKorganizerPrintrrc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + korganizerPrinterrcStr;
        if (QFile(oldKorganizerPrintrrc).exists()) {
            if (overwriteConfigMessageBox(oldKorganizerPrintrrc)) {
                copyToFile(korganizerFile, oldKorganizerPrintrrc, oldKorganizerPrintrrcStr, Utils::configsPath());
            }
        } else {
            copyToFile(korganizerFile, oldKorganizerPrintrrc, oldKorganizerPrintrrcStr, Utils::configsPath());
        }
    } else {
        const KArchiveEntry* korganizerPrinterEntry  = mArchiveDirectory->entry(Utils::configsPath() + korganizerPrinterrcStr);
        if (korganizerPrinterEntry && korganizerPrinterEntry->isFile()) {
            const KArchiveFile* korganizerFile = static_cast<const KArchiveFile*>(korganizerPrinterEntry);
            const QString korganizerPrinterrc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + korganizerPrinterrcStr;
            if (QFile(korganizerPrinterrc).exists()) {
                if (overwriteConfigMessageBox(korganizerPrinterrcStr)) {
                    copyToFile(korganizerFile, korganizerPrinterrc, korganizerPrinterrcStr, Utils::configsPath());
                }
            } else {
                copyToFile(korganizerFile, korganizerPrinterrc, korganizerPrinterrcStr, Utils::configsPath());
            }
        }
    }

    const QString korganizerStr(QLatin1String("korganizerrc"));
    const KArchiveEntry* korganizerrcentry  = mArchiveDirectory->entry(Utils::configsPath() + korganizerStr);
    if (korganizerrcentry && korganizerrcentry->isFile()) {
        const KArchiveFile* korganizerrcFile = static_cast<const KArchiveFile*>(korganizerrcentry);
        const QString korganizerrc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + korganizerStr;
        if (QFile(korganizerrc).exists()) {
            if (overwriteConfigMessageBox(korganizerStr)) {
                importkorganizerConfig(korganizerrcFile, korganizerrc, korganizerStr, Utils::configsPath());
            }
        } else {
            importkorganizerConfig(korganizerrcFile, korganizerrc, korganizerStr, Utils::configsPath());
        }
    }

    const QString korgacStr(QLatin1String("korgacrc"));
    const KArchiveEntry* korgacrcentry  = mArchiveDirectory->entry(Utils::configsPath() + korgacStr);
    if (korgacrcentry && korgacrcentry->isFile()) {
        const KArchiveFile* korgacrcFile = static_cast<const KArchiveFile*>(korgacrcentry);
        const QString korgacrc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + korgacStr;
        if (QFile(korgacrc).exists()) {
            if (overwriteConfigMessageBox(korgacStr)) {
                copyToFile(korgacrcFile, korgacrc, korgacStr, Utils::configsPath());
            }
        } else {
            copyToFile(korgacrcFile, korgacrc, korgacStr, Utils::configsPath());
        }
    }

    const QString freebusyStr(QLatin1String("freebusyurls"));
    const KArchiveEntry* freebusyentry  = mArchiveDirectory->entry(Utils::dataPath() + QLatin1String("korganizer/") + freebusyStr);
    if (freebusyentry && freebusyentry->isFile()) {
        const KArchiveFile* freebusyrcFile = static_cast<const KArchiveFile*>(freebusyentry);

        const QString freebusypath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/korganizer/") + freebusyStr ;
        if (QFile(freebusypath).exists()) {
            //TODO 4.12 merge it.
            if (overwriteConfigMessageBox(freebusyStr)) {
                copyToFile(freebusyrcFile, freebusypath, freebusyStr, Utils::dataPath() );
            }
        } else {
            copyToFile(freebusyrcFile, freebusypath, freebusyStr, Utils::dataPath());
        }
    }


    const KArchiveEntry *templateEntry  = mArchiveDirectory->entry(Utils::dataPath() + QLatin1String( "korganizer/templates/" ) );
    if (templateEntry && templateEntry->isDirectory()) {
        //TODO 4.12 verify if template already exists.
        const QString templatePath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + QLatin1String("korganizer/templates/");
        const KArchiveDirectory *templateDir = static_cast<const KArchiveDirectory*>(templateEntry);
        templateDir->copyTo(templatePath);
    }


    Q_EMIT info(i18n("Config restored."));
    nextStep();
}

void ImportCalendarJob::importkorganizerConfig(const KArchiveFile* file, const QString &config, const QString &filename,const QString &prefix)
{
    copyToFile(file, config, filename, prefix);
    KSharedConfig::Ptr korganizerConfig = KSharedConfig::openConfig(config);


    const QString collectionsStr(QLatin1String("GlobalCollectionSelection"));
    if (korganizerConfig->hasGroup(collectionsStr)) {
        KConfigGroup group = korganizerConfig->group(collectionsStr);
        const QString selectionKey(QLatin1String("Selection"));
        convertRealPathToCollectionList(group, selectionKey, true);
    }
    korganizerConfig->sync();
}


