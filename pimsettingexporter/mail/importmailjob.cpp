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

#include "importmailjob.h"
#include "akonadidatabase.h"
#include "archivestorage.h"

#include "mailcommon/filter/filtermanager.h"
#include "mailcommon/filter/filterimporterexporter.h"
#include "mailcommon/util/mailutil.h"
#include "pimcommon/util/createresource.h"

#include "messageviewer/utils/kcursorsaver.h"


#include <mailtransport/transportmanager.h>

#include <kpimidentities/identitymanager.h>
#include <kpimidentities/identity.h>
#include <KStandardDirs>
#include <KLocalizedString>
#include <KProcess>
#include <KTemporaryFile>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KMessageBox>
#include <KArchiveFile>
#include <KZip>

#include <akonadi/agenttype.h>
#include <akonadi/agentmanager.h>
#include <akonadi/agentinstancecreatejob.h>

#include <QMetaMethod>
#include <QDir>

using namespace Akonadi;

static const QString storeMails = QLatin1String("backupmail/");

ImportMailJob::ImportMailJob(QWidget *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep)
    : AbstractImportExportJob(parent,archiveStorage,typeSelected,numberOfStep)
{
    initializeImportJob();
}

ImportMailJob::~ImportMailJob()
{
}

void ImportMailJob::start()
{
    Q_EMIT title(i18n("Start import KMail settings..."));
    mArchiveDirectory = archive()->directory();
    searchAllFiles(mArchiveDirectory,QString());
    if (!mFileList.isEmpty()|| !mListResourceFile.isEmpty()) {
        initializeListStep();
        nextStep();
    } else {
        Q_EMIT jobFinished();
    }
}

void ImportMailJob::nextStep()
{
    ++mIndex;
    if (mIndex < mListStep.count()) {
        Utils::StoredType type = mListStep.at(mIndex);
        if (type == Utils::MailTransport)
            restoreTransports();
        if (type == Utils::Mails)
            restoreMails();
        if (type == Utils::Resources)
            restoreResources();
        if (type == Utils::Identity)
            restoreIdentity();
        if (type == Utils::Config)
            restoreConfig();
        if (type == Utils::AkonadiDb)
            restoreAkonadiDb();
    } else {
        Q_EMIT jobFinished();
    }
}

void ImportMailJob::searchAllFiles(const KArchiveDirectory*dir,const QString&prefix)
{
    Q_FOREACH(const QString& entryName, dir->entries()) {
        const KArchiveEntry *entry = dir->entry(entryName);
        if (entry && entry->isDirectory()) {
            const QString newPrefix = (prefix.isEmpty() ? prefix : prefix + QLatin1Char('/')) + entryName;
            if (entryName == QLatin1String("mails")) {
                storeMailArchiveResource(static_cast<const KArchiveDirectory*>(entry),entryName);
            } else {
                searchAllFiles(static_cast<const KArchiveDirectory*>(entry), newPrefix);
            }
        } else if (entry) {
            const QString fileName = prefix.isEmpty() ? entry->name() : prefix + QLatin1Char('/') + entry->name();
            mFileList<<fileName;
        }
    }
}

void ImportMailJob::storeMailArchiveResource(const KArchiveDirectory*dir, const QString& prefix)
{
    Q_FOREACH(const QString& entryName, dir->entries()) {
        const KArchiveEntry *entry = dir->entry(entryName);
        if (entry && entry->isDirectory()) {
            const KArchiveDirectory*resourceDir = static_cast<const KArchiveDirectory*>(entry);
            const QStringList lst = resourceDir->entries();
            if (lst.count() >= 2) {
                const QString archPath(prefix + QLatin1Char('/') + entryName + QLatin1Char('/'));
                resourceFiles files;
                Q_FOREACH(const QString &name, lst) {
                    if (name.endsWith(QLatin1String("rc"))&&
                            (name.contains(QLatin1String("akonadi_mbox_resource_")) ||
                             name.contains(QLatin1String("akonadi_mixedmaildir_resource_")) ||
                             name.contains(QLatin1String("akonadi_maildir_resource_")))) {
                        files.akonadiConfigFile = archPath + name;
                    } else if (name.startsWith(Utils::prefixAkonadiConfigFile())) {
                        files.akonadiAgentConfigFile = archPath + name;
                    } else {
                        files.akonadiResources = archPath + name;
                    }
                }
                //Show debug:
                files.debug();
                mListResourceFile.append(files);
            } else {
                kDebug()<<" Problem in archive. number of file "<<lst.count();
            }
        }
    }
}

void ImportMailJob::restoreTransports()
{
    const QString path = Utils::transportsPath()+QLatin1String("mailtransports");
    if (!mFileList.contains(path)) {
        Q_EMIT error(i18n("mailtransports file could not be found in the archive."));
    } else {
        Q_EMIT info(i18n("Restore transports..."));
        MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
        const KArchiveEntry* transport = mArchiveDirectory->entry(path);
        if (transport && transport->isFile()) {
            const KArchiveFile* fileTransport = static_cast<const KArchiveFile*>(transport);

            fileTransport->copyTo(mTempDirName);
            KSharedConfig::Ptr transportConfig = KSharedConfig::openConfig(mTempDirName + QLatin1Char('/') +QLatin1String("mailtransports"));

            int defaultTransport = -1;
            if (transportConfig->hasGroup(QLatin1String("General"))) {
                KConfigGroup group = transportConfig->group(QLatin1String("General"));
                defaultTransport = group.readEntry(QLatin1String("default-transport"),-1);
            }

            const QStringList transportList = transportConfig->groupList().filter( QRegExp( QLatin1String("Transport \\d+") ) );
            Q_FOREACH(const QString&transport, transportList) {
                KConfigGroup group = transportConfig->group(transport);
                const int transportId = group.readEntry(QLatin1String("id"), -1);
                MailTransport::Transport* mt = MailTransport::TransportManager::self()->createTransport();
                mt->setName(group.readEntry(QLatin1String("name")));
                const QString hostStr(QLatin1String("host"));
                if (group.hasKey(hostStr)) {
                    mt->setHost(group.readEntry(hostStr));
                }
                const QString portStr(QLatin1String("port"));
                if (group.hasKey(portStr)) {
                    mt->setPort(group.readEntry(portStr,-1));
                }
                const QString userNameStr(QLatin1String("userName"));
                if (group.hasKey(userNameStr)) {
                    mt->setUserName(group.readEntry(userNameStr));
                }
                const QString precommandStr(QLatin1String("precommand"));
                if (group.hasKey(precommandStr)) {
                    mt->setPrecommand(group.readEntry(precommandStr));
                }
                const QString requiresAuthenticationStr(QLatin1String("requiresAuthentication"));
                if (group.hasKey(requiresAuthenticationStr)) {
                    mt->setRequiresAuthentication(group.readEntry(requiresAuthenticationStr,false));
                }
                const QString specifyHostnameStr(QLatin1String("specifyHostname"));
                if (group.hasKey(specifyHostnameStr)) {
                    mt->setSpecifyHostname(group.readEntry(specifyHostnameStr,false));
                }
                const QString localHostnameStr(QLatin1String("localHostname"));
                if (group.hasKey(localHostnameStr)) {
                    mt->setLocalHostname(group.readEntry(localHostnameStr));
                }
                const QString specifySenderOverwriteAddressStr(QLatin1String("specifySenderOverwriteAddress"));
                if (group.hasKey(specifySenderOverwriteAddressStr)) {
                    mt->setSpecifySenderOverwriteAddress(group.readEntry(specifySenderOverwriteAddressStr,false));
                }
                const QString storePasswordStr(QLatin1String("storePassword"));
                if (group.hasKey(storePasswordStr)) {
                    mt->setStorePassword(group.readEntry(storePasswordStr,false));
                }
                const QString senderOverwriteAddressStr(QLatin1String("senderOverwriteAddress"));
                if (group.hasKey(senderOverwriteAddressStr)) {
                    mt->setSenderOverwriteAddress(group.readEntry(senderOverwriteAddressStr));
                }
                const QString encryptionStr(QLatin1String("encryption"));
                if (group.hasKey(encryptionStr)) {
                    mt->setEncryption(group.readEntry(encryptionStr,1)); //TODO verify
                }
                const QString authenticationTypeStr(QLatin1String("authenticationType"));
                if (group.hasKey(authenticationTypeStr)) {
                    mt->setAuthenticationType(group.readEntry(authenticationTypeStr,1));//TODO verify
                }

                mt->forceUniqueName();
                mt->writeConfig();
                MailTransport::TransportManager::self()->addTransport( mt );
                if ( transportId == defaultTransport )
                    MailTransport::TransportManager::self()->setDefaultTransport( mt->id() );
                mHashTransport.insert(transportId, mt->id());
            }
            Q_EMIT info(i18n("Transports restored."));
        } else {
            Q_EMIT error(i18n("Failed to restore transports file."));
        }
    }
    nextStep();
}

