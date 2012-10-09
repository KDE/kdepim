/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

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
#include "messageviewer/kcursorsaver.h"
#include "mailcommon/mailutil.h"
#include "mailcommon/filter/filtermanager.h"
#include "mailcommon/filter/filterimporterexporter.h"

#include <Akonadi/AgentManager>
#include <Akonadi/Collection>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>

#include <Mailtransport/TransportManager>

#include <KMime/KMimeMessage>

#include <KZip>
#include <KLocale>
#include <KTemporaryFile>
#include <KStandardDirs>
#include <KProcess>

#include <QDebug>
#include <QDir>

ExportMailJob::ExportMailJob(QWidget *parent, BackupMailUtil::BackupTypes typeSelected, ArchiveStorage *archiveStorage,int numberOfStep)
  :AbstractImportExportJob(parent,archiveStorage,typeSelected,numberOfStep)
{
}

ExportMailJob::~ExportMailJob()
{
}

void ExportMailJob::start()
{
  startBackup();
}

void ExportMailJob::startBackup()
{
  createProgressDialog();

  if(mTypeSelected & BackupMailUtil::Identity) {
    backupIdentity();
    increaseProgressDialog();
    if(wasCanceled()) {
      return;
    }
  }
  if(mTypeSelected & BackupMailUtil::MailTransport) {
    backupTransports();
    increaseProgressDialog();
    if(wasCanceled()) {
      return;
    }
  }
  if(mTypeSelected & BackupMailUtil::Mails) {
    backupMails();
    increaseProgressDialog();
    if(wasCanceled()) {
      return;
    }
  }
  if(mTypeSelected & BackupMailUtil::Resources) {
    backupResources();
    increaseProgressDialog();
    if(wasCanceled()) {
      return;
    }
  }
  if(mTypeSelected & BackupMailUtil::Config) {
    backupConfig();
    increaseProgressDialog();
    if(wasCanceled()) {
      return;
    }
  }
  if(mTypeSelected & BackupMailUtil::AkonadiDb) {
    backupAkonadiDb();
    increaseProgressDialog();
    if(wasCanceled()) {
      return;
    }
  }
  if(mTypeSelected & BackupMailUtil::Nepomuk) {
    backupNepomuk();
    increaseProgressDialog();
    if(wasCanceled()) {
      return;
    }
  }
}

void ExportMailJob::backupTransports()
{
  showInfo(i18n("Backing up transports..."));
  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );

  const QString mailtransportsStr("mailtransports");
  const QString maitransportsrc = KStandardDirs::locateLocal( "config",  mailtransportsStr);
  if(!QFile(maitransportsrc).exists()) {
    Q_EMIT info(i18n("Transports backup done."));
    return;
  }
  KSharedConfigPtr mailtransportsConfig = KSharedConfig::openConfig( mailtransportsStr );

  KTemporaryFile tmp;
  tmp.open();
  KConfig *config = mailtransportsConfig->copyTo( tmp.fileName() );

  config->sync();
  const bool fileAdded  = archive()->addLocalFile(tmp.fileName(), BackupMailUtil::transportsPath() + QLatin1String("mailtransports"));
  if(fileAdded)
    Q_EMIT info(i18n("Transports backup done."));
  else
    Q_EMIT error(i18n("Transport file cannot be added to backup file."));
}

