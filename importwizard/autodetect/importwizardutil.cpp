/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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
#include "importwizard_debug.h"
#include <kwallet.h>
#include <AkonadiCore/Tag>
#include <AkonadiCore/TagAttribute>
#include <AkonadiCore/TagCreateJob>

void ImportWizardUtil::mergeLdap(const ldapStruct &ldap)
{
    KSharedConfigPtr ldapConfig = KSharedConfig::openConfig(QStringLiteral("kabldaprc"));
    int numberOfLdapSelected = 0;
    KConfigGroup grp;
    if (ldapConfig->hasGroup(QStringLiteral("LDAP"))) {
        grp = ldapConfig->group(QStringLiteral("LDAP"));
        numberOfLdapSelected = grp.readEntry(QStringLiteral("NumSelectedHosts"), 0);
        grp.writeEntry(QStringLiteral("NumSelectedHosts"), (numberOfLdapSelected + 1));
    } else {
        grp = ldapConfig->group(QStringLiteral("LDAP"));
        grp.writeEntry(QStringLiteral("NumSelectedHosts"), 1);

        KConfigGroup ldapSeach = ldapConfig->group(QStringLiteral("LDAPSearch"));
        ldapSeach.writeEntry(QStringLiteral("SearchType"), 0);
    }
    const int port = ldap.port;
    if (port != -1) {
        grp.writeEntry(QStringLiteral("SelectedPort%1").arg(numberOfLdapSelected), port);
    }
    grp.writeEntry(QStringLiteral("SelectedHost%1").arg(numberOfLdapSelected), ldap.ldapUrl.host());
    if (ldap.useSSL) {
        grp.writeEntry(QStringLiteral("SelectedSecurity%1").arg(numberOfLdapSelected), QStringLiteral("SSL"));
    } else if (ldap.useTLS) {
        grp.writeEntry(QStringLiteral("SelectedSecurity%1").arg(numberOfLdapSelected), QStringLiteral("TLS"));
    } else {
        grp.writeEntry(QStringLiteral("SelectedSecurity%1").arg(numberOfLdapSelected), QStringLiteral("None"));
    }

    if (ldap.saslMech == QLatin1String("GSSAPI")) {
        grp.writeEntry(QStringLiteral("SelectedMech%1").arg(numberOfLdapSelected), QStringLiteral("GSSAPI"));
        grp.writeEntry(QStringLiteral("SelectedAuth%1").arg(numberOfLdapSelected), QStringLiteral("SASL"));
    } else if (ldap.saslMech.isEmpty()) {
        grp.writeEntry(QStringLiteral("SelectedMech%1").arg(numberOfLdapSelected), QStringLiteral("PLAIN"));
        grp.writeEntry(QStringLiteral("SelectedAuth%1").arg(numberOfLdapSelected), QStringLiteral("Simple"));
    } else {
        qCDebug(IMPORTWIZARD_LOG) << " Mech SASL undefined" << ldap.saslMech;
    }
    grp.writeEntry(QStringLiteral("SelectedVersion%1").arg(numberOfLdapSelected), QString::number(3));
    grp.writeEntry(QStringLiteral("SelectedBind%1").arg(numberOfLdapSelected), ldap.dn);
    //TODO: Verify selectedbase
    grp.writeEntry(QStringLiteral("SelectedBase%1").arg(numberOfLdapSelected), ldap.ldapUrl.path());
    if (ldap.timeout != -1) {
        grp.writeEntry(QStringLiteral("SelectedTimeLimit%1").arg(numberOfLdapSelected), ldap.timeout);
    }
    if (ldap.limit != -1) {
        grp.writeEntry(QStringLiteral("SelectedSizeLimit%1").arg(numberOfLdapSelected), ldap.limit);
    }
    if (!ldap.password.isEmpty()) {
        storeInKWallet(QStringLiteral("SelectedPwdBind%1").arg(numberOfLdapSelected), ImportWizardUtil::Ldap, ldap.password);
    }
    grp.sync();
}

void ImportWizardUtil::addAkonadiTag(const QVector<tagStruct> &tagList)
{
    for (int i = 0; i < tagList.size(); ++i) {
        Akonadi::Tag tag(tagList.at(i).name);
        if (tagList.at(i).color.isValid()) {
            tag.attribute<Akonadi::TagAttribute>(Akonadi::AttributeEntity::AddIfMissing)->setTextColor(tagList.at(i).color);
        }
        new Akonadi::TagCreateJob(tag);
    }
}

void ImportWizardUtil::storeInKWallet(const QString &name, ImportWizardUtil::ResourceType type, const QString &password)
{
    KWallet::Wallet *wallet = 0;
    switch (type) {
    case Imap:
        wallet = KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(), 0);
        if (wallet && wallet->isOpen()) {
            if (!wallet->hasFolder(QStringLiteral("imap"))) {
                wallet->createFolder(QStringLiteral("imap"));
            }
            wallet->setFolder(QLatin1String("imap"));
            wallet->writePassword(name + QLatin1String("rc"), password);
        }
        break;
    case Pop3:
        wallet = KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(), 0);
        if (wallet && wallet->isOpen()) {
            if (!wallet->hasFolder(QStringLiteral("pop3"))) {
                wallet->createFolder(QStringLiteral("pop3"));
            }
            wallet->setFolder(QStringLiteral("pop3"));
            wallet->writePassword(name, password);
        }
        break;
    case Ldap:
        wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(), 0);
        if (wallet && wallet->isOpen()) {
            if (!wallet->hasFolder(QStringLiteral("ldapclient"))) {
                wallet->createFolder(QStringLiteral("ldapclient"));
            }
            wallet->setFolder(QStringLiteral("ldapclient"));
            wallet->writePassword(name, password);
        }
    }
    delete wallet;

}
