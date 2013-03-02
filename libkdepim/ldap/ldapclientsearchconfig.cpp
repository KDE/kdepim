/* Copyright (C) 2013 Laurent Montel <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ldapclientsearchconfig.h"
#include <kldap/ldapserver.h>

#include <KConfig>
#include <KConfigGroup>

#include <kwallet.h>

using namespace KLDAP;

class LdapClientSearchConfig::Private
{
public:
    Private()
        : useWallet( false ),
          wallet( 0 )
    {

    }
    ~Private()
    {
        if (useWallet) {
            wallet->deleteLater();
            wallet = 0;
        }
    }
    bool useWallet;
    KWallet::Wallet* wallet;
};

LdapClientSearchConfig::LdapClientSearchConfig(QObject *parent)
    : QObject(parent), d(new LdapClientSearchConfig::Private())
{
}

LdapClientSearchConfig::~LdapClientSearchConfig()
{
    delete d;
}

void LdapClientSearchConfig::readConfig( KLDAP::LdapServer &server, const KConfigGroup &config, int j, bool active )
{
    QString prefix;
    if ( active ) {
        prefix = "Selected";
    }

    const QString host =  config.readEntry( prefix + QString::fromLatin1( "Host%1" ).arg( j ),
                                            QString() ).trimmed();
    if ( !host.isEmpty() ) {
        server.setHost( host );
    }

    const int port = config.readEntry( prefix + QString::fromLatin1( "Port%1" ).arg( j ), 389 );
    server.setPort( port );

    const QString base = config.readEntry( prefix + QString::fromLatin1( "Base%1" ).arg( j ),
                                           QString() ).trimmed();
    if ( !base.isEmpty() ) {
        server.setBaseDn( KLDAP::LdapDN( base ) );
    }

    const QString user = config.readEntry( prefix + QString::fromLatin1( "User%1" ).arg( j ),
                                           QString() ).trimmed();
    if ( !user.isEmpty() ) {
        server.setUser( user );
    }

    QString bindDN = config.readEntry( prefix + QString::fromLatin1( "Bind%1" ).arg( j ), QString() ).trimmed();
    if ( !bindDN.isEmpty() ) {
        server.setBindDn( bindDN );
    }

    const QString pwdBindBNEntry = prefix + QString::fromLatin1( "PwdBind%1" ).arg( j );
    QString pwdBindDN = config.readEntry( pwdBindBNEntry, QString() );
    if ( !pwdBindDN.isEmpty() ) {
        server.setPassword( pwdBindDN );
    } else { //Look at in Wallet
        d->wallet = KWallet::Wallet::openWallet( KWallet::Wallet::LocalWallet(), 0 );
        if ( d->wallet ) {
            d->useWallet = true;
            if ( !d->wallet->setFolder( QLatin1String("ldapclient") ) ) {
                d->wallet->createFolder( QLatin1String("ldapclient") );
                d->wallet->setFolder( QLatin1String("ldapclient") );
            }
            d->wallet->readPassword( pwdBindBNEntry, pwdBindDN );
            if (!pwdBindDN.isEmpty())
                server.setPassword( pwdBindDN );
        } else {
            d->useWallet = false;
        }
    }

    server.setTimeLimit( config.readEntry( prefix + QString::fromLatin1( "TimeLimit%1" ).arg( j ), 0 ) );
    server.setSizeLimit( config.readEntry( prefix + QString::fromLatin1( "SizeLimit%1" ).arg( j ), 0 ) );
    server.setPageSize( config.readEntry( prefix + QString::fromLatin1( "PageSize%1" ).arg( j ), 0 ) );
    server.setVersion( config.readEntry( prefix + QString::fromLatin1( "Version%1" ).arg( j ), 3 ) );

    QString tmp;
    tmp = config.readEntry( prefix + QString::fromLatin1( "Security%1" ).arg( j ),
                            QString::fromLatin1( "None" ) );
    server.setSecurity( KLDAP::LdapServer::None );
    if ( tmp == "SSL" ) {
        server.setSecurity( KLDAP::LdapServer::SSL );
    } else if ( tmp == "TLS" ) {
        server.setSecurity( KLDAP::LdapServer::TLS );
    }

    tmp = config.readEntry( prefix + QString::fromLatin1( "Auth%1" ).arg( j ),
                            QString::fromLatin1( "Anonymous" ) );
    server.setAuth( KLDAP::LdapServer::Anonymous );
    if ( tmp == "Simple" ) {
        server.setAuth( KLDAP::LdapServer::Simple );
    } else if ( tmp == "SASL" ) {
        server.setAuth( KLDAP::LdapServer::SASL );
    }

    server.setMech( config.readEntry( prefix + QString::fromLatin1( "Mech%1" ).arg( j ), QString() ) );
}

void LdapClientSearchConfig::writeConfig( const KLDAP::LdapServer &server, KConfigGroup &config, int j, bool active )
{
    QString prefix;
    if ( active ) {
        prefix = "Selected";
    }

    config.writeEntry( prefix + QString::fromLatin1( "Host%1" ).arg( j ), server.host() );
    config.writeEntry( prefix + QString::fromLatin1( "Port%1" ).arg( j ), server.port() );
    config.writeEntry( prefix + QString::fromLatin1( "Base%1" ).arg( j ), server.baseDn().toString() );
    config.writeEntry( prefix + QString::fromLatin1( "User%1" ).arg( j ), server.user() );
    config.writeEntry( prefix + QString::fromLatin1( "Bind%1" ).arg( j ), server.bindDn() );

    const QString passwordEntry = prefix + QString::fromLatin1( "PwdBind%1" ).arg( j );
    const QString password = server.password();
    if (!password.isEmpty()) {
        if (d->useWallet && d->wallet) {
            d->wallet->writePassword(passwordEntry, password );
        } else {
            config.writeEntry( prefix + passwordEntry, password );
        }
    }

    config.writeEntry( prefix + QString::fromLatin1( "TimeLimit%1" ).arg( j ), server.timeLimit() );
    config.writeEntry( prefix + QString::fromLatin1( "SizeLimit%1" ).arg( j ), server.sizeLimit() );
    config.writeEntry( prefix + QString::fromLatin1( "PageSize%1" ).arg( j ), server.pageSize() );
    config.writeEntry( prefix + QString::fromLatin1( "Version%1" ).arg( j ), server.version() );
    QString tmp;
    switch ( server.security() ) {
    case KLDAP::LdapServer::TLS:
        tmp = "TLS";
        break;
    case KLDAP::LdapServer::SSL:
        tmp = "SSL";
        break;
    default:
        tmp = "None";
    }
    config.writeEntry( prefix + QString::fromLatin1( "Security%1" ).arg( j ), tmp );
    switch ( server.auth() ) {
    case KLDAP::LdapServer::Simple:
        tmp = "Simple";
        break;
    case KLDAP::LdapServer::SSL:
        tmp = "SASL";
        break;
    default:
        tmp = "Anonymous";
    }
    config.writeEntry( prefix + QString::fromLatin1( "Auth%1" ).arg( j ), tmp );
    config.writeEntry( prefix + QString::fromLatin1( "Mech%1" ).arg( j ), server.mech() );
}


#include "ldapclientsearchconfig.moc"
