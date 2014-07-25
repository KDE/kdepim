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

#include "abstractimportexportjob.h"
#include "archivestorage.h"
#include "synchronizeresourcejob.h"

#include "mailcommon/util/mailutil.h"

#include "pimcommon/util/createresource.h"

#include <KPIMIdentities/kpimidentities/identitymanager.h>
#include <KZip>
#include <QTemporaryDir>
#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardDirs>
#include <QTemporaryFile>

#include <QWidget>
#include <QProgressDialog>
#include <QFile>
#include <QDir>
#include <QStandardPaths>

int AbstractImportExportJob::sArchiveVersion = -1;

AbstractImportExportJob::AbstractImportExportJob(QWidget *parent, ArchiveStorage *archiveStorage, Utils::StoredTypes typeSelected, int numberOfStep)
    : QObject(parent),
      mTypeSelected(typeSelected),
      mArchiveStorage(archiveStorage),
      mIdentityManager(new KPIMIdentities::IdentityManager( false, this, "mIdentityManager" )),
      mParent(parent),
      mTempDir(0),
      mProgressDialog(0),
      mArchiveDirectory(0),
      mNumberOfStep(numberOfStep),
      mCreateResource(0),
      mIndex(-1)
{
}

AbstractImportExportJob::~AbstractImportExportJob()
{
    delete mCreateResource;
    delete mIdentityManager;
    delete mTempDir;
    delete mProgressDialog;
}

QProgressDialog *AbstractImportExportJob::progressDialog()
{
    return mProgressDialog;
}

void AbstractImportExportJob::createProgressDialog()
{
    if (!mProgressDialog) {
        mProgressDialog = new QProgressDialog(mParent);
        mProgressDialog->setWindowModality(Qt::WindowModal);
        mProgressDialog->setMinimum(0);
        mProgressDialog->setMaximum(mNumberOfStep);
    }
    mProgressDialog->show();
    mProgressDialog->setValue(0);
}


bool AbstractImportExportJob::wasCanceled() const
{
    if (mProgressDialog)
        return mProgressDialog->wasCanceled();
    return false;
}

void AbstractImportExportJob::increaseProgressDialog()
{
    if (mProgressDialog) {
        mProgressDialog->setValue(mProgressDialog->value()+1);
    }
}

void AbstractImportExportJob::showInfo(const QString &text)
{
    if (mProgressDialog) {
        mProgressDialog->setLabelText(text);
    }
    Q_EMIT info(text);
}

KZip *AbstractImportExportJob::archive()
{
    return mArchiveStorage->archive();
}

void AbstractImportExportJob::backupConfigFile(const QString &configFileName)
{
    const QString configrcStr(configFileName);
    const QString configrc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + configrcStr;
    if (QFile(configrc).exists()) {
        backupFile(configrc, Utils::configsPath(), configrcStr);
    }
}

void AbstractImportExportJob::backupFile(const QString&filename, const QString& path, const QString&storedName)
{
    if (QFile(filename).exists()) {
        const bool fileAdded  = archive()->addLocalFile(filename, path + storedName);
        if (fileAdded)
            Q_EMIT info(i18n("\"%1\" backup done.",storedName));
        else
            Q_EMIT error(i18n("\"%1\" cannot be exported.",storedName));
    }
}

int AbstractImportExportJob::mergeConfigMessageBox(const QString &configName) const
{
    return KMessageBox::warningYesNoCancel(mParent,i18n("\"%1\" already exists. Do you want to overwrite it or merge it?", configName),i18n("Restore"),KGuiItem(i18n("Overwrite")),KGuiItem(i18n("Merge")) );
}

bool AbstractImportExportJob::overwriteConfigMessageBox(const QString &configName) const
{
    return (KMessageBox::warningYesNo(mParent,i18n("\"%1\" already exists. Do you want to overwrite it?", configName),i18n("Restore")) == KMessageBox::Yes);
}

void AbstractImportExportJob::overwriteDirectory(const QString &path, const KArchiveEntry *entry)
{
    if (QDir(path).exists()) {
        if (overwriteDirectoryMessageBox(path)) {
            const KArchiveDirectory *dirEntry = static_cast<const KArchiveDirectory*>(entry);
            dirEntry->copyTo(path);
        }
    } else {
        const KArchiveDirectory *dirEntry = static_cast<const KArchiveDirectory*>(entry);
        dirEntry->copyTo(path);
    }
}

bool AbstractImportExportJob::overwriteDirectoryMessageBox(const QString &directory) const
{
    return (KMessageBox::warningYesNo(mParent,i18n("Directory \"%1\" already exists. Do you want to overwrite it?", directory),i18n("Restore")) == KMessageBox::Yes);
}

