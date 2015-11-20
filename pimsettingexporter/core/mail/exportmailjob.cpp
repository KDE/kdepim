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

#include "exportmailjob.h"
#include "akonadidatabase.h"

#include "MailCommon/MailUtil"
#include "MailCommon/FilterManager"
#include "MailCommon/FilterImporterExporter"
#include "importexportprogressindicatorbase.h"

#include <AkonadiCore/AgentManager>
#include <AkonadiCore/Collection>
#include <AkonadiCore/CollectionFetchJob>
#include <AkonadiCore/CollectionFetchScope>

#include <MailTransport/TransportManager>

#include <KZip>
#include <KLocalizedString>
#include <QTemporaryFile>

#include <KProcess>
#include "pimsettingexportcore_debug.h"

#include <QFile>
#include <QDir>
#include <QTimer>
#include <QStandardPaths>
#include <exportresourcearchivejob.h>
#include <QRegularExpression>

ExportMailJob::ExportMailJob(QObject *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep)
    : AbstractImportExportJob(parent, archiveStorage, typeSelected, numberOfStep),
      mArchiveTime(QDateTime::currentDateTime()),
      mIndexIdentifier(0)
{
}

ExportMailJob::~ExportMailJob()
{
}

#if 0
bool ExportMailJob::checkProgram()
{
    if (QStandardPaths::findExecutable(QStringLiteral("mysqldump")).isEmpty()) {
        Q_EMIT error(i18n("mysqldump not found. Export data aborted"));
        return false;
    }
    return true;
}
#endif

bool ExportMailJob::checkBackupType(Utils::StoredType type) const
{
    return (mTypeSelected & type);
}

void ExportMailJob::start()
{
    Q_EMIT title(i18n("Start export KMail settings..."));
    createProgressDialog(i18n("Export KMail settings"));
    if (checkBackupType(Utils::Identity)) {
        QTimer::singleShot(0, this, SLOT(slotCheckBackupIdentity()));
    } else if (checkBackupType(Utils::MailTransport)) {
        QTimer::singleShot(0, this, SLOT(slotCheckBackupMailTransport()));
    } else if (checkBackupType(Utils::Config)) {
        QTimer::singleShot(0, this, SLOT(slotCheckBackupConfig()));
    } else if (checkBackupType(Utils::Mails)) {
        QTimer::singleShot(0, this, SLOT(slotCheckBackupMails()));
    } else if (checkBackupType(Utils::Resources)) {
        QTimer::singleShot(0, this, SLOT(slotCheckBackupResources()));
    } else {
        Q_EMIT jobFinished();
    }
}

void ExportMailJob::slotCheckBackupIdentity()
{
    if (checkBackupType(Utils::Identity)) {
        backupIdentity();
        increaseProgressDialog();
        if (wasCanceled()) {
            Q_EMIT jobFinished();
            return;
        }
    }
    QTimer::singleShot(0, this, SLOT(slotCheckBackupMailTransport()));
}

void ExportMailJob::slotCheckBackupMailTransport()
{
    if (checkBackupType(Utils::MailTransport)) {
        backupTransports();
        increaseProgressDialog();
        if (wasCanceled()) {
            Q_EMIT jobFinished();
            return;
        }
    }
    QTimer::singleShot(0, this, SLOT(slotCheckBackupConfig()));
}

void ExportMailJob::slotCheckBackupConfig()
{
    if (checkBackupType(Utils::Config)) {
        backupConfig();
        increaseProgressDialog();
        if (wasCanceled()) {
            Q_EMIT jobFinished();
            return;
        }
    }
    QTimer::singleShot(0, this, SLOT(slotCheckBackupMails()));
}

void ExportMailJob::slotCheckBackupMails()
{
    if (checkBackupType(Utils::Mails)) {
        increaseProgressDialog();
        setProgressDialogLabel(i18n("Backing up Mails..."));
        QTimer::singleShot(0, this, SLOT(slotWriteNextArchiveResource()));
        return;
    }
    QTimer::singleShot(0, this, SLOT(slotCheckBackupResources()));
}

void ExportMailJob::slotMailsJobTerminated()
{
    if (wasCanceled()) {
        Q_EMIT jobFinished();
        return;
    }
    mIndexIdentifier++;
    QTimer::singleShot(0, this, SLOT(slotWriteNextArchiveResource()));
}

