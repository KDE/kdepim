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

KPIMIdentities::Identity* AbstractSettings::createIdentity()
{
  KPIMIdentities::Identity* identity = &mManager->newFromScratch( QString() );
  addImportInfo(i18n("Setting up identity..."));
  return identity;
}

void AbstractSettings::storeIdentity(KPIMIdentities::Identity* identity)
{
  mManager->setAsDefault( identity->uoid() );
  mManager->commit();
  addImportInfo(i18n("Identity set up."));
}


MailTransport::Transport *AbstractSettings::createTransport()
{
  MailTransport::Transport* mt = MailTransport::TransportManager::self()->createTransport();
  addImportInfo(i18n("Setting up transport..."));
  return mt;
}

void AbstractSettings::storeTransport(MailTransport::Transport * mt, bool isDefault )
{
  mt->forceUniqueName();
  mt->writeConfig();
  MailTransport::TransportManager::self()->addTransport( mt );
  if ( isDefault )
    MailTransport::TransportManager::self()->setDefaultTransport( mt->id() );
  addImportInfo(i18n("Transport set up."));
}

void AbstractSettings::addImportInfo( const QString& log )
{
  mImportWizard->importSettingPage()->addImportInfo( log );
}

void AbstractSettings::addImportError( const QString& log )
{
  mImportWizard->importSettingPage()->addImportError( log );
}

void AbstractSettings::addCheckMailOnStartup(const QString& agentIdentifyName,bool loginAtStartup)
{
  if(agentIdentifyName.isEmpty())
    return;
  const QString groupName = QString::fromLatin1("Resource %1").arg(agentIdentifyName);
  addKmailConfig(groupName,QLatin1String("CheckOnStartup"), loginAtStartup);
}

void AbstractSettings::addToManualCheck(const QString& agentIdentifyName,bool manualCheck)
{
  if(agentIdentifyName.isEmpty())
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

void AbstractSettings::addKmailConfig( const QString& groupName, const QString& key, const QString& value)
{
  KConfigGroup group = mKmailConfig->group(groupName);
  group.writeEntry(key,value);
}

void AbstractSettings::addKmailConfig( const QString& groupName, const QString& key, bool value)
{
  KConfigGroup group = mKmailConfig->group(groupName);
  group.writeEntry(key,value);
}

void AbstractSettings::addKmailConfig( const QString& groupName, const QString& key, int value)
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
  //TODO
}

void AbstractSettings::addAkregatorConfig(const QString& groupName, const QString& key, bool value)
{
  //TODO
}


int AbstractSettings::readKmailSettings( const QString&groupName, const QString& key)
{
  KConfigGroup group = mKmailConfig->group(groupName);
  int value = group.readEntry(key,-1);
  return value;
}

void AbstractSettings::mergeLdap(const ldapStruct &ldap)
{
  KSharedConfigPtr ldapConfig = KSharedConfig::openConfig( QLatin1String( "kabldaprc" ) );
  int numberOfLdapSelected = 0;
  KConfigGroup grp;
  if(ldapConfig->hasGroup(QLatin1String("LDAP"))) {
    grp = ldapConfig->group(QLatin1String("LDAP"));
    numberOfLdapSelected = grp.readEntry(QLatin1String("NumSelectedHosts"),0);
    grp.writeEntry(QLatin1String("NumSelectedHosts"),(numberOfLdapSelected+1));
  } else {
    grp = ldapConfig->group(QLatin1String("LDAP"));
    grp.writeEntry(QLatin1String("NumSelectedHosts"),1);

    KConfigGroup ldapSeach = ldapConfig->group(QLatin1String("LDAPSearch"));
    ldapSeach.writeEntry(QLatin1String("SearchType"), 0);
  }
  const int port = ldap.port;
  if(port!=-1)
    grp.writeEntry(QString::fromLatin1("SelectedPort%1").arg(numberOfLdapSelected),port);
  grp.writeEntry(QString::fromLatin1("SelectedHost%1").arg(numberOfLdapSelected),ldap.ldapUrl.host());
  if(ldap.useSSL) {
    grp.writeEntry(QString::fromLatin1("SelectedSecurity%1").arg(numberOfLdapSelected),QString::fromLatin1("SSL"));
  } else if(ldap.useTLS){
    grp.writeEntry(QString::fromLatin1("SelectedSecurity%1").arg(numberOfLdapSelected),QString::fromLatin1("TLS"));
  } else {
    grp.writeEntry(QString::fromLatin1("SelectedSecurity%1").arg(numberOfLdapSelected),QString::fromLatin1("None"));
  }

  if(ldap.saslMech == QLatin1String("GSSAPI")) {
    grp.writeEntry(QString::fromLatin1("SelectedMech%1").arg(numberOfLdapSelected),QString::fromLatin1("GSSAPI"));
    grp.writeEntry(QString::fromLatin1("SelectedAuth%1").arg(numberOfLdapSelected),QString::fromLatin1("SASL"));
  } else if(ldap.saslMech.isEmpty()) {
    grp.writeEntry(QString::fromLatin1("SelectedMech%1").arg(numberOfLdapSelected),QString::fromLatin1("PLAIN"));
    grp.writeEntry(QString::fromLatin1("SelectedAuth%1").arg(numberOfLdapSelected),QString::fromLatin1("Simple"));
  } else {
    qDebug()<<" Mech SASL undefined"<<ldap.saslMech;
  }
  grp.writeEntry(QString::fromLatin1("SelectedVersion%1").arg(numberOfLdapSelected),QString::number(3));
  grp.writeEntry(QString::fromLatin1("SelectedBind%1").arg(numberOfLdapSelected),ldap.dn);
  //TODO: Verify selectedbase
  grp.writeEntry(QString::fromLatin1("SelectedBase%1").arg(numberOfLdapSelected),ldap.ldapUrl.path());
  if(ldap.timeout != -1) {
    grp.writeEntry(QString::fromLatin1("SelectedTimeLimit%1").arg(numberOfLdapSelected),ldap.timeout);
  }
  if(ldap.limit != -1) {
    grp.writeEntry(QString::fromLatin1("SelectedSizeLimit%1").arg(numberOfLdapSelected),ldap.limit);
  }
  grp.sync();
}
