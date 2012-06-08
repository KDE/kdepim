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
#include <KTempDir>
#include <KTemporaryFile>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KMessageBox>

#include <akonadi/agenttype.h>
#include <akonadi/agentmanager.h>
#include <akonadi/agentinstancecreatejob.h>

#include <QDBusReply>
#include <QDBusInterface>
#include <QMetaMethod>

using namespace Akonadi;

RestoreData::RestoreData(QWidget *parent, BackupMailUtil::BackupTypes typeSelected,const QString& filename)
  :AbstractData(parent,filename,typeSelected), mArchiveDirectory(0)
{
  mTempDir = new KTempDir();
}

RestoreData::~RestoreData()
{
  delete mTempDir;
}

void RestoreData::startRestore()
{
  if(!openArchive(false /*readonly*/))
    return;
  mArchiveDirectory = mArchive->directory();
  searchAllFiles(mArchiveDirectory,QString());

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

void RestoreData::searchAllFiles(const KArchiveDirectory*dir,const QString&prefix)
{
  Q_FOREACH(const QString& entryName, dir->entries()) {
    const KArchiveEntry *entry = dir->entry(entryName);
    if (entry->isDirectory()) {
      const QString newPrefix = (prefix.isEmpty() ? prefix : prefix + QLatin1Char('/')) + entryName;
      searchAllFiles(static_cast<const KArchiveDirectory*>(entry), newPrefix);
    } else {
      QString fileName = prefix.isEmpty() ? entry->name() : prefix + QLatin1Char('/') + entry->name();
      mFileList<<fileName;
    }
  }
}

void RestoreData::restoreTransports()
{
  const QString path = BackupMailUtil::transportsPath()+QLatin1String("mailtransports");
  if(!mFileList.contains(path)) {
    Q_EMIT error(i18n("mailtransports file could not be found in the archive."));
    return;
  }
  Q_EMIT info(i18n("Restore transports..."));
  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
  const KArchiveEntry* transport = mArchiveDirectory->entry(path);
  if(transport->isFile()) {
    const KArchiveFile* fileTransport = static_cast<const KArchiveFile*>(transport);

    fileTransport->copyTo(mTempDir->name());
    KSharedConfig::Ptr transportConfig = KSharedConfig::openConfig(mTempDir->name() + QLatin1Char('/') +QLatin1String("mailtransports"));
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
        QMap<QString, QVariant> settings;
        if(filename.contains(QLatin1String("pop3"))) {
          KConfigGroup general = resourceConfig->group(QLatin1String("General"));
          if(general.hasKey(QLatin1String("login"))) {
            settings.insert(QLatin1String("Login"),general.readEntry("login"));
          }
          if(general.hasKey(QLatin1String("host"))) {
            settings.insert(QLatin1String("Host"),general.readEntry("host"));
          }
          if(general.hasKey(QLatin1String("port"))) {
            settings.insert(QLatin1String("Port"),general.readEntry("port",110));
          }
          if(general.hasKey(QLatin1String("authenticationMethod"))) {
            settings.insert(QLatin1String("AuthenticationMethod"),general.readEntry("authenticationMethod",7));
          }
          if(general.hasKey(QLatin1String("useSSL"))) {
            settings.insert(QLatin1String("UseSSL"),general.readEntry("useSSL",false));
          }
          if(general.hasKey(QLatin1String("useTLS"))) {
            settings.insert(QLatin1String("UseTLS"),general.readEntry("useTLS",false));
          }
          if(general.hasKey(QLatin1String("pipelining"))) {
            settings.insert(QLatin1String("Pipelining"),general.readEntry("pipelining",false));
          }
          if(general.hasKey(QLatin1String("leaveOnServer"))) {
            settings.insert(QLatin1String("LeaveOnServer"),general.readEntry("leaveOnServer",false));
          }
          if(general.hasKey(QLatin1String("leaveOnServerDays"))) {
            settings.insert(QLatin1String("LeaveOnServerDays"),general.readEntry("leaveOnServerDays",-1));
          }
          if(general.hasKey(QLatin1String("leaveOnServerCount"))) {
            settings.insert(QLatin1String("LeaveOnServerCount"),general.readEntry("leaveOnServerCount",-1));
          }
          if(general.hasKey(QLatin1String("leaveOnServerSize"))) {
            settings.insert(QLatin1String("LeaveOnServerSize"),general.readEntry("leaveOnServerSize",-1));
          }
          if(general.hasKey(QLatin1String("filterOnServer"))) {
            settings.insert(QLatin1String("FilterOnServer"),general.readEntry("filterOnServer",false));
          }
          if(general.hasKey(QLatin1String("filterCheckSize"))) {
            settings.insert(QLatin1String("FilterCheckSize"),general.readEntry("filterCheckSize"));
          }
          if(general.hasKey(QLatin1String("targetCollection"))) {
            const int collection = adaptFolderId(general.readEntry("targetCollection"));
            if(collection != -1)
              settings.insert(QLatin1String("TargetCollection"),collection);
          }
          if(general.hasKey(QLatin1String("precommand"))) {
            settings.insert(QLatin1String("Precommand"),general.readEntry("precommand"));
          }
          if(general.hasKey(QLatin1String("intervalCheckEnabled"))) {
            settings.insert(QLatin1String("IntervalCheckEnabled"),general.readEntry("intervalCheckEnabled",false));
          }
          if(general.hasKey(QLatin1String("intervalCheckInterval"))) {
            settings.insert(QLatin1String("IntervalCheckInterval"),general.readEntry("intervalCheckInterval",5));
          }

          KConfigGroup leaveOnserver = resourceConfig->group(QLatin1String("LeaveOnServer"));

          if(leaveOnserver.hasKey(QLatin1String("seenUidList"))) {
            settings.insert(QLatin1String("SeenUidList"),leaveOnserver.readEntry("seenUidList",QStringList()));
          }
          if(leaveOnserver.hasKey(QLatin1String("seenUidTimeList"))) {
            //FIXME
            //settings.insert(QLatin1String("SeenUidTimeList"),QVariant::fromValue<QList<int> >(leaveOnserver.readEntry("seenUidTimeList",QList<int>())));
          }
          if(leaveOnserver.hasKey(QLatin1String("downloadLater"))) {
            settings.insert(QLatin1String("DownloadLater"),leaveOnserver.readEntry("downloadLater",QStringList()));
          }
          const QString newResource = createResource( QString::fromLatin1("akonadi_imap_resource"), filename, settings );
          if(!newResource.isEmpty())
            mHashResources.insert(filename,newResource);
        } else if(filename.contains(QLatin1String("imap"))) {
          KConfigGroup network = resourceConfig->group(QLatin1String("network"));
          if(network.hasKey(QLatin1String("Authentication"))) {
            settings.insert(QLatin1String("Authentication"),network.readEntry("Authentication",1));
          }
          if(network.hasKey(QLatin1String("ImapPort"))) {
            settings.insert(QLatin1String("ImapPort"),network.readEntry("ImapPort",993));
          }
          if(network.hasKey(QLatin1String("ImapServer"))) {
            settings.insert(QLatin1String("ImapServer"),network.readEntry("ImapServer"));
          }
          if(network.hasKey(QLatin1String("Safety"))) {
            settings.insert(QLatin1String("Safety"),network.readEntry("Safety","SSL"));
          }
          if(network.hasKey(QLatin1String("SubscriptionEnabled"))) {
            settings.insert(QLatin1String("SubscriptionEnabled"),network.readEntry("SubscriptionEnabled",false));
          }
          if(network.hasKey(QLatin1String("UserName"))) {
            settings.insert(QLatin1String("UserName"),network.readEntry("UserName"));
          }

          if(network.hasKey(QLatin1String("SessionTimeout"))) {
            settings.insert(QLatin1String("SessionTimeout"),network.readEntry("SessionTimeout",30));
          }

          KConfigGroup cache = resourceConfig->group(QLatin1String("cache"));

          if(cache.hasKey(QLatin1String("AccountIdentity"))) {
            const int identity = cache.readEntry("AccountIdentity",-1);
            if(identity!=-1) {
              if(mHashIdentity.contains(identity)) {
                settings.insert(QLatin1String("AccountIdentity"),mHashIdentity.value(identity));
              } else {
                settings.insert(QLatin1String("AccountIdentity"),identity);
              }
            }
          }
          if(cache.hasKey(QLatin1String("IntervalCheckEnabled"))) {
            settings.insert(QLatin1String("IntervalCheckEnabled"),cache.readEntry("IntervalCheckEnabled",true));
          }
          if(cache.hasKey(QLatin1String("RetrieveMetadataOnFolderListing"))) {
            settings.insert(QLatin1String("RetrieveMetadataOnFolderListing"),cache.readEntry("RetrieveMetadataOnFolderListing",true));
          }
          if(cache.hasKey(QLatin1String("AutomaticExpungeEnabled"))) {
            settings.insert(QLatin1String("AutomaticExpungeEnabled"),cache.readEntry("AutomaticExpungeEnabled",true));
          }
          if(cache.hasKey(QLatin1String(""))) {
            settings.insert(QLatin1String(""),cache.readEntry(""));
          }
          if(cache.hasKey(QLatin1String("DisconnectedModeEnabled"))) {
            settings.insert(QLatin1String("DisconnectedModeEnabled"),cache.readEntry("DisconnectedModeEnabled",false));
          }
          if(cache.hasKey(QLatin1String("IntervalCheckTime"))) {
            settings.insert(QLatin1String("IntervalCheckTime"),cache.readEntry("IntervalCheckTime",-1));
          }
          if(cache.hasKey(QLatin1String("UseDefaultIdentity"))) {
            settings.insert(QLatin1String("UseDefaultIdentity"),cache.readEntry("UseDefaultIdentity",true));
          }
          if(cache.hasKey(QLatin1String("TrashCollection"))) {
            const int collection = adaptFolderId(cache.readEntry("TrashCollection"));
            if(collection != -1) {
              settings.insert(QLatin1String("TrashCollection"),collection);
            }
          }


          KConfigGroup siever = resourceConfig->group(QLatin1String("siever"));
          if(siever.hasKey(QLatin1String("SieveSupport"))) {
            settings.insert(QLatin1String("SieveSupport"),siever.readEntry("SieveSupport",false));
          }
          if(siever.hasKey(QLatin1String("SieveReuseConfig"))) {
            settings.insert(QLatin1String("SieveReuseConfig"),siever.readEntry("SieveReuseConfig",true));
          }
          if(siever.hasKey(QLatin1String("SievePort"))) {
            settings.insert(QLatin1String("SievePort"),siever.readEntry("SievePort",4190));
          }
          if(siever.hasKey(QLatin1String("SieveAlternateUrl"))) {
            settings.insert(QLatin1String("SieveAlternateUrl"),siever.readEntry("SieveAlternateUrl"));
          }
          if(siever.hasKey(QLatin1String("SieveVacationFilename"))) {
            settings.insert(QLatin1String("SieveVacationFilename"),siever.readEntry("SieveVacationFilename"));
          }


          const QString newResource = createResource( QString::fromLatin1("akonadi_imap_resource"), filename, settings );
          if(!newResource.isEmpty())
            mHashResources.insert(filename,newResource);
        } else {
          qDebug()<<" problem with resource";
        }
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
           filename.contains(QLatin1String("akonadi_mbox_resource_")) ||
           filename.contains(QLatin1String("akonadi_mixedmaildir_resource_"))) {
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
  const QString filtersPath(BackupMailUtil::configsPath() + QLatin1String("filters"));
  if(!mFileList.contains(filtersPath)) {
    Q_EMIT error(i18n("filters file could not be found in the archive."));
    return;
  }
  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
  const KArchiveEntry* filter = mArchiveDirectory->entry(filtersPath);
  if(filter->isFile()) {
    const KArchiveFile* fileFilter = static_cast<const KArchiveFile*>(filter);

    fileFilter->copyTo(mTempDir->name());
    const QString filterFileName(mTempDir->name() + QLatin1Char('/') +QLatin1String("filters"));
    KSharedConfig::Ptr filtersConfig = KSharedConfig::openConfig(filterFileName);
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
    QList<MailCommon::MailFilter*> lstFilter = exportFilters.importFilters(canceled, MailCommon::FilterImporterExporter::KMailFilter, filterFileName);
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
  const QString path(BackupMailUtil::identitiesPath() +QLatin1String("emailidentities"));
  if(!mFileList.contains(path)) {
    Q_EMIT error(i18n("emailidentities file could not be found in the archive."));
    return;
  }
  Q_EMIT info(i18n("Restore identities..."));
  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
  const KArchiveEntry* identity = mArchiveDirectory->entry(path);
  if(identity->isFile()) {

    const KArchiveFile* fileIdentity = static_cast<const KArchiveFile*>(identity);
    fileIdentity->copyTo(mTempDir->name());

    KSharedConfig::Ptr identityConfig = KSharedConfig::openConfig(mTempDir->name() + QLatin1Char('/') +QLatin1String("emailidentities"));
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


//code from accountwizard
static QVariant::Type argumentType( const QMetaObject *mo, const QString &method )
{
  QMetaMethod m;
  const int numberOfMethod( mo->methodCount() );
  for ( int i = 0; i < numberOfMethod; ++i ) {
    const QString signature = QString::fromLatin1( mo->method( i ).signature() );
    if ( signature.contains(method + QLatin1Char('(') )) {
      m = mo->method( i );
      break;
    }
  }

  if ( !m.signature() ) {
    kWarning() << "Did not find D-Bus method: " << method << " available methods are:";
    const int numberOfMethod(mo->methodCount());
    for ( int i = 0; i < numberOfMethod; ++ i )
      kWarning() << mo->method( i ).signature();
    return QVariant::Invalid;
  }

  const QList<QByteArray> argTypes = m.parameterTypes();
  if ( argTypes.count() != 1 )
    return QVariant::Invalid;

  return QVariant::nameToType( argTypes.first() );
}


QString RestoreData::createResource( const QString& resources, const QString& name, const QMap<QString, QVariant>& settings )
{
  const AgentType type = AgentManager::self()->type( resources );
  if ( !type.isValid() ) {
    Q_EMIT error( i18n( "Resource type '%1' is not available.", resources ) );
    return QString();
  }

  // check if unique instance already exists
  kDebug() << type.capabilities();
  if ( type.capabilities().contains( QLatin1String( "Unique" ) ) ) {
    Q_FOREACH ( const AgentInstance &instance, AgentManager::self()->instances() ) {
      kDebug() << instance.type().identifier() << (instance.type() == type);
      if ( instance.type() == type ) {
        Q_EMIT info(i18n( "Resource '%1' is already set up.", type.name() ) );
        return QString();
      }
    }
  }

  Q_EMIT info( i18n( "Creating resource instance for '%1'...", type.name() ) );
  AgentInstanceCreateJob *job = new AgentInstanceCreateJob( type, this );
  if(job->exec()) {
    Akonadi::AgentInstance instance = job->instance();

    if ( !settings.isEmpty() ) {
      Q_EMIT info( i18n( "Configuring resource instance..." ) );
      QDBusInterface iface( "org.freedesktop.Akonadi.Resource." + instance.identifier(), "/Settings" );
      if ( !iface.isValid() ) {
        Q_EMIT error( i18n( "Unable to configure resource instance." ) );
        return QString();
      }

      // configure resource
      if ( !name.isEmpty() )
        instance.setName( name );
      QMap<QString, QVariant>::const_iterator end( settings.constEnd());
      for ( QMap<QString, QVariant>::const_iterator it = settings.constBegin(); it != end; ++it ) {
        kDebug() << "Setting up " << it.key() << " for agent " << instance.identifier();
        const QString methodName = QString::fromLatin1("set%1").arg( it.key() );
        QVariant arg = it.value();
        const QVariant::Type targetType = argumentType( iface.metaObject(), methodName );
        if ( !arg.canConvert( targetType ) ) {
          Q_EMIT error( i18n( "Could not convert value of setting '%1' to required type %2.", it.key(), QVariant::typeToName( targetType ) ) );
          return QString();
        }
        arg.convert( targetType );
        QDBusReply<void> reply = iface.call( methodName, arg );
        if ( !reply.isValid() ) {
          Q_EMIT error( i18n( "Could not set setting '%1': %2", it.key(), reply.error().message() ) );
          return QString();
        }
      }
      instance.reconfigure();
    }

    Q_EMIT info( i18n( "Resource setup completed." ) );
    return instance.identifier();
  } else {
    if ( job->error() ) {
      Q_EMIT error( i18n( "Failed to create resource instance: %1", job->errorText() ) );
    }
  }
  return QString();
}