void ExportMailJob::backupResources()
{
  showInfo(i18n("Backing up resources..."));
  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );

  Akonadi::AgentManager *manager = Akonadi::AgentManager::self();
  const Akonadi::AgentInstance::List list = manager->instances();
  foreach( const Akonadi::AgentInstance &agent, list ) {
    const QStringList capabilities( agent.type().capabilities() );
    if(agent.type().mimeTypes().contains( KMime::Message::mimeType())) {
      if ( capabilities.contains( "Resource" ) &&
            !capabilities.contains( "Virtual" ) &&
            !capabilities.contains( "MailTransport" ) )
      {
        const QString identifier = agent.identifier();
        //Store just pop3/imap account. Store other config when we copy data.
        if(identifier.contains(QLatin1String("pop3")) || identifier.contains(QLatin1String("imap"))) {
          storeResources(identifier,BackupMailUtil::resourcesPath());
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
  if(!lstFilter.isEmpty()) {
    KTemporaryFile tmp;
    tmp.open();
    KUrl url(tmp.fileName());
    MailCommon::FilterImporterExporter exportFilters;
    exportFilters.exportFilters(lstFilter,url, true);
    const bool fileAdded  = archive()->addLocalFile(tmp.fileName(), BackupMailUtil::configsPath() + QLatin1String("filters"));
    if(fileAdded)
      Q_EMIT info(i18n("Filters backup done."));
    else
      Q_EMIT error(i18n("Filters cannot be exported."));
  }
  const QString labldaprcStr("kabldaprc");
  const QString labldaprc = KStandardDirs::locateLocal( "config", labldaprcStr);
  if(QFile(labldaprc).exists()) {
    backupFile(labldaprc, BackupMailUtil::configsPath(), labldaprcStr);
  }

  const QString kmailsnippetrcStr("kmailsnippetrc");
  const QString kmailsnippetrc = KStandardDirs::locateLocal( "config",  kmailsnippetrcStr);
  if(QFile(kmailsnippetrc).exists()) {
    backupFile(kmailsnippetrc, BackupMailUtil::configsPath(), kmailsnippetrcStr);
  }

  const QString archiveMailAgentConfigurationStr("akonadi_archivemail_agentrc");
  const QString archiveMailAgentconfigurationrc = KStandardDirs::locateLocal( "config", archiveMailAgentConfigurationStr );
  if(QFile(archiveMailAgentconfigurationrc).exists()) {
    KSharedConfigPtr archivemailrc = KSharedConfig::openConfig(archiveMailAgentConfigurationStr);

    KTemporaryFile tmp;
    tmp.open();

    KConfig *archiveConfig = archivemailrc->copyTo( tmp.fileName() );
    const QStringList archiveList = archiveConfig->groupList().filter( QRegExp( "ArchiveMailCollection \\d+" ) );
    const QString archiveGroupPattern = QLatin1String( "ArchiveMailCollection " );

    Q_FOREACH(const QString& str, archiveList) {
      bool found = false;
      const int collectionId = str.right(str.length()-archiveGroupPattern.length()).toInt(&found);
      if(found) {
        KConfigGroup oldGroup = archiveConfig->group(str);
        const QString realPath = MailCommon::Util::fullCollectionPath(Akonadi::Collection( collectionId ));
        if(!realPath.isEmpty()) {
          const QString collectionPath(archiveGroupPattern + realPath);
          KConfigGroup newGroup( archiveConfig, collectionPath);
          oldGroup.copyTo( &newGroup );
          newGroup.writeEntry(QLatin1String("saveCollectionId"),collectionPath);
        }
        oldGroup.deleteGroup();
      }
    }
    archiveConfig->sync();

    backupFile(tmp.fileName(), BackupMailUtil::configsPath(), archiveMailAgentConfigurationStr);
  }

  const QString templatesconfigurationrcStr("templatesconfigurationrc");
  const QString templatesconfigurationrc = KStandardDirs::locateLocal( "config",  templatesconfigurationrcStr);
  if(QFile(templatesconfigurationrc).exists()) {
    KSharedConfigPtr templaterc = KSharedConfig::openConfig(templatesconfigurationrcStr);

    KTemporaryFile tmp;
    tmp.open();

    KConfig *templateConfig = templaterc->copyTo( tmp.fileName() );
    const QString templateGroupPattern = QLatin1String( "Templates #" );
    const QStringList templateList = templateConfig->groupList().filter( QRegExp( "Templates #\\d+" ) );
    Q_FOREACH(const QString& str, templateList) {
      bool found = false;
      const int collectionId = str.right(str.length()-templateGroupPattern.length()).toInt(&found);
      if(found) {
        KConfigGroup oldGroup = templateConfig->group(str);
        const QString realPath = MailCommon::Util::fullCollectionPath(Akonadi::Collection( collectionId ));
        if(!realPath.isEmpty()) {
          KConfigGroup newGroup( templateConfig, templateGroupPattern + realPath);
          oldGroup.copyTo( &newGroup );
        }
        oldGroup.deleteGroup();
      }
    }
    templateConfig->sync();

    backupFile(tmp.fileName(), BackupMailUtil::configsPath(), templatesconfigurationrcStr);
  }

  const QString kmailStr("kmail2rc");
  const QString kmail2rc = KStandardDirs::locateLocal( "config",  kmailStr);
  if(QFile(kmail2rc).exists()) {
    KSharedConfigPtr kmailrc = KSharedConfig::openConfig(kmail2rc);

    KTemporaryFile tmp;
    tmp.open();

    KConfig *kmailConfig = kmailrc->copyTo( tmp.fileName() );
    const QString folderGroupPattern = QLatin1String( "Folder-" );
    const QStringList folderList = kmailConfig->groupList().filter( QRegExp( "Folder-\\d+" ) );
    Q_FOREACH(const QString& str, folderList) {
      bool found = false;
      const int collectionId = str.right(str.length()-folderGroupPattern.length()).toInt(&found);
      if(found) {
        KConfigGroup oldGroup = kmailConfig->group(str);
        const QString realPath = MailCommon::Util::fullCollectionPath(Akonadi::Collection( collectionId ));
        if(!realPath.isEmpty()) {
          KConfigGroup newGroup( kmailConfig, folderGroupPattern + realPath);
          oldGroup.copyTo( &newGroup );
        }
        oldGroup.deleteGroup();
      }
    }
    const QString composerStr("Composer");
    if(kmailConfig->hasGroup(composerStr)) {
      KConfigGroup composerGroup = kmailConfig->group(composerStr);
      const QString previousStr("previous-fcc");
      if(composerGroup.hasKey(previousStr)) {
        const int collectionId = composerGroup.readEntry(previousStr,-1);
        if(collectionId!=-1) {
          const QString realPath = MailCommon::Util::fullCollectionPath(Akonadi::Collection( collectionId ));
          composerGroup.writeEntry(previousStr,realPath);
        }
      }
    }

    const QString generalStr("General");
    if(kmailConfig->hasGroup(generalStr)) {
      KConfigGroup generalGroup = kmailConfig->group(generalStr);
      const QString startupFolderStr("startupFolder");
      if(generalGroup.hasKey(startupFolderStr)) {
        const int collectionId = generalGroup.readEntry(startupFolderStr,-1);
        if(collectionId!=-1) {
          const QString realPath = MailCommon::Util::fullCollectionPath(Akonadi::Collection( collectionId ));
          generalGroup.writeEntry(startupFolderStr,realPath);
        }
      }
    }

    const QString storageModelSelectedMessageStr("MessageListView::StorageModelSelectedMessages");
    if(kmailConfig->hasGroup(storageModelSelectedMessageStr)) {
      KConfigGroup storageGroup = kmailConfig->group(storageModelSelectedMessageStr);
      const QString storageModelSelectedPattern("MessageUniqueIdForStorageModel");
      const QStringList storageList = storageGroup.keyList().filter( QRegExp( "MessageUniqueIdForStorageModel\\d+" ) );
      Q_FOREACH(const QString& str, storageList) {
        bool found = false;
        const int collectionId = str.right(str.length()-storageModelSelectedPattern.length()).toInt(&found);
        const QString oldValue = storageGroup.readEntry(str);
        if(found) {
          const QString realPath = MailCommon::Util::fullCollectionPath(Akonadi::Collection( collectionId ));
          if(!realPath.isEmpty()) {
            storageGroup.writeEntry(QString::fromLatin1("%1%2").arg(storageModelSelectedPattern).arg(realPath),oldValue);
            storageGroup.deleteEntry(str);
          }
        }
      }
    }
    const QString favoriteCollectionStr("FavoriteCollections");
    if(kmailConfig->hasGroup(favoriteCollectionStr)) {
      KConfigGroup favoriteGroup = kmailConfig->group(favoriteCollectionStr);
      const QString favoriteCollectionIdsStr("FavoriteCollectionIds");
      if(favoriteGroup.hasKey(favoriteCollectionIdsStr)) {
        const QStringList value = favoriteGroup.readEntry(favoriteCollectionIdsStr,QStringList());
        QStringList newValue;
        Q_FOREACH(const QString&str,value) {
          bool found = false;
          const int collectionId = str.toInt(&found);
          if(found) {
            const QString realPath = MailCommon::Util::fullCollectionPath(Akonadi::Collection( collectionId ));
            if(!realPath.isEmpty()) {
              newValue<<realPath;
            }
          }
        }
        favoriteGroup.writeEntry(favoriteCollectionIdsStr,newValue);
      }
    }

    kmailConfig->sync();
    backupFile(tmp.fileName(), BackupMailUtil::configsPath(), kmailStr);
  }

  Q_EMIT info(i18n("Config backup done."));
}


void ExportMailJob::backupIdentity()
{
  showInfo(i18n("Backing up identity..."));
  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
  const QString emailidentitiesStr("emailidentities");
  const QString emailidentitiesrc = KStandardDirs::locateLocal( "config",  emailidentitiesStr);
  if(!QFile(emailidentitiesrc).exists()) {
    return;
  }

  KSharedConfigPtr identity = KSharedConfig::openConfig( emailidentitiesrc );

  KTemporaryFile tmp;
  tmp.open();

  KConfig *identityConfig = identity->copyTo( tmp.fileName() );
  const QStringList accountList = identityConfig->groupList().filter( QRegExp( "Identity #\\d+" ) );
  Q_FOREACH(const QString& account, accountList) {
    KConfigGroup group = identityConfig->group(account);
    const QString fcc =QLatin1String("Fcc");
    if(group.hasKey(fcc)) {
      group.writeEntry(fcc,MailCommon::Util::fullCollectionPath(Akonadi::Collection(group.readEntry(fcc).toLongLong())));
    }
    const QString draft = QLatin1String("Drafts");
    if(group.hasKey(draft)) {
      group.writeEntry(draft,MailCommon::Util::fullCollectionPath(Akonadi::Collection(group.readEntry(draft).toLongLong())));
    }
    const QString templates = QLatin1String("Templates");
    if(group.hasKey(templates)) {
      group.writeEntry(templates,MailCommon::Util::fullCollectionPath(Akonadi::Collection(group.readEntry(templates).toLongLong())));
    }
    const QString vcard = QLatin1String("VCardFile");
    if(group.hasKey(vcard)) {
      const QString vcardFileName = group.readEntry(vcard);
      if(!vcardFileName.isEmpty()) {
        const int uoid = group.readEntry(QLatin1String("uoid"),-1);
        QFile file(vcardFileName);
        const bool fileAdded  = archive()->addLocalFile(vcardFileName, BackupMailUtil::identitiesPath() + QString::number(uoid) + QDir::separator() + file.fileName());
        if(fileAdded)
          Q_EMIT error(i18n("vcard file \"%1\" cannot be saved.",file.fileName()));
      }
    }
  }

  identityConfig->sync();
  const bool fileAdded  = archive()->addLocalFile(tmp.fileName(), BackupMailUtil::identitiesPath() + QLatin1String("emailidentities"));
  if(fileAdded)
    Q_EMIT info(i18n("Identity backup done."));
  else
    Q_EMIT error(i18n("Identity file cannot be added to backup file."));
}

KUrl ExportMailJob::resourcePath(const Akonadi::AgentInstance& agent) const
{
  const QString agentFileName = agent.identifier() + QLatin1String("rc");
  const QString configFileName = KStandardDirs::locateLocal( "config", agentFileName );

  KSharedConfigPtr resourceConfig = KSharedConfig::openConfig( configFileName );
  KUrl url = BackupMailUtil::resourcePath(resourceConfig);
  return url;
}

void ExportMailJob::backupMails()
{
  showInfo(i18n("Backing up Mails..."));
  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
  Akonadi::AgentManager *manager = Akonadi::AgentManager::self();
  const Akonadi::AgentInstance::List list = manager->instances();
  foreach( const Akonadi::AgentInstance &agent, list ) {
    const QStringList capabilities( agent.type().capabilities() );
    if(agent.type().mimeTypes().contains( KMime::Message::mimeType())) {
      if ( capabilities.contains( "Resource" ) &&
            !capabilities.contains( "Virtual" ) &&
            !capabilities.contains( "MailTransport" ) )
      {
        const QString identifier = agent.identifier();        
        const QString archivePath = BackupMailUtil::mailsPath() + identifier + QDir::separator();
        if(identifier.contains(QLatin1String("akonadi_mbox_resource_"))) {          
          KUrl url = resourcePath(agent);
          if(!url.isEmpty()) {
            const QString filename = url.fileName();
            const bool fileAdded  = archive()->addLocalFile(url.path(), archivePath + filename);
            if(fileAdded) {
              storeResources(identifier, archivePath );
              Q_EMIT info(i18n("MBox \"%1\" was backuped.",filename));
            }
            else
              Q_EMIT error(i18n("MBox \"%1\" file cannot be added to backup file.",filename));
          }
        } else if(identifier.contains(QLatin1String("akonadi_maildir_resource_")) ||
                  identifier.contains(QLatin1String("akonadi_mixedmaildir_resource_"))) {
          const KUrl url = resourcePath(agent);

          if(backupMailData(url, archivePath)) {
            storeResources(identifier, archivePath );
          }
        }
      }
    }
  }

  Q_EMIT info(i18n("Mails backup done."));
}

void ExportMailJob::writeDirectory(QString path, const QString& relativePath, KZip *mailArchive)
{
  QDir dir(path);
  mailArchive->writeDir(path.remove(relativePath),"","");
  const QFileInfoList lst= dir.entryInfoList(QDir::NoDot|QDir::NoDotDot|QDir::Dirs|QDir::AllDirs|QDir::Hidden|QDir::Files);
  const int numberItems(lst.count());
  for(int i = 0; i < numberItems;++i) {
    const QString filename(lst.at(i).fileName());
    if(lst.at(i).isDir()) {
        writeDirectory(relativePath + path + QLatin1Char('/') + filename,relativePath,mailArchive);
    } else {
        mailArchive->addLocalFile(lst.at(i).absoluteFilePath(),path.remove(relativePath) + QLatin1Char('/') + filename);
    }
  }
}

bool ExportMailJob::backupMailData(const KUrl& url,const QString& archivePath)
{
  const QString filename = url.fileName();
  KTemporaryFile tmp;
  tmp.open();
  KZip *mailArchive = new KZip(tmp.fileName());
  bool result = mailArchive->open(QIODevice::WriteOnly);
  if(!result) {
    Q_EMIT error(i18n("Mail \"%1\" file cannot be added to backup file.",filename));
    delete mailArchive;
    return false;
  }

  QString relatifPath = url.path();
  const int parentDirEndIndex = relatifPath.lastIndexOf( filename );
  relatifPath = relatifPath.left( parentDirEndIndex );

  writeDirectory(url.path(),relatifPath, mailArchive);
  KUrl subDir = subdirPath(url);
  if(QFile(subDir.path()).exists()) {
    writeDirectory(subDir.path(),relatifPath,mailArchive);
  }
  mailArchive->close();

  //TODO: store as an uniq file
  mailArchive->setCompression(KZip::NoCompression);
  const bool fileAdded = archive()->addLocalFile(tmp.fileName(), archivePath + filename);
  mailArchive->setCompression(KZip::DeflateCompression);
  delete mailArchive;

  if(fileAdded) {
    Q_EMIT info(i18n("Mail \"%1\" was backuped.",filename));
    return true;
  } else {
    Q_EMIT error(i18n("Mail \"%1\" file cannot be added to backup file.",filename));
    return false;
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
  if( dbDriver == QLatin1String("QMYSQL") ) {
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
  if(dbDumpApp.isEmpty()) {
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
  const bool fileAdded  = archive()->addLocalFile(tmp.fileName(), BackupMailUtil::akonadiPath() + QLatin1String("akonadidatabase.sql"));
  if(!fileAdded)
    Q_EMIT error(i18n("Akonadi Database \"%1\" cannot be added to backup file.", QString::fromLatin1("akonadidatabase.sql")));
  else
    Q_EMIT info(i18n("Akonadi Database backup done."));
}

void ExportMailJob::backupNepomuk()
{
  showInfo(i18n("Backing up Nepomuk Database..."));
  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
  Q_EMIT info(i18n("Nepomuk Database backup done."));
}

void ExportMailJob::storeResources(const QString&identifier, const QString& path)
{
  const QString agentFileName = identifier + QLatin1String("rc");
  const QString configFileName = KStandardDirs::locateLocal( "config", agentFileName );

  KSharedConfigPtr resourceConfig = KSharedConfig::openConfig( configFileName );
  KTemporaryFile tmp;
  tmp.open();
  KConfig * config = resourceConfig->copyTo( tmp.fileName() );

  if(identifier.contains(QLatin1String("akonadi_pop3_resource"))) {
    const QString targetCollection = QLatin1String("targetCollection");
    KConfigGroup group = config->group("General");
    if(group.hasKey(targetCollection)) {
      group.writeEntry(targetCollection,MailCommon::Util::fullCollectionPath(Akonadi::Collection(group.readEntry(targetCollection).toLongLong())));
    }
  } else if(identifier.contains(QLatin1String("akonadi_imap_resource"))) {
    const QString trash = QLatin1String("TrashCollection");
    KConfigGroup group = config->group("cache");
    if(group.hasKey(trash)) {
      group.writeEntry(trash,MailCommon::Util::fullCollectionPath(Akonadi::Collection(group.readEntry(trash).toLongLong())));
    }
  }
  config->sync();
  const bool fileAdded  = archive()->addLocalFile(tmp.fileName(), path + agentFileName);
  if(!fileAdded)
    Q_EMIT error(i18n("Resource file \"%1\" cannot be added to backup file.", agentFileName));
}

void ExportMailJob::backupFile(const QString&filename, const QString& path, const QString&storedName)
{
  const bool fileAdded  = archive()->addLocalFile(filename, path + storedName);
  if(fileAdded)
    Q_EMIT info(i18n("\"%1\" backup done.",storedName));
  else
    Q_EMIT error(i18n("\"%1\" cannot be exported.",storedName));
}

KUrl ExportMailJob::subdirPath( const KUrl& url) const
{
  const QString filename(url.fileName());
  QString path = url.path();
  const int parentDirEndIndex = path.lastIndexOf( filename );
  path = path.left( parentDirEndIndex );
  path.append( '.' + filename + ".directory" );
  return KUrl(path);
}
