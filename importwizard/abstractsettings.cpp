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
#include "abstractsettings.h"
#include "importwizard.h"
#include "importsettingpage.h"

#include "mailcommon/filter/filteractionmissingargumentdialog.h"

#include <kpimidentities/identitymanager.h>
#include <kpimidentities/identity.h>
#include <mailtransport/transportmanager.h>

#include <KLocale>
#include <KDebug>
#include <KSharedConfig>

#include <akonadi/agenttype.h>
#include <akonadi/agentmanager.h>
#include <akonadi/agentinstancecreatejob.h>

#include <QDBusReply>
#include <QDBusInterface>
#include <QMetaMethod>

using namespace Akonadi;

AbstractSettings::AbstractSettings(ImportWizard *parent)
  :mImportWizard(parent)
{
  mManager = new KPIMIdentities::IdentityManager( false, this, "mIdentityManager" );
}

AbstractSettings::~AbstractSettings()
{
  delete mManager;
}

KPIMIdentities::Identity* AbstractSettings::createIdentity()
{
  KPIMIdentities::Identity* identity = &mManager->newFromScratch( QString() );
  addFilterImportInfo(i18n("Setting up identity..."));
  return identity;
}

void AbstractSettings::storeIdentity(KPIMIdentities::Identity* identity)
{
  mManager->setAsDefault( identity->uoid() );
  mManager->commit();
  addFilterImportInfo(i18n("Identity set up."));
}


MailTransport::Transport *AbstractSettings::createTransport()
{
  MailTransport::Transport* mt = MailTransport::TransportManager::self()->createTransport();
  addFilterImportInfo(i18n("Setting up transport..."));
  return mt;
}

void AbstractSettings::storeTransport(MailTransport::Transport * mt, bool isDefault )
{
  mt->forceUniqueName();
  mt->writeConfig();
  MailTransport::TransportManager::self()->addTransport( mt );
  if ( isDefault )
    MailTransport::TransportManager::self()->setDefaultTransport( mt->id() );
  addFilterImportInfo(i18n("Transport set up."));
}


//code from accountwizard
static QVariant::Type argumentType( const QMetaObject *mo, const QString &method )
{
  QMetaMethod m;
  const int numberOfMethod( mo->methodCount() );
  for ( int i = 0; i < numberOfMethod; ++i ) {
    const QString signature = QString::fromLatin1( mo->method( i ).signature() );
    if ( signature.contains(method + QLatin1Char('(') )) {
      m = mo->method( i );
      break;
    }
  }

  if ( !m.signature() ) {
    kWarning() << "Did not find D-Bus method: " << method << " available methods are:";
    const int numberOfMethod(mo->methodCount());
    for ( int i = 0; i < numberOfMethod; ++ i )
      kWarning() << mo->method( i ).signature();
    return QVariant::Invalid;
  }

  const QList<QByteArray> argTypes = m.parameterTypes();
  if ( argTypes.count() != 1 )
    return QVariant::Invalid;

  return QVariant::nameToType( argTypes.first() );
}


QString AbstractSettings::createResource( const QString& resources, const QString& name, const QMap<QString, QVariant>& settings )
{
  const AgentType type = AgentManager::self()->type( resources );
  if ( !type.isValid() ) {
    addFilterImportError( i18n( "Resource type '%1' is not available.", resources ) );
    return QString();
  }

  // check if unique instance already exists
  kDebug() << type.capabilities();
  if ( type.capabilities().contains( QLatin1String( "Unique" ) ) ) {
    Q_FOREACH ( const AgentInstance &instance, AgentManager::self()->instances() ) {
      kDebug() << instance.type().identifier() << (instance.type() == type);
      if ( instance.type() == type ) {
        addFilterImportInfo( i18n( "Resource '%1' is already set up.", type.name() ) );
        return QString();
      }
    }
  }

  addFilterImportInfo( i18n( "Creating resource instance for '%1'...", type.name() ) );
  AgentInstanceCreateJob *job = new AgentInstanceCreateJob( type, this );
  if(job->exec()) {
    Akonadi::AgentInstance instance = job->instance();

    if ( !settings.isEmpty() ) {
      addFilterImportInfo( i18n( "Configuring resource instance..." ) );
      QDBusInterface iface( "org.freedesktop.Akonadi.Resource." + instance.identifier(), "/Settings" );
      if ( !iface.isValid() ) {
        addFilterImportError( i18n( "Unable to configure resource instance." ) );
        return QString();
      }

      // configure resource
      if ( !name.isEmpty() )
        instance.setName( name );
      QMap<QString, QVariant>::const_iterator end( settings.constEnd());
      for ( QMap<QString, QVariant>::const_iterator it = settings.constBegin(); it != end; ++it ) {
        kDebug() << "Setting up " << it.key() << " for agent " << instance.identifier();
        const QString methodName = QString::fromLatin1("set%1").arg( it.key() );
        QVariant arg = it.value();
        const QVariant::Type targetType = argumentType( iface.metaObject(), methodName );
        if ( !arg.canConvert( targetType ) ) {
          addFilterImportError( i18n( "Could not convert value of setting '%1' to required type %2.", it.key(), QVariant::typeToName( targetType ) ) );
          return QString();
        }
        arg.convert( targetType );
        QDBusReply<void> reply = iface.call( methodName, arg );
        if ( !reply.isValid() ) {
          addFilterImportError( i18n( "Could not set setting '%1': %2", it.key(), reply.error().message() ) );
          return QString();
        }
      }
      instance.reconfigure();
    }

    addFilterImportInfo( i18n( "Resource setup completed." ) );
    return instance.identifier();
  } else {
    if ( job->error() ) {
      addFilterImportError( i18n( "Failed to create resource instance: %1", job->errorText() ) );
    }
  }
  return QString();
}

void AbstractSettings::addFilterImportInfo( const QString& log )
{
  mImportWizard->importSettingPage()->addFilterImportInfo( log );
}

void AbstractSettings::addFilterImportError( const QString& log )
{
  mImportWizard->importSettingPage()->addFilterImportError( log );
}

Akonadi::Collection::Id AbstractSettings::adaptFolderId( const QString& folder)
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

QString AbstractSettings::adaptFolder( const QString& folder)
{
  Akonadi::Collection::Id newFolderId= adaptFolderId(folder);
  if(newFolderId == -1 )
    return QString();
  return QString::number(newFolderId);
}

void AbstractSettings::addKmailConfig( const QString& groupName, const QString& key, const QString& value)
{
  KSharedConfigPtr kmailConfig = KSharedConfig::openConfig( QLatin1String( "kmail2rc" ) );
  //TODO
}

#include "abstractsettings.moc"
