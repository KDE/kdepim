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

#include <QDebug>

BackupData::BackupData(Util::BackupTypes typeSelected, const QString &filename)
  :mArchive(new KZip(filename))
{
  bool good = mArchive->open(QIODevice::WriteOnly);
  mIdentityManager = new KPIMIdentities::IdentityManager( false, this, "mIdentityManager" );
  if(typeSelected & Util::Identity)
    backupIdentity();
  if(typeSelected & Util::MailTransport)
    backupTransports();
  if(typeSelected & Util::Mails)
    backupMails();
  if(typeSelected & Util::Resources)
    backupResources();
  if(typeSelected & Util::Config)
    backupConfig();
  if(typeSelected & Util::AkonadiDb)
    backupAkonadiDb();
  closeArchive();
}

BackupData::~BackupData()
{
  closeArchive();
  //TODO Verify
  delete mArchive;
  delete mIdentityManager;
}

void BackupData::backupTransports()
{
  Q_EMIT info(i18n("Backup transports..."));
  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
  const QList<MailTransport::Transport *> listTransport = MailTransport::TransportManager::self()->transports();
  Q_FOREACH( MailTransport::Transport *mt, listTransport) {
    //TODO save it
  }
  Q_EMIT info(i18n("Transports backuped."));
}

void BackupData::closeArchive()
{
  //TODO
  if(mArchive) {

  }
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
  //FIXME
  KConfig config( "/home/laurent/testrc" );
  KPIMIdentities::IdentityManager::ConstIterator end( mIdentityManager->end() );
  for ( KPIMIdentities::IdentityManager::ConstIterator it = mIdentityManager->begin(); it != end; ++it ) {
    KConfigGroup group(&config,"DD");
    (*it).writeConfig(group);
  }
  config.sync();
  Q_EMIT info(i18n("Identity backuped."));
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

#include "backupdata.moc"
