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

#include "restoredata.h"
#include "akonadidatabase.h"

#include "mailcommon/filter/filtermanager.h"
#include "mailcommon/filter/filterimporterexporter.h"
#include "mailcommon/filter/filteractionmissingargumentdialog.h"


#include "messageviewer/kcursorsaver.h"


#include <mailtransport/transportmanager.h>

#include <kpimidentities/identitymanager.h>
#include <kpimidentities/identity.h>
#include <KZip>
#include <KStandardDirs>
#include <KLocale>
#include <KProcess>
#include <KTemporaryFile>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KMessageBox>

RestoreData::RestoreData(QWidget *parent, BackupMailUtil::BackupTypes typeSelected,const QString& filename)
  :AbstractData(parent,filename,typeSelected), mArchiveDirectory(0)
{
}

RestoreData::~RestoreData()
{
}

void RestoreData::startRestore()
{
  if(!openArchive(false /*readonly*/))
    return;
  mArchiveDirectory = mArchive->directory();
  mFileList = mArchiveDirectory->entries();

  if(mTypeSelected & BackupMailUtil::MailTransport)
    restoreTransports();
  if(mTypeSelected & BackupMailUtil::Resources)
    restoreResources();
  if(mTypeSelected & BackupMailUtil::Identity)
    restoreIdentity();
  if(mTypeSelected & BackupMailUtil::Mails)
    restoreMails();
  if(mTypeSelected & BackupMailUtil::Config)
    restoreConfig();
  if(mTypeSelected & BackupMailUtil::AkonadiDb)
    restoreAkonadiDb();
  if(mTypeSelected & BackupMailUtil::Nepomuk)
    restoreNepomuk();
  closeArchive();
}

void RestoreData::restoreTransports()
{
  if(!mFileList.contains(BackupMailUtil::transportsPath()+QLatin1String("mailtransports"))) {
    Q_EMIT error(i18n("mailtransports file could not be found in the archive."));
    return;
  }
  Q_EMIT info(i18n("Restore transports..."));
  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
  const KArchiveEntry* transport = mArchiveDirectory->entry(BackupMailUtil::transportsPath()+QLatin1String("mailtransports"));
  if(transport->isFile()) {
    const KArchiveFile* fileTransport = static_cast<const KArchiveFile*>(transport);

    KTemporaryFile tmp;
    tmp.open();

    fileTransport->copyTo(tmp.fileName());
    KSharedConfig::Ptr transportConfig = KSharedConfig::openConfig(tmp.fileName());
    const QStringList transportList = transportConfig->groupList().filter( QRegExp( "Transport \\d+" ) );
    Q_FOREACH(const QString&transport, transportList) {
      KConfigGroup group = transportConfig->group(transport);
      const int transportId = group.readEntry(QLatin1String("id"), -1);
      MailTransport::Transport* mt = MailTransport::TransportManager::self()->createTransport();
      mt->setName(group.readEntry(QLatin1String("name")));
      const QString hostStr(QLatin1String("host"));
      if(group.hasKey(hostStr)) {
        mt->setHost(group.readEntry(hostStr));
      }
      const QString portStr(QLatin1String("port"));
      if(group.hasKey(portStr)) {
        mt->setPort(group.readEntry(portStr,-1));
      }
      const QString userNameStr(QLatin1String("userName"));
      if(group.hasKey(userNameStr)) {
        mt->setUserName(group.readEntry(userNameStr));
      }
      const QString precommandStr(QLatin1String("precommand"));
      if(group.hasKey(precommandStr)) {
        mt->setPrecommand(group.readEntry(precommandStr));
      }
      const QString requiresAuthenticationStr(QLatin1String("requiresAuthentication"));
      if(group.hasKey(requiresAuthenticationStr)) {
        mt->setRequiresAuthentication(group.readEntry(requiresAuthenticationStr,false));
      }
      const QString specifyHostnameStr(QLatin1String("specifyHostname"));
      if(group.hasKey(specifyHostnameStr)) {
        mt->setSpecifyHostname(group.readEntry(specifyHostnameStr,false));
      }
      const QString localHostnameStr(QLatin1String("localHostname"));
      if(group.hasKey(localHostnameStr)) {
        mt->setLocalHostname(group.readEntry(localHostnameStr));
      }
      const QString specifySenderOverwriteAddressStr(QLatin1String("specifySenderOverwriteAddress"));
      if(group.hasKey(specifySenderOverwriteAddressStr)) {
        mt->setSpecifySenderOverwriteAddress(group.readEntry(specifySenderOverwriteAddressStr,false));
      }
      const QString storePasswordStr(QLatin1String("storePassword"));
      if(group.hasKey(storePasswordStr)) {
        mt->setStorePassword(group.readEntry(storePasswordStr,false));
      }
      const QString senderOverwriteAddressStr(QLatin1String("senderOverwriteAddress"));
      if(group.hasKey(senderOverwriteAddressStr)) {
        mt->setSenderOverwriteAddress(group.readEntry(senderOverwriteAddressStr));
      }
      const QString encryptionStr(QLatin1String("encryption"));
      if(group.hasKey(encryptionStr)) {
        mt->setEncryption(group.readEntry(encryptionStr,1)); //TODO verify
      }
      const QString authenticationTypeStr(QLatin1String("authenticationType"));
      if(group.hasKey(authenticationTypeStr)) {
        mt->setAuthenticationType(group.readEntry(authenticationTypeStr,1));//TODO verify
      }

      mHashTransport.insert(transportId, mt->id());
    }
    Q_EMIT info(i18n("Transports restored."));
  } else {
    Q_EMIT error(i18n("Failed to restore transports file."));
  }
}