void ImportMailJob::restoreResources()
{
    Q_EMIT info(i18n("Restore resources..."));
    MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
    QDir dir(mTempDirName);
    dir.mkdir(Utils::resourcesPath());
    Q_FOREACH(const QString& filename, mFileList) {
        if (filename.startsWith(Utils::resourcesPath())) {
            const KArchiveEntry* fileEntry = mArchiveDirectory->entry(filename);
            if (fileEntry && fileEntry->isFile()) {
                const KArchiveFile* file = static_cast<const KArchiveFile*>(fileEntry);
                const QString destDirectory = mTempDirName + QLatin1Char('/') + Utils::resourcesPath();

                file->copyTo(destDirectory);

                const QString filename(file->name());
                const QString resourceFileName = destDirectory + QLatin1Char('/') + filename;
                KSharedConfig::Ptr resourceConfig = KSharedConfig::openConfig(resourceFileName);
                QMap<QString, QVariant> settings;
                if (filename.contains(QLatin1String("pop3"))) {
                    KConfigGroup general = resourceConfig->group(QLatin1String("General"));
                    if (general.hasKey(QLatin1String("login"))) {
                        settings.insert(QLatin1String("Login"),general.readEntry("login"));
                    }
                    if (general.hasKey(QLatin1String("host"))) {
                        settings.insert(QLatin1String("Host"),general.readEntry("host"));
                    }
                    if (general.hasKey(QLatin1String("port"))) {
                        settings.insert(QLatin1String("Port"),general.readEntry("port",110));
                    }
                    if (general.hasKey(QLatin1String("authenticationMethod"))) {
                        settings.insert(QLatin1String("AuthenticationMethod"),general.readEntry("authenticationMethod",7));
                    }
                    if (general.hasKey(QLatin1String("useSSL"))) {
                        settings.insert(QLatin1String("UseSSL"),general.readEntry("useSSL",false));
                    }
                    if (general.hasKey(QLatin1String("useTLS"))) {
                        settings.insert(QLatin1String("UseTLS"),general.readEntry("useTLS",false));
                    }
                    if (general.hasKey(QLatin1String("pipelining"))) {
                        settings.insert(QLatin1String("Pipelining"),general.readEntry("pipelining",false));
                    }
                    if (general.hasKey(QLatin1String("leaveOnServer"))) {
                        settings.insert(QLatin1String("LeaveOnServer"),general.readEntry("leaveOnServer",false));
                    }
                    if (general.hasKey(QLatin1String("leaveOnServerDays"))) {
                        settings.insert(QLatin1String("LeaveOnServerDays"),general.readEntry("leaveOnServerDays",-1));
                    }
                    if (general.hasKey(QLatin1String("leaveOnServerCount"))) {
                        settings.insert(QLatin1String("LeaveOnServerCount"),general.readEntry("leaveOnServerCount",-1));
                    }
                    if (general.hasKey(QLatin1String("leaveOnServerSize"))) {
                        settings.insert(QLatin1String("LeaveOnServerSize"),general.readEntry("leaveOnServerSize",-1));
                    }
                    if (general.hasKey(QLatin1String("filterOnServer"))) {
                        settings.insert(QLatin1String("FilterOnServer"),general.readEntry("filterOnServer",false));
                    }
                    if (general.hasKey(QLatin1String("filterCheckSize"))) {
                        settings.insert(QLatin1String("FilterCheckSize"),general.readEntry("filterCheckSize"));
                    }
                    if (general.hasKey(QLatin1String("targetCollection"))) {
                        const Akonadi::Collection::Id collection = convertPathToId(general.readEntry("targetCollection"));
                        if (collection != -1) {
                            settings.insert(QLatin1String("TargetCollection"),collection);
                        }
                    }
                    if (general.hasKey(QLatin1String("precommand"))) {
                        settings.insert(QLatin1String("Precommand"),general.readEntry("precommand"));
                    }
                    if (general.hasKey(QLatin1String("intervalCheckEnabled"))) {
                        settings.insert(QLatin1String("IntervalCheckEnabled"),general.readEntry("intervalCheckEnabled",false));
                    }
                    if (general.hasKey(QLatin1String("intervalCheckInterval"))) {
                        settings.insert(QLatin1String("IntervalCheckInterval"),general.readEntry("intervalCheckInterval",5));
                    }

                    KConfigGroup leaveOnserver = resourceConfig->group(QLatin1String("LeaveOnServer"));

                    if (leaveOnserver.hasKey(QLatin1String("seenUidList"))) {
                        settings.insert(QLatin1String("SeenUidList"),leaveOnserver.readEntry("seenUidList",QStringList()));
                    }
                    if (leaveOnserver.hasKey(QLatin1String("seenUidTimeList"))) {
                        //FIXME
                        //settings.insert(QLatin1String("SeenUidTimeList"),QVariant::fromValue<QList<int> >(leaveOnserver.readEntry("seenUidTimeList",QList<int>())));
                    }
                    if (leaveOnserver.hasKey(QLatin1String("downloadLater"))) {
                        settings.insert(QLatin1String("DownloadLater"),leaveOnserver.readEntry("downloadLater",QStringList()));
                    }
                    const QString newResource = mCreateResource->createResource( QString::fromLatin1("akonadi_pop3_resource"), filename, settings );
                    if (!newResource.isEmpty()) {
                        mHashResources.insert(filename,newResource);
                        infoAboutNewResource(newResource);
                    }
                } else if (filename.contains(QLatin1String("imap")) || filename.contains(QLatin1String("kolab_"))) {
                    KConfigGroup network = resourceConfig->group(QLatin1String("network"));
                    if (network.hasKey(QLatin1String("Authentication"))) {
                        settings.insert(QLatin1String("Authentication"),network.readEntry("Authentication",1));
                    }
                    if (network.hasKey(QLatin1String("ImapPort"))) {
                        settings.insert(QLatin1String("ImapPort"),network.readEntry("ImapPort",993));
                    }
                    if (network.hasKey(QLatin1String("ImapServer"))) {
                        settings.insert(QLatin1String("ImapServer"),network.readEntry("ImapServer"));
                    }
                    if (network.hasKey(QLatin1String("Safety"))) {
                        settings.insert(QLatin1String("Safety"),network.readEntry("Safety","SSL"));
                    }
                    if (network.hasKey(QLatin1String("SubscriptionEnabled"))) {
                        settings.insert(QLatin1String("SubscriptionEnabled"),network.readEntry("SubscriptionEnabled",false));
                    }
                    if (network.hasKey(QLatin1String("UserName"))) {
                        settings.insert(QLatin1String("UserName"),network.readEntry("UserName"));
                    }

                    if (network.hasKey(QLatin1String("SessionTimeout"))) {
                        settings.insert(QLatin1String("SessionTimeout"),network.readEntry("SessionTimeout",30));
                    }

                    KConfigGroup cache = resourceConfig->group(QLatin1String("cache"));

                    if (cache.hasKey(QLatin1String("AccountIdentity"))) {
                        const int identity = cache.readEntry("AccountIdentity",-1);
                        if (identity!=-1) {
                            if (mHashIdentity.contains(identity)) {
                                settings.insert(QLatin1String("AccountIdentity"),mHashIdentity.value(identity));
                            } else {
                                settings.insert(QLatin1String("AccountIdentity"),identity);
                            }
                        }
                    }
                    if (cache.hasKey(QLatin1String("IntervalCheckEnabled"))) {
                        settings.insert(QLatin1String("IntervalCheckEnabled"),cache.readEntry("IntervalCheckEnabled",true));
                    }
                    if (cache.hasKey(QLatin1String("RetrieveMetadataOnFolderListing"))) {
                        settings.insert(QLatin1String("RetrieveMetadataOnFolderListing"),cache.readEntry("RetrieveMetadataOnFolderListing",true));
                    }
                    if (cache.hasKey(QLatin1String("AutomaticExpungeEnabled"))) {
                        settings.insert(QLatin1String("AutomaticExpungeEnabled"),cache.readEntry("AutomaticExpungeEnabled",true));
                    }
                    if (cache.hasKey(QLatin1String(""))) {
                        settings.insert(QLatin1String(""),cache.readEntry(""));
                    }
                    if (cache.hasKey(QLatin1String("DisconnectedModeEnabled"))) {
                        settings.insert(QLatin1String("DisconnectedModeEnabled"),cache.readEntry("DisconnectedModeEnabled",false));
                    }
                    if (cache.hasKey(QLatin1String("IntervalCheckTime"))) {
                        settings.insert(QLatin1String("IntervalCheckTime"),cache.readEntry("IntervalCheckTime",-1));
                    }
                    if (cache.hasKey(QLatin1String("UseDefaultIdentity"))) {
                        settings.insert(QLatin1String("UseDefaultIdentity"),cache.readEntry("UseDefaultIdentity",true));
                    }
                    if (cache.hasKey(QLatin1String("TrashCollection"))) {
                        const Akonadi::Collection::Id collection = convertPathToId(cache.readEntry("TrashCollection"));
                        if (collection != -1) {
                            settings.insert(QLatin1String("TrashCollection"),collection);
                        } else {
                            kDebug()<<" Use default trash folder";
                        }
                    }


                    KConfigGroup siever = resourceConfig->group(QLatin1String("siever"));
                    if (siever.hasKey(QLatin1String("SieveSupport"))) {
                        settings.insert(QLatin1String("SieveSupport"),siever.readEntry("SieveSupport",false));
                    }
                    if (siever.hasKey(QLatin1String("SieveReuseConfig"))) {
                        settings.insert(QLatin1String("SieveReuseConfig"),siever.readEntry("SieveReuseConfig",true));
                    }
                    if (siever.hasKey(QLatin1String("SievePort"))) {
                        settings.insert(QLatin1String("SievePort"),siever.readEntry("SievePort",4190));
                    }
                    if (siever.hasKey(QLatin1String("SieveAlternateUrl"))) {
                        settings.insert(QLatin1String("SieveAlternateUrl"),siever.readEntry("SieveAlternateUrl"));
                    }
                    if (siever.hasKey(QLatin1String("SieveVacationFilename"))) {
                        settings.insert(QLatin1String("SieveVacationFilename"),siever.readEntry("SieveVacationFilename"));
                    }

                    QString newResource;
                    if (filename.contains(QLatin1String("kolab_"))) {
                        newResource = mCreateResource->createResource( QString::fromLatin1("akonadi_kolab_resource"), filename, settings );
                    } else {
                        newResource = mCreateResource->createResource( QString::fromLatin1("akonadi_imap_resource"), filename, settings );
                    }
                    if (!newResource.isEmpty()) {
                        mHashResources.insert(filename,newResource);
                        infoAboutNewResource(newResource);
                    }
                } else {
                    kDebug()<<" problem with resource";
                }
            }
        }
    }
    //TODO synctree ?

    Q_EMIT info(i18n("Resources restored."));
    nextStep();
}

