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

#include "importmailjob.h"
#include "akonadidatabase.h"

#include "mailcommon/filter/filtermanager.h"
#include "mailcommon/filter/filterimporterexporter.h"
#include "mailcommon/mailutil.h"
#include "mailcommon/createresource.h"

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
#include <QDir>

using namespace Akonadi;

ImportMailJob::ImportMailJob(QWidget *parent, BackupMailUtil::BackupTypes typeSelected,const QString& filename, int numberOfStep)
  :AbstractImportExportJob(parent,filename,typeSelected,numberOfStep), mArchiveDirectory(0)
{
  mTempDir = new KTempDir();
  mTempDirName = mTempDir->name();
  mCreateResource = new MailCommon::CreateResource();
  connect(mCreateResource,SIGNAL(createResourceInfo(QString)),SIGNAL(info(QString)));
  connect(mCreateResource,SIGNAL(createResourceError(QString)),SIGNAL(error(QString)));
}

ImportMailJob::~ImportMailJob()
{
  delete mTempDir;
  delete mCreateResource;
}

void ImportMailJob::start()
{
  startRestore();
}

void ImportMailJob::startRestore()
{
  if(!openArchive(false /*readonly*/))
    return;
  mArchiveDirectory = mArchive->directory();
  searchAllFiles(mArchiveDirectory,QString());
  if(!mFileList.isEmpty()|| !mHashMailArchive.isEmpty()) {
    if(mTypeSelected & BackupMailUtil::MailTransport)
      restoreTransports();
    if(mTypeSelected & BackupMailUtil::Resources)
      restoreResources();
    if(mTypeSelected & BackupMailUtil::Mails)
      restoreMails();
    if(mTypeSelected & BackupMailUtil::Identity)
      restoreIdentity();
    if(mTypeSelected & BackupMailUtil::Config)
      restoreConfig();
    if(mTypeSelected & BackupMailUtil::AkonadiDb)
      restoreAkonadiDb();
    if(mTypeSelected & BackupMailUtil::Nepomuk)
      restoreNepomuk();
  }
  closeArchive();
}

void ImportMailJob::searchAllFiles(const KArchiveDirectory*dir,const QString&prefix)
{
  Q_FOREACH(const QString& entryName, dir->entries()) {
    const KArchiveEntry *entry = dir->entry(entryName);
    if (entry && entry->isDirectory()) {
      const QString newPrefix = (prefix.isEmpty() ? prefix : prefix + QLatin1Char('/')) + entryName;
      if(entryName == QLatin1String("mails")) {
        storeMailArchiveResource(static_cast<const KArchiveDirectory*>(entry));
      } else {
        searchAllFiles(static_cast<const KArchiveDirectory*>(entry), newPrefix);
      }
    } else {
      QString fileName = prefix.isEmpty() ? entry->name() : prefix + QLatin1Char('/') + entry->name();
      mFileList<<fileName;
    }
  }
}

void ImportMailJob::storeMailArchiveResource(const KArchiveDirectory*dir)
{
  Q_FOREACH(const QString& entryName, dir->entries()) {
    const KArchiveEntry *entry = dir->entry(entryName);
    if (entry && entry->isDirectory()) {
      const KArchiveDirectory*resourceDir = static_cast<const KArchiveDirectory*>(entry);
      const QStringList lst = resourceDir->entries();
      if(lst.count() == 2) {
        const QString name(lst.at(0));
        if(name.endsWith(QLatin1String("rc"))&&
           (name.contains(QLatin1String("akonadi_mbox_resource_")) ||
            name.contains(QLatin1String("akonadi_mixedmaildir_resource_")) ||
            name.contains(QLatin1String("akonadi_maildir_resource_")))) {
          mHashMailArchive.insert(name,lst.at(1));
        } else {
          mHashMailArchive.insert(lst.at(1),name);
        }
      } else {
        kDebug()<<" lst.at(0)"<<lst.at(0);
        kDebug()<<" Problem in archive. number of file "<<lst.count();
      }
    }
  }
}

