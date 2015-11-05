/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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

#include "MailCommon/FilterManager"
#include "MailCommon/FilterImporterExporter"
#include "MailCommon/MailUtil"
#include "PimCommon/CreateResource"

#include <MailTransport/mailtransport/transportmanager.h>

#include <KIdentityManagement/kidentitymanagement/identitymanager.h>
#include <KIdentityManagement/kidentitymanagement/identity.h>

#include <KLocalizedString>
#include <KProcess>
#include <QTemporaryFile>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KMessageBox>
#include <KArchiveFile>
#include <KZip>
#include "pimsettingexportcore_debug.h"

#include <AkonadiCore/agenttype.h>
#include <AkonadiCore/agentmanager.h>
#include <AkonadiCore/agentinstancecreatejob.h>

#include <QMetaMethod>
#include <QDir>
#include <QStandardPaths>
#include <QTimer>

using namespace Akonadi;
namespace
{
inline const QString storeMails()
{
    return QStringLiteral("backupmail/");
}
}

ImportMailJob::ImportMailJob(QObject *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep)
    : AbstractImportExportJob(parent, archiveStorage, typeSelected, numberOfStep)
{
    initializeImportJob();
}

ImportMailJob::~ImportMailJob()
{
}

void ImportMailJob::start()
{
    Q_EMIT title(i18n("Start import KMail settings..."));
    createProgressDialog(i18n("Import KMail settings"));
    mArchiveDirectory = archive()->directory();
    searchAllFiles(mArchiveDirectory, QString());
    if (!mFileList.isEmpty() || !mListResourceFile.isEmpty()) {
        initializeListStep();
        QTimer::singleShot(0, this, &ImportMailJob::slotNextStep);
    } else {
        Q_EMIT jobFinished();
    }
}

void ImportMailJob::slotNextStep()
{
    ++mIndex;
    if (mIndex < mListStep.count()) {
        const Utils::StoredType type = mListStep.at(mIndex);
        if (type == Utils::MailTransport) {
            restoreTransports();
        } else if (type == Utils::Mails) {
            restoreMails();
        } else if (type == Utils::Resources) {
            restoreResources();
        } else if (type == Utils::Identity) {
            restoreIdentity();
        } else if (type == Utils::Config) {
            restoreConfig();
        } else if (type == Utils::AkonadiDb) {
            restoreAkonadiDb();
        } else {
            qCDebug(PIMSETTINGEXPORTERCORE_LOG) << Q_FUNC_INFO << " not supported type " << type;
            slotNextStep();
        }
    } else {
        Q_EMIT jobFinished();
    }
}

void ImportMailJob::searchAllFiles(const KArchiveDirectory *dir, const QString &prefix)
{
    Q_FOREACH (const QString &entryName, dir->entries()) {
        const KArchiveEntry *entry = dir->entry(entryName);
        if (entry && entry->isDirectory()) {
            const QString newPrefix = (prefix.isEmpty() ? prefix : prefix + QLatin1Char('/')) + entryName;
            if (entryName == QLatin1String("mails")) {
                storeMailArchiveResource(static_cast<const KArchiveDirectory *>(entry), entryName);
            } else {
                searchAllFiles(static_cast<const KArchiveDirectory *>(entry), newPrefix);
            }
        } else if (entry) {
            const QString fileName = prefix.isEmpty() ? entry->name() : prefix + QLatin1Char('/') + entry->name();
            mFileList << fileName;
        }
    }
}

