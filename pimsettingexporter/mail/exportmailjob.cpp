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

#include "exportmailjob.h"
#include "akonadidatabase.h"
#include "messageviewer/utils/kcursorsaver.h"
#include "mailcommon/util/mailutil.h"
#include "mailcommon/filter/filtermanager.h"
#include "mailcommon/filter/filterimporterexporter.h"

#include <Akonadi/AgentManager>
#include <Akonadi/Collection>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>

#include <Mailtransport/TransportManager>


#include <KZip>
#include <KLocalizedString>
#include <KTemporaryFile>
#include <KStandardDirs>
#include <KProcess>

#include <QDebug>
#include <QFile>
#include <QDir>

ExportMailJob::ExportMailJob(QWidget *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage,int numberOfStep)
    : AbstractImportExportJob(parent,archiveStorage,typeSelected,numberOfStep),
      mArchiveTime(QDateTime::currentDateTime().toTime_t())
{
}

ExportMailJob::~ExportMailJob()
{
}

bool ExportMailJob::checkProgram()
{
    if (KStandardDirs::findExe(QLatin1String("mysqldump")).isEmpty()) {
        Q_EMIT error(i18n("mysqldump not found. Export data aborted"));
        return false;
    }
    return true;
}

void ExportMailJob::start()
{
    if (!checkProgram()) {
        Q_EMIT jobFinished();
        return;
    }

    Q_EMIT title(i18n("Start export KNotes settings..."));
    createProgressDialog();

    if (mTypeSelected & Utils::Identity) {
        backupIdentity();
        increaseProgressDialog();
        if (wasCanceled()) {
            Q_EMIT jobFinished();
            return;
        }
    }
    if (mTypeSelected & Utils::MailTransport) {
        backupTransports();
        increaseProgressDialog();
        if (wasCanceled()) {
            Q_EMIT jobFinished();
            return;
        }
    }
    if (mTypeSelected & Utils::Mails) {
        backupMails();
        increaseProgressDialog();
        if (wasCanceled()) {
            Q_EMIT jobFinished();
            return;
        }
    }
    if (mTypeSelected & Utils::Resources) {
        backupResources();
        increaseProgressDialog();
        if (wasCanceled()) {
            Q_EMIT jobFinished();
            return;
        }
    }
    if (mTypeSelected & Utils::Config) {
        backupConfig();
        increaseProgressDialog();
        if (wasCanceled()) {
            Q_EMIT jobFinished();
            return;
        }
    }
    if (mTypeSelected & Utils::AkonadiDb) {
        backupAkonadiDb();
        increaseProgressDialog();
        if (wasCanceled()) {
            Q_EMIT jobFinished();
            return;
        }
    }
    Q_EMIT jobFinished();
}

void ExportMailJob::backupTransports()
{
    showInfo(i18n("Backing up transports..."));
    MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );

    const QString mailtransportsStr(QLatin1String("mailtransports"));
    const QString maitransportsrc = KStandardDirs::locateLocal( "config",  mailtransportsStr);
    if (!QFile(maitransportsrc).exists()) {
        Q_EMIT info(i18n("Transports backup done."));
    } else {
        KSharedConfigPtr mailtransportsConfig = KSharedConfig::openConfig( mailtransportsStr );

        KTemporaryFile tmp;
        tmp.open();
        KConfig *config = mailtransportsConfig->copyTo( tmp.fileName() );

        config->sync();
        const bool fileAdded  = archive()->addLocalFile(tmp.fileName(), Utils::transportsPath() + QLatin1String("mailtransports"));
        delete config;
        if (fileAdded)
            Q_EMIT info(i18n("Transports backup done."));
        else
            Q_EMIT error(i18n("Transport file cannot be added to backup file."));
    }
}