void ExportMailJob::slotWriteNextArchiveResource()
{
    Akonadi::AgentManager *manager = Akonadi::AgentManager::self();
    const Akonadi::AgentInstance::List list = manager->instances();
    if (mIndexIdentifier < list.count()) {
        const Akonadi::AgentInstance agent = list.at(mIndexIdentifier);
        const QStringList capabilities(agent.type().capabilities());
        if (agent.type().mimeTypes().contains(KMime::Message::mimeType())) {
            if (capabilities.contains(QStringLiteral("Resource")) &&
                    !capabilities.contains(QStringLiteral("Virtual")) &&
                    !capabilities.contains(QStringLiteral("MailTransport"))) {

                const QString identifier = agent.identifier();
                if (identifier.contains(QStringLiteral("akonadi_maildir_resource_")) ||
                        identifier.contains(QStringLiteral("akonadi_mixedmaildir_resource_"))) {
                    const QString archivePath = Utils::mailsPath() + identifier + QDir::separator();

                    const QString url = Utils::resourcePath(agent);
                    if (!mAgentPaths.contains(url)) {
                        mAgentPaths << url;
                        if (!url.isEmpty()) {
                            ExportResourceArchiveJob *resourceJob = new ExportResourceArchiveJob(this);
                            resourceJob->setArchivePath(archivePath);
                            resourceJob->setUrl(url);
                            resourceJob->setIdentifier(identifier);
                            resourceJob->setArchive(archive());
                            resourceJob->setArchiveName(QStringLiteral("mail.zip"));
                            connect(resourceJob, &ExportResourceArchiveJob::error, this, &ExportMailJob::error);
                            connect(resourceJob, &ExportResourceArchiveJob::info, this, &ExportMailJob::info);
                            connect(resourceJob, &ExportResourceArchiveJob::terminated, this, &ExportMailJob::slotMailsJobTerminated);
                            connect(this, &ExportMailJob::taskCanceled, resourceJob, &ExportResourceArchiveJob::slotTaskCanceled);
                            resourceJob->start();
                        }
                    } else {
                        QTimer::singleShot(0, this, SLOT(slotMailsJobTerminated()));
                    }
                } else if (identifier.contains(QStringLiteral("akonadi_mbox_resource_"))) {
                    backupResourceFile(agent, Utils::addressbookPath());
                    QTimer::singleShot(0, this, SLOT(slotMailsJobTerminated()));
                } else {
                    QTimer::singleShot(0, this, SLOT(slotMailsJobTerminated()));
                }
            } else {
                QTimer::singleShot(0, this, SLOT(slotMailsJobTerminated()));
            }
        } else {
            QTimer::singleShot(0, this, SLOT(slotMailsJobTerminated()));
        }
    } else {
        Q_EMIT info(i18n("Resources backup done."));
        QTimer::singleShot(0, this, SLOT(slotCheckBackupResources()));
    }
}

void ExportMailJob::backupTransports()
{
    setProgressDialogLabel(i18n("Backing up transports..."));

    const QString mailtransportsStr(QStringLiteral("mailtransports"));
    const QString maitransportsrc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + mailtransportsStr;
    if (!QFile(maitransportsrc).exists()) {
        Q_EMIT info(i18n("Transports backup done."));
    } else {
        KSharedConfigPtr mailtransportsConfig = KSharedConfig::openConfig(mailtransportsStr);

        QTemporaryFile tmp;
        tmp.open();
        KConfig *config = mailtransportsConfig->copyTo(tmp.fileName());

        config->sync();
        const bool fileAdded  = archive()->addLocalFile(tmp.fileName(), Utils::transportsPath() + QLatin1String("mailtransports"));
        delete config;
        if (fileAdded) {
            Q_EMIT info(i18n("Transports backup done."));
        } else {
            Q_EMIT error(i18n("Transport file cannot be added to backup file."));
        }
    }
}

void ExportMailJob::slotCheckBackupResources()
{
    if (checkBackupType(Utils::Resources)) {
        backupResources();
        increaseProgressDialog();
        if (wasCanceled()) {
            Q_EMIT jobFinished();
            return;
        }
    }
    Q_EMIT jobFinished();
}