void ImportMailJob::storeMailArchiveResource(const KArchiveDirectory *dir, const QString &prefix)
{
    Q_FOREACH (const QString &entryName, dir->entries()) {
        const KArchiveEntry *entry = dir->entry(entryName);
        if (entry && entry->isDirectory()) {
            const KArchiveDirectory *resourceDir = static_cast<const KArchiveDirectory *>(entry);
            const QStringList lst = resourceDir->entries();
            if (lst.count() >= 2) {
                const QString archPath(prefix + QLatin1Char('/') + entryName + QLatin1Char('/'));
                resourceFiles files;
                Q_FOREACH (const QString &name, lst) {
                    if (name.endsWith(QLatin1String("rc")) &&
                            (name.contains(QStringLiteral("akonadi_mbox_resource_")) ||
                             name.contains(QStringLiteral("akonadi_mixedmaildir_resource_")) ||
                             name.contains(QStringLiteral("akonadi_maildir_resource_")))) {
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
                qCDebug(PIMSETTINGEXPORTERCORE_LOG) << " Problem in archive. number of file " << lst.count();
            }
        }
    }
}

void ImportMailJob::restoreTransports()
{
    setProgressDialogLabel(i18n("Restore transports..."));
    increaseProgressDialog();
    const QString path = Utils::transportsPath() + QLatin1String("mailtransports");
    if (!mFileList.contains(path)) {
        Q_EMIT error(i18n("mailtransports file could not be found in the archive."));
    } else {
        Q_EMIT info(i18n("Restore transports..."));

        const KArchiveEntry *transport = mArchiveDirectory->entry(path);
        if (transport && transport->isFile()) {
            const KArchiveFile *fileTransport = static_cast<const KArchiveFile *>(transport);

            fileTransport->copyTo(mTempDirName);
            KSharedConfig::Ptr transportConfig = KSharedConfig::openConfig(mTempDirName + QLatin1Char('/') + QLatin1String("mailtransports"));

            int defaultTransport = -1;
            if (transportConfig->hasGroup(QStringLiteral("General"))) {
                KConfigGroup group = transportConfig->group(QStringLiteral("General"));
                defaultTransport = group.readEntry(QStringLiteral("default-transport"), -1);
            }

            const QStringList transportList = transportConfig->groupList().filter(QRegExp(QLatin1String("Transport \\d+")));
            Q_FOREACH (const QString &transport, transportList) {
                KConfigGroup group = transportConfig->group(transport);
                const int transportId = group.readEntry(QStringLiteral("id"), -1);
                MailTransport::Transport *mt = MailTransport::TransportManager::self()->createTransport();
                mt->setName(group.readEntry(QStringLiteral("name")));
                const QString hostStr(QStringLiteral("host"));
                if (group.hasKey(hostStr)) {
                    mt->setHost(group.readEntry(hostStr));
                }
                const QString portStr(QStringLiteral("port"));
                if (group.hasKey(portStr)) {
                    mt->setPort(group.readEntry(portStr, -1));
                }
                const QString userNameStr(QStringLiteral("userName"));
                if (group.hasKey(userNameStr)) {
                    mt->setUserName(group.readEntry(userNameStr));
                }
                const QString precommandStr(QStringLiteral("precommand"));
                if (group.hasKey(precommandStr)) {
                    mt->setPrecommand(group.readEntry(precommandStr));
                }
                const QString requiresAuthenticationStr(QStringLiteral("requiresAuthentication"));
                if (group.hasKey(requiresAuthenticationStr)) {
                    mt->setRequiresAuthentication(group.readEntry(requiresAuthenticationStr, false));
                }
                const QString specifyHostnameStr(QStringLiteral("specifyHostname"));
                if (group.hasKey(specifyHostnameStr)) {
                    mt->setSpecifyHostname(group.readEntry(specifyHostnameStr, false));
                }
                const QString localHostnameStr(QStringLiteral("localHostname"));
                if (group.hasKey(localHostnameStr)) {
                    mt->setLocalHostname(group.readEntry(localHostnameStr));
                }
                const QString specifySenderOverwriteAddressStr(QStringLiteral("specifySenderOverwriteAddress"));
                if (group.hasKey(specifySenderOverwriteAddressStr)) {
                    mt->setSpecifySenderOverwriteAddress(group.readEntry(specifySenderOverwriteAddressStr, false));
                }
                const QString storePasswordStr(QStringLiteral("storePassword"));
                if (group.hasKey(storePasswordStr)) {
                    mt->setStorePassword(group.readEntry(storePasswordStr, false));
                }
                const QString senderOverwriteAddressStr(QStringLiteral("senderOverwriteAddress"));
                if (group.hasKey(senderOverwriteAddressStr)) {
                    mt->setSenderOverwriteAddress(group.readEntry(senderOverwriteAddressStr));
                }
                const QString encryptionStr(QStringLiteral("encryption"));
                if (group.hasKey(encryptionStr)) {
                    mt->setEncryption(group.readEntry(encryptionStr, 1)); //TODO verify
                }
                const QString authenticationTypeStr(QStringLiteral("authenticationType"));
                if (group.hasKey(authenticationTypeStr)) {
                    mt->setAuthenticationType(group.readEntry(authenticationTypeStr, 1)); //TODO verify
                }

                mt->forceUniqueName();
                mt->save();
                MailTransport::TransportManager::self()->addTransport(mt);
                if (transportId == defaultTransport) {
                    MailTransport::TransportManager::self()->setDefaultTransport(mt->id());
                }
                mHashTransport.insert(transportId, mt->id());
            }
            Q_EMIT info(i18n("Transports restored."));
        } else {
            Q_EMIT error(i18n("Failed to restore transports file."));
        }
    }
    QTimer::singleShot(0, this, &ImportMailJob::slotNextStep);
}

void ImportMailJob::restoreResources()
{
    increaseProgressDialog();
    setProgressDialogLabel(i18n("Restore resources..."));
    Q_EMIT info(i18n("Restore resources..."));

    QDir dir(mTempDirName);
    dir.mkdir(Utils::resourcesPath());
    Q_FOREACH (const QString &filename, mFileList) {
        if (filename.startsWith(Utils::resourcesPath())) {
            const KArchiveEntry *fileEntry = mArchiveDirectory->entry(filename);
            if (fileEntry && fileEntry->isFile()) {
                const KArchiveFile *file = static_cast<const KArchiveFile *>(fileEntry);
                const QString destDirectory = mTempDirName + QLatin1Char('/') + Utils::resourcesPath();

                file->copyTo(destDirectory);

                const QString filename(file->name());
                const QString resourceFileName = destDirectory + QLatin1Char('/') + filename;
                KSharedConfig::Ptr resourceConfig = KSharedConfig::openConfig(resourceFileName);
                QMap<QString, QVariant> settings;
                if (filename.contains(QStringLiteral("pop3"))) {
                    KConfigGroup general = resourceConfig->group(QStringLiteral("General"));
                    if (general.hasKey(QStringLiteral("login"))) {
                        settings.insert(QStringLiteral("Login"), general.readEntry("login"));
                    }
                    if (general.hasKey(QStringLiteral("host"))) {
                        settings.insert(QStringLiteral("Host"), general.readEntry("host"));
                    }
                    if (general.hasKey(QStringLiteral("port"))) {
                        settings.insert(QStringLiteral("Port"), general.readEntry("port", 110));
                    }
                    if (general.hasKey(QStringLiteral("authenticationMethod"))) {
                        settings.insert(QStringLiteral("AuthenticationMethod"), general.readEntry("authenticationMethod", 7));
                    }
                    if (general.hasKey(QStringLiteral("useSSL"))) {
                        settings.insert(QStringLiteral("UseSSL"), general.readEntry("useSSL", false));
                    }
                    if (general.hasKey(QStringLiteral("useTLS"))) {
                        settings.insert(QStringLiteral("UseTLS"), general.readEntry("useTLS", false));
                    }
                    if (general.hasKey(QStringLiteral("pipelining"))) {
                        settings.insert(QStringLiteral("Pipelining"), general.readEntry("pipelining", false));
                    }
                    if (general.hasKey(QStringLiteral("leaveOnServer"))) {
                        settings.insert(QStringLiteral("LeaveOnServer"), general.readEntry("leaveOnServer", false));
                    }
                    if (general.hasKey(QStringLiteral("leaveOnServerDays"))) {
                        settings.insert(QStringLiteral("LeaveOnServerDays"), general.readEntry("leaveOnServerDays", -1));
                    }
                    if (general.hasKey(QStringLiteral("leaveOnServerCount"))) {
                        settings.insert(QStringLiteral("LeaveOnServerCount"), general.readEntry("leaveOnServerCount", -1));
                    }
                    if (general.hasKey(QStringLiteral("leaveOnServerSize"))) {
                        settings.insert(QStringLiteral("LeaveOnServerSize"), general.readEntry("leaveOnServerSize", -1));
                    }
                    if (general.hasKey(QStringLiteral("filterOnServer"))) {
                        settings.insert(QStringLiteral("FilterOnServer"), general.readEntry("filterOnServer", false));
                    }
                    if (general.hasKey(QStringLiteral("filterCheckSize"))) {
                        settings.insert(QStringLiteral("FilterCheckSize"), general.readEntry("filterCheckSize"));
                    }
                    if (general.hasKey(QStringLiteral("targetCollection"))) {
                        const Akonadi::Collection::Id collection = convertPathToId(general.readEntry("targetCollection"));
                        if (collection != -1) {
                            settings.insert(QStringLiteral("TargetCollection"), collection);
                        }
                    }
                    if (general.hasKey(QStringLiteral("precommand"))) {
                        settings.insert(QStringLiteral("Precommand"), general.readEntry("precommand"));
                    }
                    if (general.hasKey(QStringLiteral("intervalCheckEnabled"))) {
                        settings.insert(QStringLiteral("IntervalCheckEnabled"), general.readEntry("intervalCheckEnabled", false));
                    }
                    if (general.hasKey(QStringLiteral("intervalCheckInterval"))) {
                        settings.insert(QStringLiteral("IntervalCheckInterval"), general.readEntry("intervalCheckInterval", 5));
                    }

                    KConfigGroup leaveOnserver = resourceConfig->group(QStringLiteral("LeaveOnServer"));

                    if (leaveOnserver.hasKey(QStringLiteral("seenUidList"))) {
                        settings.insert(QStringLiteral("SeenUidList"), leaveOnserver.readEntry("seenUidList", QStringList()));
                    }
#if 0
                    if (leaveOnserver.hasKey(QStringLiteral("seenUidTimeList"))) {
                        //FIXME
                        //settings.insert(QLatin1String("SeenUidTimeList"),QVariant::fromValue<QList<int> >(leaveOnserver.readEntry("seenUidTimeList",QList<int>())));
                    }
#endif
                    if (leaveOnserver.hasKey(QStringLiteral("downloadLater"))) {
                        settings.insert(QStringLiteral("DownloadLater"), leaveOnserver.readEntry("downloadLater", QStringList()));
                    }
                    const QString newResource = mCreateResource->createResource(QStringLiteral("akonadi_pop3_resource"), filename, settings);
                    if (!newResource.isEmpty()) {
                        mHashResources.insert(filename, newResource);
                        infoAboutNewResource(newResource);
                    }
                } else if (filename.contains(QStringLiteral("imap")) || filename.contains(QStringLiteral("kolab_")) || filename.contains(QStringLiteral("gmail_"))) {
                    KConfigGroup network = resourceConfig->group(QStringLiteral("network"));
                    if (network.hasKey(QStringLiteral("Authentication"))) {
                        settings.insert(QStringLiteral("Authentication"), network.readEntry("Authentication", 1));
                    }
                    if (network.hasKey(QStringLiteral("ImapPort"))) {
                        settings.insert(QStringLiteral("ImapPort"), network.readEntry("ImapPort", 993));
                    }
                    if (network.hasKey(QStringLiteral("ImapServer"))) {
                        settings.insert(QStringLiteral("ImapServer"), network.readEntry("ImapServer"));
                    }
                    if (network.hasKey(QStringLiteral("Safety"))) {
                        settings.insert(QStringLiteral("Safety"), network.readEntry("Safety", "SSL"));
                    }
                    if (network.hasKey(QStringLiteral("SubscriptionEnabled"))) {
                        settings.insert(QStringLiteral("SubscriptionEnabled"), network.readEntry("SubscriptionEnabled", false));
                    }
                    if (network.hasKey(QStringLiteral("UserName"))) {
                        settings.insert(QStringLiteral("UserName"), network.readEntry("UserName"));
                    }

                    if (network.hasKey(QStringLiteral("SessionTimeout"))) {
                        settings.insert(QStringLiteral("SessionTimeout"), network.readEntry("SessionTimeout", 30));
                    }

                    KConfigGroup cache = resourceConfig->group(QStringLiteral("cache"));

                    if (cache.hasKey(QStringLiteral("AccountIdentity"))) {
                        const int identity = cache.readEntry("AccountIdentity", -1);
                        if (identity != -1) {
                            if (mHashIdentity.contains(identity)) {
                                settings.insert(QStringLiteral("AccountIdentity"), mHashIdentity.value(identity));
                            } else {
                                settings.insert(QStringLiteral("AccountIdentity"), identity);
                            }
                        }
                    }
                    if (cache.hasKey(QStringLiteral("IntervalCheckEnabled"))) {
                        settings.insert(QStringLiteral("IntervalCheckEnabled"), cache.readEntry("IntervalCheckEnabled", true));
                    }
                    if (cache.hasKey(QStringLiteral("RetrieveMetadataOnFolderListing"))) {
                        settings.insert(QStringLiteral("RetrieveMetadataOnFolderListing"), cache.readEntry("RetrieveMetadataOnFolderListing", true));
                    }
                    if (cache.hasKey(QStringLiteral("AutomaticExpungeEnabled"))) {
                        settings.insert(QStringLiteral("AutomaticExpungeEnabled"), cache.readEntry("AutomaticExpungeEnabled", true));
                    }
                    if (cache.hasKey(QLatin1String(""))) {
                        settings.insert(QLatin1String(""), cache.readEntry(""));
                    }
                    if (cache.hasKey(QStringLiteral("DisconnectedModeEnabled"))) {
                        settings.insert(QStringLiteral("DisconnectedModeEnabled"), cache.readEntry("DisconnectedModeEnabled", false));
                    }
                    if (cache.hasKey(QStringLiteral("IntervalCheckTime"))) {
                        settings.insert(QStringLiteral("IntervalCheckTime"), cache.readEntry("IntervalCheckTime", -1));
                    }
                    if (cache.hasKey(QStringLiteral("UseDefaultIdentity"))) {
                        settings.insert(QStringLiteral("UseDefaultIdentity"), cache.readEntry("UseDefaultIdentity", true));
                    }
                    if (cache.hasKey(QStringLiteral("TrashCollection"))) {
                        const Akonadi::Collection::Id collection = convertPathToId(cache.readEntry("TrashCollection"));
                        if (collection != -1) {
                            settings.insert(QStringLiteral("TrashCollection"), collection);
                        } else {
                            qCDebug(PIMSETTINGEXPORTERCORE_LOG) << " Use default trash folder";
                        }
                    }

                    KConfigGroup siever = resourceConfig->group(QStringLiteral("siever"));
                    if (siever.hasKey(QStringLiteral("SieveSupport"))) {
                        settings.insert(QStringLiteral("SieveSupport"), siever.readEntry("SieveSupport", false));
                    }
                    if (siever.hasKey(QStringLiteral("SieveReuseConfig"))) {
                        settings.insert(QStringLiteral("SieveReuseConfig"), siever.readEntry("SieveReuseConfig", true));
                    }
                    if (siever.hasKey(QStringLiteral("SievePort"))) {
                        settings.insert(QStringLiteral("SievePort"), siever.readEntry("SievePort", 4190));
                    }
                    if (siever.hasKey(QStringLiteral("SieveAlternateUrl"))) {
                        settings.insert(QStringLiteral("SieveAlternateUrl"), siever.readEntry("SieveAlternateUrl"));
                    }
                    if (siever.hasKey(QStringLiteral("AlternateAuthentication"))) {
                        settings.insert(QStringLiteral("AlternateAuthentication"), siever.readEntry("AlternateAuthentication"));
                    }
                    if (siever.hasKey(QStringLiteral("SieveVacationFilename"))) {
                        settings.insert(QStringLiteral("SieveVacationFilename"), siever.readEntry("SieveVacationFilename"));
                    }
                    if (siever.hasKey(QStringLiteral("SieveCustomUsername"))) {
                        settings.insert(QStringLiteral("SieveCustomUsername"), siever.readEntry("SieveCustomUsername"));
                    }
                    if (siever.hasKey(QStringLiteral("SieveCustomAuthentification"))) {
                        settings.insert(QStringLiteral("SieveCustomAuthentification"), siever.readEntry("SieveCustomAuthentification"));
                    }

                    QString newResource;
                    if (filename.contains(QStringLiteral("kolab_"))) {
                        newResource = mCreateResource->createResource(QStringLiteral("akonadi_kolab_resource"), filename, settings);
                    } else if (filename.contains(QStringLiteral("gmail_"))) {
                        newResource = mCreateResource->createResource(QStringLiteral("akonadi_gmail_resource"), filename, settings);
                    } else {
                        newResource = mCreateResource->createResource(QStringLiteral("akonadi_imap_resource"), filename, settings);
                    }
                    if (!newResource.isEmpty()) {
                        mHashResources.insert(filename, newResource);
                        infoAboutNewResource(newResource);
                    }
                } else {
                    qCDebug(PIMSETTINGEXPORTERCORE_LOG) << " problem with resource";
                }
            }
        }
    }
    //TODO synctree ?

    Q_EMIT info(i18n("Resources restored."));
    QTimer::singleShot(0, this, &ImportMailJob::slotNextStep);
}

void ImportMailJob::restoreMails()
{
    increaseProgressDialog();
    setProgressDialogLabel(i18n("Restore mails..."));
    QStringList listResourceToSync;
    Q_EMIT info(i18n("Restore mails..."));

    QDir dir(mTempDirName);
    dir.mkdir(Utils::mailsPath());
    const QString copyToDirName(mTempDirName + QLatin1Char('/') + Utils::mailsPath());
    const int numberOfResourceFile = mListResourceFile.size();
    for (int i = 0; i < numberOfResourceFile; ++i) {

        resourceFiles value = mListResourceFile.at(i);
        value.debug();
        const QString resourceFile = value.akonadiConfigFile;
        const KArchiveEntry *fileResouceEntry = mArchiveDirectory->entry(resourceFile);
        if (fileResouceEntry && fileResouceEntry->isFile()) {
            const KArchiveFile *file = static_cast<const KArchiveFile *>(fileResouceEntry);
            file->copyTo(copyToDirName);
            QString resourceName(file->name());
            QString filename(file->name());
            //qCDebug(PIMSETTINGEXPORTERCORE_LOG)<<" filename "<<filename<<" resourceName"<<resourceName;
            KSharedConfig::Ptr resourceConfig = KSharedConfig::openConfig(copyToDirName + QLatin1Char('/') + resourceName);

            const QString newUrl = Utils::adaptResourcePath(resourceConfig, storeMails());

            const QString agentConfigFile = value.akonadiAgentConfigFile;
            if (!agentConfigFile.isEmpty()) {
                const KArchiveEntry *akonadiAgentConfigEntry = mArchiveDirectory->entry(agentConfigFile);
                if (akonadiAgentConfigEntry->isFile()) {
                    const KArchiveFile *file = static_cast<const KArchiveFile *>(akonadiAgentConfigEntry);
                    file->copyTo(copyToDirName);
                    resourceName = file->name();
                    filename = Utils::akonadiAgentName(copyToDirName + QLatin1Char('/') + resourceName);
                }
            }

            QMap<QString, QVariant> settings;
            if (resourceName.contains(QStringLiteral("akonadi_mbox_resource_"))) {
                const QString dataFile = value.akonadiResources;
                const KArchiveEntry *dataResouceEntry = mArchiveDirectory->entry(dataFile);
                if (dataResouceEntry->isFile()) {
                    const KArchiveFile *file = static_cast<const KArchiveFile *>(dataResouceEntry);
                    file->copyTo(newUrl);
                }
                settings.insert(QStringLiteral("Path"), newUrl);

                KConfigGroup general = resourceConfig->group(QStringLiteral("General"));
                if (general.hasKey(QStringLiteral("DisplayName"))) {
                    settings.insert(QStringLiteral("DisplayName"), general.readEntry(QStringLiteral("DisplayName")));
                }
                if (general.hasKey(QStringLiteral("ReadOnly"))) {
                    settings.insert(QStringLiteral("ReadOnly"), general.readEntry(QStringLiteral("ReadOnly"), false));
                }
                if (general.hasKey(QStringLiteral("MonitorFile"))) {
                    settings.insert(QStringLiteral("MonitorFile"), general.readEntry(QStringLiteral("MonitorFile"), false));
                }
                if (resourceConfig->hasGroup(QStringLiteral("Locking"))) {
                    KConfigGroup locking = resourceConfig->group(QStringLiteral("Locking"));
                    if (locking.hasKey(QStringLiteral("Lockfile"))) {
                        settings.insert(QStringLiteral("Lockfile"), locking.readEntry(QStringLiteral("Lockfile")));
                    }
                    //TODO verify
                    if (locking.hasKey(QStringLiteral("LockfileMethod"))) {
                        settings.insert(QStringLiteral("LockfileMethod"), locking.readEntry(QStringLiteral("LockfileMethod"), 4));
                    }
                }
                if (resourceConfig->hasGroup(QStringLiteral("Compacting"))) {
                    KConfigGroup compacting = resourceConfig->group(QStringLiteral("Compacting"));
                    if (compacting.hasKey(QStringLiteral("CompactFrequency"))) {
                        settings.insert(QStringLiteral("CompactFrequency"), compacting.readEntry(QStringLiteral("CompactFrequency"), 1));
                    }
                    if (compacting.hasKey(QStringLiteral("MessageCount"))) {
                        settings.insert(QStringLiteral("MessageCount"), compacting.readEntry(QStringLiteral("MessageCount"), 50));
                    }
                }
                const QString newResource = mCreateResource->createResource(QStringLiteral("akonadi_mbox_resource"), filename, settings);
                if (!newResource.isEmpty()) {
                    mHashResources.insert(filename, newResource);
                    infoAboutNewResource(newResource);
                }

            } else if (resourceName.contains(QStringLiteral("akonadi_maildir_resource_")) ||
                       resourceName.contains(QStringLiteral("akonadi_mixedmaildir_resource_"))) {
                settings.insert(QStringLiteral("Path"), newUrl);
                KConfigGroup general = resourceConfig->group(QStringLiteral("General"));
                if (general.hasKey(QStringLiteral("TopLevelIsContainer"))) {
                    settings.insert(QStringLiteral("TopLevelIsContainer"), general.readEntry(QStringLiteral("TopLevelIsContainer"), false));
                }
                if (general.hasKey(QStringLiteral("ReadOnly"))) {
                    settings.insert(QStringLiteral("ReadOnly"), general.readEntry(QStringLiteral("ReadOnly"), false));
                }
                if (general.hasKey(QStringLiteral("MonitorFilesystem"))) {
                    settings.insert(QStringLiteral("MonitorFilesystem"), general.readEntry(QStringLiteral("MonitorFilesystem"), true));
                }

                const QString newResource = mCreateResource->createResource(resourceName.contains(QStringLiteral("akonadi_mixedmaildir_resource_")) ?
                                            QStringLiteral("akonadi_mixedmaildir_resource")
                                            : QStringLiteral("akonadi_maildir_resource")
                                            , filename, settings);
                if (!newResource.isEmpty()) {
                    mHashResources.insert(filename, newResource);
                    infoAboutNewResource(newResource);
                }

                const QString mailFile = value.akonadiResources;
                const KArchiveEntry *dataResouceEntry = mArchiveDirectory->entry(mailFile);
                if (dataResouceEntry && dataResouceEntry->isFile()) {
                    const KArchiveFile *file = static_cast<const KArchiveFile *>(dataResouceEntry);
                    //TODO Fix me not correct zip filename.
                    extractZipFile(file, copyToDirName, newUrl);
                }
                listResourceToSync << newResource;
            } else {
                qCDebug(PIMSETTINGEXPORTERCORE_LOG) << " resource name not supported " << resourceName;
            }
            //qCDebug(PIMSETTINGEXPORTERCORE_LOG)<<"url "<<url;
        }
    }
    Q_EMIT info(i18n("Mails restored."));
    startSynchronizeResources(listResourceToSync);
}

void ImportMailJob::restoreConfig()
{

    increaseProgressDialog();
    setProgressDialogLabel(i18n("Restore config..."));
    const QString filtersPath(Utils::configsPath() + QLatin1String("filters"));
    if (!mFileList.contains(filtersPath)) {
        Q_EMIT error(i18n("filters file could not be found in the archive."));
    } else {
        const KArchiveEntry *filter = mArchiveDirectory->entry(filtersPath);
        if (filter && filter->isFile()) {
            const KArchiveFile *fileFilter = static_cast<const KArchiveFile *>(filter);

            fileFilter->copyTo(mTempDirName);
            const QString filterFileName(mTempDirName + QLatin1Char('/') + QLatin1String("filters"));
            KSharedConfig::Ptr filtersConfig = KSharedConfig::openConfig(filterFileName);
            const QStringList filterList = filtersConfig->groupList().filter(QRegExp(QLatin1String("Filter #\\d+")));
            Q_FOREACH (const QString &filterStr, filterList) {
                KConfigGroup group = filtersConfig->group(filterStr);
                const QString accountStr(QStringLiteral("accounts-set"));
                if (group.hasKey(accountStr)) {
                    const QString accounts = group.readEntry(accountStr);
                    if (!accounts.isEmpty()) {
                        const QStringList lstAccounts = accounts.split(QLatin1Char(','));
                        QStringList newLstAccounts;
                        Q_FOREACH (const QString &acc, lstAccounts) {
                            if (mHashResources.contains(acc)) {
                                newLstAccounts.append(mHashResources.value(acc));
                            } else {
                                newLstAccounts.append(acc);
                            }
                        }
                        group.writeEntry(accountStr, newLstAccounts);
                    }
                }
                const int numActions = group.readEntry("actions", 0);
                QString actName;
                QString argsName;
                for (int i = 0; i < numActions; ++i) {
                    actName.sprintf("action-name-%d", i);
                    argsName.sprintf("action-args-%d", i);
                    const QString actValue = group.readEntry(actName);
                    if (actValue == QLatin1String("set identity")) {
                        const int argsValue = group.readEntry(argsName, -1);
                        if (argsValue != -1) {
                            if (mHashIdentity.contains(argsValue)) {
                                group.writeEntry(argsName, mHashIdentity.value(argsValue));
                            }
                        }
                    } else if (actValue == QLatin1String("set transport")) {
                        const int argsValue = group.readEntry(argsName, -1);
                        if (argsValue != -1) {
                            if (mHashTransport.contains(argsValue)) {
                                group.writeEntry(argsName, mHashTransport.value(argsValue));
                            }
                        }
                    }
                }
            }
            filtersConfig->sync();

            bool canceled = false;
            MailCommon::FilterImporterExporter exportFilters;
            QList<MailCommon::MailFilter *> lstFilter = exportFilters.importFilters(canceled, MailCommon::FilterImporterExporter::KMailFilter, filterFileName);
            if (canceled) {
                MailCommon::FilterManager::instance()->appendFilters(lstFilter);
            }
        }
    }
    const QString kmailsnippetrcStr(QStringLiteral("kmailsnippetrc"));
    const KArchiveEntry *kmailsnippetentry  = mArchiveDirectory->entry(Utils::configsPath() + kmailsnippetrcStr);
    if (kmailsnippetentry && kmailsnippetentry->isFile()) {
        const KArchiveFile *kmailsnippet = static_cast<const KArchiveFile *>(kmailsnippetentry);
        const QString kmailsnippetrc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + kmailsnippetrcStr;
        if (QFile(kmailsnippetrc).exists()) {
            //TODO 4.13 allow to merge config.
            if (overwriteConfigMessageBox(kmailsnippetrcStr)) {
                copyToFile(kmailsnippet, kmailsnippetrc, kmailsnippetrcStr, Utils::configsPath());
            }
        } else {
            copyToFile(kmailsnippet, kmailsnippetrc, kmailsnippetrcStr, Utils::configsPath());
        }
    }

    const QString labldaprcStr(QStringLiteral("kabldaprc"));
    const KArchiveEntry *kabldapentry  = mArchiveDirectory->entry(Utils::configsPath() + labldaprcStr);
    if (kabldapentry && kabldapentry->isFile()) {
        const KArchiveFile *kabldap = static_cast<const KArchiveFile *>(kabldapentry);
        const QString kabldaprc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + labldaprcStr;
        if (QFile(kabldaprc).exists()) {
            const int result = mergeConfigMessageBox(labldaprcStr);
            if (result == KMessageBox::Yes) {
                copyToFile(kabldap, kabldaprc, labldaprcStr, Utils::configsPath());
            } else if (result == KMessageBox::No) {
                mergeLdapConfig(kabldap, labldaprcStr, Utils::configsPath());
            }
        } else {
            copyToFile(kabldap, kabldaprc, labldaprcStr, Utils::configsPath());
        }
    }
    const QString archiveconfigurationrcStr(QStringLiteral("akonadi_archivemail_agentrc"));
    const KArchiveEntry *archiveconfigurationentry  = mArchiveDirectory->entry(Utils::configsPath() + archiveconfigurationrcStr);
    if (archiveconfigurationentry &&  archiveconfigurationentry->isFile()) {
        const KArchiveFile *archiveconfiguration = static_cast<const KArchiveFile *>(archiveconfigurationentry);
        const QString archiveconfigurationrc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + archiveconfigurationrcStr;
        if (QFile(archiveconfigurationrc).exists()) {
            const int result = mergeConfigMessageBox(archiveconfigurationrcStr);
            if (result == KMessageBox::Yes) {
                importArchiveConfig(archiveconfiguration, archiveconfigurationrc, archiveconfigurationrcStr, Utils::configsPath());
            } else if (result == KMessageBox::No) {
                mergeArchiveMailAgentConfig(archiveconfiguration, archiveconfigurationrcStr, Utils::configsPath());
            }
        } else {
            importArchiveConfig(archiveconfiguration, archiveconfigurationrc, archiveconfigurationrcStr, Utils::configsPath());
        }
    }

    const QString templatesconfigurationrcStr(QStringLiteral("templatesconfigurationrc"));
    const KArchiveEntry *templatesconfigurationentry  = mArchiveDirectory->entry(Utils::configsPath() + templatesconfigurationrcStr);
    if (templatesconfigurationentry &&  templatesconfigurationentry->isFile()) {
        const KArchiveFile *templatesconfiguration = static_cast<const KArchiveFile *>(templatesconfigurationentry);
        const QString templatesconfigurationrc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + templatesconfigurationrcStr;
        if (QFile(templatesconfigurationrc).exists()) {
            //TODO 4.13 allow to merge config.
            if (overwriteConfigMessageBox(templatesconfigurationrcStr)) {
                importTemplatesConfig(templatesconfiguration, templatesconfigurationrc, templatesconfigurationrcStr, Utils::configsPath());
            }
        } else {
            importTemplatesConfig(templatesconfiguration, templatesconfigurationrc, templatesconfigurationrcStr, Utils::configsPath());
        }
    }

    const QString kmailStr(QStringLiteral("kmail2rc"));
    const KArchiveEntry *kmail2rcentry  = mArchiveDirectory->entry(Utils::configsPath() + kmailStr);
    if (kmail2rcentry && kmail2rcentry->isFile()) {
        const KArchiveFile *kmailrc = static_cast<const KArchiveFile *>(kmail2rcentry);
        const QString kmail2rc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + kmailStr;
        if (QFile(kmail2rc).exists()) {
            if (overwriteConfigMessageBox(kmailStr)) {
                importKmailConfig(kmailrc, kmail2rc, kmailStr, Utils::configsPath());
            }
        } else {
            importKmailConfig(kmailrc, kmail2rc, kmailStr, Utils::configsPath());
        }
    }

    const QString sievetemplatercStr(QStringLiteral("sievetemplaterc"));
    const KArchiveEntry *sievetemplatentry  = mArchiveDirectory->entry(Utils::configsPath() + sievetemplatercStr);
    if (sievetemplatentry &&  sievetemplatentry->isFile()) {
        const KArchiveFile *sievetemplateconfiguration = static_cast<const KArchiveFile *>(sievetemplatentry);
        const QString sievetemplaterc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + sievetemplatercStr;
        if (QFile(sievetemplaterc).exists()) {
            const int result = mergeConfigMessageBox(sievetemplatercStr);
            if (result == KMessageBox::Yes) {
                copyToFile(sievetemplateconfiguration, sievetemplaterc, sievetemplatercStr, Utils::configsPath());
            } else if (result == KMessageBox::No) {
                mergeSieveTemplate(sievetemplateconfiguration, sievetemplatercStr, Utils::configsPath());
            }
        } else {
            copyToFile(sievetemplateconfiguration, sievetemplaterc, sievetemplatercStr, Utils::configsPath());
        }
    }

    const QString customTemplateStr(QStringLiteral("customtemplatesrc"));
    const KArchiveEntry *customtemplatentry  = mArchiveDirectory->entry(Utils::configsPath() + customTemplateStr);
    if (customtemplatentry &&  customtemplatentry->isFile()) {
        const KArchiveFile *customtemplateconfiguration = static_cast<const KArchiveFile *>(customtemplatentry);
        const QString customtemplaterc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + customTemplateStr;
        if (QFile(customtemplaterc).exists()) {
            //TODO 4.13 allow to merge config.
            if (overwriteConfigMessageBox(customTemplateStr)) {
                copyToFile(customtemplateconfiguration, customtemplaterc, customTemplateStr, Utils::configsPath());
            }
        } else {
            copyToFile(customtemplateconfiguration, customtemplaterc, customTemplateStr, Utils::configsPath());
        }
    }

    const QString adblockStr(QStringLiteral("messagevieweradblockrc"));
    const KArchiveEntry *adblockentry  = mArchiveDirectory->entry(Utils::configsPath() + adblockStr);
    if (adblockentry &&  adblockentry->isFile()) {
        const KArchiveFile *adblockconfiguration = static_cast<const KArchiveFile *>(adblockentry);
        const QString adblockrc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + adblockStr;
        if (QFile(adblockrc).exists()) {
            //TODO 4.13 allow to merge config.
            if (overwriteConfigMessageBox(adblockStr)) {
                copyToFile(adblockconfiguration, adblockrc, adblockStr, Utils::configsPath());
            }
        } else {
            copyToFile(adblockconfiguration, adblockrc, adblockStr, Utils::configsPath());
        }
    }

    restoreUiRcFile(QStringLiteral("sieveeditorui.rc"), QStringLiteral("sieveeditor"));
    restoreUiRcFile(QStringLiteral("storageservicemanagerui.rc"), QStringLiteral("storageservicemanager"));

    restoreUiRcFile(QStringLiteral("kmreadermainwin.rc"), QStringLiteral("kmail2"));
    restoreUiRcFile(QStringLiteral("kmcomposerui.rc"), QStringLiteral("kmail2"));
    restoreUiRcFile(QStringLiteral("kmmainwin.rc"), QStringLiteral("kmail2"));
    restoreUiRcFile(QStringLiteral("kmail_part.rc"), QStringLiteral("kmail2"));
    restoreUiRcFile(QStringLiteral("kontactui.rc"), QStringLiteral("kontact"));
    restoreUiRcFile(QStringLiteral("kleopatra.rc"), QStringLiteral("kleopatra"));

    restoreConfigFile(QStringLiteral("kontactrc"));

    restoreConfigFile(QStringLiteral("kontact_summaryrc"));
    restoreConfigFile(QStringLiteral("storageservicerc"));
    restoreConfigFile(QStringLiteral("kpimbalooblacklist"));
    restoreConfigFile(QStringLiteral("kleopatrarc"));
    restoreConfigFile(QStringLiteral("sieveeditorrc"));
    //Restore notify file
    QStringList lstNotify;
    lstNotify << QStringLiteral("akonadi_mailfilter_agent.notifyrc")
              << QStringLiteral("akonadi_sendlater_agent.notifyrc")
              << QStringLiteral("akonadi_archivemail_agent.notifyrc")
              << QStringLiteral("kmail2.notifyrc")
              << QStringLiteral("akonadi_newmailnotifier_agent.notifyrc")
              << QStringLiteral("akonadi_maildispatcher_agent.notifyrc")
              << QStringLiteral("akonadi_followupreminder_agent.notifyrc")
              << QStringLiteral("messageviewer.notifyrc");

    //We can't merge it.
    Q_FOREACH (const QString &filename, lstNotify) {
        restoreConfigFile(filename);
    }

    const KArchiveEntry *autocorrectionEntry  = mArchiveDirectory->entry(Utils::dataPath() + QLatin1String("autocorrect/"));
    if (autocorrectionEntry && autocorrectionEntry->isDirectory()) {
        const KArchiveDirectory *autoCorrectionDir = static_cast<const KArchiveDirectory *>(autocorrectionEntry);
        Q_FOREACH (const QString &entryName, autoCorrectionDir->entries()) {
            const KArchiveEntry *entry = autoCorrectionDir->entry(entryName);
            if (entry && entry->isFile()) {
                const KArchiveFile *autocorrectionFile = static_cast<const KArchiveFile *>(entry);
                const QString name = autocorrectionFile->name();
                QString autocorrectionPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + QLatin1String("autocorrect/");
                if (QFile(autocorrectionPath).exists()) {
                    if (overwriteConfigMessageBox(name)) {
                        copyToFile(autocorrectionFile, autocorrectionPath + QLatin1Char('/') + name, name, Utils::dataPath() + QLatin1String("autocorrect/"));
                    }
                } else {
                    autocorrectionPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + QLatin1String("autocorrect/");
                    copyToFile(autocorrectionFile, autocorrectionPath + QLatin1Char('/') + name, name, Utils::dataPath() + QLatin1String("autocorrect/"));
                }
            }
        }
    }
    const KArchiveEntry *kmail2Entry  = mArchiveDirectory->entry(Utils::dataPath() + QLatin1String("kmail2/adblockrules_local"));
    if (kmail2Entry && kmail2Entry->isFile()) {
        const KArchiveFile *entry = static_cast<const KArchiveFile *>(kmail2Entry);
        const QString adblockPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + QLatin1String("kmail2/adblockrules_local");
        if (QFile(adblockPath).exists()) {
            if (overwriteConfigMessageBox(QStringLiteral("adblockrules_local"))) {
                copyToFile(entry, QString(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/") + QLatin1String("kmail2/adblockrules_local")), QStringLiteral("adblockrules_local"), Utils::dataPath() + QLatin1String("kmail2/"));
            }
        } else {
            copyToFile(entry, QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + QLatin1String("kmail2/adblockrules_local"), QStringLiteral("adblockrules_local"), Utils::dataPath() + QLatin1String("kmail2/"));
        }
    }

    const KArchiveEntry *themeEntry  = mArchiveDirectory->entry(Utils::dataPath() + QLatin1String("messageviewer/themes/"));
    if (themeEntry && themeEntry->isDirectory()) {
        const KArchiveDirectory *themeDir = static_cast<const KArchiveDirectory *>(themeEntry);
        Q_FOREACH (const QString &entryName, themeDir->entries()) {
            const KArchiveEntry *entry = themeDir->entry(entryName);
            if (entry && entry->isDirectory()) {
                QString subFolderName = entryName;
                QDir themeDirectory(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + QStringLiteral("messageviewer/themes/%1").arg(entryName));
                int i = 1;
                while (themeDirectory.exists()) {
                    subFolderName = entryName + QStringLiteral("_%1").arg(i);
                    themeDirectory = QDir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + QStringLiteral("messageviewer/themes/%1").arg(subFolderName));
                    ++i;
                }
                copyToDirectory(entry, QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + QStringLiteral("messageviewer/themes/%1").arg(subFolderName));
            }
        }
    }

    Q_EMIT info(i18n("Config restored."));
    QTimer::singleShot(0, this, &ImportMailJob::slotNextStep);
}

void ImportMailJob::restoreIdentity()
{
    increaseProgressDialog();
    setProgressDialogLabel(i18n("Restore identities..."));
    const QString path(Utils::identitiesPath() + QLatin1String("emailidentities"));
    if (!mFileList.contains(path)) {
        Q_EMIT error(i18n("emailidentities file could not be found in the archive."));
    } else {
        Q_EMIT info(i18n("Restore identities..."));

        const KArchiveEntry *identity = mArchiveDirectory->entry(path);
        if (identity && identity->isFile()) {
            const KArchiveFile *fileIdentity = static_cast<const KArchiveFile *>(identity);
            fileIdentity->copyTo(mTempDirName);
            KSharedConfig::Ptr identityConfig = KSharedConfig::openConfig(mTempDirName + QLatin1Char('/') + QLatin1String("emailidentities"));
            KConfigGroup general = identityConfig->group(QStringLiteral("General"));
            const int defaultIdentity = general.readEntry(QStringLiteral("Default Identity"), -1);

            const QStringList identityList = identityConfig->groupList().filter(QRegExp(QLatin1String("Identity #\\d+")));
            Q_FOREACH (const QString &identityStr, identityList) {
                KConfigGroup group = identityConfig->group(identityStr);
                int oldUid = -1;
                const QString uidStr(QStringLiteral("uoid"));
                if (group.hasKey(uidStr)) {
                    oldUid = group.readEntry(uidStr).toUInt();
                    group.deleteEntry(uidStr);
                }
                const QString fcc(QStringLiteral("Fcc"));
                convertRealPathToCollection(group, fcc);

                const QString draft = QStringLiteral("Drafts");
                convertRealPathToCollection(group, draft);

                const QString templates = QStringLiteral("Templates");
                convertRealPathToCollection(group, templates);

                if (oldUid != -1) {
                    const QString vcard = QStringLiteral("VCardFile");
                    if (group.hasKey(vcard)) {
                        const QString vcardFileName = group.readEntry(vcard);
                        if (!vcardFileName.isEmpty()) {

                            QFileInfo fileInfo(vcardFileName);
                            QFile file(vcardFileName);
                            const KArchiveEntry *vcardEntry = mArchiveDirectory->entry(Utils::identitiesPath() + QString::number(oldUid) + QDir::separator() + file.fileName());
                            if (vcardEntry && vcardEntry->isFile()) {
                                const KArchiveFile *vcardFile = static_cast<const KArchiveFile *>(vcardEntry);
                                QString vcardFilePath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + QStringLiteral("kmail2/%1").arg(fileInfo.fileName());
                                int i = 1;
                                while (QFile(vcardFileName).exists()) {
                                    vcardFilePath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + QStringLiteral("kmail2/%1_%2").arg(i).arg(fileInfo.fileName());
                                    ++i;
                                }
                                vcardFile->copyTo(QFileInfo(vcardFilePath).absolutePath());
                                group.writeEntry(vcard, vcardFilePath);
                            }
                        }
                    }
                }
                QString name =  group.readEntry(QStringLiteral("Name"));

                KIdentityManagement::Identity *identity = &mIdentityManager->newFromScratch(uniqueIdentityName(name));
                group.writeEntry(QStringLiteral("Name"), name);
                group.sync();

                identity->readConfig(group);

                if (oldUid != -1) {
                    mHashIdentity.insert(oldUid, identity->uoid());
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
    QTimer::singleShot(0, this, &ImportMailJob::slotNextStep);
}

QString ImportMailJob::uniqueIdentityName(const QString &name)
{
    QString newName(name);
    int i = 0;
    while (!mIdentityManager->isUnique(newName)) {
        newName = QStringLiteral("%1_%2").arg(name).arg(i);
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

        const KArchiveEntry *akonadiDataBaseEntry = mArchiveDirectory->entry(akonadiDbPath);
        if (akonadiDataBaseEntry && akonadiDataBaseEntry->isFile()) {

            const KArchiveFile *akonadiDataBaseFile = static_cast<const KArchiveFile *>(akonadiDataBaseEntry);

            QTemporaryFile tmp;
            tmp.open();

            akonadiDataBaseFile->copyTo(tmp.fileName());

            /* Restore the database */
            AkonadiDataBase akonadiDataBase;

            const QString dbDriver(akonadiDataBase.driver());
            QStringList params;
            QString dbRestoreAppName;
            if (dbDriver == QLatin1String("QPSQL")) {
                dbRestoreAppName = QStringLiteral("pg_restore");
                params << akonadiDataBase.options()
                       << QStringLiteral("--dbname=") + akonadiDataBase.name()
                       << QStringLiteral("--format=custom")
                       << QStringLiteral("--clean")
                       << QStringLiteral("--no-owner")
                       << QStringLiteral("--no-privileges")
                       << tmp.fileName();
            } else if (dbDriver == QLatin1String("QMYSQL")) {
                dbRestoreAppName = QStringLiteral("mysql");
                params << akonadiDataBase.options()
                       << QStringLiteral("--database=") + akonadiDataBase.name();
            } else {
                Q_EMIT error(i18n("Database driver \"%1\" not supported.", dbDriver));
                slotNextStep();
                return;
            }

            const QString dbRestoreApp = QStandardPaths::findExecutable(dbRestoreAppName);

            if (dbRestoreApp.isEmpty()) {
                Q_EMIT error(i18n("Could not find \"%1\" necessary to restore database.", dbRestoreAppName));
                slotNextStep();
                return;
            }
            KProcess *proc = new KProcess(this);
            proc->setProgram(QStandardPaths::findExecutable(dbRestoreApp), params);
            proc->setStandardInputFile(tmp.fileName());
            const int result = proc->execute();
            delete proc;
            if (result != 0) {
                Q_EMIT error(i18n("Failed to restore Akonadi Database."));
                slotNextStep();
                return;
            }
        }
        Q_EMIT info(i18n("Akonadi Database restored."));
    }
    slotNextStep();
}

void ImportMailJob::importArchiveConfig(const KArchiveFile *archiveconfiguration, const QString &archiveconfigurationrc, const QString &filename, const QString &prefix)
{
    copyToFile(archiveconfiguration, archiveconfigurationrc, filename, prefix);
    KSharedConfig::Ptr archiveConfig = KSharedConfig::openConfig(archiveconfigurationrc);

    copyArchiveMailAgentConfigGroup(archiveConfig, archiveConfig);
    archiveConfig->sync();
}

void ImportMailJob::importFolderArchiveConfig(const KArchiveFile *archiveconfiguration, const QString &archiveconfigurationrc, const QString &filename, const QString &prefix)
{
    copyToFile(archiveconfiguration, archiveconfigurationrc, filename, prefix);
    KSharedConfig::Ptr archiveConfig = KSharedConfig::openConfig(archiveconfigurationrc);

    const QStringList archiveList = archiveConfig->groupList().filter(QRegExp(QLatin1String("FolderArchiveAccount ")));

    Q_FOREACH (const QString &str, archiveList) {
        KConfigGroup oldGroup = archiveConfig->group(str);
        const Akonadi::Collection::Id id = convertPathToId(oldGroup.readEntry(QStringLiteral("topLevelCollectionId")));
        if (id != -1) {
            oldGroup.writeEntry(QStringLiteral("topLevelCollectionId"), id);
        }
    }

    archiveConfig->sync();
}

void ImportMailJob::copyArchiveMailAgentConfigGroup(KSharedConfig::Ptr archiveConfigOrigin, KSharedConfig::Ptr archiveConfigDestination)
{
    //adapt id
    const QString archiveGroupPattern = QStringLiteral("ArchiveMailCollection ");
    const QStringList archiveList = archiveConfigOrigin->groupList().filter(archiveGroupPattern);
    Q_FOREACH (const QString &str, archiveList) {
        const QString path = str.right(str.length() - archiveGroupPattern.length());
        if (!path.isEmpty()) {
            KConfigGroup oldGroup = archiveConfigOrigin->group(str);
            const Akonadi::Collection::Id id = convertPathToId(path);
            if (id != -1) {
                KConfigGroup newGroup(archiveConfigDestination, archiveGroupPattern + QString::number(id));
                oldGroup.copyTo(&newGroup);
                newGroup.writeEntry(QStringLiteral("saveCollectionId"), id);
                QUrl path = newGroup.readEntry("storePath", QUrl());
                if (!QDir(path.path()).exists()) {
                    newGroup.writeEntry(QStringLiteral("storePath"), QUrl::fromLocalFile(QDir::homePath()));
                }
            }
            oldGroup.deleteGroup();
        }
    }
}

void ImportMailJob::importTemplatesConfig(const KArchiveFile *templatesconfiguration, const QString &templatesconfigurationrc, const QString &filename, const QString &prefix)
{
    copyToFile(templatesconfiguration, templatesconfigurationrc, filename, prefix);
    KSharedConfig::Ptr templateConfig = KSharedConfig::openConfig(templatesconfigurationrc);

    //adapt id
    const QString templateGroupPattern = QStringLiteral("Templates #");
    const QStringList templateList = templateConfig->groupList().filter(templateGroupPattern);
    Q_FOREACH (const QString &str, templateList) {
        const QString path = str.right(str.length() - templateGroupPattern.length());
        if (!path.isEmpty()) {
            KConfigGroup oldGroup = templateConfig->group(str);
            const Akonadi::Collection::Id id = convertPathToId(path);
            if (id != -1) {
                KConfigGroup newGroup(templateConfig, templateGroupPattern + QString::number(id));
                oldGroup.copyTo(&newGroup);
            }
            oldGroup.deleteGroup();
        }
    }
    //adapt identity
    const QString templateGroupIdentityPattern = QStringLiteral("Templates #IDENTITY_");
    const QStringList templateListIdentity = templateConfig->groupList().filter(templateGroupIdentityPattern);
    Q_FOREACH (const QString &str, templateListIdentity) {
        bool found = false;
        const int identity = str.rightRef(str.length() - templateGroupIdentityPattern.length()).toInt(&found);
        if (found) {
            KConfigGroup oldGroup = templateConfig->group(str);
            if (mHashIdentity.contains(identity)) {
                KConfigGroup newGroup(templateConfig, templateGroupPattern + QString::number(mHashIdentity.value(identity)));
                oldGroup.copyTo(&newGroup);
            }
            oldGroup.deleteGroup();
        }
    }
    templateConfig->sync();
}

void ImportMailJob::importKmailConfig(const KArchiveFile *kmailsnippet, const QString &kmail2rc, const QString &filename, const QString &prefix)
{
    copyToFile(kmailsnippet, kmail2rc, filename, prefix);
    KSharedConfig::Ptr kmailConfig = KSharedConfig::openConfig(kmail2rc);

    //Be sure to delete Search group
    const QString search(QStringLiteral("Search"));
    if (kmailConfig->hasGroup(search)) {
        KConfigGroup searchGroup = kmailConfig->group(search);
        searchGroup.deleteGroup();
    }

    //adapt folder id
    const QString folderGroupPattern = QStringLiteral("Folder-");
    const QStringList folderList = kmailConfig->groupList().filter(folderGroupPattern);
    Q_FOREACH (const QString &str, folderList) {
        const QString path = str.right(str.length() - folderGroupPattern.length());
        if (!path.isEmpty()) {
            KConfigGroup oldGroup = kmailConfig->group(str);
            const Akonadi::Collection::Id id = convertPathToId(path);
            if (id != -1) {
                KConfigGroup newGroup(kmailConfig, folderGroupPattern + QString::number(id));
                oldGroup.copyTo(&newGroup);
            }
            oldGroup.deleteGroup();
        }
    }
    const QString accountOrder(QStringLiteral("AccountOrder"));
    if (kmailConfig->hasGroup(accountOrder)) {
        KConfigGroup group = kmailConfig->group(accountOrder);
        QStringList orderList = group.readEntry(QStringLiteral("order"), QStringList());
        QStringList newOrderList;
        if (!orderList.isEmpty()) {
            Q_FOREACH (const QString &account, orderList) {
                if (mHashResources.contains(account)) {
                    newOrderList.append(mHashResources.value(account));
                } else {
                    newOrderList.append(account);
                }
            }
        }
    }

    const QString composerStr(QStringLiteral("Composer"));
    if (kmailConfig->hasGroup(composerStr)) {
        KConfigGroup composerGroup = kmailConfig->group(composerStr);
        const QString previousStr(QStringLiteral("previous-fcc"));
        convertRealPathToCollection(composerGroup, previousStr);

        const QString previousIdentityStr(QStringLiteral("previous-identity"));
        if (composerGroup.hasKey(previousIdentityStr)) {
            const int identityValue = composerGroup.readEntry(previousIdentityStr, -1);
            if (identityValue != -1) {
                if (mHashIdentity.contains(identityValue)) {
                    composerGroup.writeEntry(previousIdentityStr, mHashIdentity.value(identityValue));
                } else {
                    composerGroup.writeEntry(previousIdentityStr, identityValue);
                }
            }
        }
    }

    const QString collectionFolderViewStr(QStringLiteral("CollectionFolderView"));
    if (kmailConfig->hasGroup(collectionFolderViewStr)) {
        KConfigGroup favoriteGroup = kmailConfig->group(collectionFolderViewStr);
        const QString currentKey(QStringLiteral("Current"));
        convertRealPathToCollection(favoriteGroup, currentKey, true);

        const QString expensionKey(QStringLiteral("Expansion"));
        convertRealPathToCollectionList(favoriteGroup, expensionKey);
    }

    const QString generalStr(QStringLiteral("General"));
    if (kmailConfig->hasGroup(generalStr)) {
        KConfigGroup generalGroup = kmailConfig->group(generalStr);
        //Be sure to delete default domain
        const QString defaultDomainStr(QStringLiteral("Default domain"));
        if (generalGroup.hasKey(defaultDomainStr)) {
            generalGroup.deleteEntry(defaultDomainStr);
        }

        const QString startupFolderStr(QStringLiteral("startupFolder"));
        convertRealPathToCollection(generalGroup, startupFolderStr);
    }

    const QString resourceGroupPattern = QStringLiteral("Resource ");
    const QStringList resourceList = kmailConfig->groupList().filter(resourceGroupPattern);
    Q_FOREACH (const QString &str, resourceList) {
        const QString res = str.right(str.length() - resourceGroupPattern.length());
        if (!res.isEmpty()) {
            KConfigGroup oldGroup = kmailConfig->group(str);
            if (mHashResources.contains(res)) {
                KConfigGroup newGroup(kmailConfig, folderGroupPattern + mHashResources.value(res));
                oldGroup.copyTo(&newGroup);
            }
            oldGroup.deleteGroup();
        }
    }

    kmailConfig->sync();
}

void ImportMailJob::mergeLdapConfig(const KArchiveFile *archivefile, const QString &filename, const QString &prefix)
{
    QDir dir(mTempDirName);
    dir.mkdir(prefix);

    const QString copyToDirName(mTempDirName + QLatin1Char('/') + prefix);
    archivefile->copyTo(copyToDirName);

    KSharedConfig::Ptr existingConfig = KSharedConfig::openConfig(filename);
    KConfigGroup grpExisting = existingConfig->group(QStringLiteral("LDAP"));
    int existingNumberHosts = grpExisting.readEntry(QStringLiteral("NumHosts"), 0);
    int existingNumberSelectedHosts = grpExisting.readEntry(QStringLiteral("NumSelectedHosts"), 0);

    KSharedConfig::Ptr importingLdapConfig = KSharedConfig::openConfig(copyToDirName + QLatin1Char('/') + filename);
    KConfigGroup grpImporting = importingLdapConfig->group(QStringLiteral("LDAP"));
    int importingNumberHosts = grpImporting.readEntry(QStringLiteral("NumHosts"), 0);
    int importingNumberSelectedHosts = grpImporting.readEntry(QStringLiteral("NumSelectedHosts"), 0);

    grpExisting.writeEntry(QStringLiteral("NumHosts"), (existingNumberHosts + importingNumberHosts));
    grpExisting.writeEntry(QStringLiteral("NumSelectedHosts"), (existingNumberSelectedHosts + importingNumberSelectedHosts));

    for (int i = 0; i < importingNumberSelectedHosts; ++i) {
        const QString auth = grpImporting.readEntry(QStringLiteral("SelectedAuth%1").arg(i), QString());
        grpExisting.writeEntry(QStringLiteral("SelectedAuth%1").arg(existingNumberSelectedHosts + i + 1), auth);
        grpExisting.writeEntry(QStringLiteral("SelectedBase%1").arg(existingNumberSelectedHosts + i + 1), grpImporting.readEntry(QStringLiteral("SelectedBase%1").arg(i), QString()));
        grpExisting.writeEntry(QStringLiteral("SelectedBind%1").arg(existingNumberSelectedHosts + i + 1), grpImporting.readEntry(QStringLiteral("SelectedBind%1").arg(i), QString()));
        grpExisting.writeEntry(QStringLiteral("SelectedHost%1").arg(existingNumberSelectedHosts + i + 1), grpImporting.readEntry(QStringLiteral("SelectedHost%1").arg(i), QString()));
        grpExisting.writeEntry(QStringLiteral("SelectedMech%1").arg(existingNumberSelectedHosts + i + 1), grpImporting.readEntry(QStringLiteral("SelectedMech%1").arg(i), QString()));
        grpExisting.writeEntry(QStringLiteral("SelectedPageSize%1").arg(existingNumberSelectedHosts + i + 1), grpImporting.readEntry(QStringLiteral("SelectedPageSize%1").arg(i), 0));
        grpExisting.writeEntry(QStringLiteral("SelectedPort%1").arg(existingNumberSelectedHosts + i + 1), grpImporting.readEntry(QStringLiteral("SelectedPort%1").arg(i), 0));
        grpExisting.writeEntry(QStringLiteral("SelectedPwdBind%1").arg(existingNumberSelectedHosts + i + 1), grpImporting.readEntry(QStringLiteral("SelectedPwdBind%1").arg(i), QString()));
        grpExisting.writeEntry(QStringLiteral("SelectedSecurity%1").arg(existingNumberSelectedHosts + i + 1), grpImporting.readEntry(QStringLiteral("SelectedSecurity%1").arg(i), QString()));
        grpExisting.writeEntry(QStringLiteral("SelectedSizeLimit%1").arg(existingNumberSelectedHosts + i + 1), grpImporting.readEntry(QStringLiteral("SelectedSizeLimit%1").arg(i), 0));
        grpExisting.writeEntry(QStringLiteral("SelectedTimeLimit%1").arg(existingNumberSelectedHosts + i + 1), grpImporting.readEntry(QStringLiteral("SelectedTimeLimit%1").arg(i), 0));
        grpExisting.writeEntry(QStringLiteral("SelectedUser%1").arg(existingNumberSelectedHosts + i + 1), grpImporting.readEntry(QStringLiteral("SelectedUser%1").arg(i), QString()));
        grpExisting.writeEntry(QStringLiteral("SelectedVersion%1").arg(existingNumberSelectedHosts + i + 1), grpImporting.readEntry(QStringLiteral("SelectedVersion%1").arg(i), 0));
        grpExisting.writeEntry(QStringLiteral("SelectedUserFilter%1").arg(existingNumberSelectedHosts + i + 1), grpImporting.readEntry(QStringLiteral("SelectedUserFilter%1").arg(i), 0));
    }

    for (int i = 0; i < importingNumberHosts; ++i) {
        grpExisting.writeEntry(QStringLiteral("Auth%1").arg(existingNumberHosts + i + 1), grpImporting.readEntry(QStringLiteral("Auth%1").arg(i), QString()));
        grpExisting.writeEntry(QStringLiteral("Base%1").arg(existingNumberHosts + i + 1), grpImporting.readEntry(QStringLiteral("Base%1").arg(i), QString()));
        grpExisting.writeEntry(QStringLiteral("Bind%1").arg(existingNumberHosts + i + 1), grpImporting.readEntry(QStringLiteral("Bind%1").arg(i), QString()));
        grpExisting.writeEntry(QStringLiteral("Host%1").arg(existingNumberHosts + i + 1), grpImporting.readEntry(QStringLiteral("Host%1").arg(i), QString()));
        grpExisting.writeEntry(QStringLiteral("Mech%1").arg(existingNumberHosts + i + 1), grpImporting.readEntry(QStringLiteral("Mech%1").arg(i), QString()));
        grpExisting.writeEntry(QStringLiteral("PageSize%1").arg(existingNumberHosts + i + 1), grpImporting.readEntry(QStringLiteral("PageSize%1").arg(i), 0));
        grpExisting.writeEntry(QStringLiteral("Port%1").arg(existingNumberHosts + i + 1), grpImporting.readEntry(QStringLiteral("Port%1").arg(i), 0));
        grpExisting.writeEntry(QStringLiteral("PwdBind%1").arg(existingNumberHosts + i + 1), grpImporting.readEntry(QStringLiteral("PwdBind%1").arg(i), QString()));
        grpExisting.writeEntry(QStringLiteral("Security%1").arg(existingNumberHosts + i + 1), grpImporting.readEntry(QStringLiteral("Security%1").arg(i), QString()));
        grpExisting.writeEntry(QStringLiteral("SizeLimit%1").arg(existingNumberHosts + i + 1), grpImporting.readEntry(QStringLiteral("SizeLimit%1").arg(i), 0));
        grpExisting.writeEntry(QStringLiteral("TimeLimit%1").arg(existingNumberHosts + i + 1), grpImporting.readEntry(QStringLiteral("TimeLimit%1").arg(i), 0));
        grpExisting.writeEntry(QStringLiteral("User%1").arg(existingNumberHosts + i + 1), grpImporting.readEntry(QStringLiteral("User%1").arg(i), QString()));
        grpExisting.writeEntry(QStringLiteral("Version%1").arg(existingNumberHosts + i + 1), grpImporting.readEntry(QStringLiteral("Version%1").arg(i), 0));
        grpExisting.writeEntry(QStringLiteral("UserFilter%1").arg(existingNumberSelectedHosts + i + 1), grpImporting.readEntry(QStringLiteral("UserFilter%1").arg(i), 0));
    }

    grpExisting.sync();
}

void ImportMailJob::mergeKmailSnippetConfig(const KArchiveFile *archivefile, const QString &filename, const QString &prefix)
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

    KConfigGroup grpExisting = existingConfig->group(QStringLiteral("template"));
    int numberOfExistingTemplate = grpExisting.readEntry(QStringLiteral("templateCount"), 0);

    KConfigGroup grpImportExisting = importingSieveTemplateConfig->group(QStringLiteral("template"));
    const int numberOfImportingTemplate = grpImportExisting.readEntry(QStringLiteral("templateCount"), 0);

    for (int i = 0; i < numberOfImportingTemplate; ++i) {
        KConfigGroup templateDefine = importingSieveTemplateConfig->group(QStringLiteral("templateDefine_%1").arg(i));

        KConfigGroup newTemplateDefineGrp = existingConfig->group(QStringLiteral("templateDefine_%1").arg(numberOfExistingTemplate));
        newTemplateDefineGrp.writeEntry(QStringLiteral("Name"), templateDefine.readEntry(QStringLiteral("Name")));
        newTemplateDefineGrp.writeEntry(QStringLiteral("Text"), templateDefine.readEntry(QStringLiteral("Text")));
        ++numberOfExistingTemplate;
        newTemplateDefineGrp.sync();
    }
    grpExisting.writeEntry(QStringLiteral("templateCount"), numberOfExistingTemplate);
    grpExisting.sync();
}
