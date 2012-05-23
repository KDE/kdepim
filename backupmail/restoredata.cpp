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

#include "mailcommon/filter/filtermanager.h"
#include "mailcommon/filter/filterimporterexporter.h"
#include "mailcommon/filter/filteractionmissingargumentdialog.h"


#include "messageviewer/kcursorsaver.h"


#include <mailtransport/transportmanager.h>

#include <kpimidentities/identitymanager.h>
#include <kpimidentities/identity.h>
#include <KZip>
#include <KLocale>
#include <KTemporaryFile>
#include <KSharedConfig>
#include <KConfigGroup>

RestoreData::RestoreData(BackupMailUtil::BackupTypes typeSelected,const QString& filename)
  :AbstractData(filename,typeSelected), mArchiveDirectory(0)
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
  Q_FOREACH(const QString& filename, mFileList) {
    if(filename.startsWith(BackupMailUtil::resourcesPath())) {
      const KArchiveEntry* fileEntry = mArchiveDirectory->entry(filename);
      if(fileEntry->isFile()) {
        const KArchiveFile* file = static_cast<const KArchiveFile*>(fileEntry);
        KTemporaryFile tmp;
        tmp.open();
        file->copyTo(tmp.fileName());
        //TODO
      }
    }
  }
}

void RestoreData::restoreMails()
{
  Q_FOREACH(const QString& filename, mFileList) {
    if(filename.startsWith(BackupMailUtil::mailsPath())) {
      const KArchiveEntry* fileEntry = mArchiveDirectory->entry(filename);
      if(fileEntry->isFile()) {
        const KArchiveFile* file = static_cast<const KArchiveFile*>(fileEntry);
        KTemporaryFile tmp;
        tmp.open();
        file->copyTo(tmp.fileName());
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
      uint oldUid = -1;
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
  Q_EMIT info(i18n("Akonadi Database restored."));
  Q_EMIT error(i18n("Failed to restore Akonadi Database."));
  //TODO
}

void RestoreData::restoreNepomuk()
{
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