void ImportMailJob::restoreMails()
{
    QStringList listResourceToSync;
    Q_EMIT info(i18n("Restore mails..."));
    MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
    QDir dir(mTempDirName);
    dir.mkdir(Utils::mailsPath());
    const QString copyToDirName(mTempDirName + QLatin1Char('/') + Utils::mailsPath());
    for (int i = 0; i < mListResourceFile.size(); ++i) {

        resourceFiles value = mListResourceFile.at(i);
        value.debug();
        const QString resourceFile = value.akonadiConfigFile;
        const KArchiveEntry* fileResouceEntry = mArchiveDirectory->entry(resourceFile);
        if (fileResouceEntry && fileResouceEntry->isFile()) {
            const KArchiveFile* file = static_cast<const KArchiveFile*>(fileResouceEntry);
            file->copyTo(copyToDirName);
            QString resourceName(file->name());
            QString filename(file->name());
            //qDebug()<<" filename "<<filename<<" resourceName"<<resourceName;
            KSharedConfig::Ptr resourceConfig = KSharedConfig::openConfig(copyToDirName + QLatin1Char('/') + resourceName);

            const KUrl newUrl = Utils::adaptResourcePath(resourceConfig, storeMails);

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


            QMap<QString, QVariant> settings;
            if (resourceName.contains(QLatin1String("akonadi_mbox_resource_"))) {
                const QString dataFile = value.akonadiResources;
                const KArchiveEntry* dataResouceEntry = mArchiveDirectory->entry(dataFile);
                if (dataResouceEntry->isFile()) {
                    const KArchiveFile* file = static_cast<const KArchiveFile*>(dataResouceEntry);
                    file->copyTo(newUrl.path());
                }
                settings.insert(QLatin1String("Path"),newUrl.path());


                KConfigGroup general = resourceConfig->group(QLatin1String("General"));
                if (general.hasKey(QLatin1String("DisplayName"))) {
                    settings.insert(QLatin1String("DisplayName"),general.readEntry(QLatin1String("DisplayName")));
                }
                if (general.hasKey(QLatin1String("ReadOnly"))) {
                    settings.insert(QLatin1String("ReadOnly"),general.readEntry(QLatin1String("ReadOnly"),false));
                }
                if (general.hasKey(QLatin1String("MonitorFile"))) {
                    settings.insert(QLatin1String("MonitorFile"),general.readEntry(QLatin1String("MonitorFile"),false));
                }
                if (resourceConfig->hasGroup(QLatin1String("Locking"))) {
                    KConfigGroup locking = resourceConfig->group(QLatin1String("Locking"));
                    if (locking.hasKey(QLatin1String("Lockfile"))) {
                        settings.insert(QLatin1String("Lockfile"),locking.readEntry(QLatin1String("Lockfile")));
                    }
                    //TODO verify
                    if (locking.hasKey(QLatin1String("LockfileMethod"))) {
                        settings.insert(QLatin1String("LockfileMethod"),locking.readEntry(QLatin1String("LockfileMethod"),4));
                    }
                }
                if (resourceConfig->hasGroup(QLatin1String("Compacting"))) {
                    KConfigGroup compacting = resourceConfig->group(QLatin1String("Compacting"));
                    if (compacting.hasKey(QLatin1String("CompactFrequency"))) {
                        settings.insert(QLatin1String("CompactFrequency"),compacting.readEntry(QLatin1String("CompactFrequency"),1));
                    }
                    if (compacting.hasKey(QLatin1String("MessageCount"))) {
                        settings.insert(QLatin1String("MessageCount"),compacting.readEntry(QLatin1String("MessageCount"),50));
                    }
                }
                const QString newResource = mCreateResource->createResource( QString::fromLatin1("akonadi_mbox_resource"), filename, settings );
                if (!newResource.isEmpty()) {
                    mHashResources.insert(filename,newResource);
                    infoAboutNewResource(newResource);
                }

            } else if (resourceName.contains(QLatin1String("akonadi_maildir_resource_")) ||
                      resourceName.contains(QLatin1String("akonadi_mixedmaildir_resource_"))) {
                settings.insert(QLatin1String("Path"),newUrl.path());
                KConfigGroup general = resourceConfig->group(QLatin1String("General"));
                if (general.hasKey(QLatin1String("TopLevelIsContainer"))) {
                    settings.insert(QLatin1String("TopLevelIsContainer"),general.readEntry(QLatin1String("TopLevelIsContainer"),false));
                }
                if (general.hasKey(QLatin1String("ReadOnly"))) {
                    settings.insert(QLatin1String("ReadOnly"),general.readEntry(QLatin1String("ReadOnly"),false));
                }
                if (general.hasKey(QLatin1String("MonitorFilesystem"))) {
                    settings.insert(QLatin1String("MonitorFilesystem"),general.readEntry(QLatin1String("MonitorFilesystem"),true));
                }

                const QString newResource = mCreateResource->createResource( resourceName.contains(QLatin1String("akonadi_mixedmaildir_resource_")) ?
                                                                                 QString::fromLatin1("akonadi_mixedmaildir_resource")
                                                                               : QString::fromLatin1("akonadi_maildir_resource")
                                                                                 , filename, settings );
                if (!newResource.isEmpty()) {
                    mHashResources.insert(filename,newResource);
                    infoAboutNewResource(newResource);
                }

                const QString mailFile = value.akonadiResources;
                const KArchiveEntry* dataResouceEntry = mArchiveDirectory->entry(mailFile);
                if (dataResouceEntry && dataResouceEntry->isFile()) {
                    const KArchiveFile* file = static_cast<const KArchiveFile*>(dataResouceEntry);
                    //TODO Fix me not correct zip filename.
                    extractZipFile(file, copyToDirName, newUrl.path());
                }
                listResourceToSync << newResource;
            } else {
                kDebug()<<" resource name not supported "<<resourceName;
            }
            //qDebug()<<"url "<<url;
        }
    }
    Q_EMIT info(i18n("Mails restored."));
    startSynchronizeResources(listResourceToSync);
}

