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

#include "importwizardutil.h"
#include <KSharedConfig>
#include <KConfigGroup>
#include <QDebug>
#include <kwallet.h>
#include <Akonadi/Tag>
#include <Akonadi/TagAttribute>
#include <Akonadi/TagCreateJob>



void ImportWizardUtil::mergeLdap(const ldapStruct &ldap)
{
    KSharedConfigPtr ldapConfig = KSharedConfig::openConfig( QLatin1String( "kabldaprc" ) );
    int numberOfLdapSelected = 0;
    KConfigGroup grp;
    if (ldapConfig->hasGroup(QLatin1String("LDAP"))) {
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
    if (port!=-1)
        grp.writeEntry(QString::fromLatin1("SelectedPort%1").arg(numberOfLdapSelected),port);
    grp.writeEntry(QString::fromLatin1("SelectedHost%1").arg(numberOfLdapSelected),ldap.ldapUrl.host());
    if (ldap.useSSL) {
        grp.writeEntry(QString::fromLatin1("SelectedSecurity%1").arg(numberOfLdapSelected),QString::fromLatin1("SSL"));
    } else if (ldap.useTLS){
        grp.writeEntry(QString::fromLatin1("SelectedSecurity%1").arg(numberOfLdapSelected),QString::fromLatin1("TLS"));
    } else {
        grp.writeEntry(QString::fromLatin1("SelectedSecurity%1").arg(numberOfLdapSelected),QString::fromLatin1("None"));
    }

    if (ldap.saslMech == QLatin1String("GSSAPI")) {
        grp.writeEntry(QString::fromLatin1("SelectedMech%1").arg(numberOfLdapSelected),QString::fromLatin1("GSSAPI"));
        grp.writeEntry(QString::fromLatin1("SelectedAuth%1").arg(numberOfLdapSelected),QString::fromLatin1("SASL"));
    } else if (ldap.saslMech.isEmpty()) {
        grp.writeEntry(QString::fromLatin1("SelectedMech%1").arg(numberOfLdapSelected),QString::fromLatin1("PLAIN"));
        grp.writeEntry(QString::fromLatin1("SelectedAuth%1").arg(numberOfLdapSelected),QString::fromLatin1("Simple"));
    } else {
        qDebug()<<" Mech SASL undefined"<<ldap.saslMech;
    }
    grp.writeEntry(QString::fromLatin1("SelectedVersion%1").arg(numberOfLdapSelected),QString::number(3));
    grp.writeEntry(QString::fromLatin1("SelectedBind%1").arg(numberOfLdapSelected),ldap.dn);
    //TODO: Verify selectedbase
    grp.writeEntry(QString::fromLatin1("SelectedBase%1").arg(numberOfLdapSelected),ldap.ldapUrl.path());
    if (ldap.timeout != -1) {
        grp.writeEntry(QString::fromLatin1("SelectedTimeLimit%1").arg(numberOfLdapSelected),ldap.timeout);
    }
    if (ldap.limit != -1) {
        grp.writeEntry(QString::fromLatin1("SelectedSizeLimit%1").arg(numberOfLdapSelected),ldap.limit);
    }
    if (!ldap.password.isEmpty()) {
        storeInKWallet(QString::fromLatin1("SelectedPwdBind%1").arg(numberOfLdapSelected), ImportWizardUtil::Ldap, ldap.password);
    }
    grp.sync();
}

void ImportWizardUtil::addAkonadiTag(const QList<tagStruct>& tagList)
{
    for(int i = 0; i<tagList.size(); ++i) {
        Akonadi::Tag tag(tagList.at(i).name);
        if ( tagList.at(i).color.isValid() ) {
            tag.attribute<Akonadi::TagAttribute>(Akonadi::AttributeEntity::AddIfMissing)->setTextColor(tagList.at(i).color);
        }
        new Akonadi::TagCreateJob(tag);
    }
}

void ImportWizardUtil::storeInKWallet(const QString &name, ImportWizardUtil::ResourceType type, const QString &password)
{
    KWallet::Wallet * wallet = 0;
    switch(type) {
    case Imap:
        wallet = KWallet::Wallet::openWallet( KWallet::Wallet::NetworkWallet(), 0 );
        if ( wallet && wallet->isOpen() ) {
            if ( !wallet->hasFolder( QLatin1String("imap") ) )
                wallet->createFolder( QLatin1String("imap") );
            wallet->setFolder( QLatin1String("imap") );
            wallet->writePassword( name+QLatin1String("rc"), password );
        }
        break;
    case Pop3:
        wallet = KWallet::Wallet::openWallet( KWallet::Wallet::NetworkWallet(), 0);
        if ( wallet && wallet->isOpen() ) {
            if ( !wallet->hasFolder( QLatin1String("pop3") ) ) {
                wallet->createFolder( QLatin1String("pop3") );
            }
            wallet->setFolder( QLatin1String("pop3") );
            wallet->writePassword( name, password );
        }
        break;
    case Ldap:
        wallet = KWallet::Wallet::openWallet( KWallet::Wallet::LocalWallet(), 0 );
        if ( wallet && wallet->isOpen() ) {
            if ( !wallet->hasFolder( QLatin1String("ldapclient") ) ) {
                wallet->createFolder( QLatin1String("ldapclient") );
            }
            wallet->setFolder( QLatin1String("ldapclient") );
            wallet->writePassword(name, password );
        }
    }
    delete wallet;

}
