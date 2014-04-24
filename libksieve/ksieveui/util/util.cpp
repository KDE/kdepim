/*******************************************************************************
**
** Filename   : util
** Created on : 03 April, 2005
** Copyright  : (c) 2005 Till Adam
** Email      : <adam@kde.org>
**
*******************************************************************************/

/*******************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   It is distributed in the hope that it will be useful, but
**   WITHOUT ANY WARRANTY; without even the implied warranty of
**   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**   General Public License for more details.
**
**   You should have received a copy of the GNU General Public License
**   along with this program; if not, write to the Free Software
**   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
**   In addition, as a special exception, the copyright holders give
**   permission to link the code of this program with any edition of
**   the Qt library by Trolltech AS, Norway (or with modified versions
**   of Qt that use the same license as Qt), and distribute linked
**   combinations including the two.  You must obey the GNU General
**   Public License in all respects for all of the code used other than
**   Qt.  If you modify this file, you may extend this exception to
**   your version of the file, but you are not obligated to do so.  If
**   you do not wish to do so, delete this exception statement from
**   your version.
**
*******************************************************************************/


#include "util.h"
#include "pimcommon/util/pimutil.h"
#include "imapresourcesettings.h"
#include "sieve-vacation.h"

#include <agentmanager.h>
#include <kimap/loginjob.h>
#include <kmime/kmime_message.h>
#include <mailtransport/transport.h>

using namespace KSieveUi;

KUrl KSieveUi::Util::findSieveUrlForAccount( const QString &identifier )
{
    QScopedPointer<OrgKdeAkonadiImapSettingsInterface> interface( PimCommon::Util::createImapSettingsInterface(identifier) );

    if ( !interface->sieveSupport() )
        return KUrl();

    if ( interface->sieveReuseConfig() ) {
        // assemble Sieve url from the settings of the account:
        KUrl u;
        u.setScheme( QLatin1String("sieve") );
        QString server;
        QDBusReply<QString> reply = interface->imapServer();
        if ( reply.isValid() ) {
            server = reply;
            server = server.section( QLatin1Char(':'), 0, 0 );
        } else {
            return KUrl();
        }
        u.setHost( server );
        u.setUserName( interface->userName() );

        QDBusInterface resourceSettings( QLatin1String( "org.freedesktop.Akonadi.Resource." ) + identifier, QLatin1String("/Settings"), QLatin1String("org.kde.Akonadi.Imap.Wallet") );

        QString pwd;
        QDBusReply<QString> replyPass = resourceSettings.call( QLatin1String("password") );
        if ( replyPass.isValid() ) {
            pwd = replyPass;
        }
        u.setPassword( pwd );
        u.setPort( interface->sievePort() );
        QString authStr;
        switch( interface->authentication() ) {
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
        const QString resultSafety = interface->safety();
        if ( resultSafety == QLatin1String("None"))
            u.addQueryItem( QLatin1String("x-allow-unencrypted"), QLatin1String("true") );
        u.setFileName( interface->sieveVacationFilename() );
        return u;
    } else {
        KUrl u( interface->sieveAlternateUrl() );
        const QString resultSafety = interface->safety();
        if ( u.protocol().toLower() == QLatin1String("sieve") && ( resultSafety ==  QLatin1String("None") ) && u.queryItem(QLatin1String("x-allow-unencrypted")).isEmpty() )
            u.addQueryItem( QLatin1String("x-allow-unencrypted"), QLatin1String("true") );

        const QString resultCustomAuthentication = interface->sieveCustomAuthentification();
        if (resultCustomAuthentication == QLatin1String("ImapUserPassword")) {
            u.setUserName( interface->userName() );
            QDBusInterface resourceSettings( QLatin1String( "org.freedesktop.Akonadi.Resource." ) + identifier, QLatin1String("/Settings"), QLatin1String("org.kde.Akonadi.Imap.Wallet") );
            QString pwd;
            QDBusReply<QString> replyPass = resourceSettings.call( QLatin1String("password") );
            if ( replyPass.isValid() ) {
                pwd = replyPass;
            }
            u.setPassword( pwd );
        } else if (resultCustomAuthentication == QLatin1String("CustomUserPassword")) {
            QDBusInterface resourceSettings( QLatin1String( "org.freedesktop.Akonadi.Resource." ) + identifier, QLatin1String("/Settings"), QLatin1String("org.kde.Akonadi.Imap.Wallet") );
            QString pwd;
            QDBusReply<QString> replyPass = resourceSettings.call( QLatin1String("sieveCustomPassword") );
            if ( replyPass.isValid() ) {
                pwd = replyPass;
            }
            u.setPassword( pwd );
            u.setUserName( interface->sieveCustomUsername() );
        }
        u.setFileName( interface->sieveVacationFilename() );
        return u;
    }
}

Akonadi::AgentInstance::List KSieveUi::Util::imapAgentInstances()
{
    Akonadi::AgentInstance::List relevantInstances;
    foreach ( const Akonadi::AgentInstance &instance, Akonadi::AgentManager::self()->instances() ) {
        if ( instance.type().mimeTypes().contains( KMime::Message::mimeType() ) &&
             instance.type().capabilities().contains( QLatin1String("Resource") ) &&
             !instance.type().capabilities().contains( QLatin1String("Virtual") ) ) {

            if ( instance.identifier().contains( IMAP_RESOURCE_IDENTIFIER ) )
                relevantInstances << instance;
        }
    }
    return relevantInstances;
}

bool KSieveUi::Util::checkOutOfOfficeOnStartup()
{
    return VacationSettings::self()->checkOutOfOfficeOnStartup();
}

bool KSieveUi::Util::allowOutOfOfficeSettings()
{
    return VacationSettings::self()->allowOutOfOfficeSettings();
}