void ImportMailJob::restoreConfig()
{
    MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
    const QString filtersPath(Utils::configsPath() + QLatin1String("filters"));
    if (!mFileList.contains(filtersPath)) {
        Q_EMIT error(i18n("filters file could not be found in the archive."));
    } else {
        const KArchiveEntry* filter = mArchiveDirectory->entry(filtersPath);
        if (filter && filter->isFile()) {
            const KArchiveFile* fileFilter = static_cast<const KArchiveFile*>(filter);

            fileFilter->copyTo(mTempDirName);
            const QString filterFileName(mTempDirName + QLatin1Char('/') +QLatin1String("filters"));
            KSharedConfig::Ptr filtersConfig = KSharedConfig::openConfig(filterFileName);
            const QStringList filterList = filtersConfig->groupList().filter( QRegExp( QLatin1String("Filter #\\d+") ) );
            Q_FOREACH(const QString&filterStr, filterList) {
                KConfigGroup group = filtersConfig->group(filterStr);
                const QString accountStr(QLatin1String("accounts-set"));
                if (group.hasKey(accountStr)) {
                    const QString accounts = group.readEntry(accountStr);
                    if (!accounts.isEmpty()) {
                        const QStringList lstAccounts = accounts.split(QLatin1Char(','));
                        QStringList newLstAccounts;
                        Q_FOREACH(const QString&acc, lstAccounts) {
                            if (mHashResources.contains(acc)) {
                                newLstAccounts.append(mHashResources.value(acc));
                            } else {
                                newLstAccounts.append(acc);
                            }
                        }
                        group.writeEntry(accountStr,newLstAccounts);
                    }
                }
                const int numActions = group.readEntry( "actions", 0 );
                QString actName;
                QString argsName;
                for ( int i=0 ; i < numActions ; ++i ) {
                    actName.sprintf("action-name-%d", i);
                    argsName.sprintf("action-args-%d", i);
                    const QString actValue = group.readEntry(actName);
                    if (actValue==QLatin1String("set identity")) {
                        const int argsValue = group.readEntry(argsName,-1);
                        if (argsValue!=-1) {
                            if (mHashIdentity.contains(argsValue)) {
                                group.writeEntry(argsName,mHashIdentity.value(argsValue));
                            }
                        }
                    } else if (actValue==QLatin1String("set transport")) {
                        const int argsValue = group.readEntry(argsName,-1);
                        if (argsValue!=-1) {
                            if (mHashTransport.contains(argsValue)) {
                                group.writeEntry(argsName,mHashTransport.value(argsValue));
                            }
                        }
                    }
                }
            }
            filtersConfig->sync();

            bool canceled = false;
            MailCommon::FilterImporterExporter exportFilters;
            QList<MailCommon::MailFilter*> lstFilter = exportFilters.importFilters(canceled, MailCommon::FilterImporterExporter::KMailFilter, filterFileName);
            if (canceled) {
                MailCommon::FilterManager::instance()->appendFilters(lstFilter);
            }
        }
    }
    const QString kmailsnippetrcStr(QLatin1String("kmailsnippetrc"));
    const KArchiveEntry* kmailsnippetentry  = mArchiveDirectory->entry(Utils::configsPath() + kmailsnippetrcStr);
    if (kmailsnippetentry && kmailsnippetentry->isFile()) {
        const KArchiveFile* kmailsnippet = static_cast<const KArchiveFile*>(kmailsnippetentry);
        const QString kmailsnippetrc = KStandardDirs::locateLocal( "config",  kmailsnippetrcStr);
        if (QFile(kmailsnippetrc).exists()) {
            //TODO 4.13 allow to merge config.
            if (overwriteConfigMessageBox(kmailsnippetrcStr)) {
                copyToFile(kmailsnippet, kmailsnippetrc,kmailsnippetrcStr,Utils::configsPath());
            }
        } else {
            copyToFile(kmailsnippet, kmailsnippetrc,kmailsnippetrcStr,Utils::configsPath());
        }
    }

    const QString labldaprcStr(QLatin1String("kabldaprc"));
    const KArchiveEntry* kabldapentry  = mArchiveDirectory->entry(Utils::configsPath() + labldaprcStr);
    if (kabldapentry && kabldapentry->isFile()) {
        const KArchiveFile* kabldap= static_cast<const KArchiveFile*>(kabldapentry);
        const QString kabldaprc = KStandardDirs::locateLocal( "config",  labldaprcStr);
        if (QFile(kabldaprc).exists()) {
            const int result = mergeConfigMessageBox(labldaprcStr);
            if ( result == KMessageBox::Yes) {
                copyToFile(kabldap, kabldaprc, labldaprcStr,Utils::configsPath());
            } else if (result == KMessageBox::No) {
                mergeLdapConfig(kabldap,labldaprcStr,Utils::configsPath());
            }
        } else {
            copyToFile(kabldap, kabldaprc, labldaprcStr,Utils::configsPath());
        }
    }
    const QString archiveconfigurationrcStr(QLatin1String("akonadi_archivemail_agentrc"));
    const KArchiveEntry* archiveconfigurationentry  = mArchiveDirectory->entry(Utils::configsPath() + archiveconfigurationrcStr);
    if ( archiveconfigurationentry &&  archiveconfigurationentry->isFile()) {
        const KArchiveFile* archiveconfiguration = static_cast<const KArchiveFile*>(archiveconfigurationentry);
        const QString archiveconfigurationrc = KStandardDirs::locateLocal( "config",  archiveconfigurationrcStr);
        if (QFile(archiveconfigurationrc).exists()) {
            const int result = mergeConfigMessageBox(archiveconfigurationrcStr);
            if ( result == KMessageBox::Yes) {
                importArchiveConfig(archiveconfiguration, archiveconfigurationrc, archiveconfigurationrcStr, Utils::configsPath());
            } else if (result == KMessageBox::No) {
                mergeArchiveMailAgentConfig(archiveconfiguration,archiveconfigurationrcStr,Utils::configsPath());
            }
        } else {
            importArchiveConfig(archiveconfiguration, archiveconfigurationrc, archiveconfigurationrcStr, Utils::configsPath());
        }
    }

    const QString templatesconfigurationrcStr(QLatin1String("templatesconfigurationrc"));
    const KArchiveEntry* templatesconfigurationentry  = mArchiveDirectory->entry(Utils::configsPath() + templatesconfigurationrcStr);
    if ( templatesconfigurationentry &&  templatesconfigurationentry->isFile()) {
        const KArchiveFile* templatesconfiguration = static_cast<const KArchiveFile*>(templatesconfigurationentry);
        const QString templatesconfigurationrc = KStandardDirs::locateLocal( "config",  templatesconfigurationrcStr);
        if (QFile(templatesconfigurationrc).exists()) {
            //TODO 4.13 allow to merge config.
            if (overwriteConfigMessageBox(templatesconfigurationrcStr)) {
                importTemplatesConfig(templatesconfiguration, templatesconfigurationrc, templatesconfigurationrcStr, Utils::configsPath());
            }
        } else {
            importTemplatesConfig(templatesconfiguration, templatesconfigurationrc, templatesconfigurationrcStr, Utils::configsPath());
        }
    }

    const QString kmailStr(QLatin1String("kmail2rc"));
    const KArchiveEntry* kmail2rcentry  = mArchiveDirectory->entry(Utils::configsPath() + kmailStr);
    if (kmail2rcentry && kmail2rcentry->isFile()) {
        const KArchiveFile* kmailrc = static_cast<const KArchiveFile*>(kmail2rcentry);
        const QString kmail2rc = KStandardDirs::locateLocal( "config",  kmailStr);
        if (QFile(kmail2rc).exists()) {
            if (overwriteConfigMessageBox(kmailStr)) {
                importKmailConfig(kmailrc,kmail2rc,kmailStr,Utils::configsPath());
            }
        } else {
            importKmailConfig(kmailrc,kmail2rc,kmailStr,Utils::configsPath());
        }
    }

    const QString sievetemplatercStr(QLatin1String("sievetemplaterc"));
    const KArchiveEntry* sievetemplatentry  = mArchiveDirectory->entry(Utils::configsPath() + sievetemplatercStr);
    if ( sievetemplatentry &&  sievetemplatentry->isFile()) {
        const KArchiveFile* sievetemplateconfiguration = static_cast<const KArchiveFile*>(sievetemplatentry);
        const QString sievetemplaterc = KStandardDirs::locateLocal( "config",  sievetemplatercStr);
        if (QFile(sievetemplaterc).exists()) {
            const int result = mergeConfigMessageBox(sievetemplatercStr);
            if ( result == KMessageBox::Yes) {
                copyToFile(sievetemplateconfiguration, sievetemplaterc, sievetemplatercStr, Utils::configsPath());
            } else if (result == KMessageBox::No) {
                mergeSieveTemplate(sievetemplateconfiguration, sievetemplatercStr,Utils::configsPath());
            }
        } else {
            copyToFile(sievetemplateconfiguration, sievetemplaterc, sievetemplatercStr, Utils::configsPath());
        }
    }

    const QString customTemplateStr(QLatin1String("customtemplatesrc"));
    const KArchiveEntry* customtemplatentry  = mArchiveDirectory->entry(Utils::configsPath() + customTemplateStr);
    if ( customtemplatentry &&  customtemplatentry->isFile()) {
        const KArchiveFile* customtemplateconfiguration = static_cast<const KArchiveFile*>(customtemplatentry);
        const QString customtemplaterc = KStandardDirs::locateLocal( "config",  customTemplateStr);
        if (QFile(customtemplaterc).exists()) {
            //TODO 4.13 allow to merge config.
            if (overwriteConfigMessageBox(customTemplateStr)) {
                copyToFile(customtemplateconfiguration, customtemplaterc, customTemplateStr, Utils::configsPath());
            }
        } else {
            copyToFile(customtemplateconfiguration, customtemplaterc, customTemplateStr, Utils::configsPath());
        }
    }

    const QString adblockStr(QLatin1String("messagevieweradblockrc"));
    const KArchiveEntry* adblockentry  = mArchiveDirectory->entry(Utils::configsPath() + adblockStr);
    if ( adblockentry &&  adblockentry->isFile()) {
        const KArchiveFile* adblockconfiguration = static_cast<const KArchiveFile*>(adblockentry);
        const QString adblockrc = KStandardDirs::locateLocal( "config",  adblockStr);
        if (QFile(adblockrc).exists()) {
            //TODO 4.13 allow to merge config.
            if (overwriteConfigMessageBox(adblockStr)) {
                copyToFile(adblockconfiguration, adblockrc, adblockStr, Utils::configsPath());
            }
        } else {
            copyToFile(adblockconfiguration, adblockrc, adblockStr, Utils::configsPath());
        }
    }

    restoreConfigFile(QLatin1String("kontactrc"));

    restoreConfigFile(QLatin1String("kontact_summaryrc"));
    restoreConfigFile(QLatin1String("storageservicerc"));
    //Restore notify file
    QStringList lstNotify;
    lstNotify << QLatin1String("akonadi_mailfilter_agent.notifyrc")
              << QLatin1String("akonadi_sendlater_agent.notifyrc")
              << QLatin1String("akonadi_archivemail_agent.notifyrc")
              << QLatin1String("kmail2.notifyrc")
              << QLatin1String("akonadi_newmailnotifier_agent.notifyrc")
              << QLatin1String("akonadi_maildispatcher_agent.notifyrc")
              //<< QLatin1String("akonadi_followupreminder_agent.notifyrc")
              << QLatin1String("messageviewer.notifyrc");

    //We can't merge it.
    Q_FOREACH (const QString &filename, lstNotify) {
        restoreConfigFile(filename);
    }


    const KArchiveEntry* autocorrectionEntry  = mArchiveDirectory->entry(Utils::dataPath() + QLatin1String( "autocorrect/" ) );
    if (autocorrectionEntry && autocorrectionEntry->isDirectory()) {
        const KArchiveDirectory *autoCorrectionDir = static_cast<const KArchiveDirectory*>(autocorrectionEntry);
        Q_FOREACH(const QString& entryName, autoCorrectionDir->entries()) {
            const KArchiveEntry *entry = autoCorrectionDir->entry(entryName);
            if (entry && entry->isFile()) {
                const KArchiveFile* autocorrectionFile = static_cast<const KArchiveFile*>(entry);
                const QString name = autocorrectionFile->name();
                QString autocorrectionPath = KGlobal::dirs()->saveLocation("data", QLatin1String("autocorrect/"), false);
                if (QFile(autocorrectionPath).exists()) {
                    if (overwriteConfigMessageBox(name)) {
                        copyToFile(autocorrectionFile, autocorrectionPath + QLatin1Char('/') + name, name, Utils::dataPath() + QLatin1String( "autocorrect/" ));
                    }
                } else {
                    autocorrectionPath = KGlobal::dirs()->saveLocation("data", QLatin1String("autocorrect/"), true);
                    copyToFile(autocorrectionFile, autocorrectionPath + QLatin1Char('/') + name, name, Utils::dataPath() + QLatin1String( "autocorrect/" ));
                }
            }
        }
    }
    const KArchiveEntry* kmail2Entry  = mArchiveDirectory->entry(Utils::dataPath() + QLatin1String( "kmail2/adblockrules_local" ) );
    if (kmail2Entry && kmail2Entry->isFile()) {
        const KArchiveFile *entry = static_cast<const KArchiveFile*>(kmail2Entry);
        const QString adblockPath = KGlobal::dirs()->saveLocation( "data", QLatin1String( "kmail2") + QLatin1String("/adblockrules_local"), false);
        if (QFile(adblockPath).exists()) {
            if (overwriteConfigMessageBox(QLatin1String("adblockrules_local"))) {
                copyToFile(entry, KGlobal::dirs()->saveLocation( "data", QLatin1String( "kmail2")) + QLatin1String("/adblockrules_local"), QLatin1String("adblockrules_local"), Utils::dataPath() + QLatin1String( "kmail2/"));
            }
        } else {
            copyToFile(entry, KGlobal::dirs()->saveLocation( "data", QLatin1String( "kmail2")) + QLatin1String("/adblockrules_local"), QLatin1String("adblockrules_local"), Utils::dataPath() + QLatin1String( "kmail2/"));
        }
    }

    const KArchiveEntry* themeEntry  = mArchiveDirectory->entry(Utils::dataPath() + QLatin1String( "messageviewer/themes/" ) );
    if (themeEntry && themeEntry->isDirectory()) {
        const KArchiveDirectory *themeDir = static_cast<const KArchiveDirectory*>(themeEntry);
        Q_FOREACH(const QString& entryName, themeDir->entries()) {
            const KArchiveEntry *entry = themeDir->entry(entryName);
            if (entry && entry->isDirectory()) {
                QString subFolderName = entryName;
                QDir themeDirectory( KStandardDirs::locateLocal( "data", QString::fromLatin1( "messageviewer/themes/%1" ).arg(entryName) ) );
                int i = 1;
                while (themeDirectory.exists()) {
                    subFolderName = entryName + QString::fromLatin1("_%1").arg(i);
                    themeDirectory = QDir( KStandardDirs::locateLocal( "data", QString::fromLatin1( "messageviewer/themes/%1" ).arg(subFolderName) ) );
                    ++i;
                }
                copyToDirectory(entry, KGlobal::dirs()->saveLocation( "data", QString::fromLatin1( "messageviewer/themes/%1" ).arg(subFolderName)));
            }
        }
    }

    Q_EMIT info(i18n("Config restored."));
    nextStep();
}

