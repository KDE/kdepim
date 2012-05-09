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

#include <Akonadi/AgentManager>

#include <Mailtransport/TransportManager>

#include <KMime/KMimeMessage>

#include <KZip>
#include <KLocale>
#include <KTemporaryFile>
#include <KStandardDirs>

#include <QDebug>

BackupData::BackupData(BackupMailUtil::BackupTypes typeSelected, const QString &filename)
  :AbstractData(filename,typeSelected)
{
}

BackupData::~BackupData()
{
}

void BackupData::startBackup()
{
  if(!openArchive(true))
    return;

  if(mTypeSelected & BackupMailUtil::Identity)
    backupIdentity();
  if(mTypeSelected & BackupMailUtil::MailTransport)
    backupTransports();
  if(mTypeSelected & BackupMailUtil::Mails)
    backupMails();
  if(mTypeSelected & BackupMailUtil::Resources)
    backupResources();
  if(mTypeSelected & BackupMailUtil::Config)
    backupConfig();
  if(mTypeSelected & BackupMailUtil::AkonadiDb)
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
  const bool fileAdded  = mArchive->addLocalFile(tmp.fileName(), BackupMailUtil::transportsPath() + QLatin1String("mailtransports"));
  if(fileAdded)
    Q_EMIT info(i18n("Transports backuped."));
  else
    Q_EMIT error(i18n("Transport file cannot be added to backup file."));
}

void BackupData::backupResources()
{
  Q_EMIT info(i18n("Backup resources..."));
  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );

  Akonadi::AgentManager *manager = Akonadi::AgentManager::self();
  const Akonadi::AgentInstance::List list = manager->instances();
  foreach( const Akonadi::AgentInstance &agent, list ) {
    const QStringList capabilities( agent.type().capabilities() );
    if(agent.type().mimeTypes().contains( KMime::Message::mimeType())) {
      if ( capabilities.contains( "Resource" ) &&
            !capabilities.contains( "Virtual" ) &&
            !capabilities.contains( "MailTransport" ) )
      {

        const QString agentFileName = agent.identifier() + QLatin1String("rc");
        const QString configFileName = KStandardDirs::locateLocal( "config", agentFileName );

        const bool fileAdded  = mArchive->addLocalFile(configFileName, BackupMailUtil::resourcesPath() + agentFileName);
        if(!fileAdded)
          Q_EMIT error(i18n("Resource file \"%1\" cannot be added to backup file.", agentFileName));
      }
    }
  }

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
  KSharedConfigPtr identity = KSharedConfig::openConfig( QLatin1String( "emailidentities" ) );

  KTemporaryFile tmp;
  tmp.open();

  KSharedConfig::Ptr identityConfig = KSharedConfig::openConfig(tmp.fileName());

  identityConfig->copyTo( tmp.fileName(), identityConfig.data() );

  identityConfig->sync();
  const bool fileAdded  = mArchive->addLocalFile(tmp.fileName(), BackupMailUtil::identitiesPath() + QLatin1String("emailidentities"));
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