void AbstractImportExportJob::convertRealPathToCollection(KConfigGroup &group, const QString &currentKey, bool addCollectionPrefix)
{
    if (group.hasKey(currentKey)) {
        const QString path = group.readEntry(currentKey);
        if (!path.isEmpty()) {
            const Akonadi::Collection::Id id = convertPathToId(path);
            if (id != -1) {
                if (addCollectionPrefix) {
                    group.writeEntry(currentKey, QString::fromLatin1("c%1").arg(id));
                } else {
                    group.writeEntry(currentKey, id);
                }
            } else {
                group.deleteEntry(currentKey);
            }
        }
    }
}

void AbstractImportExportJob::convertRealPathToCollectionList(KConfigGroup &group, const QString &currentKey, bool addCollectionPrefix)
{
    if (group.hasKey(currentKey)) {
        const QStringList listExpension = group.readEntry(currentKey, QStringList());
        QStringList result;
        if (!listExpension.isEmpty()) {
            Q_FOREACH (const QString &collection, listExpension) {
                const Akonadi::Collection::Id id = convertPathToId(collection);
                if (id != -1 ) {
                    if (addCollectionPrefix) {
                        result<< QString::fromLatin1("c%1").arg(id);
                    } else {
                        result<< QString::fromLatin1("%1").arg(id);
                    }
                }
            }
            if (result.isEmpty()) {
                group.deleteEntry(currentKey);
            } else {
                group.writeEntry(currentKey, result);
            }
        }
    }
}

Akonadi::Collection::Id AbstractImportExportJob::convertPathToId(const QString& path)
{
    if (mHashConvertPathCollectionId.contains(path)) {
        return mHashConvertPathCollectionId.value(path);
    }
    const Akonadi::Collection::Id id = MailCommon::Util::convertFolderPathToCollectionId(path);
    if (id != -1) {
        mHashConvertPathCollectionId.insert(path,id);
    }
    return id;
}

void AbstractImportExportJob::initializeImportJob()
{
    if (mTempDir) {
        qDebug()<<" initializeImportJob already called";
    } else {
        mTempDir = new QTemporaryDir();
        mTempDirName = mTempDir->path();
        mCreateResource = new PimCommon::CreateResource();
        connect(mCreateResource,SIGNAL(createResourceInfo(QString)),SIGNAL(info(QString)));
        connect(mCreateResource,SIGNAL(createResourceError(QString)),SIGNAL(error(QString)));
    }
}

void AbstractImportExportJob::copyToDirectory(const KArchiveEntry *entry, const QString &dest)
{
    const KArchiveDirectory *subfolderDir = static_cast<const KArchiveDirectory*>(entry);
    subfolderDir->copyTo(dest);
    Q_EMIT info(i18n("\"%1\" was copied.",dest));
}

void AbstractImportExportJob::copyToFile(const KArchiveFile *archivefile, const QString &dest, const QString &filename, const QString &prefix)
{
    QDir dir(mTempDirName);
    const QString copyToDirName(mTempDirName + QLatin1Char('/') + prefix);
    const bool created = dir.mkpath(copyToDirName);
    if (!created) {
        qDebug()<<" directory :"<<prefix<<" not created";
    }

    archivefile->copyTo(copyToDirName);
    QFile file;
    file.setFileName(copyToDirName + QLatin1Char('/') + filename);

    //QFile doesn't overwrite => remove old file before
    //qDebug()<<" dest "<<dest;
    //qDebug()<<" file "<<file.fileName();
    QFile destination(dest);
    if (destination.exists()) {
        destination.remove();
    }
    if (!file.copy(dest)) {
        KMessageBox::error(mParent,i18n("File \"%1\" cannot be copied to \"%2\".",filename,dest),i18n("Copy file"));
    }
}

void AbstractImportExportJob::backupResourceDirectory(const Akonadi::AgentInstance &agent, const QString &defaultPath)
{
    const QString identifier = agent.identifier();
    const QString archivePath = defaultPath + identifier + QDir::separator();

    KUrl url = Utils::resourcePath(agent);
    if (!url.isEmpty()) {
        QString filename = url.fileName();
        if (QDir(url.path()).exists()) {
            const bool fileAdded  = archive()->addLocalDirectory(url.path(), archivePath + filename);
            if (fileAdded) {
                const QString errorStr = Utils::storeResources(archive(), identifier, archivePath);
                if (!errorStr.isEmpty())
                    Q_EMIT error(errorStr);
                Q_EMIT info(i18n("\"%1\" was backuped.",filename));

                url = Utils::akonadiAgentConfigPath(identifier);
                if (!url.isEmpty()) {
                    filename = url.fileName();

                    if (QDir(url.path()).exists()) {
                        const bool fileAdded  = archive()->addLocalFile(url.path(), archivePath + filename);
                        if (fileAdded)
                            Q_EMIT info(i18n("\"%1\" was backuped.",filename));
                        else
                            Q_EMIT error(i18n("\"%1\" file cannot be added to backup file.",filename));
                    }
                }

            } else {
                Q_EMIT error(i18n("\"%1\" file cannot be added to backup file.",filename));
            }
        } else {
            Q_EMIT error(i18n("Resource was not configured correctly, not archiving it."));
        }
    }
}