void ImportMailJob::restoreIdentity()
{
    const QString path(Utils::identitiesPath() +QLatin1String("emailidentities"));
    if (!mFileList.contains(path)) {
        Q_EMIT error(i18n("emailidentities file could not be found in the archive."));
    } else {
        Q_EMIT info(i18n("Restore identities..."));
        MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
        const KArchiveEntry* identity = mArchiveDirectory->entry(path);
        if (identity && identity->isFile()) {
            const KArchiveFile* fileIdentity = static_cast<const KArchiveFile*>(identity);
            fileIdentity->copyTo(mTempDirName);
            KSharedConfig::Ptr identityConfig = KSharedConfig::openConfig(mTempDirName + QLatin1Char('/') +QLatin1String("emailidentities"));
            KConfigGroup general = identityConfig->group(QLatin1String("General"));
            const int defaultIdentity = general.readEntry(QLatin1String("Default Identity"),-1);

            const QStringList identityList = identityConfig->groupList().filter( QRegExp( QLatin1String("Identity #\\d+") ) );
            Q_FOREACH(const QString&identityStr, identityList) {
                KConfigGroup group = identityConfig->group(identityStr);
                int oldUid = -1;
                const QString uidStr(QLatin1String("uoid"));
                if (group.hasKey(uidStr)) {
                    oldUid = group.readEntry(uidStr).toUInt();
                    group.deleteEntry(uidStr);
                }
                const QString fcc(QLatin1String("Fcc"));
                convertRealPathToCollection(group, fcc);

                const QString draft = QLatin1String("Drafts");
                convertRealPathToCollection(group, draft);

                const QString templates = QLatin1String("Templates");
                convertRealPathToCollection(group, templates);

                if (oldUid != -1) {
                    const QString vcard = QLatin1String("VCardFile");
                    if (group.hasKey(vcard)) {
                        const QString vcardFileName = group.readEntry(vcard);
                        if (!vcardFileName.isEmpty()) {

                            QFileInfo fileInfo(vcardFileName);
                            QFile file(vcardFileName);
                            const KArchiveEntry* vcardEntry = mArchiveDirectory->entry(Utils::identitiesPath() + QString::number(oldUid) + QDir::separator() + file.fileName());
                            if (vcardEntry && vcardEntry->isFile()) {
                                const KArchiveFile* vcardFile = static_cast<const KArchiveFile*>(vcardEntry);
                                QString vcardFilePath = KStandardDirs::locateLocal("data",QString::fromLatin1("kmail2/%1").arg(fileInfo.fileName() ));
                                int i = 1;
                                while(QFile(vcardFileName).exists()) {
                                    vcardFilePath = KStandardDirs::locateLocal("data", QString::fromLatin1("kmail2/%1_%2").arg(i).arg(fileInfo.fileName()) );
                                    ++i;
                                }
                                vcardFile->copyTo(QFileInfo(vcardFilePath).absolutePath());
                                group.writeEntry(vcard, vcardFilePath);
                            }
                        }
                    }
                }
                QString name =  group.readEntry(QLatin1String("Name"));

                KPIMIdentities::Identity* identity = &mIdentityManager->newFromScratch( uniqueIdentityName(name) );
                group.writeEntry(QLatin1String("Name"), name);
                group.sync();

                identity->readConfig(group);

                if (oldUid != -1) {
                    mHashIdentity.insert(oldUid,identity->uoid());
                    if (oldUid == defaultIdentity) {
                        mIdentityManager->setAsDefault(identity->uoid());
                    }
                }
                mIdentityManager->commit();
            }
            Q_EMIT info(i18n("Identities restored."));
        } else {
            Q_EMIT error(i18n("Failed to restore identity file."));
        }
    }
    nextStep();
}