void ImportMailJob::restoreTransports()
{
  const QString path = BackupMailUtil::transportsPath()+QLatin1String("mailtransports");
  if(!mFileList.contains(path)) {
    Q_EMIT error(i18n("mailtransports file could not be found in the archive."));
    return;
  }
  Q_EMIT info(i18n("Restore transports..."));
  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
  const KArchiveEntry* transport = mArchiveDirectory->entry(path);
  if(transport && transport->isFile()) {
    const KArchiveFile* fileTransport = static_cast<const KArchiveFile*>(transport);

    fileTransport->copyTo(mTempDirName);
    KSharedConfig::Ptr transportConfig = KSharedConfig::openConfig(mTempDirName + QLatin1Char('/') +QLatin1String("mailtransports"));

    int defaultTransport = -1;
    if(transportConfig->hasGroup(QLatin1String("General"))) {
      KConfigGroup group = transportConfig->group(QLatin1String("General"));
      defaultTransport = group.readEntry(QLatin1String("default-transport"),-1);
    }

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

void ImportMailJob::restoreResources()
{
  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
  Q_FOREACH(const QString& filename, mFileList) {
    if(filename.startsWith(BackupMailUtil::resourcesPath())) {
      const KArchiveEntry* fileEntry = mArchiveDirectory->entry(filename);
      if(fileEntry && fileEntry->isFile()) {
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
            const Akonadi::Collection::Id collection = convertPathToId(general.readEntry("targetCollection"));
            if(collection != -1) {
              settings.insert(QLatin1String("TargetCollection"),collection);
            }
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
          const QString newResource = mCreateResource->createResource( QString::fromLatin1("akonadi_imap_resource"), filename, settings );
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
            const Akonadi::Collection::Id collection = convertPathToId(cache.readEntry("TrashCollection"));
            if(collection != -1) {
              settings.insert(QLatin1String("TrashCollection"),collection);
            } else {
              kDebug()<<" Use default trash folder";
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


          const QString newResource = mCreateResource->createResource( QString::fromLatin1("akonadi_pop3_resource"), filename, settings );
          if(!newResource.isEmpty())
            mHashResources.insert(filename,newResource);
        } else {
          kDebug()<<" problem with resource";
        }
      }
    }
  }
}

void ImportMailJob::restoreMails()
{
  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
  QDir dir(mTempDirName);
  dir.mkdir(BackupMailUtil::mailsPath());
  const QString copyToDirName(mTempDirName + QLatin1Char('/') + BackupMailUtil::mailsPath());
  QHashIterator<QString, QString> res(mHashMailArchive);
  while (res.hasNext()) {
    res.next();
    const QString resourceFile = res.key();
    const KArchiveEntry* fileResouceEntry = mArchiveDirectory->entry(resourceFile);
    if(fileResouceEntry && fileResouceEntry->isFile()) {
      const KArchiveFile* file = static_cast<const KArchiveFile*>(fileResouceEntry);
      file->copyTo(copyToDirName);
      const QString resourceName(file->name());
      KSharedConfig::Ptr resourceConfig = KSharedConfig::openConfig(copyToDirName + QLatin1Char('/') + resourceName);
      const KUrl url = BackupMailUtil::resourcePath(resourceConfig);
      const QString filename(file->name());

      KUrl newUrl;
      if(QFile(url.fileName()).exists()) {
        QString newFileName = url.fileName();
        for(int i = 0;; ++i) {
          newFileName = url.fileName() + QString::fromLatin1("_%1").arg(i);
          if(!QFile(newFileName).exists()) {
            break;
          }
        }
        newUrl.setFileName(newFileName);
      } else {
        newUrl= url;
      }
      QMap<QString, QVariant> settings;

      if(resourceName.contains(QLatin1String("akonadi_mbox_resource_"))) {
        const QString dataFile = res.value();
        const KArchiveEntry* dataResouceEntry = mArchiveDirectory->entry(dataFile);
        if(dataResouceEntry->isFile()) {
          const KArchiveFile* file = static_cast<const KArchiveFile*>(dataResouceEntry);
          file->copyTo(newUrl.path());
        }
        settings.insert(QLatin1String("Path"),newUrl.path());


        KConfigGroup general = resourceConfig->group(QLatin1String("General"));
        if(general.hasKey(QLatin1String("DisplayName"))) {
          settings.insert(QLatin1String("DisplayName"),general.readEntry(QLatin1String("DisplayName")));
        }
        if(general.hasKey(QLatin1String("ReadOnly"))) {
          settings.insert(QLatin1String("ReadOnly"),general.readEntry(QLatin1String("ReadOnly"),false));
        }
        if(general.hasKey(QLatin1String("MonitorFile"))) {
          settings.insert(QLatin1String("MonitorFile"),general.readEntry(QLatin1String("MonitorFile"),false));
        }
        if(resourceConfig->hasGroup(QLatin1String("Locking"))) {
          KConfigGroup locking = resourceConfig->group(QLatin1String("Locking"));
          if(locking.hasKey(QLatin1String("Lockfile"))) {
            settings.insert(QLatin1String("Lockfile"),locking.readEntry(QLatin1String("Lockfile")));
          }
          //TODO verify
          if(locking.hasKey(QLatin1String("LockfileMethod"))) {
            settings.insert(QLatin1String("LockfileMethod"),locking.readEntry(QLatin1String("LockfileMethod"),4));
          }
        }
        if(resourceConfig->hasGroup(QLatin1String("Compacting"))) {
          KConfigGroup compacting = resourceConfig->group(QLatin1String("Compacting"));
          if(compacting.hasKey(QLatin1String("CompactFrequency"))) {
            settings.insert(QLatin1String("CompactFrequency"),compacting.readEntry(QLatin1String("CompactFrequency"),1));
          }
          if(compacting.hasKey(QLatin1String("MessageCount"))) {
            settings.insert(QLatin1String("MessageCount"),compacting.readEntry(QLatin1String("MessageCount"),50));
          }
        }
        const QString newResource = mCreateResource->createResource( QString::fromLatin1("akonadi_mbox_resource"), filename, settings );
        if(!newResource.isEmpty())
          mHashResources.insert(filename,newResource);
      } else if(resourceName.contains(QLatin1String("akonadi_maildir_resource_")) ||
                resourceName.contains(QLatin1String("akonadi_mixedmaildir_resource_"))) {
        settings.insert(QLatin1String("Path"),newUrl.path());
        KConfigGroup general = resourceConfig->group(QLatin1String("General"));
        if(general.hasKey(QLatin1String("TopLevelIsContainer"))) {
          settings.insert(QLatin1String("TopLevelIsContainer"),general.readEntry(QLatin1String("TopLevelIsContainer"),false));
        }
        if(general.hasKey(QLatin1String("ReadOnly"))) {
          settings.insert(QLatin1String("ReadOnly"),general.readEntry(QLatin1String("ReadOnly"),false));
        }
        if(general.hasKey(QLatin1String("MonitorFilesystem"))) {
          settings.insert(QLatin1String("MonitorFilesystem"),general.readEntry(QLatin1String("MonitorFilesystem"),true));
        }

        const QString newResource = mCreateResource->createResource( resourceName.contains(QLatin1String("akonadi_mixedmaildir_resource_")) ?
                                                                       QString::fromLatin1("akonadi_mixedmaildir_resource")
                                                                     : QString::fromLatin1("akonadi_maildir_resource")
                                                                       , filename, settings );
        if(!newResource.isEmpty())
          mHashResources.insert(filename,newResource);
      } else {
        kDebug()<<" resource name not supported "<<resourceName;
      }
      //qDebug()<<"url "<<url;
    }
  }
}

void ImportMailJob::copyToFile(const KArchiveFile * archivefile, const QString& dest, const QString&filename, const QString&prefix)
{
  QDir dir(mTempDirName);
  dir.mkdir(prefix);

  const QString copyToDirName(mTempDirName + QLatin1Char('/') + prefix);
  archivefile->copyTo(copyToDirName);
  QFile file;
  file.setFileName(copyToDirName + QLatin1Char('/') + filename);

  if(!file.copy(dest)) {
    KMessageBox::error(mParent,i18n("File \"%1\" can not be copied to \"%2\".",filename,dest),i18n("Copy file"));
  }
}

void ImportMailJob::restoreConfig()
{
  const QString filtersPath(BackupMailUtil::configsPath() + QLatin1String("filters"));
  if(!mFileList.contains(filtersPath)) {
    Q_EMIT error(i18n("filters file could not be found in the archive."));
    return;
  }
  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
  const KArchiveEntry* filter = mArchiveDirectory->entry(filtersPath);
  if(filter && filter->isFile()) {
    const KArchiveFile* fileFilter = static_cast<const KArchiveFile*>(filter);

    fileFilter->copyTo(mTempDirName);
    const QString filterFileName(mTempDirName + QLatin1Char('/') +QLatin1String("filters"));
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
  if(kmailsnippetentry && kmailsnippetentry->isFile()) {
    const KArchiveFile* kmailsnippet = static_cast<const KArchiveFile*>(kmailsnippetentry);
    const QString kmailsnippetrc = KStandardDirs::locateLocal( "config",  kmailsnippetrcStr);
    if(QFile(kmailsnippetrc).exists()) {
      //TODO 4.10 allow to merge config.
      if(KMessageBox::warningYesNo(mParent,i18n("\"%1\" already exists. Do you want to overwrite it ?",kmailsnippetrcStr),i18n("Restore"))== KMessageBox::Yes) {
        copyToFile(kmailsnippet, kmailsnippetrc,kmailsnippetrcStr,BackupMailUtil::configsPath());
      }
    } else {
      copyToFile(kmailsnippet, kmailsnippetrc,kmailsnippetrcStr,BackupMailUtil::configsPath());
    }
  }

  const QString labldaprcStr("kabldaprc");
  const KArchiveEntry* kabldapentry  = mArchiveDirectory->entry(BackupMailUtil::configsPath() + labldaprcStr);
  if(kabldapentry && kabldapentry->isFile()) {
    const KArchiveFile* kabldap= static_cast<const KArchiveFile*>(kabldapentry);
    const QString kabldaprc = KStandardDirs::locateLocal( "config",  labldaprcStr);
    if(QFile(kabldaprc).exists()) {
      //TODO 4.10 allow to merge config.
      if(KMessageBox::warningYesNo(mParent,i18n("\"%1\" already exists. Do you want to overwrite it ?",labldaprcStr),i18n("Restore"))== KMessageBox::Yes) {
        copyToFile(kabldap, kabldaprc, labldaprcStr,BackupMailUtil::configsPath());
      }
    } else {
      copyToFile(kabldap, kabldaprc, labldaprcStr,BackupMailUtil::configsPath());
    }
  }

  const QString templatesconfigurationrcStr("templatesconfigurationrc");
  const KArchiveEntry* templatesconfigurationentry  = mArchiveDirectory->entry(BackupMailUtil::configsPath() + templatesconfigurationrcStr);
  if( templatesconfigurationentry &&  templatesconfigurationentry->isFile()) {
    const KArchiveFile* templatesconfiguration = static_cast<const KArchiveFile*>(templatesconfigurationentry);
    const QString templatesconfigurationrc = KStandardDirs::locateLocal( "config",  templatesconfigurationrcStr);
    if(QFile(templatesconfigurationrc).exists()) {
      //TODO 4.10 allow to merge config.
      if(KMessageBox::warningYesNo(mParent,i18n("\"%1\" already exists. Do you want to overwrite it ?",templatesconfigurationrcStr),i18n("Restore"))== KMessageBox::Yes) {
        importTemplatesConfig(templatesconfiguration, templatesconfigurationrc, templatesconfigurationrcStr, BackupMailUtil::configsPath());
      }
    } else {
      importTemplatesConfig(templatesconfiguration, templatesconfigurationrc, templatesconfigurationrcStr, BackupMailUtil::configsPath());
    }
  }



  const QString kmailStr("kmail2rc");
  const KArchiveEntry* kmail2rcentry  = mArchiveDirectory->entry(BackupMailUtil::configsPath() + kmailStr);
  if(kmail2rcentry && kmail2rcentry->isFile()) {
    const KArchiveFile* kmailrc = static_cast<const KArchiveFile*>(kmail2rcentry);
    const QString kmail2rc = KStandardDirs::locateLocal( "config",  kmailStr);
    if(QFile(kmail2rc).exists()) {
      //TODO 4.10 allow to merge config.
      if(KMessageBox::warningYesNo(mParent,i18n("\"%1\" already exists. Do you want to overwrite it ?",kmailStr),i18n("Restore"))== KMessageBox::Yes) {
        importKmailConfig(kmailrc,kmail2rc,kmailStr,BackupMailUtil::configsPath());
      }
    } else {
      importKmailConfig(kmailrc,kmail2rc,kmailStr,BackupMailUtil::configsPath());
    }
  }

  Q_EMIT info(i18n("Config restored."));
}

void ImportMailJob::restoreIdentity()
{
  const QString path(BackupMailUtil::identitiesPath() +QLatin1String("emailidentities"));
  if(!mFileList.contains(path)) {
    Q_EMIT error(i18n("emailidentities file could not be found in the archive."));
    return;
  }
  Q_EMIT info(i18n("Restore identities..."));
  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
  const KArchiveEntry* identity = mArchiveDirectory->entry(path);
  if(identity && identity->isFile()) {
    const KArchiveFile* fileIdentity = static_cast<const KArchiveFile*>(identity);
    fileIdentity->copyTo(mTempDirName);
    KSharedConfig::Ptr identityConfig = KSharedConfig::openConfig(mTempDirName + QLatin1Char('/') +QLatin1String("emailidentities"));
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
        const Akonadi::Collection::Id fccId = convertPathToId(group.readEntry(fcc));
        if(fccId != -1 )
          group.writeEntry(fcc,fccId);
        else
          group.deleteEntry(fcc);
      }
      const QString draft = QLatin1String("Drafts");
      if(group.hasKey(draft)) {
        const Akonadi::Collection::Id draftId = convertPathToId(group.readEntry(draft));
        if(draftId != -1)
          group.writeEntry(draft,draftId);
        else
          group.deleteEntry(draft);
      }
      const QString templates = QLatin1String("Templates");
      if(group.hasKey(templates)) {
        const Akonadi::Collection::Id templateId = convertPathToId(group.readEntry(templates));
        if(templateId != -1)
          group.writeEntry(templates,templateId);
        else
          group.deleteEntry(templates);
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
      mIdentityManager->commit();
    }
    Q_EMIT info(i18n("Identities restored."));
  } else {
    Q_EMIT error(i18n("Failed to restore identity file."));
  }
}

void ImportMailJob::restoreAkonadiDb()
{
  const QString akonadiDbPath(BackupMailUtil::akonadiPath() + QLatin1String("akonadidatabase.sql"));
  if(!mFileList.contains(akonadiDbPath)) {
    Q_EMIT error(i18n("akonadi database file could not be found in the archive."));
    return;
  }

  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );

  const KArchiveEntry* akonadiDataBaseEntry = mArchiveDirectory->entry(akonadiDbPath);
  if(akonadiDataBaseEntry && akonadiDataBaseEntry->isFile()) {

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

void ImportMailJob::restoreNepomuk()
{
  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
  Q_EMIT info(i18n("Nepomuk Database restored."));
  Q_EMIT error(i18n("Failed to restore Nepomuk Database."));
  //TODO
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
    if(!path.isEmpty())
    {
      KConfigGroup oldGroup = templateConfig->group(str);
      const Akonadi::Collection::Id id = convertPathToId(path);
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

void ImportMailJob::importKmailConfig(const KArchiveFile* kmailsnippet, const QString& kmail2rc, const QString&filename,const QString& prefix)
{
  copyToFile(kmailsnippet,kmail2rc,filename,prefix);
  KSharedConfig::Ptr kmailConfig = KSharedConfig::openConfig(kmail2rc);

  //adapt folder id
  const QString folderGroupPattern = QLatin1String( "Folder-" );
  const QStringList folderList = kmailConfig->groupList().filter( folderGroupPattern );
  Q_FOREACH(const QString&str, folderList) {
    const QString path = str.right(str.length()-folderGroupPattern.length());
    if(!path.isEmpty()) {
      KConfigGroup oldGroup = kmailConfig->group(str);
      const Akonadi::Collection::Id id = convertPathToId(path);
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
        const Akonadi::Collection::Id id = convertPathToId(path);
        if(id != -1) {
          composerGroup.writeEntry(previousStr,id);
        } else {
          composerGroup.deleteEntry(previousStr);
        }
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
        const Akonadi::Collection::Id id = convertPathToId(path);
        if(id != -1) {
          generalGroup.writeEntry(startupFolderStr,id);
        } else {
          generalGroup.deleteEntry(startupFolderStr);
        }
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

  kmailConfig->sync();
}

Akonadi::Collection::Id ImportMailJob::convertPathToId(const QString& path)
{
  if(mHashConvertPathCollectionId.contains(path)) {
    return mHashConvertPathCollectionId.value(path);
  }
  const Akonadi::Collection::Id id = MailCommon::Util::convertFolderPathToCollectionId(path);
  if(id != -1) {
    mHashConvertPathCollectionId.insert(path,id);
  }
  return id;
}