void ExportMailJob::backupResources()
{
    setProgressDialogLabel(i18n("Backing up resources..."));

    Akonadi::AgentManager *manager = Akonadi::AgentManager::self();
    const Akonadi::AgentInstance::List list = manager->instances();
    foreach (const Akonadi::AgentInstance &agent, list) {
        const QStringList capabilities(agent.type().capabilities());
        if (agent.type().mimeTypes().contains(KMime::Message::mimeType())) {
            if (capabilities.contains(QStringLiteral("Resource")) &&
                    !capabilities.contains(QStringLiteral("Virtual")) &&
                    !capabilities.contains(QStringLiteral("MailTransport"))) {
                const QString identifier = agent.identifier();
                //Store just pop3/imap/kolab/gmail account. Store other config when we copy data.
                if (identifier.contains(QStringLiteral("pop3")) || identifier.contains(QStringLiteral("imap"))
                        || identifier.contains(QStringLiteral("_kolab_")) || identifier.contains(QStringLiteral("_gmail_"))) {
                    const QString errorStr = Utils::storeResources(archive(), identifier, Utils::resourcesPath());
                    if (!errorStr.isEmpty()) {
                        Q_EMIT error(errorStr);
                    }
                } else {
                    qCDebug(PIMSETTINGEXPORTERCORE_LOG) << " resource \"" << identifier << "\" will not store";
                }
            }
        }
    }

    Q_EMIT info(i18n("Resources backup done."));
}