QString ImportMailJob::uniqueIdentityName(const QString& name)
{
    QString newName(name);
    int i = 0;
    while(!mIdentityManager->isUnique( newName )) {
        newName = QString::fromLatin1("%1_%2").arg(name).arg(i);
        ++i;
    }
    return newName;
}

void ImportMailJob::restoreAkonadiDb()
{
    const QString akonadiDbPath(Utils::akonadiPath() + QLatin1String("akonadidatabase.sql"));
    if (!mFileList.contains(akonadiDbPath)) {
        Q_EMIT error(i18n("Akonadi database file could not be found in the archive."));
    } else {
        Q_EMIT info(i18n("Restore Akonadi Database..."));
        MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );

        const KArchiveEntry *akonadiDataBaseEntry = mArchiveDirectory->entry(akonadiDbPath);
        if (akonadiDataBaseEntry && akonadiDataBaseEntry->isFile()) {

            const KArchiveFile *akonadiDataBaseFile = static_cast<const KArchiveFile*>(akonadiDataBaseEntry);

            KTemporaryFile tmp;
            tmp.open();

            akonadiDataBaseFile->copyTo(tmp.fileName());

            /* Restore the database */
            AkonadiDataBase akonadiDataBase;

            const QString dbDriver(akonadiDataBase.driver());
            QStringList params;
            QString dbRestoreAppName;
            if ( dbDriver == QLatin1String("QPSQL") ) {
                dbRestoreAppName = QLatin1String("pg_restore");
                params << akonadiDataBase.options()
                       << QLatin1String("--dbname=") + akonadiDataBase.name()
                       << QLatin1String("--format=custom")
                       << QLatin1String("--clean")
                       << QLatin1String("--no-owner")
                       << QLatin1String("--no-privileges")
                       << tmp.fileName();
            } else if (dbDriver == QLatin1String("QMYSQL") ) {
                dbRestoreAppName = QLatin1String("mysql");
                params << akonadiDataBase.options()
                       << QLatin1String("--database=") + akonadiDataBase.name();
            } else {
                Q_EMIT error(i18n("Database driver \"%1\" not supported.",dbDriver));
                nextStep();
                return;
            }

            const QString dbRestoreApp = KStandardDirs::findExe( dbRestoreAppName );

            if (dbRestoreApp.isEmpty()) {
                Q_EMIT error(i18n("Could not find \"%1\" necessary to restore database.",dbRestoreAppName));
                nextStep();
                return;
            }
            KProcess *proc = new KProcess( this );
            proc->setProgram( KStandardDirs::findExe( dbRestoreApp ), params );
            proc->setStandardInputFile(tmp.fileName());
            const int result = proc->execute();
            delete proc;
            if ( result != 0 ) {
                Q_EMIT error(i18n("Failed to restore Akonadi Database."));
                nextStep();
                return;
            }
        }
        Q_EMIT info(i18n("Akonadi Database restored."));
    }
    nextStep();
}

void ImportMailJob::importArchiveConfig(const KArchiveFile* archiveconfiguration, const QString& archiveconfigurationrc, const QString&filename,const QString& prefix)
{
    copyToFile(archiveconfiguration,archiveconfigurationrc,filename,prefix);
    KSharedConfig::Ptr archiveConfig = KSharedConfig::openConfig(archiveconfigurationrc);

    copyArchiveMailAgentConfigGroup(archiveConfig, archiveConfig);
    archiveConfig->sync();
}

void ImportMailJob::importFolderArchiveConfig(const KArchiveFile* archiveconfiguration, const QString& archiveconfigurationrc, const QString&filename,const QString& prefix)
{
    copyToFile(archiveconfiguration,archiveconfigurationrc,filename,prefix);
    KSharedConfig::Ptr archiveConfig = KSharedConfig::openConfig(archiveconfigurationrc);

    const QStringList archiveList = archiveConfig->groupList().filter( QRegExp( QLatin1String("FolderArchiveAccount ") ) );

    Q_FOREACH(const QString& str, archiveList) {
        KConfigGroup oldGroup = archiveConfig->group(str);
        const Akonadi::Collection::Id id = convertPathToId(oldGroup.readEntry(QLatin1String("topLevelCollectionId")));
        if (id!=-1) {
            oldGroup.writeEntry(QLatin1String("topLevelCollectionId"), id);
        }
    }

    archiveConfig->sync();
}


void ImportMailJob::copyArchiveMailAgentConfigGroup(KSharedConfig::Ptr archiveConfigOrigin, KSharedConfig::Ptr archiveConfigDestination)
{
    //adapt id
    const QString archiveGroupPattern = QLatin1String( "ArchiveMailCollection " );
    const QStringList archiveList = archiveConfigOrigin->groupList().filter( archiveGroupPattern );
    Q_FOREACH(const QString& str, archiveList) {
        const QString path = str.right(str.length()-archiveGroupPattern.length());
        if (!path.isEmpty())
        {
            KConfigGroup oldGroup = archiveConfigOrigin->group(str);
            const Akonadi::Collection::Id id = convertPathToId(path);
            if (id!=-1) {
                KConfigGroup newGroup( archiveConfigDestination, archiveGroupPattern + QString::number(id));
                oldGroup.copyTo( &newGroup );
                newGroup.writeEntry(QLatin1String("saveCollectionId"),id);
                KUrl path = newGroup.readEntry("storePath",KUrl());
                if (!QDir(path.path()).exists()) {
                    newGroup.writeEntry(QLatin1String("storePath"),KUrl(QDir::homePath()));
                }
            }
            oldGroup.deleteGroup();
        }
    }
}

