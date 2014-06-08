/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/


#include "sieveeditorutil.h"
#include "sieveserversettings.h"

#include <kwallet.h>

#include <KConfig>
#include <KGlobal>
#include <KLocale>
#include <KConfigGroup>

#include <QRegExp>
#include <KSharedConfig>


QUrl SieveEditorUtil::SieveServerConfig::url() const {
    QUrl u;
    u.setHost(serverName);
    u.setUserName(userName);
    u.setPassword(password);
    u.setPort(port);

    QString authStr;
    switch( authenticationType ) {
    case MailTransport::Transport::EnumAuthenticationType::CLEAR:
    case MailTransport::Transport::EnumAuthenticationType::PLAIN:
        authStr = QLatin1String("PLAIN");
        break;
    case MailTransport::Transport::EnumAuthenticationType::LOGIN:
        authStr = QLatin1String("LOGIN");
        break;
    case MailTransport::Transport::EnumAuthenticationType::CRAM_MD5:
        authStr = QLatin1String("CRAM-MD5");
        break;
    case MailTransport::Transport::EnumAuthenticationType::DIGEST_MD5:
        authStr = QLatin1String("DIGEST-MD5");
        break;
    case MailTransport::Transport::EnumAuthenticationType::GSSAPI:
        authStr = QLatin1String("GSSAPI");
        break;
    case MailTransport::Transport::EnumAuthenticationType::ANONYMOUS:
        authStr = QLatin1String("ANONYMOUS");
        break;
    default:
        authStr = QLatin1String("PLAIN");
        break;
    }
    u.addQueryItem( QLatin1String("x-mech"), authStr );

    return u;
}

QList<SieveEditorUtil::SieveServerConfig> SieveEditorUtil::readServerSieveConfig()
{
    QList<SieveServerConfig> lstConfig;
    KSharedConfigPtr cfg = KSharedConfig::openConfig();
    QRegExp re( QLatin1String( "^ServerSieve (.+)$" ) );
    const QStringList groups = cfg->groupList().filter( re );
    KWallet::Wallet *wallet = SieveServerSettings::self()->wallet();
    if ( wallet && !wallet->setFolder( QLatin1String("sieveeditor") ) ) {
        wallet->createFolder( QLatin1String("sieveeditor"));
        wallet->setFolder( QLatin1String("sieveeditor") );
    }


    Q_FOREACH (const QString &conf, groups) {
        SieveServerConfig sieve;
        KConfigGroup group = cfg->group(conf);
        sieve.port = group.readEntry(QLatin1String("Port"), 0);
        sieve.serverName = group.readEntry(QLatin1String("ServerName"));
        sieve.userName = group.readEntry(QLatin1String("UserName"));
        const QString walletEntry = sieve.userName + QLatin1Char('@') + sieve.serverName;
        if (wallet && wallet->hasEntry(walletEntry)) {
            wallet->readPassword(walletEntry, sieve.password);
        }
        sieve.authenticationType = static_cast<MailTransport::Transport::EnumAuthenticationType::type>(group.readEntry(QLatin1String("Authentication"), static_cast<int>(MailTransport::Transport::EnumAuthenticationType::PLAIN)));
        lstConfig.append(sieve);
    }
    return lstConfig;
}

void SieveEditorUtil::writeServerSieveConfig(const QList<SieveEditorUtil::SieveServerConfig> &lstConfig)
{
    KSharedConfigPtr cfg = KSharedConfig::openConfig();
    const QRegExp re( QLatin1String( "^ServerSieve (.+)$" ) );
    //Delete Old Group
    const QStringList groups = cfg->groupList().filter( re );
    Q_FOREACH (const QString &conf, groups) {
        KConfigGroup group = cfg->group(conf);
        group.deleteGroup();
    }

    int i = 0;
    KWallet::Wallet *wallet = SieveServerSettings::self()->wallet();
    if (wallet) {
        if ( !wallet->hasFolder( QLatin1String("sieveeditor") ) ) {
            wallet->createFolder( QLatin1String("sieveeditor") );
        }
        wallet->setFolder( QLatin1String("sieveeditor") );
    }

    Q_FOREACH (const SieveEditorUtil::SieveServerConfig &conf, lstConfig) {
        KConfigGroup group = cfg->group(QString::fromLatin1("ServerSieve %1").arg(i));
        group.writeEntry(QLatin1String("Port"), conf.port);
        group.writeEntry(QLatin1String("ServerName"), conf.serverName);
        group.writeEntry(QLatin1String("UserName"), conf.userName);
        const QString walletEntry = conf.userName + QLatin1Char('@') + conf.serverName;
        if (wallet)
            wallet->writePassword(walletEntry, conf.password);
        group.writeEntry(QLatin1String("Authentication"), static_cast<int>(conf.authenticationType));

        ++i;
    }
    cfg->sync();
    cfg->reparseConfiguration();
}

void SieveEditorUtil::addServerSieveConfig(const SieveEditorUtil::SieveServerConfig &conf)
{
    KWallet::Wallet *wallet = SieveServerSettings::self()->wallet();
    if (wallet) {
        if ( !wallet->hasFolder( QLatin1String("sieveeditor") ) ) {
            wallet->createFolder( QLatin1String("sieveeditor") );
        }
        wallet->setFolder( QLatin1String("sieveeditor") );
    }
    KSharedConfigPtr cfg = KSharedConfig::openConfig();
    const QRegExp re( QLatin1String( "^ServerSieve (.+)$" ) );
    const QStringList groups = cfg->groupList().filter( re );
    KConfigGroup group = cfg->group(QString::fromLatin1("ServerSieve %1").arg(groups.count()));
    group.writeEntry(QLatin1String("Port"), conf.port);
    group.writeEntry(QLatin1String("ServerName"), conf.serverName);
    group.writeEntry(QLatin1String("UserName"), conf.userName);
    const QString walletEntry = conf.userName + QLatin1Char('@') + conf.serverName;

    if (wallet)
        wallet->writePassword(walletEntry, conf.password);

    group.writeEntry(QLatin1String("Authentication"), static_cast<int>(conf.authenticationType));
    cfg->sync();
}