void ExportMailJob::backupConfig()
{
    setProgressDialogLabel(i18n("Backing up config..."));

    QList<MailCommon::MailFilter *> lstFilter = MailCommon::FilterManager::instance()->filters();
    if (!lstFilter.isEmpty()) {
        QTemporaryFile tmp;
        tmp.open();
        QUrl url(tmp.fileName());
        MailCommon::FilterImporterExporter exportFilters;
        exportFilters.exportFilters(lstFilter, url, true);
        const bool fileAdded  = archive()->addLocalFile(tmp.fileName(), Utils::configsPath() + QLatin1String("filters"));
        if (fileAdded) {
            Q_EMIT info(i18n("Filters backup done."));
        } else {
            Q_EMIT error(i18n("Filters cannot be exported."));
        }
    }

    backupUiRcFile(QStringLiteral("sieveeditorui.rc"), QStringLiteral("sieveeditor"));
    backupUiRcFile(QStringLiteral("storageservicemanagerui.rc"), QStringLiteral("storageservicemanager"));
    backupUiRcFile(QStringLiteral("kmreadermainwin.rc"), QStringLiteral("kmail2"));
    backupUiRcFile(QStringLiteral("kmcomposerui.rc"), QStringLiteral("kmail2"));
    backupUiRcFile(QStringLiteral("kmmainwin.rc"), QStringLiteral("kmail2"));
    backupUiRcFile(QStringLiteral("kmail_part.rc"), QStringLiteral("kmail2"));
    backupUiRcFile(QStringLiteral("kontactsummary_part.rc"), QStringLiteral("kontactsummary"));
    backupUiRcFile(QStringLiteral("kontactui.rc"), QStringLiteral("kontact"));
    backupUiRcFile(QStringLiteral("kleopatra.rc"), QStringLiteral("kleopatra"));
    backupUiRcFile(QStringLiteral("headerthemeeditorui.rc"), QStringLiteral("headerthemeeditor"));
    backupUiRcFile(QStringLiteral("contactthemeeditorui.rc"), QStringLiteral("contactthemeeditor"));
    backupUiRcFile(QStringLiteral("contactprintthemeeditorui.rc"), QStringLiteral("contactprintthemeeditor"));
    backupUiRcFile(QStringLiteral("kwatchgnupgui.rc"), QStringLiteral("kwatchgnupg"));
    backupUiRcFile(QStringLiteral("akonadiconsoleui.rc"), QStringLiteral("akonadiconsole"));

    backupConfigFile(QStringLiteral("kabldaprc"));
    backupConfigFile(QStringLiteral("kmailsnippetrc"));
    backupConfigFile(QStringLiteral("sievetemplaterc"));
    backupConfigFile(QStringLiteral("customtemplatesrc"));
    backupConfigFile(QStringLiteral("kontactrc"));
    backupConfigFile(QStringLiteral("kontact_summaryrc"));
    backupConfigFile(QStringLiteral("storageservicerc"));
    backupConfigFile(QStringLiteral("kpimbalooblacklist"));
    backupConfigFile(QStringLiteral("kleopatrarc"));
    backupConfigFile(QStringLiteral("sieveeditorrc"));
    backupConfigFile(QStringLiteral("kwatchgnupgrc"));

    //Notify file config
    backupConfigFile(QStringLiteral("akonadi_mailfilter_agent.notifyrc"));
    backupConfigFile(QStringLiteral("akonadi_sendlater_agent.notifyrc"));
    backupConfigFile(QStringLiteral("akonadi_archivemail_agent.notifyrc"));
    backupConfigFile(QStringLiteral("kmail2.notifyrc"));
    backupConfigFile(QStringLiteral("akonadi_newmailnotifier_agent.notifyrc"));
    backupConfigFile(QStringLiteral("akonadi_maildispatcher_agent.notifyrc"));
    backupConfigFile(QStringLiteral("akonadi_followupreminder_agent.notifyrc"));
    backupConfigFile(QStringLiteral("messagevieweradblockrc"));
    backupConfigFile(QStringLiteral("messageviewer.notifyrc"));
    backupConfigFile(QStringLiteral("storageservicemanager.notifyrc"));


    const QString folderMailArchiveStr(QStringLiteral("foldermailarchiverc"));
    const QString folderMailArchiverc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + folderMailArchiveStr;
    if (QFile(folderMailArchiverc).exists()) {
        KSharedConfigPtr archivemailrc = KSharedConfig::openConfig(folderMailArchiveStr);

        QTemporaryFile tmp;
        tmp.open();

        KConfig *archiveConfig = archivemailrc->copyTo(tmp.fileName());
        const QStringList archiveList = archiveConfig->groupList().filter(QRegularExpression(QStringLiteral("FolderArchiveAccount")));

        Q_FOREACH (const QString &str, archiveList) {
            KConfigGroup oldGroup = archiveConfig->group(str);
            qint64 id = oldGroup.readEntry("topLevelCollectionId", -1);
            if (id != -1)  {
                const QString realPath = MailCommon::Util::fullCollectionPath(Akonadi::Collection(id));
                if (!realPath.isEmpty()) {
                    oldGroup.writeEntry(QStringLiteral("topLevelCollectionId"), realPath);
                }
            }
        }
        archiveConfig->sync();

        backupFile(tmp.fileName(), Utils::configsPath(), folderMailArchiveStr);
        delete archiveConfig;
    }


    const QString archiveMailAgentConfigurationStr(QStringLiteral("akonadi_archivemail_agentrc"));
    const QString archiveMailAgentconfigurationrc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + archiveMailAgentConfigurationStr;
    if (QFile(archiveMailAgentconfigurationrc).exists()) {
        KSharedConfigPtr archivemailrc = KSharedConfig::openConfig(archiveMailAgentConfigurationStr);

        QTemporaryFile tmp;
        tmp.open();

        KConfig *archiveConfig = archivemailrc->copyTo(tmp.fileName());
        const QStringList archiveList = archiveConfig->groupList().filter(QRegularExpression(QLatin1String("ArchiveMailCollection \\d+")));
        const QString archiveGroupPattern = QStringLiteral("ArchiveMailCollection ");

        Q_FOREACH (const QString &str, archiveList) {
            bool found = false;
            const int collectionId = str.rightRef(str.length() - archiveGroupPattern.length()).toInt(&found);
            if (found) {
                KConfigGroup oldGroup = archiveConfig->group(str);
                const QString realPath = MailCommon::Util::fullCollectionPath(Akonadi::Collection(collectionId));
                if (!realPath.isEmpty()) {
                    const QString collectionPath(archiveGroupPattern + realPath);
                    KConfigGroup newGroup(archiveConfig, collectionPath);
                    oldGroup.copyTo(&newGroup);
                    newGroup.writeEntry(QStringLiteral("saveCollectionId"), collectionPath);
                }
                oldGroup.deleteGroup();
            }
        }
        archiveConfig->sync();

        backupFile(tmp.fileName(), Utils::configsPath(), archiveMailAgentConfigurationStr);
        delete archiveConfig;
    }

    const QString templatesconfigurationrcStr(QStringLiteral("templatesconfigurationrc"));
    const QString templatesconfigurationrc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + templatesconfigurationrcStr;
    if (QFile(templatesconfigurationrc).exists()) {
        KSharedConfigPtr templaterc = KSharedConfig::openConfig(templatesconfigurationrcStr);

        QTemporaryFile tmp;
        tmp.open();

        KConfig *templateConfig = templaterc->copyTo(tmp.fileName());
        const QString templateGroupPattern = QStringLiteral("Templates #");
        const QStringList templateList = templateConfig->groupList().filter(QRegularExpression(QStringLiteral("Templates #\\d+")));
        Q_FOREACH (const QString &str, templateList) {
            bool found = false;
            const int collectionId = str.rightRef(str.length() - templateGroupPattern.length()).toInt(&found);
            if (found) {
                KConfigGroup oldGroup = templateConfig->group(str);
                const QString realPath = MailCommon::Util::fullCollectionPath(Akonadi::Collection(collectionId));
                if (!realPath.isEmpty()) {
                    KConfigGroup newGroup(templateConfig, templateGroupPattern + realPath);
                    oldGroup.copyTo(&newGroup);
                }
                oldGroup.deleteGroup();
            }
        }
        templateConfig->sync();

        backupFile(tmp.fileName(), Utils::configsPath(), templatesconfigurationrcStr);
        delete templateConfig;
    }

    storeDirectory(QStringLiteral("/messageviewer/themes/"));

    const QDir gravatarDirectory(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/gravatar/"));
    if (gravatarDirectory.exists()) {
        const QFileInfoList listFileInfo = gravatarDirectory.entryInfoList(QStringList() << QStringLiteral("*.png"), QDir::Files);
        const int listSize(listFileInfo.size());
        for (int i = 0; i < listSize; ++i) {
            backupFile(listFileInfo.at(i).absoluteFilePath(), Utils::dataPath() + QLatin1String("gravatar/"), listFileInfo.at(i).fileName());
        }
    }

    const QDir autocorrectDirectory(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/autocorrect/"));
    if (autocorrectDirectory.exists()) {
        const QFileInfoList listFileInfo = autocorrectDirectory.entryInfoList(QStringList() << QStringLiteral("*.xml"), QDir::Files);
        const int listSize(listFileInfo.size());
        for (int i = 0; i < listSize; ++i) {
            backupFile(listFileInfo.at(i).absoluteFilePath(), Utils::dataPath() + QLatin1String("autocorrect/"), listFileInfo.at(i).fileName());
        }
    }
    const QString adblockFilePath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kmail2/adblockrules_local");
    if (!adblockFilePath.isEmpty()) {
        backupFile(adblockFilePath, Utils::dataPath() + QLatin1String("kmail2/"), QStringLiteral("adblockrules_local"));
    }

    const QString kmailStr(QStringLiteral("kmail2rc"));
    const QString kmail2rc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + kmailStr;
    if (QFile(kmail2rc).exists()) {
        KSharedConfigPtr kmailrc = KSharedConfig::openConfig(kmail2rc);

        QTemporaryFile tmp;
        tmp.open();

        KConfig *kmailConfig = kmailrc->copyTo(tmp.fileName());
        const QString folderGroupPattern = QStringLiteral("Folder-");
        const QStringList folderList = kmailConfig->groupList().filter(QRegularExpression(QStringLiteral("Folder-\\d+")));
        Q_FOREACH (const QString &str, folderList) {
            bool found = false;
            const int collectionId = str.rightRef(str.length() - folderGroupPattern.length()).toInt(&found);
            if (found) {
                KConfigGroup oldGroup = kmailConfig->group(str);
                const QString realPath = MailCommon::Util::fullCollectionPath(Akonadi::Collection(collectionId));
                if (!realPath.isEmpty()) {
                    KConfigGroup newGroup(kmailConfig, folderGroupPattern + realPath);
                    oldGroup.copyTo(&newGroup);
                }
                oldGroup.deleteGroup();
            }
        }
        const QString composerStr(QStringLiteral("Composer"));
        if (kmailConfig->hasGroup(composerStr)) {
            KConfigGroup composerGroup = kmailConfig->group(composerStr);
            const QString previousStr(QStringLiteral("previous-fcc"));
            if (composerGroup.hasKey(previousStr)) {
                const int collectionId = composerGroup.readEntry(previousStr, -1);
                if (collectionId != -1) {
                    const QString realPath = MailCommon::Util::fullCollectionPath(Akonadi::Collection(collectionId));
                    composerGroup.writeEntry(previousStr, realPath);
                }
            }
        }

        const QString generalStr(QStringLiteral("General"));
        if (kmailConfig->hasGroup(generalStr)) {
            KConfigGroup generalGroup = kmailConfig->group(generalStr);
            const QString startupFolderStr(QStringLiteral("startupFolder"));
            if (generalGroup.hasKey(startupFolderStr)) {
                const int collectionId = generalGroup.readEntry(startupFolderStr, -1);
                if (collectionId != -1) {
                    const QString realPath = MailCommon::Util::fullCollectionPath(Akonadi::Collection(collectionId));
                    generalGroup.writeEntry(startupFolderStr, realPath);
                }
            }
        }

        const QString storageModelSelectedMessageStr(QStringLiteral("MessageListView::StorageModelSelectedMessages"));
        if (kmailConfig->hasGroup(storageModelSelectedMessageStr)) {
            KConfigGroup storageGroup = kmailConfig->group(storageModelSelectedMessageStr);
            const QString storageModelSelectedPattern(QStringLiteral("MessageUniqueIdForStorageModel"));
            const QStringList storageList = storageGroup.keyList().filter(QRegularExpression(QLatin1String("MessageUniqueIdForStorageModel\\d+")));
            Q_FOREACH (const QString &str, storageList) {
                bool found = false;
                const int collectionId = str.rightRef(str.length() - storageModelSelectedPattern.length()).toInt(&found);
                const QString oldValue = storageGroup.readEntry(str);
                if (found) {
                    const QString realPath = MailCommon::Util::fullCollectionPath(Akonadi::Collection(collectionId));
                    if (!realPath.isEmpty()) {
                        storageGroup.writeEntry(QStringLiteral("%1%2").arg(storageModelSelectedPattern, realPath), oldValue);
                        storageGroup.deleteEntry(str);
                    }
                }
            }
        }

        const QString collectionFolderViewStr(QStringLiteral("CollectionFolderView"));
        if (kmailConfig->hasGroup(collectionFolderViewStr)) {
            KConfigGroup favoriteGroup = kmailConfig->group(collectionFolderViewStr);

            const QString currentKey(QStringLiteral("Current"));
            Utils::convertCollectionToRealPath(favoriteGroup, currentKey);

            const QString expensionKey(QStringLiteral("Expansion"));
            Utils::convertCollectionListToRealPath(favoriteGroup, expensionKey);
        }

        const QString favoriteCollectionStr(QStringLiteral("FavoriteCollections"));
        if (kmailConfig->hasGroup(favoriteCollectionStr)) {
            KConfigGroup favoriteGroup = kmailConfig->group(favoriteCollectionStr);

            const QString favoriteCollectionIdsStr(QStringLiteral("FavoriteCollectionIds"));
            Utils::convertCollectionIdsToRealPath(favoriteGroup, favoriteCollectionIdsStr);
        }

        kmailConfig->sync();
        backupFile(tmp.fileName(), Utils::configsPath(), kmailStr);
        delete kmailConfig;
    }

    Q_EMIT info(i18n("Config backup done."));
}

void ExportMailJob::backupIdentity()
{
    setProgressDialogLabel(i18n("Backing up identity..."));

    const QString emailidentitiesStr(QStringLiteral("emailidentities"));
    const QString emailidentitiesrc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + emailidentitiesStr;
    if (QFile(emailidentitiesrc).exists()) {

        KSharedConfigPtr identity = KSharedConfig::openConfig(emailidentitiesrc);

        QTemporaryFile tmp;
        tmp.open();

        KConfig *identityConfig = identity->copyTo(tmp.fileName());
        const QStringList accountList = identityConfig->groupList().filter(QRegularExpression(QStringLiteral("Identity #\\d+")));
        Q_FOREACH (const QString &account, accountList) {
            KConfigGroup group = identityConfig->group(account);
            const QString fcc = QStringLiteral("Fcc");
            if (group.hasKey(fcc)) {
                group.writeEntry(fcc, MailCommon::Util::fullCollectionPath(Akonadi::Collection(group.readEntry(fcc).toLongLong())));
            }
            const QString draft = QStringLiteral("Drafts");
            if (group.hasKey(draft)) {
                group.writeEntry(draft, MailCommon::Util::fullCollectionPath(Akonadi::Collection(group.readEntry(draft).toLongLong())));
            }
            const QString templates = QStringLiteral("Templates");
            if (group.hasKey(templates)) {
                group.writeEntry(templates, MailCommon::Util::fullCollectionPath(Akonadi::Collection(group.readEntry(templates).toLongLong())));
            }
            const QString vcard = QStringLiteral("VCardFile");
            if (group.hasKey(vcard)) {
                const QString vcardFileName = group.readEntry(vcard);
                if (!vcardFileName.isEmpty()) {
                    const int uoid = group.readEntry(QStringLiteral("uoid"), -1);
                    QFile file(vcardFileName);
                    if (file.exists()) {
                        const bool fileAdded  = archive()->addLocalFile(vcardFileName, Utils::identitiesPath() + QString::number(uoid) + QDir::separator() + file.fileName());
                        if (!fileAdded) {
                            Q_EMIT error(i18n("vCard file \"%1\" cannot be saved.", file.fileName()));
                        }
                    } else {
                        group.deleteEntry(vcard);
                    }
                }
            }
        }

        identityConfig->sync();
        const bool fileAdded  = archive()->addLocalFile(tmp.fileName(), Utils::identitiesPath() + QLatin1String("emailidentities"));
        delete identityConfig;
        if (fileAdded) {
            Q_EMIT info(i18n("Identity backup done."));
        } else {
            Q_EMIT error(i18n("Identity file cannot be added to backup file."));
        }
    }
}

#if 0
void ExportMailJob::backupAkonadiDb()
{
    setProgressDialogLabel(i18n("Backing up Akonadi Database..."));

    AkonadiDataBase akonadiDataBase;
    const QString dbDriver(akonadiDataBase.driver());

    QTemporaryFile tmp;
    tmp.open();

    QStringList params;
    QString dbDumpAppName;
    if (dbDriver == QLatin1String("QMYSQL")) {
        dbDumpAppName = QStringLiteral("mysqldump");

        params << QStringLiteral("--single-transaction")
               << QStringLiteral("--flush-logs")
               << QStringLiteral("--triggers")
               << QStringLiteral("--result-file=") + tmp.fileName()
               << akonadiDataBase.options()
               << akonadiDataBase.name();
    } else if (dbDriver == QLatin1String("QPSQL")) {
        dbDumpAppName = QStringLiteral("pg_dump");
        params << QStringLiteral("--format=custom")
               << QStringLiteral("--blobs")
               << QStringLiteral("--file=") + tmp.fileName()
               << akonadiDataBase.options()
               << akonadiDataBase.name();
    } else {
        Q_EMIT error(i18n("Database driver \"%1\" not supported.", dbDriver));
        return;
    }
    const QString dbDumpApp = QStandardPaths::findExecutable(dbDumpAppName);
    if (dbDumpApp.isEmpty()) {
        Q_EMIT error(i18n("Could not find \"%1\" necessary to dump database.", dbDumpAppName));
        return;
    }
    KProcess *proc = new KProcess(this);
    proc->setProgram(dbDumpApp, params);
    const int result = proc->execute();
    delete proc;
    if (result != 0) {
        qCDebug(PIMSETTINGEXPORTERCORE_LOG) << " Error during dump Database";
        return;
    }
    const bool fileAdded  = archive()->addLocalFile(tmp.fileName(), Utils::akonadiPath() + QLatin1String("akonadidatabase.sql"));
    if (!fileAdded) {
        Q_EMIT error(i18n("Akonadi Database \"%1\" cannot be added to backup file.", QStringLiteral("akonadidatabase.sql")));
    } else {
        Q_EMIT info(i18n("Akonadi Database backup done."));
    }
}
#endif
