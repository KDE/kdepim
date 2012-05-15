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
      mt->setHost(group.readEntry(QLatin1String("host")));
      mt->setPort(group.readEntry(QLatin1String("port"),-1));
      //TODO
      //TODO load it.
      //Save new Id
    }

    //TODO modify it.
    Q_EMIT info(i18n("Transports restored."));
  } else {
    Q_EMIT error(i18n("Failed to restore transports file."));
  }
}

void RestoreData::restoreResources()
{

}

void RestoreData::restoreMails()
{

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

    //FIX before to append filters.
    //TODO fix identity.
    //TODO fix transport.

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
      }
    }
    Q_EMIT info(i18n("Identities restored."));
  } else {
    Q_EMIT error(i18n("Failed to restore identity file."));
  }
}

void RestoreData::restoreAkonadiDb()
{
  //TODO
}

void RestoreData::restoreNepomuk()
{
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
