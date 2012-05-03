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

#include "backupdata.h"
#include "messageviewer/kcursorsaver.h"

#include <kpimidentities/identity.h>
#include <kpimidentities/identitymanager.h>

#include <Mailtransport/TransportManager>

#include <KZip>
#include <KLocale>
#include <KTemporaryFile>

#include <QDebug>

BackupData::BackupData(Util::BackupTypes typeSelected, const QString &filename)
  :AbstractData(filename,typeSelected)
{
}

BackupData::~BackupData()
{
}

void BackupData::startBackup()
{
  bool good = mArchive->open(QIODevice::WriteOnly);
  if(!good) {
    //TODO
  }

  if(mTypeSelected & Util::Identity)
    backupIdentity();
  if(mTypeSelected & Util::MailTransport)
    backupTransports();
  if(mTypeSelected & Util::Mails)
    backupMails();
  if(mTypeSelected & Util::Resources)
    backupResources();
  if(mTypeSelected & Util::Config)
    backupConfig();
  if(mTypeSelected & Util::AkonadiDb)
    backupAkonadiDb();
  closeArchive();
}

void BackupData::backupTransports()
{
  Q_EMIT info(i18n("Backup transports..."));
  KSharedConfigPtr mailtransportsConfig = KSharedConfig::openConfig( QLatin1String( "mailtransports" ) );

  KTemporaryFile tmp;
  tmp.open();
  KSharedConfig::Ptr transportConfig = KSharedConfig::openConfig(tmp.fileName());

  mailtransportsConfig->copyTo( tmp.fileName(), transportConfig.data() );

  transportConfig->sync();
  const bool fileAdded  = mArchive->addLocalFile(tmp.fileName(), QLatin1String("transportrc"));
  if(fileAdded)
    Q_EMIT info(i18n("Transports backuped."));
  else
    Q_EMIT error(i18n("Transport file cannot be added to backup file."));
}

void BackupData::backupResources()
{
  Q_EMIT info(i18n("Backup resources..."));
  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
  Q_EMIT info(i18n("Resources backuped."));
}

void BackupData::backupConfig()
{
  Q_EMIT info(i18n("Backup config..."));
  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
  Q_EMIT info(i18n("Config backuped."));
}

void BackupData::backupIdentity()
{
  Q_EMIT info(i18n("Backup identity..."));
  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
  KTemporaryFile tmp;
  tmp.open();
  KConfig config( tmp.fileName() );
  int i = 0;
  KPIMIdentities::IdentityManager::ConstIterator end( mIdentityManager->end() );
  for ( KPIMIdentities::IdentityManager::ConstIterator it = mIdentityManager->begin(); it != end; ++it ) {
    KConfigGroup group(&config,QString::fromLatin1("Identity %1").arg(QString::number(i)));
    (*it).writeConfig(group);
    i++;
  }
  config.sync();
  const bool fileAdded  = mArchive->addLocalFile(tmp.fileName(), QLatin1String("identityrc"));
  if(fileAdded)
    Q_EMIT info(i18n("Identity backuped."));
  else
    Q_EMIT error(i18n("Identity file cannot be added to backup file."));
}

void BackupData::backupMails()
{
  Q_EMIT info(i18n("Backup Mails..."));
  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
  Q_EMIT info(i18n("Mails backuped."));

}

void BackupData::backupAkonadiDb()
{
  Q_EMIT info(i18n("Backup Akonadi Database..."));
  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
  Q_EMIT info(i18n("Akonadi Database backuped."));

}

qint64 BackupData::writeFile(const char* data, qint64 len)
{
  if (len == 0)
    return 0;

  if (!mArchive->isOpen()) {
    qDebug()<<" Open Archive before to write";
    return 0;
  }
  if (mArchive->writeData(data, len)) {
    return len;
  }
  return 0;
}