void ExportMailJob::backupResources()
{
    showInfo(i18n("Backing up resources..."));
    MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );

    Akonadi::AgentManager *manager = Akonadi::AgentManager::self();
    const Akonadi::AgentInstance::List list = manager->instances();
    foreach( const Akonadi::AgentInstance &agent, list ) {
        const QStringList capabilities( agent.type().capabilities() );
        if (agent.type().mimeTypes().contains( KMime::Message::mimeType())) {
            if ( capabilities.contains( QLatin1String("Resource") ) &&
                 !capabilities.contains( QLatin1String("Virtual") ) &&
                 !capabilities.contains( QLatin1String("MailTransport") ) )
            {
                const QString identifier = agent.identifier();
                //Store just pop3/imap account. Store other config when we copy data.
                if (identifier.contains(QLatin1String("pop3")) || identifier.contains(QLatin1String("imap"))) {
                    const QString errorStr = Utils::storeResources(archive(), identifier, Utils::resourcesPath());
                    if (!errorStr.isEmpty()) {
                        Q_EMIT error(errorStr);
                    }
                } else {
                    qDebug()<<" resource \""<<identifier<<"\" will not store";
                }
            }
        }
    }

    Q_EMIT info(i18n("Resources backup done."));
}

void ExportMailJob::backupConfig()
{
    showInfo(i18n("Backing up config..."));
    MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
    QList<MailCommon::MailFilter*> lstFilter = MailCommon::FilterManager::instance()->filters();
    if (!lstFilter.isEmpty()) {
        KTemporaryFile tmp;
        tmp.open();
        KUrl url(tmp.fileName());
        MailCommon::FilterImporterExporter exportFilters;
        exportFilters.exportFilters(lstFilter,url, true);
        const bool fileAdded  = archive()->addLocalFile(tmp.fileName(), Utils::configsPath() + QLatin1String("filters"));
        if (fileAdded)
            Q_EMIT info(i18n("Filters backup done."));
        else
            Q_EMIT error(i18n("Filters cannot be exported."));
    }

    backupConfigFile(QLatin1String("kabldaprc"));
    backupConfigFile(QLatin1String("kmailsnippetrc"));
    backupConfigFile(QLatin1String("sievetemplaterc"));
    backupConfigFile(QLatin1String("customtemplatesrc"));
    backupConfigFile(QLatin1String("kontactrc"));
    backupConfigFile(QLatin1String("kontact_summaryrc"));
    backupConfigFile(QLatin1String("storageservicerc"));

    //Notify file config
    backupConfigFile(QLatin1String("akonadi_mailfilter_agent.notifyrc"));
    backupConfigFile(QLatin1String("akonadi_sendlater_agent.notifyrc"));
    backupConfigFile(QLatin1String("akonadi_archivemail_agent.notifyrc"));
    backupConfigFile(QLatin1String("kmail2.notifyrc"));
    backupConfigFile(QLatin1String("akonadi_newmailnotifier_agent.notifyrc"));
    backupConfigFile(QLatin1String("akonadi_maildispatcher_agent.notifyrc"));
    backupConfigFile(QLatin1String("akonadi_followupreminder_agent.notifyrc"));
    backupConfigFile(QLatin1String("messagevieweradblockrc"));
    backupConfigFile(QLatin1String("messageviewer.notifyrc"));

    const QString folderArchiveAgentConfigurationStr(QLatin1String("akonadi_folderarchive_agentrc"));
    const QString folderArchiveAgentConfigurationrc = KStandardDirs::locateLocal( "config", folderArchiveAgentConfigurationStr );
    if (QFile(folderArchiveAgentConfigurationrc).exists()) {
        KSharedConfigPtr archivemailrc = KSharedConfig::openConfig(folderArchiveAgentConfigurationStr);

        KTemporaryFile tmp;
        tmp.open();

        KConfig *archiveConfig = archivemailrc->copyTo( tmp.fileName() );
        const QStringList archiveList = archiveConfig->groupList().filter( QRegExp( QLatin1String("FolderArchiveAccount ") ) );

        Q_FOREACH(const QString& str, archiveList) {
            KConfigGroup oldGroup = archiveConfig->group(str);
            qint64 topLevelId = oldGroup.readEntry(QLatin1String("topLevelCollectionId"), -1);
            if (topLevelId!=-1) {
                const QString realPath = MailCommon::Util::fullCollectionPath(Akonadi::Collection( topLevelId ));
                if (!realPath.isEmpty()) {
                    oldGroup.writeEntry(QLatin1String("topLevelCollectionId"), realPath);
                }
            }
        }
        archiveConfig->sync();

        backupFile(tmp.fileName(), Utils::configsPath(), folderArchiveAgentConfigurationStr);
        delete archiveConfig;
    }

    const QString archiveMailAgentConfigurationStr(QLatin1String("akonadi_archivemail_agentrc"));
    const QString archiveMailAgentconfigurationrc = KStandardDirs::locateLocal( "config", archiveMailAgentConfigurationStr );
    if (QFile(archiveMailAgentconfigurationrc).exists()) {
        KSharedConfigPtr archivemailrc = KSharedConfig::openConfig(archiveMailAgentConfigurationStr);

        KTemporaryFile tmp;
        tmp.open();

        KConfig *archiveConfig = archivemailrc->copyTo( tmp.fileName() );
        const QStringList archiveList = archiveConfig->groupList().filter( QRegExp( QLatin1String("ArchiveMailCollection \\d+") ) );
        const QString archiveGroupPattern = QLatin1String( "ArchiveMailCollection " );

        Q_FOREACH(const QString& str, archiveList) {
            bool found = false;
            const int collectionId = str.right(str.length()-archiveGroupPattern.length()).toInt(&found);
            if (found) {
                KConfigGroup oldGroup = archiveConfig->group(str);
                const QString realPath = MailCommon::Util::fullCollectionPath(Akonadi::Collection( collectionId ));
                if (!realPath.isEmpty()) {
                    const QString collectionPath(archiveGroupPattern + realPath);
                    KConfigGroup newGroup( archiveConfig, collectionPath);
                    oldGroup.copyTo( &newGroup );
                    newGroup.writeEntry(QLatin1String("saveCollectionId"),collectionPath);
                }
                oldGroup.deleteGroup();
            }
        }
        archiveConfig->sync();

        backupFile(tmp.fileName(), Utils::configsPath(), archiveMailAgentConfigurationStr);
        delete archiveConfig;
    }

    const QString templatesconfigurationrcStr(QLatin1String("templatesconfigurationrc"));
    const QString templatesconfigurationrc = KStandardDirs::locateLocal( "config",  templatesconfigurationrcStr);
    if (QFile(templatesconfigurationrc).exists()) {
        KSharedConfigPtr templaterc = KSharedConfig::openConfig(templatesconfigurationrcStr);

        KTemporaryFile tmp;
        tmp.open();

        KConfig *templateConfig = templaterc->copyTo( tmp.fileName() );
        const QString templateGroupPattern = QLatin1String( "Templates #" );
        const QStringList templateList = templateConfig->groupList().filter( QRegExp( QLatin1String("Templates #\\d+") ) );
        Q_FOREACH(const QString& str, templateList) {
            bool found = false;
            const int collectionId = str.right(str.length()-templateGroupPattern.length()).toInt(&found);
            if (found) {
                KConfigGroup oldGroup = templateConfig->group(str);
                const QString realPath = MailCommon::Util::fullCollectionPath(Akonadi::Collection( collectionId ));
                if (!realPath.isEmpty()) {
                    KConfigGroup newGroup( templateConfig, templateGroupPattern + realPath);
                    oldGroup.copyTo( &newGroup );
                }
                oldGroup.deleteGroup();
            }
        }
        templateConfig->sync();

        backupFile(tmp.fileName(), Utils::configsPath(), templatesconfigurationrcStr);
        delete templateConfig;
    }

    const QDir themeDirectory( KStandardDirs::locateLocal( "data", QLatin1String( "messageviewer/themes/" ) ) );
    if (themeDirectory.exists()) {
        const bool themeDirAdded = archive()->addLocalDirectory(themeDirectory.path(), Utils::dataPath() + QLatin1String( "messageviewer/themes/" ));
        if (!themeDirAdded) {
            Q_EMIT error(i18n("Theme directory \"%1\" cannot be added to backup file.", themeDirectory.path()));
        }
    }

    const QDir autocorrectDirectory( KStandardDirs::locateLocal( "data", QLatin1String( "autocorrect/" ) ) );
    if (autocorrectDirectory.exists()) {
        const QFileInfoList listFileInfo = autocorrectDirectory.entryInfoList(QStringList()<< QLatin1String("*.xml"), QDir::Files);
        const int listSize(listFileInfo.size());
        for (int i = 0; i < listSize; ++i) {
            backupFile(listFileInfo.at(i).absoluteFilePath(), Utils::dataPath() + QLatin1String( "autocorrect/" ) , listFileInfo.at(i).fileName());
        }
    }
    const QString adblockFilePath = KStandardDirs::locateLocal( "data", QLatin1String( "kmail2/adblockrules_local" ) );
    if (!adblockFilePath.isEmpty()) {
        backupFile(adblockFilePath, Utils::dataPath() + QLatin1String( "kmail2/" ) , QLatin1String("adblockrules_local"));
    }

    const QString kmailStr(QLatin1String("kmail2rc"));
    const QString kmail2rc = KStandardDirs::locateLocal( "config",  kmailStr);
    if (QFile(kmail2rc).exists()) {
        KSharedConfigPtr kmailrc = KSharedConfig::openConfig(kmail2rc);

        KTemporaryFile tmp;
        tmp.open();

        KConfig *kmailConfig = kmailrc->copyTo( tmp.fileName() );
        const QString folderGroupPattern = QLatin1String( "Folder-" );
        const QStringList folderList = kmailConfig->groupList().filter( QRegExp( QLatin1String("Folder-\\d+") ) );
        Q_FOREACH(const QString& str, folderList) {
            bool found = false;
            const int collectionId = str.right(str.length()-folderGroupPattern.length()).toInt(&found);
            if (found) {
                KConfigGroup oldGroup = kmailConfig->group(str);
                const QString realPath = MailCommon::Util::fullCollectionPath(Akonadi::Collection( collectionId ));
                if (!realPath.isEmpty()) {
                    KConfigGroup newGroup( kmailConfig, folderGroupPattern + realPath);
                    oldGroup.copyTo( &newGroup );
                }
                oldGroup.deleteGroup();
            }
        }
        const QString composerStr(QLatin1String("Composer"));
        if (kmailConfig->hasGroup(composerStr)) {
            KConfigGroup composerGroup = kmailConfig->group(composerStr);
            const QString previousStr(QLatin1String("previous-fcc"));
            if (composerGroup.hasKey(previousStr)) {
                const int collectionId = composerGroup.readEntry(previousStr,-1);
                if (collectionId!=-1) {
                    const QString realPath = MailCommon::Util::fullCollectionPath(Akonadi::Collection( collectionId ));
                    composerGroup.writeEntry(previousStr,realPath);
                }
            }
        }

        const QString generalStr(QLatin1String("General"));
        if (kmailConfig->hasGroup(generalStr)) {
            KConfigGroup generalGroup = kmailConfig->group(generalStr);
            const QString startupFolderStr(QLatin1String("startupFolder"));
            if (generalGroup.hasKey(startupFolderStr)) {
                const int collectionId = generalGroup.readEntry(startupFolderStr,-1);
                if (collectionId!=-1) {
                    const QString realPath = MailCommon::Util::fullCollectionPath(Akonadi::Collection( collectionId ));
                    generalGroup.writeEntry(startupFolderStr,realPath);
                }
            }
        }

        const QString storageModelSelectedMessageStr(QLatin1String("MessageListView::StorageModelSelectedMessages"));
        if (kmailConfig->hasGroup(storageModelSelectedMessageStr)) {
            KConfigGroup storageGroup = kmailConfig->group(storageModelSelectedMessageStr);
            const QString storageModelSelectedPattern(QLatin1String("MessageUniqueIdForStorageModel"));
            const QStringList storageList = storageGroup.keyList().filter( QRegExp( QLatin1String("MessageUniqueIdForStorageModel\\d+") ) );
            Q_FOREACH(const QString& str, storageList) {
                bool found = false;
                const int collectionId = str.right(str.length()-storageModelSelectedPattern.length()).toInt(&found);
                const QString oldValue = storageGroup.readEntry(str);
                if (found) {
                    const QString realPath = MailCommon::Util::fullCollectionPath(Akonadi::Collection( collectionId ));
                    if (!realPath.isEmpty()) {
                        storageGroup.writeEntry(QString::fromLatin1("%1%2").arg(storageModelSelectedPattern).arg(realPath),oldValue);
                        storageGroup.deleteEntry(str);
                    }
                }
            }
        }

        const QString collectionFolderViewStr(QLatin1String("CollectionFolderView"));
        if (kmailConfig->hasGroup(collectionFolderViewStr)) {
            KConfigGroup favoriteGroup = kmailConfig->group(collectionFolderViewStr);

            const QString currentKey(QLatin1String("Current"));
            Utils::convertCollectionToRealPath(favoriteGroup, currentKey);

            const QString expensionKey(QLatin1String("Expansion"));
            Utils::convertCollectionListToRealPath(favoriteGroup, expensionKey);
        }

        const QString favoriteCollectionStr(QLatin1String("FavoriteCollections"));
        if (kmailConfig->hasGroup(favoriteCollectionStr)) {
            KConfigGroup favoriteGroup = kmailConfig->group(favoriteCollectionStr);

            const QString favoriteCollectionIdsStr(QLatin1String("FavoriteCollectionIds"));
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
    showInfo(i18n("Backing up identity..."));
    MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
    const QString emailidentitiesStr(QLatin1String("emailidentities"));
    const QString emailidentitiesrc = KStandardDirs::locateLocal( "config",  emailidentitiesStr);
    if (QFile(emailidentitiesrc).exists()) {


        KSharedConfigPtr identity = KSharedConfig::openConfig( emailidentitiesrc );

        KTemporaryFile tmp;
        tmp.open();

        KConfig *identityConfig = identity->copyTo( tmp.fileName() );
        const QStringList accountList = identityConfig->groupList().filter( QRegExp( QLatin1String("Identity #\\d+") ) );
        Q_FOREACH(const QString& account, accountList) {
            KConfigGroup group = identityConfig->group(account);
            const QString fcc =QLatin1String("Fcc");
            if (group.hasKey(fcc)) {
                group.writeEntry(fcc,MailCommon::Util::fullCollectionPath(Akonadi::Collection(group.readEntry(fcc).toLongLong())));
            }
            const QString draft = QLatin1String("Drafts");
            if (group.hasKey(draft)) {
                group.writeEntry(draft,MailCommon::Util::fullCollectionPath(Akonadi::Collection(group.readEntry(draft).toLongLong())));
            }
            const QString templates = QLatin1String("Templates");
            if (group.hasKey(templates)) {
                group.writeEntry(templates,MailCommon::Util::fullCollectionPath(Akonadi::Collection(group.readEntry(templates).toLongLong())));
            }
            const QString vcard = QLatin1String("VCardFile");
            if (group.hasKey(vcard)) {
                const QString vcardFileName = group.readEntry(vcard);
                if (!vcardFileName.isEmpty()) {
                    const int uoid = group.readEntry(QLatin1String("uoid"),-1);
                    QFile file(vcardFileName);
                    if (file.exists()) {
                        const bool fileAdded  = archive()->addLocalFile(vcardFileName, Utils::identitiesPath() + QString::number(uoid) + QDir::separator() + file.fileName());
                        if (!fileAdded)
                            Q_EMIT error(i18n("vCard file \"%1\" cannot be saved.",file.fileName()));
                    } else {
                        group.deleteEntry(vcard);
                    }
                }
            }
        }

        identityConfig->sync();
        const bool fileAdded  = archive()->addLocalFile(tmp.fileName(), Utils::identitiesPath() + QLatin1String("emailidentities"));
        delete identityConfig;
        if (fileAdded)
            Q_EMIT info(i18n("Identity backup done."));
        else
            Q_EMIT error(i18n("Identity file cannot be added to backup file."));
    }
}

void ExportMailJob::backupMails()
{
    showInfo(i18n("Backing up Mails..."));
    MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
    Akonadi::AgentManager *manager = Akonadi::AgentManager::self();
    const Akonadi::AgentInstance::List list = manager->instances();
    foreach( const Akonadi::AgentInstance &agent, list ) {
        const QStringList capabilities( agent.type().capabilities() );
        if (agent.type().mimeTypes().contains( KMime::Message::mimeType())) {
            if ( capabilities.contains( QLatin1String("Resource") ) &&
                 !capabilities.contains( QLatin1String("Virtual") ) &&
                 !capabilities.contains( QLatin1String("MailTransport") ) )
            {
                const QString identifier = agent.identifier();
                const QString archivePath = Utils::mailsPath() + identifier + QDir::separator();
                if (identifier.contains(QLatin1String("akonadi_mbox_resource_"))) {
                    backupResourceFile(agent, Utils::mailsPath());
                } else if (identifier.contains(QLatin1String("akonadi_maildir_resource_")) ||
                          identifier.contains(QLatin1String("akonadi_mixedmaildir_resource_"))) {
                    //Store akonadi agent config
                    KUrl url = Utils::resourcePath(agent);

                    const bool fileAdded = backupFullDirectory(url, archivePath, QLatin1String("mail.zip"));
                    if (fileAdded) {
                        const QString errorStr = Utils::storeResources(archive(), identifier, archivePath);
                        if (!errorStr.isEmpty())
                            Q_EMIT error(errorStr);
                        url = Utils::akonadiAgentConfigPath(identifier);
                        if (!url.isEmpty()) {
                            const QString filename = url.fileName();
                            const bool fileAdded  = archive()->addLocalFile(url.path(), archivePath + filename);
                            if (fileAdded)
                                Q_EMIT info(i18n("\"%1\" was backuped.",filename));
                            else
                                Q_EMIT error(i18n("\"%1\" file cannot be added to backup file.",filename));
                        }
                    }
                }
            }
        }
    }

    Q_EMIT info(i18n("Mails backup done."));
}

void ExportMailJob::writeDirectory(const QString &path, const QString &relativePath, KZip *mailArchive)
{
    QDir dir(path);
    QString currentPath(path);
    currentPath = currentPath.remove(relativePath);
    mailArchive->writeDir(currentPath, QString(), QString(), 040755, mArchiveTime, mArchiveTime, mArchiveTime );

    const QFileInfoList lst= dir.entryInfoList(QDir::NoDot|QDir::NoDotDot|QDir::Dirs|QDir::AllDirs|QDir::Hidden|QDir::Files);
    const int numberItems(lst.count());
    for (int i = 0; i < numberItems;++i) {
        const QString filename(lst.at(i).fileName());
        if (lst.at(i).isDir()) {
            writeDirectory(relativePath + path + QLatin1Char('/') + filename,relativePath,mailArchive);
        } else {
            mailArchive->addLocalFile(lst.at(i).absoluteFilePath(),currentPath + QLatin1Char('/') + filename);
        }
    }
}

void ExportMailJob::backupAkonadiDb()
{
    showInfo(i18n("Backing up Akonadi Database..."));
    MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
    AkonadiDataBase akonadiDataBase;
    const QString dbDriver(akonadiDataBase.driver());

    KTemporaryFile tmp;
    tmp.open();

    QStringList params;
    QString dbDumpAppName;
    if ( dbDriver == QLatin1String("QMYSQL") ) {
        dbDumpAppName = QString::fromLatin1("mysqldump");

        params << QLatin1String("--single-transaction")
               << QLatin1String("--flush-logs")
               << QLatin1String("--triggers")
               << QLatin1String("--result-file=") + tmp.fileName()
               << akonadiDataBase.options()
               << akonadiDataBase.name();
    } else if ( dbDriver == QLatin1String("QPSQL") ) {
        dbDumpAppName = QString::fromLatin1("pg_dump");
        params << QLatin1String("--format=custom")
               << QLatin1String("--blobs")
               << QLatin1String("--file=") + tmp.fileName()
               << akonadiDataBase.options()
               << akonadiDataBase.name();
    } else {
        Q_EMIT error(i18n("Database driver \"%1\" not supported.",dbDriver));
        return;
    }
    const QString dbDumpApp = KStandardDirs::findExe( dbDumpAppName );
    if (dbDumpApp.isEmpty()) {
        Q_EMIT error(i18n("Could not find \"%1\" necessary to dump database.",dbDumpAppName));
        return;
    }
    KProcess *proc = new KProcess( this );
    proc->setProgram( dbDumpApp, params );
    const int result = proc->execute();
    delete proc;
    if ( result != 0 ) {
        kDebug()<<" Error during dump Database";
        return;
    }
    const bool fileAdded  = archive()->addLocalFile(tmp.fileName(), Utils::akonadiPath() + QLatin1String("akonadidatabase.sql"));
    if (!fileAdded)
        Q_EMIT error(i18n("Akonadi Database \"%1\" cannot be added to backup file.", QString::fromLatin1("akonadidatabase.sql")));
    else
        Q_EMIT info(i18n("Akonadi Database backup done."));
}

KUrl ExportMailJob::subdirPath( const KUrl& url) const
{
    const QString filename(url.fileName());
    QString path = url.path();
    const int parentDirEndIndex = path.lastIndexOf( filename );
    path = path.left( parentDirEndIndex );
    path.append( QLatin1Char('.') + filename + QLatin1String(".directory") );
    return KUrl(path);
}