void ImportMailJob::importTemplatesConfig(const KArchiveFile* templatesconfiguration, const QString& templatesconfigurationrc, const QString&filename,const QString& prefix)
{
    copyToFile(templatesconfiguration,templatesconfigurationrc,filename,prefix);
    KSharedConfig::Ptr templateConfig = KSharedConfig::openConfig(templatesconfigurationrc);

    //adapt id
    const QString templateGroupPattern = QLatin1String( "Templates #" );
    const QStringList templateList = templateConfig->groupList().filter( templateGroupPattern );
    Q_FOREACH(const QString& str, templateList) {
        const QString path = str.right(str.length()-templateGroupPattern.length());
        if (!path.isEmpty())
        {
            KConfigGroup oldGroup = templateConfig->group(str);
            const Akonadi::Collection::Id id = convertPathToId(path);
            if (id!=-1) {
                KConfigGroup newGroup( templateConfig, templateGroupPattern + QString::number(id));
                oldGroup.copyTo( &newGroup );
            }
            oldGroup.deleteGroup();
        }
    }
    //adapt identity
    const QString templateGroupIdentityPattern = QLatin1String( "Templates #IDENTITY_" );
    const QStringList templateListIdentity = templateConfig->groupList().filter( templateGroupIdentityPattern );
    Q_FOREACH(const QString& str, templateListIdentity) {
        bool found = false;
        const int identity = str.right(str.length()-templateGroupIdentityPattern.length()).toInt(&found);
        if (found)
        {
            KConfigGroup oldGroup = templateConfig->group(str);
            if (mHashIdentity.contains(identity)) {
                KConfigGroup newGroup( templateConfig, templateGroupPattern + QString::number(mHashIdentity.value(identity)));
                oldGroup.copyTo( &newGroup );
            }
            oldGroup.deleteGroup();
        }
    }
    templateConfig->sync();
}

void ImportMailJob::importKmailConfig(const KArchiveFile* kmailsnippet, const QString& kmail2rc, const QString&filename,const QString& prefix)
{
    copyToFile(kmailsnippet,kmail2rc,filename,prefix);
    KSharedConfig::Ptr kmailConfig = KSharedConfig::openConfig(kmail2rc);

    //Be sure to delete Search group
    const QString search(QLatin1String("Search"));
    if (kmailConfig->hasGroup(search)) {
        KConfigGroup searchGroup = kmailConfig->group(search);
        searchGroup.deleteGroup();
    }

    //adapt folder id
    const QString folderGroupPattern = QLatin1String( "Folder-" );
    const QStringList folderList = kmailConfig->groupList().filter( folderGroupPattern );
    Q_FOREACH(const QString&str, folderList) {
        const QString path = str.right(str.length()-folderGroupPattern.length());
        if (!path.isEmpty()) {
            KConfigGroup oldGroup = kmailConfig->group(str);
            const Akonadi::Collection::Id id = convertPathToId(path);
            if (id!=-1) {
                KConfigGroup newGroup( kmailConfig, folderGroupPattern + QString::number(id));
                oldGroup.copyTo( &newGroup );
            }
            oldGroup.deleteGroup();
        }
    }
    const QString accountOrder(QLatin1String("AccountOrder"));
    if (kmailConfig->hasGroup(accountOrder)) {
        KConfigGroup group = kmailConfig->group(accountOrder);
        QStringList orderList = group.readEntry(QLatin1String("order"),QStringList());
        QStringList newOrderList;
        if (!orderList.isEmpty()) {
            Q_FOREACH(const QString&account, orderList) {
                if (mHashResources.contains(account)) {
                    newOrderList.append(mHashResources.value(account));
                } else {
                    newOrderList.append(account);
                }
            }
        }
    }

    const QString composerStr(QLatin1String("Composer"));
    if (kmailConfig->hasGroup(composerStr)) {
        KConfigGroup composerGroup = kmailConfig->group(composerStr);
        const QString previousStr(QLatin1String("previous-fcc"));
        convertRealPathToCollection(composerGroup, previousStr);

        const QString previousIdentityStr(QLatin1String("previous-identity"));
        if (composerGroup.hasKey(previousIdentityStr)) {
            const int identityValue = composerGroup.readEntry(previousIdentityStr, -1);
            if (identityValue!=-1) {
                if (mHashIdentity.contains(identityValue)) {
                    composerGroup.writeEntry(previousIdentityStr,mHashIdentity.value(identityValue));
                } else {
                    composerGroup.writeEntry(previousIdentityStr,identityValue);
                }
            }
        }
    }

    const QString collectionFolderViewStr(QLatin1String("CollectionFolderView"));
    if (kmailConfig->hasGroup(collectionFolderViewStr)) {
        KConfigGroup favoriteGroup = kmailConfig->group(collectionFolderViewStr);
        const QString currentKey(QLatin1String("Current"));
        convertRealPathToCollection(favoriteGroup, currentKey, true);

        const QString expensionKey(QLatin1String("Expansion"));
        convertRealPathToCollectionList(favoriteGroup, expensionKey);
    }

    const QString generalStr(QLatin1String("General"));
    if (kmailConfig->hasGroup(generalStr)) {
        KConfigGroup generalGroup = kmailConfig->group(generalStr);
        //Be sure to delete default domain
        const QString defaultDomainStr(QLatin1String("Default domain"));
        if (generalGroup.hasKey(defaultDomainStr)) {
            generalGroup.deleteEntry(defaultDomainStr);
        }

        const QString startupFolderStr(QLatin1String("startupFolder"));
        convertRealPathToCollection(generalGroup, startupFolderStr);
    }

    const QString resourceGroupPattern = QLatin1String( "Resource " );
    const QStringList resourceList = kmailConfig->groupList().filter( resourceGroupPattern );
    Q_FOREACH(const QString&str, resourceList) {
        const QString res = str.right(str.length()-resourceGroupPattern.length());
        if (!res.isEmpty()) {
            KConfigGroup oldGroup = kmailConfig->group(str);
            if (mHashResources.contains(res)) {
                KConfigGroup newGroup( kmailConfig, folderGroupPattern + mHashResources.value(res));
                oldGroup.copyTo( &newGroup );
            }
            oldGroup.deleteGroup();
        }
    }

    kmailConfig->sync();
}

