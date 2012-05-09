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

#include "messageviewer/kcursorsaver.h"

#include <KZip>
#include <KLocale>
#include <KTemporaryFile>
#include <KSharedConfig>

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

  if(mTypeSelected & BackupMailUtil::Identity)
    restoreIdentity();
  if(mTypeSelected & BackupMailUtil::MailTransport)
    restoreTransports();
  if(mTypeSelected & BackupMailUtil::Mails)
    restoreMails();
  if(mTypeSelected & BackupMailUtil::Resources)
    restoreResources();
  if(mTypeSelected & BackupMailUtil::Config)
    restoreConfig();
  if(mTypeSelected & BackupMailUtil::AkonadiDb)
    restoreAkonadiDb();
  closeArchive();
}

void RestoreData::restoreTransports()
{
  if(!mFileList.contains(QLatin1String("mailtransports"))) {
    Q_EMIT error(i18n("mailtransports file not found in archive."));
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
    KSharedConfig::Ptr identityConfig = KSharedConfig::openConfig(tmp.fileName());
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

}

void RestoreData::restoreIdentity()
{
  if(!mFileList.contains(QLatin1String("emailidentities"))) {
    Q_EMIT error(i18n("emailidentitied file not found in archive."));
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

    //TODO
    Q_EMIT info(i18n("Identities restored."));
  } else {
    Q_EMIT error(i18n("Failed to restore identity file."));
  }
}

void RestoreData::restoreAkonadiDb()
{

}