void RestoreData::restoreResources()
{
  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
  Q_FOREACH(const QString& filename, mFileList) {
    if(filename.startsWith(BackupMailUtil::resourcesPath())) {
      const KArchiveEntry* fileEntry = mArchiveDirectory->entry(filename);
      if(fileEntry->isFile()) {
        const KArchiveFile* file = static_cast<const KArchiveFile*>(fileEntry);
        KTemporaryFile tmp;
        tmp.open();
        file->copyTo(tmp.fileName());
        KSharedConfig::Ptr resourceConfig = KSharedConfig::openConfig(tmp.fileName());
        const QString filename(file->name());
        if(filename.contains(QLatin1String("pop3"))) {
          //TODO
        } else if(filename.contains(QLatin1String("imap"))) {
          //TODO
        } else {
          qDebug()<<" problem with resource";
        }

        //TODO restore it
      }
    }
  }
}

void RestoreData::restoreMails()
{
  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
  Q_FOREACH(const QString& filename, mFileList) {
    if(filename.startsWith(BackupMailUtil::mailsPath())) {
      const KArchiveEntry* fileEntry = mArchiveDirectory->entry(filename);
      if(fileEntry->isFile()) {
        const KArchiveFile* file = static_cast<const KArchiveFile*>(fileEntry);
        KTemporaryFile tmp;
        tmp.open();
        file->copyTo(tmp.fileName());
        const QString filename(file->name());
        if(filename.contains(QLatin1String("akonadi_maildir_resource_")) ||
           filename.contains(QLatin1String("akonadi_mbox_resource_"))) {
          //TODO resource file.
          KSharedConfig::Ptr resourceConfig = KSharedConfig::openConfig(tmp.fileName());
          KUrl url = BackupMailUtil::resourcePath(resourceConfig);
        }
        //TODO
      }
    }
  }
}