void ImportMailJob::mergeLdapConfig(const KArchiveFile * archivefile, const QString&filename, const QString&prefix)
{
    QDir dir(mTempDirName);
    dir.mkdir(prefix);

    const QString copyToDirName(mTempDirName + QLatin1Char('/') + prefix);
    archivefile->copyTo(copyToDirName);

    KSharedConfig::Ptr existingConfig = KSharedConfig::openConfig(filename);
    KConfigGroup grpExisting = existingConfig->group(QLatin1String("LDAP"));
    int existingNumberHosts = grpExisting.readEntry(QLatin1String("NumHosts"),0);
    int existingNumberSelectedHosts = grpExisting.readEntry(QLatin1String("NumSelectedHosts"),0);

    KSharedConfig::Ptr importingLdapConfig = KSharedConfig::openConfig(copyToDirName + QLatin1Char('/') + filename);
    KConfigGroup grpImporting = importingLdapConfig->group(QLatin1String("LDAP"));
    int importingNumberHosts = grpImporting.readEntry(QLatin1String("NumHosts"),0);
    int importingNumberSelectedHosts = grpImporting.readEntry(QLatin1String("NumSelectedHosts"),0);

    grpExisting.writeEntry(QLatin1String("NumHosts"),(existingNumberHosts+importingNumberHosts));
    grpExisting.writeEntry(QLatin1String("NumSelectedHosts"),(existingNumberSelectedHosts+importingNumberSelectedHosts));

    for (int i = 0; i<importingNumberSelectedHosts; ++i ) {
        const QString auth = grpImporting.readEntry(QString::fromLatin1("SelectedAuth%1").arg(i),QString());
        grpExisting.writeEntry(QString::fromLatin1("SelectedAuth%1").arg(existingNumberSelectedHosts+i+1),auth);
        grpExisting.writeEntry(QString::fromLatin1("SelectedBase%1").arg(existingNumberSelectedHosts+i+1),grpImporting.readEntry(QString::fromLatin1("SelectedBase%1").arg(i),QString()));
        grpExisting.writeEntry(QString::fromLatin1("SelectedBind%1").arg(existingNumberSelectedHosts+i+1),grpImporting.readEntry(QString::fromLatin1("SelectedBind%1").arg(i),QString()));
        grpExisting.writeEntry(QString::fromLatin1("SelectedHost%1").arg(existingNumberSelectedHosts+i+1),grpImporting.readEntry(QString::fromLatin1("SelectedHost%1").arg(i),QString()));
        grpExisting.writeEntry(QString::fromLatin1("SelectedMech%1").arg(existingNumberSelectedHosts+i+1),grpImporting.readEntry(QString::fromLatin1("SelectedMech%1").arg(i),QString()));
        grpExisting.writeEntry(QString::fromLatin1("SelectedPageSize%1").arg(existingNumberSelectedHosts+i+1),grpImporting.readEntry(QString::fromLatin1("SelectedPageSize%1").arg(i),0));
        grpExisting.writeEntry(QString::fromLatin1("SelectedPort%1").arg(existingNumberSelectedHosts+i+1),grpImporting.readEntry(QString::fromLatin1("SelectedPort%1").arg(i),0));
        grpExisting.writeEntry(QString::fromLatin1("SelectedPwdBind%1").arg(existingNumberSelectedHosts+i+1),grpImporting.readEntry(QString::fromLatin1("SelectedPwdBind%1").arg(i),QString()));
        grpExisting.writeEntry(QString::fromLatin1("SelectedSecurity%1").arg(existingNumberSelectedHosts+i+1),grpImporting.readEntry(QString::fromLatin1("SelectedSecurity%1").arg(i),QString()));
        grpExisting.writeEntry(QString::fromLatin1("SelectedSizeLimit%1").arg(existingNumberSelectedHosts+i+1),grpImporting.readEntry(QString::fromLatin1("SelectedSizeLimit%1").arg(i),0));
        grpExisting.writeEntry(QString::fromLatin1("SelectedTimeLimit%1").arg(existingNumberSelectedHosts+i+1),grpImporting.readEntry(QString::fromLatin1("SelectedTimeLimit%1").arg(i),0));
        grpExisting.writeEntry(QString::fromLatin1("SelectedUser%1").arg(existingNumberSelectedHosts+i+1),grpImporting.readEntry(QString::fromLatin1("SelectedUser%1").arg(i),QString()));
        grpExisting.writeEntry(QString::fromLatin1("SelectedVersion%1").arg(existingNumberSelectedHosts+i+1),grpImporting.readEntry(QString::fromLatin1("SelectedVersion%1").arg(i),0));
    }

    for (int i = 0; i<importingNumberHosts; ++i ) {
        grpExisting.writeEntry(QString::fromLatin1("Auth%1").arg(existingNumberHosts+i+1),grpImporting.readEntry(QString::fromLatin1("Auth%1").arg(i),QString()));
        grpExisting.writeEntry(QString::fromLatin1("Base%1").arg(existingNumberHosts+i+1),grpImporting.readEntry(QString::fromLatin1("Base%1").arg(i),QString()));
        grpExisting.writeEntry(QString::fromLatin1("Bind%1").arg(existingNumberHosts+i+1),grpImporting.readEntry(QString::fromLatin1("Bind%1").arg(i),QString()));
        grpExisting.writeEntry(QString::fromLatin1("Host%1").arg(existingNumberHosts+i+1),grpImporting.readEntry(QString::fromLatin1("Host%1").arg(i),QString()));
        grpExisting.writeEntry(QString::fromLatin1("Mech%1").arg(existingNumberHosts+i+1),grpImporting.readEntry(QString::fromLatin1("Mech%1").arg(i),QString()));
        grpExisting.writeEntry(QString::fromLatin1("PageSize%1").arg(existingNumberHosts+i+1),grpImporting.readEntry(QString::fromLatin1("PageSize%1").arg(i),0));
        grpExisting.writeEntry(QString::fromLatin1("Port%1").arg(existingNumberHosts+i+1),grpImporting.readEntry(QString::fromLatin1("Port%1").arg(i),0));
        grpExisting.writeEntry(QString::fromLatin1("PwdBind%1").arg(existingNumberHosts+i+1),grpImporting.readEntry(QString::fromLatin1("PwdBind%1").arg(i),QString()));
        grpExisting.writeEntry(QString::fromLatin1("Security%1").arg(existingNumberHosts+i+1),grpImporting.readEntry(QString::fromLatin1("Security%1").arg(i),QString()));
        grpExisting.writeEntry(QString::fromLatin1("SizeLimit%1").arg(existingNumberHosts+i+1),grpImporting.readEntry(QString::fromLatin1("SizeLimit%1").arg(i),0));
        grpExisting.writeEntry(QString::fromLatin1("TimeLimit%1").arg(existingNumberHosts+i+1),grpImporting.readEntry(QString::fromLatin1("TimeLimit%1").arg(i),0));
        grpExisting.writeEntry(QString::fromLatin1("User%1").arg(existingNumberHosts+i+1),grpImporting.readEntry(QString::fromLatin1("User%1").arg(i),QString()));
        grpExisting.writeEntry(QString::fromLatin1("Version%1").arg(existingNumberHosts+i+1),grpImporting.readEntry(QString::fromLatin1("Version%1").arg(i),0));
    }

    grpExisting.sync();
}

void ImportMailJob::mergeKmailSnippetConfig(const KArchiveFile * archivefile, const QString&filename, const QString&prefix)
{
    //TODO
    QDir dir(mTempDirName);
    dir.mkdir(prefix);

    const QString copyToDirName(mTempDirName + QLatin1Char('/') + prefix);
    archivefile->copyTo(copyToDirName);

    KSharedConfig::Ptr existingConfig = KSharedConfig::openConfig(filename);

    KSharedConfig::Ptr importingKMailSnipperConfig = KSharedConfig::openConfig(copyToDirName + QLatin1Char('/') + filename);
}


void ImportMailJob::mergeArchiveMailAgentConfig(const KArchiveFile *archivefile, const QString &filename, const QString &prefix)
{
    QDir dir(mTempDirName);
    dir.mkdir(prefix);

    const QString copyToDirName(mTempDirName + QLatin1Char('/') + prefix);
    archivefile->copyTo(copyToDirName);

    KSharedConfig::Ptr existingConfig = KSharedConfig::openConfig(filename);

    KSharedConfig::Ptr importingArchiveMailAgentConfig = KSharedConfig::openConfig(copyToDirName + QLatin1Char('/') + filename);

    copyArchiveMailAgentConfigGroup(importingArchiveMailAgentConfig, existingConfig);
    existingConfig->sync();
}


void ImportMailJob::mergeSieveTemplate(const KArchiveFile *archivefile, const QString &filename, const QString &prefix)
{
    QDir dir(mTempDirName);
    dir.mkdir(prefix);

    const QString copyToDirName(mTempDirName + QLatin1Char('/') + prefix);
    archivefile->copyTo(copyToDirName);

    KSharedConfig::Ptr existingConfig = KSharedConfig::openConfig(filename);

    KSharedConfig::Ptr importingSieveTemplateConfig = KSharedConfig::openConfig(copyToDirName + QLatin1Char('/') + filename);

    KConfigGroup grpExisting = existingConfig->group(QLatin1String("template"));
    int numberOfExistingTemplate = grpExisting.readEntry(QLatin1String("templateCount"), 0);

    KConfigGroup grpImportExisting = importingSieveTemplateConfig->group(QLatin1String("template"));
    const int numberOfImportingTemplate = grpImportExisting.readEntry(QLatin1String("templateCount"), 0);

    for (int i = 0; i <numberOfImportingTemplate; ++i) {
        KConfigGroup templateDefine = importingSieveTemplateConfig->group(QString::fromLatin1("templateDefine_%1").arg(i));

        KConfigGroup newTemplateDefineGrp = existingConfig->group(QString::fromLatin1("templateDefine_%1").arg(numberOfExistingTemplate));
        newTemplateDefineGrp.writeEntry(QLatin1String("Name"), templateDefine.readEntry(QLatin1String("Name")));
        newTemplateDefineGrp.writeEntry(QLatin1String("Text"), templateDefine.readEntry(QLatin1String("Text")));
        ++numberOfExistingTemplate;
        newTemplateDefineGrp.sync();
    }
    grpExisting.writeEntry(QLatin1String("templateCount"), numberOfExistingTemplate);
    grpExisting.sync();
}