void AbstractImportExportJob::backupResourceFile(const Akonadi::AgentInstance &agent, const QString &defaultPath)
{
    const QString identifier = agent.identifier();
    const QString archivePath = defaultPath + identifier + QDir::separator();

    KUrl url = Utils::resourcePath(agent);
    if (!url.isEmpty()) {
        QString filename = url.fileName();
        const bool fileAdded  = archive()->addLocalFile(url.path(), archivePath + filename);
        if (fileAdded) {
            const QString errorStr = Utils::storeResources(archive(), identifier, archivePath);
            if (!errorStr.isEmpty())
                Q_EMIT error(errorStr);
            Q_EMIT info(i18n("\"%1\" was backuped.",filename));

            url = Utils::akonadiAgentConfigPath(identifier);
            if (!url.isEmpty()) {
                filename = url.fileName();
                const bool fileAdded  = archive()->addLocalFile(url.path(), archivePath + filename);
                if (fileAdded)
                    Q_EMIT info(i18n("\"%1\" was backuped.",filename));
                else
                    Q_EMIT error(i18n("\"%1\" file cannot be added to backup file.",filename));
            }

        } else {
            Q_EMIT error(i18n("\"%1\" file cannot be added to backup file.",filename));
        }
    }
}

QStringList AbstractImportExportJob::restoreResourceFile(const QString &resourceBaseName, const QString &defaultPath, const QString &storePath, bool overwriteResources)
{
    QStringList resourceToSync;
    //TODO fix sync config after created a resource
    if (!mListResourceFile.isEmpty()) {
        QDir dir(mTempDirName);
        dir.mkdir(defaultPath);
        const QString copyToDirName(mTempDirName + QLatin1Char('/') + defaultPath);

        for (int i = 0; i < mListResourceFile.size(); ++i) {
            resourceFiles value = mListResourceFile.at(i);
            QMap<QString, QVariant> settings;
            if (value.akonadiConfigFile.contains(resourceBaseName + QLatin1Char('_'))) {
                const KArchiveEntry* fileResouceEntry = mArchiveDirectory->entry(value.akonadiConfigFile);
                if (fileResouceEntry && fileResouceEntry->isFile()) {
                    const KArchiveFile* file = static_cast<const KArchiveFile*>(fileResouceEntry);
                    file->copyTo(copyToDirName);
                    QString resourceName(file->name());

                    QString filename(file->name());
                    //TODO adapt filename otherwise it will use all the time the same filename.
                    qDebug()<<" filename :"<<filename;

                    KSharedConfig::Ptr resourceConfig = KSharedConfig::openConfig(copyToDirName + QLatin1Char('/') + resourceName);

                    KUrl newUrl;
                    if (overwriteResources) {
                        newUrl = Utils::resourcePath(resourceConfig);
                    } else {
                        newUrl = Utils::adaptResourcePath(resourceConfig, storePath);
                    }
                    const QString dataFile = value.akonadiResources;
                    const KArchiveEntry* dataResouceEntry = mArchiveDirectory->entry(dataFile);
                    if (dataResouceEntry->isFile()) {
                        const KArchiveFile* file = static_cast<const KArchiveFile*>(dataResouceEntry);
                        file->copyTo(newUrl.directory());
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

                    addSpecificResourceSettings(resourceConfig, resourceBaseName, settings);

                    const QString newResource = mCreateResource->createResource( resourceBaseName, filename, settings );
                    infoAboutNewResource(newResource);
                    resourceToSync << newResource;
                    qDebug()<<" newResource"<<newResource;
                }
            }
        }
        Q_EMIT info(i18n("Resources restored."));
    } else {
        Q_EMIT error(i18n("No resources files found."));
    }
    return resourceToSync;
}

void AbstractImportExportJob::addSpecificResourceSettings(KSharedConfig::Ptr /*resourceConfig*/, const QString &/*resourceName*/, QMap<QString, QVariant> &/*settings*/)
{
    //Redefine it in subclass
}

void AbstractImportExportJob::extractZipFile(const KArchiveFile *file, const QString &source, const QString &destination)
{
    file->copyTo(source);
    QDir dest(destination);
    if (!dest.exists()) {
        dest.mkpath(destination);
    }
    QString errorMsg;
    KZip *zip = Utils::openZip(source + QLatin1Char('/') + file->name(), errorMsg);
    if (zip) {
        const KArchiveDirectory *zipDir = zip->directory();
        Q_FOREACH(const QString &entryName, zipDir->entries()) {
            const KArchiveEntry *entry = zipDir->entry(entryName);
            if (entry) {
                if (entry->isDirectory()) {
                    const KArchiveDirectory *dir = static_cast<const KArchiveDirectory*>(entry);
                    dir->copyTo(destination + QDir::separator() + dir->name(), true);
                } else if (entry->isFile()) {
                    const KArchiveFile *dir = static_cast<const KArchiveFile*>(entry);
                    dir->copyTo(destination);
                }
            }
        }
        delete zip;
    } else {
        Q_EMIT error(errorMsg);
    }
}

bool AbstractImportExportJob::backupFullDirectory(const KUrl &url, const QString &archivePath, const QString &archivename)
{
    QTemporaryFile tmp;
    tmp.open();
    KZip *vcarddirArchive = new KZip(tmp.fileName());
    vcarddirArchive->setCompression(KZip::NoCompression);
    bool result = vcarddirArchive->open(QIODevice::WriteOnly);
    if (!result) {
        delete vcarddirArchive;
        return false;
    }
    const QString filename = url.fileName();
    const bool vcarddirAdded = vcarddirArchive->addLocalDirectory(url.path(), QString());
    //TODO add MessageBox
    if (!vcarddirAdded) {
        delete vcarddirArchive;
        return false;
    }
    vcarddirArchive->setCompression(KZip::DeflateCompression);
    vcarddirArchive->close();
    tmp.close();

    const bool fileAdded = archive()->addLocalFile(tmp.fileName(), archivePath  + archivename);
    if (fileAdded)
        Q_EMIT info(i18n("\"%1\" was backuped.",filename));
    else
        Q_EMIT error(i18n("\"%1\" file cannot be added to backup file.",filename));

    delete vcarddirArchive;
    return fileAdded;
}

void AbstractImportExportJob::restoreConfigFile(const QString &configNameStr)
{
    const KArchiveEntry* configNameentry  = mArchiveDirectory->entry(Utils::configsPath() + configNameStr);
    if ( configNameentry &&  configNameentry->isFile()) {
        const KArchiveFile* configNameconfiguration = static_cast<const KArchiveFile*>(configNameentry);
        const QString configNamerc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + configNameStr;
        if (QFile(configNamerc).exists()) {
            //TODO 4.12 allow to merge config.
            if (overwriteConfigMessageBox(configNameStr)) {
                copyToFile(configNameconfiguration, configNamerc, configNameStr, Utils::configsPath());
            }
        } else {
            copyToFile(configNameconfiguration, configNamerc, configNameStr, Utils::configsPath());
        }
    }
}

void AbstractImportExportJob::infoAboutNewResource(const QString &resourceName)
{
    Q_EMIT info(i18n("Resource \'%1\' created.", resourceName));
}

int AbstractImportExportJob::archiveVersion()
{
    return sArchiveVersion;
}

void AbstractImportExportJob::setArchiveVersion(int version)
{
    sArchiveVersion = version;
}

void AbstractImportExportJob::slotSynchronizeInstanceFailed(const QString &instance)
{
    Q_EMIT error(i18n("Failed to synchronize %1.", instance));
}

void AbstractImportExportJob::slotSynchronizeInstanceDone(const QString &instance)
{
    Q_EMIT info(i18n("Resource %1 synchronized.", instance));
}

void AbstractImportExportJob::slotAllResourceSynchronized()
{
    Q_EMIT info(i18n("All resources synchronized."));
    nextStep();
}

void AbstractImportExportJob::nextStep()
{
    //Implement in sub class.
}

void AbstractImportExportJob::startSynchronizeResources(const QStringList &listResourceToSync)
{
    SynchronizeResourceJob *job = new SynchronizeResourceJob(this);
    job->setListResources(listResourceToSync);
    connect(job, SIGNAL(synchronizationFinished()), SLOT(slotAllResourceSynchronized()));
    connect(job, SIGNAL(synchronizationInstanceDone(QString)), SLOT(slotSynchronizeInstanceDone(QString)));
    connect(job, SIGNAL(synchronizationInstanceFailed(QString)), SLOT(slotSynchronizeInstanceFailed(QString)));
    job->start();
}

void AbstractImportExportJob::initializeListStep()
{
    if (mTypeSelected & Utils::MailTransport)
        mListStep << Utils::MailTransport;
    if (mTypeSelected & Utils::Mails)
        mListStep << Utils::Mails;
    if (mTypeSelected & Utils::Resources)
        mListStep << Utils::Resources;
    if (mTypeSelected & Utils::Identity)
        mListStep << Utils::Identity;
    if (mTypeSelected & Utils::Config)
        mListStep << Utils::Config;
    if (mTypeSelected & Utils::AkonadiDb)
        mListStep << Utils::AkonadiDb;
}
