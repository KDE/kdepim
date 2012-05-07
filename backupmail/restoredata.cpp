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

RestoreData::RestoreData(Util::BackupTypes typeSelected,const QString& filename)
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

  if(mTypeSelected & Util::Identity)
    restoreIdentity();
  if(mTypeSelected & Util::MailTransport)
    restoreTransports();
  if(mTypeSelected & Util::Mails)
    restoreMails();
  if(mTypeSelected & Util::Resources)
    restoreResources();
  if(mTypeSelected & Util::Config)
    restoreConfig();
  if(mTypeSelected & Util::AkonadiDb)
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
  const KArchiveEntry* transport = mArchiveDirectory->entry(QLatin1String("mailtransports"));
  if(transport->isFile()) {
    const KArchiveFile* fileTransport = static_cast<const KArchiveFile*>(transport);
    //fileTransport->copyTo();
  }
  //TODO
  Q_EMIT info(i18n("Transports restored."));
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
  const KArchiveEntry* transport = mArchiveDirectory->entry(QLatin1String("emailidentities"));
  //TODO
  Q_EMIT info(i18n("Identities restored."));
}

void RestoreData::restoreAkonadiDb()
{

}
