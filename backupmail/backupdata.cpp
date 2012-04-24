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

#include <Mailtransport/TransportManager>

#include <KZip>

#include <QDebug>

BackupData::BackupData()
  :mArchive(new KZip("backup"))
{
  bool good = mArchive->open(QIODevice::WriteOnly);

}

BackupData::~BackupData()
{
  //TODO Verify
  delete mArchive;
}

void BackupData::backupTransports()
{
  const QList<MailTransport::Transport *> listTransport = MailTransport::TransportManager::self()->transports();
  Q_FOREACH( MailTransport::Transport *mt, listTransport) {
    //TODO save it
  }
}

void BackupData::backupResources()
{

}

void BackupData::saveConfig()
{
}

void BackupData::saveIdentity()
{

}

void BackupData::backupMails()
{

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