void RestoreData::restoreConfig()
{
  if(!mFileList.contains(BackupMailUtil::configsPath() + QLatin1String("filters"))) {
    Q_EMIT error(i18n("filters file could not be found in the archive."));
    return;
  }
  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
  const KArchiveEntry* filter = mArchiveDirectory->entry(BackupMailUtil::configsPath() + QLatin1String("filters"));
  if(filter->isFile()) {
    const KArchiveFile* fileFilter = static_cast<const KArchiveFile*>(filter);
    KTemporaryFile tmp;
    tmp.open();

    fileFilter->copyTo(tmp.fileName());

    KSharedConfig::Ptr filtersConfig = KSharedConfig::openConfig(tmp.fileName());
    const QStringList filterList = filtersConfig->groupList().filter( QRegExp( "Filter #\\d+" ) );
    Q_FOREACH(const QString&filterStr, filterList) {
      KConfigGroup group = filtersConfig->group(filterStr);
      const QString accountStr("accounts-set");
      if(group.hasKey(accountStr)) {
        const QString accounts = group.readEntry(accountStr);
        if(!accounts.isEmpty()) {
          const QStringList lstAccounts = accounts.split(QLatin1Char(','));
          QStringList newLstAccounts;
          Q_FOREACH(const QString&acc, lstAccounts) {
            if(mHashResources.contains(acc)) {
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
        if(actValue==QLatin1String("set identity")) {
          const int argsValue = group.readEntry(argsName,-1);
          if(argsValue!=-1) {
            if(mHashIdentity.contains(argsValue)) {
              group.writeEntry(argsName,mHashIdentity.value(argsValue));
            }
          }
        } else if(actValue==QLatin1String("set transport")) {
          const int argsValue = group.readEntry(argsName,-1);
          if(argsValue!=-1) {
            if(mHashTransport.contains(argsValue)) {
              group.writeEntry(argsName,mHashTransport.value(argsValue));
            }
          }
        }
      }
    }
    filtersConfig->sync();

    bool canceled = false;
    MailCommon::FilterImporterExporter exportFilters;
    QList<MailCommon::MailFilter*> lstFilter = exportFilters.importFilters(canceled, MailCommon::FilterImporterExporter::KMailFilter, tmp.fileName());
    if(canceled) {
      MailCommon::FilterManager::instance()->appendFilters(lstFilter);
    }
  }
  const QString kmailsnippetrcStr("kmailsnippetrc");
  const KArchiveEntry* kmailsnippetentry  = mArchiveDirectory->entry(BackupMailUtil::configsPath() + kmailsnippetrcStr);
  if(kmailsnippetentry->isFile()) {
    const KArchiveFile* kmailsnippet = static_cast<const KArchiveFile*>(kmailsnippetentry);
    const QString kmailsnippetrc = KStandardDirs::locateLocal( "config",  kmailsnippetrcStr);
    if(QFile(kmailsnippetrc).exists()) {
      //TODO 4.10 allow to merge config.
      if(KMessageBox::warningYesNo(mParent,i18n("\"%1\" already exists. Do you want to overwrite it ?",kmailsnippetrcStr),i18n("Restore"))== KMessageBox::Yes) {
        kmailsnippet->copyTo(kmailsnippetrc);
      }
    } else {
      kmailsnippet->copyTo(kmailsnippetrc);
    }
  }

  const QString labldaprcStr("kabldaprc");
  const KArchiveEntry* kabldapentry  = mArchiveDirectory->entry(BackupMailUtil::configsPath() + labldaprcStr);
  if(kabldapentry->isFile()) {
    const KArchiveFile* kabldap= static_cast<const KArchiveFile*>(kabldapentry);
    const QString kabldaprc = KStandardDirs::locateLocal( "config",  labldaprcStr);
    if(QFile(kabldaprc).exists()) {
      //TODO 4.10 allow to merge config.
      if(KMessageBox::warningYesNo(mParent,i18n("\"%1\" already exists. Do you want to overwrite it ?",labldaprcStr),i18n("Restore"))== KMessageBox::Yes) {
        kabldap->copyTo(kabldaprc);
      }
    } else {
      kabldap->copyTo(kabldaprc);
    }
  }


  const QString templatesconfigurationrcStr("templatesconfigurationrc");
  const KArchiveEntry* templatesconfigurationentry  = mArchiveDirectory->entry(BackupMailUtil::configsPath() + templatesconfigurationrcStr);
  if(templatesconfigurationentry->isFile()) {
    const KArchiveFile* templatesconfiguration = static_cast<const KArchiveFile*>(templatesconfigurationentry);
    const QString templatesconfigurationrc = KStandardDirs::locateLocal( "config",  templatesconfigurationrcStr);
    if(QFile(templatesconfigurationrc).exists()) {
      //TODO 4.10 allow to merge config.
      if(KMessageBox::warningYesNo(mParent,i18n("\"%1\" already exists. Do you want to overwrite it ?",templatesconfigurationrcStr),i18n("Restore"))== KMessageBox::Yes) {
        importTemplatesConfig(templatesconfiguration, templatesconfigurationrc);
      }
    } else {
      importTemplatesConfig(templatesconfiguration, templatesconfigurationrc);
    }
  }



  const QString kmailStr("kmail2rc");
  //TODO fix folder id.
  const KArchiveEntry* kmail2rcentry  = mArchiveDirectory->entry(BackupMailUtil::configsPath() + kmailStr);
  if(kmail2rcentry->isFile()) {
    const KArchiveFile* kmailrc = static_cast<const KArchiveFile*>(kmail2rcentry);
    const QString kmail2rc = KStandardDirs::locateLocal( "config",  kmailStr);
    if(QFile(kmail2rc).exists()) {
      //TODO 4.10 allow to merge config.
      if(KMessageBox::warningYesNo(mParent,i18n("\"%1\" already exists. Do you want to overwrite it ?",kmailStr),i18n("Restore"))== KMessageBox::Yes) {
        importKmailConfig(kmailrc,kmail2rc);
      }
    } else {
      importKmailConfig(kmailrc,kmail2rc);
    }
  }


  Q_EMIT info(i18n("Config restored."));
}

void RestoreData::restoreIdentity()
{
  if(!mFileList.contains(BackupMailUtil::identitiesPath() +QLatin1String("emailidentities"))) {
    Q_EMIT error(i18n("emailidentities file could not be found in the archive."));
    return;
  }
  Q_EMIT info(i18n("Restore identities..."));
  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
  const KArchiveEntry* identity = mArchiveDirectory->entry(BackupMailUtil::identitiesPath() + QLatin1String("emailidentities"));
  if(identity->isFile()) {

    const KArchiveFile* fileIdentity = static_cast<const KArchiveFile*>(identity);

    KTemporaryFile tmp;
    tmp.open();

    fileIdentity->copyTo(tmp.fileName());
    KSharedConfig::Ptr identityConfig = KSharedConfig::openConfig(tmp.fileName());
    KConfigGroup general = identityConfig->group(QLatin1String("General"));
    const int defaultIdentity = general.readEntry(QLatin1String("Default Identity"),-1);

    const QStringList identityList = identityConfig->groupList().filter( QRegExp( "Identity #\\d+" ) );
    Q_FOREACH(const QString&identityStr, identityList) {
      KConfigGroup group = identityConfig->group(identityStr);
      int oldUid = -1;
      const QString uidStr("uoid");
      if(group.hasKey(uidStr)) {
        oldUid = group.readEntry(uidStr).toUInt();
        group.deleteEntry(uidStr);
      }
      const QString fcc(QLatin1String("Fcc"));
      if(group.hasKey(fcc)) {
        group.writeEntry(fcc,adaptFolderId(group.readEntry(fcc)));
      }
      const QString draft = QLatin1String("Drafts");
      if(group.hasKey(draft)) {
        group.writeEntry(draft,adaptFolderId(group.readEntry(draft)));
      }
      const QString templates = QLatin1String("Templates");
      if(group.hasKey(templates)) {
        group.writeEntry(templates,adaptFolderId(group.readEntry(templates)));
      }
      group.sync();
      KPIMIdentities::Identity* identity = &mIdentityManager->newFromScratch( QString() );

      identity->readConfig(group);
      if(oldUid != -1) {
        mHashIdentity.insert(oldUid,identity->uoid());
        if(oldUid == defaultIdentity) {
          mIdentityManager->setAsDefault(identity->uoid());
        }
      }
    }
    Q_EMIT info(i18n("Identities restored."));
  } else {
    Q_EMIT error(i18n("Failed to restore identity file."));
  }
}

void RestoreData::restoreAkonadiDb()
{
  const QString akonadiDbPath(BackupMailUtil::akonadiPath() + QLatin1String("akonadidatabase.sql"));
  if(!mFileList.contains(akonadiDbPath)) {
    Q_EMIT error(i18n("akonadi database file could not be found in the archive."));
    return;
  }

  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );

  const KArchiveEntry* akonadiDataBaseEntry = mArchiveDirectory->entry(akonadiDbPath);
  if(akonadiDataBaseEntry->isFile()) {

    const KArchiveFile* akonadiDataBaseFile = static_cast<const KArchiveFile*>(akonadiDataBaseEntry);

    KTemporaryFile tmp;
    tmp.open();

    akonadiDataBaseFile->copyTo(tmp.fileName());

    /* Restore the database */
    AkonadiDataBase akonadiDataBase;

    const QString dbDriver(akonadiDataBase.driver());
    QStringList params;
    QString dbRestoreAppName;
    if( dbDriver == QLatin1String("QPSQL") ) {
      dbRestoreAppName = QLatin1String("pg_restore");
      params << akonadiDataBase.options()
             << QLatin1String("--dbname=") + akonadiDataBase.name()
             << QLatin1String("--format=custom")
             << QLatin1String("--clean")
             << QLatin1String("--no-owner")
             << QLatin1String("--no-privileges")
             << tmp.fileName();
    }
    else if (dbDriver == QLatin1String("QMYSQL") ) {
      dbRestoreAppName = QLatin1String("mysql");
      params << akonadiDataBase.options()
             << QLatin1String("--database=") + akonadiDataBase.name();
    } else {
      Q_EMIT error(i18n("Database driver \"%1\" not supported.",dbDriver));
      return;
    }
    const QString dbRestoreApp = KStandardDirs::findExe( dbRestoreAppName );
    if(dbRestoreApp.isEmpty()) {
      Q_EMIT error(i18n("Could not find \"%1\" necessary to restore database.",dbRestoreAppName));
      return;
    }
    KProcess *proc = new KProcess( this );
    proc->setProgram( KStandardDirs::findExe( dbRestoreApp ), params );
    proc->setStandardInputFile(tmp.fileName());
    const int result = proc->execute();
    delete proc;
    if ( result != 0 ) {
      Q_EMIT error(i18n("Failed to restore Akonadi Database."));
      return;
    }
  }
  Q_EMIT info(i18n("Akonadi Database restored."));
}

void RestoreData::restoreNepomuk()
{
  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
  Q_EMIT info(i18n("Nepomuk Database restored."));
  Q_EMIT error(i18n("Failed to restore Nepomuk Database."));
  //TODO
}


Akonadi::Collection::Id RestoreData::adaptFolderId( const QString& folder)
{
  Akonadi::Collection::Id newFolderId=-1;
  bool exactPath = false;
  Akonadi::Collection::List lst = FilterActionMissingCollectionDialog::potentialCorrectFolders( folder, exactPath );
  if ( lst.count() == 1 && exactPath )
    newFolderId = lst.at( 0 ).id();
  else {
    FilterActionMissingCollectionDialog *dlg = new FilterActionMissingCollectionDialog( lst, QString(), folder );
    if ( dlg->exec() ) {
      newFolderId = dlg->selectedCollection().id();
    }
    delete dlg;
  }
  return newFolderId;
}


void RestoreData::importTemplatesConfig(const KArchiveFile* templatesconfiguration, const QString& templatesconfigurationrc)
{
  templatesconfiguration->copyTo(templatesconfigurationrc);
  KSharedConfig::Ptr templateConfig = KSharedConfig::openConfig(templatesconfigurationrc);

  //adapt id
  const QString templateGroupPattern = QLatin1String( "Templates #" );
  const QStringList templateList = templateConfig->groupList().filter( templateGroupPattern );
  Q_FOREACH(const QString& str, templateList) {
    const QString path = str.right(str.length()-templateGroupPattern.length());
    if(!path.isEmpty())
    {
      KConfigGroup oldGroup = templateConfig->group(str);
      Akonadi::Collection::Id id = adaptFolderId(path);
      if(id!=-1) {
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
    if(found)
    {
      KConfigGroup oldGroup = templateConfig->group(str);
      if(mHashIdentity.contains(identity)) {
        KConfigGroup newGroup( templateConfig, templateGroupPattern + QString::number(mHashIdentity.value(identity)));
        oldGroup.copyTo( &newGroup );
      }
      oldGroup.deleteGroup();
    }

  }
  templateConfig->sync();
}

void RestoreData::importKmailConfig(const KArchiveFile* kmailsnippet, const QString& kmail2rc)
{
  kmailsnippet->copyTo(kmail2rc);
  KSharedConfig::Ptr kmailConfig = KSharedConfig::openConfig(kmail2rc);

  //adapt folder id
  const QString folderGroupPattern = QLatin1String( "Folder-" );
  const QStringList folderList = kmailConfig->groupList().filter( folderGroupPattern );
  Q_FOREACH(const QString&str, folderList) {
    const QString path = str.right(str.length()-folderGroupPattern.length());
    if(!path.isEmpty()) {
        KConfigGroup oldGroup = kmailConfig->group(str);
        Akonadi::Collection::Id id = adaptFolderId(path);
        if(id!=-1) {
          KConfigGroup newGroup( kmailConfig, folderGroupPattern + QString::number(id));
          oldGroup.copyTo( &newGroup );
        }
        oldGroup.deleteGroup();
    }
  }
  const QString accountOrder("AccountOrder");
  if(kmailConfig->hasGroup(accountOrder)) {
    KConfigGroup group = kmailConfig->group(accountOrder);
    QStringList orderList = group.readEntry(QLatin1String("order"),QStringList());
    QStringList newOrderList;
    if(!orderList.isEmpty()) {
      Q_FOREACH(const QString&account, orderList) {
        if(mHashResources.contains(account)) {
          newOrderList.append(mHashResources.value(account));
        } else {
          newOrderList.append(account);
        }
      }
    }
  }

  const QString composerStr("Composer");
  if(kmailConfig->hasGroup(composerStr)) {
    KConfigGroup composerGroup = kmailConfig->group(composerStr);
    const QString previousStr("previous-fcc");
    if(composerGroup.hasKey(previousStr)) {
      const QString path = composerGroup.readEntry(previousStr);
      if(!path.isEmpty()) {
        Akonadi::Collection::Id id = adaptFolderId(path);
        composerGroup.writeEntry(previousStr,id);
      }
    }
    const QString previousIdentityStr("previous-identity");
    if(composerGroup.hasKey(previousIdentityStr)) {
      const int identityValue = composerGroup.readEntry(previousIdentityStr, -1);
      if(identityValue!=-1) {
        if(mHashIdentity.contains(identityValue)) {
          composerGroup.writeEntry(previousIdentityStr,mHashIdentity.value(identityValue));
        } else {
          composerGroup.writeEntry(previousIdentityStr,identityValue);
        }
      }
    }
  }

  const QString generalStr("General");
  if(kmailConfig->hasGroup(generalStr)) {
    KConfigGroup generalGroup = kmailConfig->group(generalStr);
    const QString startupFolderStr("startupFolder");
    if(generalGroup.hasKey(startupFolderStr)) {
      const QString path = generalGroup.readEntry(startupFolderStr);
      if(!path.isEmpty()) {
        Akonadi::Collection::Id id = adaptFolderId(path);
        generalGroup.writeEntry(startupFolderStr,id);
      }
    }
  }

  const QString resourceGroupPattern = QLatin1String( "Resource " );
  const QStringList resourceList = kmailConfig->groupList().filter( resourceGroupPattern );
  Q_FOREACH(const QString&str, resourceList) {
    const QString res = str.right(str.length()-resourceGroupPattern.length());
    if(!res.isEmpty()) {
        KConfigGroup oldGroup = kmailConfig->group(str);
        if(mHashResources.contains(res)) {
          KConfigGroup newGroup( kmailConfig, folderGroupPattern + mHashResources.value(res));
          oldGroup.copyTo( &newGroup );
        }
        oldGroup.deleteGroup();
    }
  }

//TODO fix all other id
  kmailConfig->sync();
}
