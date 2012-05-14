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
#include "mailcommon/mailutil.h"
#include "mailcommon/filter/filtermanager.h"
#include "mailcommon/filter/filterimporterexporter.h"

#include <Akonadi/AgentManager>
#include <Akonadi/Collection>

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
  if(mTypeSelected & BackupMailUtil::Nepomuk)
    backupNepomuk();
  closeArchive();
}

void BackupData::backupTransports()
{
  Q_EMIT info(i18n("Backing up transports..."));
  KSharedConfigPtr mailtransportsConfig = KSharedConfig::openConfig( QLatin1String( "mailtransports" ) );

  KTemporaryFile tmp;
  tmp.open();
  KConfig *config = mailtransportsConfig->copyTo( tmp.fileName() );

  config->sync();
  const bool fileAdded  = mArchive->addLocalFile(tmp.fileName(), BackupMailUtil::transportsPath() + QLatin1String("mailtransports"));
  if(fileAdded)
    Q_EMIT info(i18n("Transports backup done."));
  else
    Q_EMIT error(i18n("Transport file cannot be added to backup file."));
}

void BackupData::backupResources()
{
  Q_EMIT info(i18n("Backing up resources..."));
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
        const QString identifier = agent.identifier();
        //Store just pop3/imap account. Store other config when we copy data.
        if(identifier.contains(QLatin1String("pop3")) || identifier.contains(QLatin1String("imap"))) {
          storeResources(identifier,BackupMailUtil::resourcesPath());
        }
      }
    }
  }

  Q_EMIT info(i18n("Resources backup done."));
}

void BackupData::backupConfig()
{
  Q_EMIT info(i18n("Backing up config..."));
  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
  QList<MailCommon::MailFilter*> lstFilter = MailCommon::FilterManager::instance()->filters();
  KTemporaryFile tmp;
  tmp.open();
  KUrl url(tmp.fileName());
  MailCommon::FilterImporterExporter exportFilters;
  exportFilters.exportFilters(lstFilter,url, true);
  const bool fileAdded  = mArchive->addLocalFile(tmp.fileName(), BackupMailUtil::configsPath() + QLatin1String("filters"));
  if(!fileAdded)
    Q_EMIT error(i18n("Filters cannot be exported."));
  Q_EMIT info(i18n("Config backup done."));
}

void BackupData::backupIdentity()
{
  Q_EMIT info(i18n("Backing up identity..."));
  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
  KSharedConfigPtr identity = KSharedConfig::openConfig( QLatin1String( "emailidentities" ) );

  KTemporaryFile tmp;
  tmp.open();

  KConfig *identityConfig = identity->copyTo( tmp.fileName() );
  const QStringList accountList = identityConfig->groupList().filter( QRegExp( "Identity #\\d+" ) );
  Q_FOREACH(const QString& account, accountList) {
    KConfigGroup group = identityConfig->group(account);
    const QString fcc =QLatin1String("Fcc");
    if(group.hasKey(fcc)) {
      group.writeEntry(fcc,MailCommon::Util::fullCollectionPath(Akonadi::Collection(group.readEntry(fcc).toLongLong())));
    }
    const QString draft = QLatin1String("Drafts");
    if(group.hasKey(draft)) {
      group.writeEntry(draft,MailCommon::Util::fullCollectionPath(Akonadi::Collection(group.readEntry(draft).toLongLong())));
    }
    const QString templates = QLatin1String("Templates");
    if(group.hasKey(templates)) {
      group.writeEntry(templates,MailCommon::Util::fullCollectionPath(Akonadi::Collection(group.readEntry(templates).toLongLong())));
    }
  }

  identityConfig->sync();
  const bool fileAdded  = mArchive->addLocalFile(tmp.fileName(), BackupMailUtil::identitiesPath() + QLatin1String("emailidentities"));
  if(fileAdded)
    Q_EMIT info(i18n("Identity backup done."));
  else
    Q_EMIT error(i18n("Identity file cannot be added to backup file."));
}

void BackupData::backupMails()
{
  Q_EMIT info(i18n("Backing up Mails..."));
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
        const QString identifier = agent.identifier();
        if(identifier.contains(QLatin1String("akonadi_mbox_resource_"))) {
          const QString agentFileName = agent.identifier() + QLatin1String("rc");
          const QString configFileName = KStandardDirs::locateLocal( "config", agentFileName );

          KSharedConfigPtr resourceConfig = KSharedConfig::openConfig( configFileName );
          //TODO get path
          //1 file
          //TODO
        } else if(identifier.contains(QLatin1String("akonadi_maildir_resource_"))) {
          const QString agentFileName = agent.identifier() + QLatin1String("rc");
          const QString configFileName = KStandardDirs::locateLocal( "config", agentFileName );

          KSharedConfigPtr resourceConfig = KSharedConfig::openConfig( configFileName );
          //TODO
          //Several file. Look at archive mail dialog
        }
      }
    }
  }

  Q_EMIT info(i18n("Mails backup done."));

}

void BackupData::backupAkonadiDb()
{
  Q_EMIT info(i18n("Backing up Akonadi Database..."));
  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
  Q_EMIT info(i18n("Akonadi Database backup done."));
}

void BackupData::backupNepomuk()
{

}

void BackupData::storeResources(const QString&identifier, const QString& path)
{
  const QString agentFileName = identifier + QLatin1String("rc");
  const QString configFileName = KStandardDirs::locateLocal( "config", agentFileName );

  KSharedConfigPtr resourceConfig = KSharedConfig::openConfig( configFileName );
  KTemporaryFile tmp;
  tmp.open();
  KConfig * config = resourceConfig->copyTo( tmp.fileName() );

  if(identifier.contains(QLatin1String("pop3"))) {
    const QString targetCollection = QLatin1String("targetCollection");
    KConfigGroup group = config->group("General");
    if(group.hasGroup(targetCollection)) {
       group.writeEntry(targetCollection,MailCommon::Util::fullCollectionPath(Akonadi::Collection(group.readEntry(targetCollection).toLongLong())));
    }
  } else if(identifier.contains(QLatin1String("imap"))) {
    const QString trash = QLatin1String("TrashCollection");
    KConfigGroup group = config->group("cache");
    if(group.hasGroup(trash)) {
      group.writeEntry(trash,MailCommon::Util::fullCollectionPath(Akonadi::Collection(group.readEntry(trash).toLongLong())));
    }
  }
  config->sync();
  const bool fileAdded  = mArchive->addLocalFile(tmp.fileName(), path + agentFileName);
  if(!fileAdded)
    Q_EMIT error(i18n("Resource file \"%1\" cannot be added to backup file.", agentFileName));
}
