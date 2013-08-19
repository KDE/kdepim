/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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
    mKmailConfig = KSharedConfig::openConfig( QLatin1String( "kmail2rc" ) );
}

AbstractSettings::~AbstractSettings()
{  
    syncKmailConfig();
    delete mManager;
}

KPIMIdentities::Identity* AbstractSettings::createIdentity(QString& name)
{
    name = uniqueIdentityName(name);
    KPIMIdentities::Identity* identity = &mManager->newFromScratch( name );
    addImportInfo(i18n("Setting up identity..."));
    return identity;
}

void AbstractSettings::storeIdentity(KPIMIdentities::Identity* identity)
{
    mManager->setAsDefault( identity->uoid() );
    mManager->commit();
    addImportInfo(i18n("Identity set up."));
}

QString AbstractSettings::uniqueIdentityName(const QString& name)
{
    QString newName(name);
    int i = 0;
    while(!mManager->isUnique( newName )) {
        newName = QString::fromLatin1("%1_%2").arg(name).arg(i);
        ++i;
    }
    return newName;
}

MailTransport::Transport *AbstractSettings::createTransport()
{
    MailTransport::Transport* mt = MailTransport::TransportManager::self()->createTransport();
    addImportInfo(i18n("Setting up transport..."));
    return mt;
}

void AbstractSettings::storeTransport(MailTransport::Transport *mt, bool isDefault )
{
    mt->forceUniqueName();
    mt->writeConfig();
    MailTransport::TransportManager::self()->addTransport( mt );
    if ( isDefault )
        MailTransport::TransportManager::self()->setDefaultTransport( mt->id() );
    addImportInfo(i18n("Transport set up."));
}

void AbstractSettings::addImportInfo( const QString &log )
{
    mImportWizard->importSettingPage()->addImportInfo( log );
}

void AbstractSettings::addImportError( const QString &log )
{
    mImportWizard->importSettingPage()->addImportError( log );
}

void AbstractSettings::addCheckMailOnStartup(const QString &agentIdentifyName, bool loginAtStartup)
{
    if (agentIdentifyName.isEmpty())
        return;
    const QString groupName = QString::fromLatin1("Resource %1").arg(agentIdentifyName);
    addKmailConfig(groupName,QLatin1String("CheckOnStartup"), loginAtStartup);
}

void AbstractSettings::addToManualCheck(const QString& agentIdentifyName, bool manualCheck)
{
    if (agentIdentifyName.isEmpty())
        return;
    const QString groupName = QString::fromLatin1("Resource %1").arg(agentIdentifyName);
    addKmailConfig(groupName,QLatin1String("IncludeInManualChecks"), manualCheck);
}


void AbstractSettings::addComposerHeaderGroup( const QString& groupName, const QString& name, const QString& value )
{
    KConfigGroup group = mKmailConfig->group(groupName);
    group.writeEntry(QLatin1String("name"),name);
    group.writeEntry(QLatin1String("value"),value);
}

void AbstractSettings::addKmailConfig( const QString &groupName, const QString &key, const QString &value)
{
    KConfigGroup group = mKmailConfig->group(groupName);
    group.writeEntry(key,value);
}

void AbstractSettings::addKmailConfig( const QString &groupName, const QString &key, bool value)
{
    KConfigGroup group = mKmailConfig->group(groupName);
    group.writeEntry(key,value);
}

void AbstractSettings::addKmailConfig( const QString & groupName, const QString &key, int value)
{
    KConfigGroup group = mKmailConfig->group(groupName);
    group.writeEntry(key,value);
}

void AbstractSettings::syncKmailConfig()
{
    mKmailConfig->sync();
}

void AbstractSettings::addKNodeConfig(const QString& groupName, const QString& key, bool value)
{
    Q_UNUSED( groupName );
    Q_UNUSED( key );
    Q_UNUSED( value );
    //TODO
}

void AbstractSettings::addAkregatorConfig(const QString& groupName, const QString& key, bool value)
{
    Q_UNUSED( groupName );
    Q_UNUSED( key );
    Q_UNUSED( value );
    //TODO
}


int AbstractSettings::readKmailSettings( const QString&groupName, const QString& key)
{
    KConfigGroup group = mKmailConfig->group(groupName);
    int value = group.readEntry(key,-1);
    return value;
}
