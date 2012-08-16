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

#include "operasettings.h"

#include <mailtransport/transportmanager.h>
#include "mailcommon/mailutil.h"

#include <kpimidentities/identity.h>
#include <kpimidentities/signature.h>


#include <KConfig>
#include <KConfigGroup>
#include <QFile>

OperaSettings::OperaSettings(const QString &filename, ImportWizard *parent)
  :AbstractSettings( parent )
{
  if(QFile( filename ).exists()) {
    KConfig config( filename );
    KConfigGroup grp = config.group(QLatin1String("Accounts"));
    readGlobalAccount(grp);
    const QStringList accountList = config.groupList().filter( QRegExp( "Account\\d+" ) );
    const QStringList::const_iterator end( accountList.constEnd() );
    for ( QStringList::const_iterator it = accountList.constBegin(); it!=end; ++it )
    {
      KConfigGroup group = config.group( *it );
      readAccount( group );
      readTransport(group);
      readIdentity(group);
    }
  }
}

OperaSettings::~OperaSettings()
{

}

void OperaSettings::readAccount(const KConfigGroup &grp)
{
  const QString incomingProtocol = grp.readEntry(QLatin1String("Incoming Protocol"));
  const int port = grp.readEntry(QLatin1String("Incoming Port"), -1);

  const QString serverName = grp.readEntry(QLatin1String("Incoming Servername"));
  const QString userName = grp.readEntry(QLatin1String("Incoming Username"));

  const int secure = grp.readEntry(QLatin1String("Secure Connection In"),-1);


  const int pollInterval = grp.readEntry(QLatin1String("Poll Interval"),-1);

  const int authMethod = grp.readEntry(QLatin1String("Incoming Authentication Method"),-1);

  QString name; //FIXME

  QMap<QString, QVariant> settings;
  if(incomingProtocol == QLatin1String("IMAP")) {
      settings.insert(QLatin1String("ImapServer"),serverName);
      settings.insert(QLatin1String("UserName"),userName);
      if ( port != -1 ) {
        settings.insert( QLatin1String( "ImapPort" ), port );
      }
      if(secure == 1) {
        settings.insert( QLatin1String( "Safety" ), QLatin1String("STARTTLS") );
      } else if( secure == 0) {
        settings.insert( QLatin1String( "Safety" ), QLatin1String("None") );
      }

      if(pollInterval == 0) {
        settings.insert(QLatin1String("IntervalCheckEnabled"), false);
      } else {
        settings.insert(QLatin1String("IntervalCheckEnabled"), true);
        settings.insert(QLatin1String("IntervalCheckTime"),pollInterval);
      }

      const QString agentIdentifyName = AbstractBase::createResource( "akonadi_imap_resource", name,settings );
      //TODO
      //addCheckMailOnStartup(agentIdentifyName,loginAtStartup);
  } else if(incomingProtocol == QLatin1String("POP")) {
      settings.insert( QLatin1String( "Host" ), serverName );
      settings.insert( QLatin1String( "Login" ), userName );
      if ( port != -1 ) {
        settings.insert( QLatin1String( "Port" ), port );
      }
      const int delay = grp.readEntry(QLatin1String("Initial Poll Delay"),-1);

      if(pollInterval == 0) {
        settings.insert(QLatin1String("IntervalCheckEnabled"), false);
      } else {
        settings.insert(QLatin1String("IntervalCheckEnabled"), true);
        settings.insert(QLatin1String("IntervalCheckInterval"),pollInterval);
      }


      if(secure == 1)
        settings.insert( QLatin1String( "UseTLS" ), true );

      //TODO
      switch(authMethod) {
      case 0: //NONE
        settings.insert(QLatin1String( "AuthenticationMethod" ), MailTransport::Transport::EnumAuthenticationType::ANONYMOUS);
        break;
      case 1: //Clear Text
        settings.insert(QLatin1String( "AuthenticationMethod" ), MailTransport::Transport::EnumAuthenticationType::CLEAR); //Verify
        break;
      case 6: //APOP
        settings.insert(QLatin1String( "AuthenticationMethod" ), MailTransport::Transport::EnumAuthenticationType::APOP);
        break;
      case 10: //CRAM-MD5
        settings.insert(QLatin1String( "AuthenticationMethod" ), MailTransport::Transport::EnumAuthenticationType::CRAM_MD5);
        break;
      case 31: //Automatic
          settings.insert(QLatin1String( "AuthenticationMethod" ), MailTransport::Transport::EnumAuthenticationType::APOP); //TODO: verify
        break;
      default:
        qDebug()<<" unknown authentification method :"<<authMethod;
        break;
      }

      const QString agentIdentifyName = AbstractBase::createResource( "akonadi_pop3_resource", name, settings );
      //TODO
      //addCheckMailOnStartup(agentIdentifyName,loginAtStartup);
  } else {
      qDebug()<<" protocol unknown : "<<incomingProtocol;
  }
  //TODO
}

void OperaSettings::readTransport(const KConfigGroup &grp)
{
  const QString outgoingProtocol = grp.readEntry(QLatin1String("Outgoing Protocol"));
  if(outgoingProtocol == QLatin1String("SMTP")) {
      const int authMethod = grp.readEntry(QLatin1String("Outgoing Authentication Method"),-1);
      MailTransport::Transport *mt = createTransport();
      const int port = grp.readEntry(QLatin1String("Outgoing Port"), -1);
      const int secure = grp.readEntry(QLatin1String("Secure Connection Out"),-1);
      if(secure == 1) {
        mt->setEncryption( MailTransport::Transport::EnumEncryption::TLS );
      }
      if ( port > 0 )
        mt->setPort( port );

      const QString hostName = grp.readEntry(QLatin1String("Outgoing Servername"));
      mt->setHost( hostName );

      const QString userName = grp.readEntry(QLatin1String("Outgoing Username"));
      if(!userName.isEmpty())
          mt->setUserName( userName );

      const int outgoingTimeOut = grp.readEntry(QLatin1String("Outgoing Timeout"),-1); //TODO ?

      switch(authMethod) {
      case 0: //NONE
          break;
      case 2: //PLAIN
          mt->setAuthenticationType(MailTransport::Transport::EnumAuthenticationType::PLAIN);
          break;
      case 5: //LOGIN
          mt->setAuthenticationType(MailTransport::Transport::EnumAuthenticationType::LOGIN);
          break;
      case 10: //CRAM-MD5
          mt->setAuthenticationType(MailTransport::Transport::EnumAuthenticationType::CRAM_MD5);
          break;
      case 31: //Automatic
          mt->setAuthenticationType(MailTransport::Transport::EnumAuthenticationType::PLAIN); //Don't know... Verify
          break;
      default:
          qDebug()<<" authMethod unknown :"<<authMethod;
      }

      storeTransport( mt, /*( smtp == defaultSmtp )*/true ); //FIXME:
  }
}

void OperaSettings::readIdentity(const KConfigGroup &grp)
{
    KPIMIdentities::Identity* newIdentity = createIdentity();
    const QString cc = grp.readEntry(QLatin1String("Auto CC"));
    newIdentity->setCc( cc );

    const QString bcc = grp.readEntry(QLatin1String("Auto BCC"));
    newIdentity->setBcc( bcc );

    const QString replyTo = grp.readEntry(QLatin1String("Replyto"));
    if(!replyTo.isEmpty())
      newIdentity->setReplyToAddr( replyTo );

    const QString realName = grp.readEntry(QLatin1String("Real Name"));
    newIdentity->setFullName( realName );
    newIdentity->setIdentityName( realName );

    const QString email = grp.readEntry(QLatin1String("Real Name"));
    newIdentity->setPrimaryEmailAddress(email);

    const QString organization = grp.readEntry(QLatin1String("Organization"));
    if(!organization.isEmpty())
      newIdentity->setOrganization(organization);

    KPIMIdentities::Signature signature;
    const QString signatureFile = grp.readEntry(QLatin1String("Signature File"));
    if(!signatureFile.isEmpty()) {
        const int signatureHtml = grp.readEntry(QLatin1String("Signature is HTML"),-1);
        switch(signatureHtml) {
        case -1:
            break;
        case 0:
            signature.setInlinedHtml( false );
            signature.setType( KPIMIdentities::Signature::Inlined );
            break;
        case 1:
            signature.setInlinedHtml( true );
            signature.setType( KPIMIdentities::Signature::Inlined );
            break;
        default:
            qDebug()<<" pb with Signature is HTML "<<signatureHtml;
            break;
        }
        //TODO load file and add text directly.
        //For the moment we can't add a signature file + html => load and add in signature directly
        //signature.setText( textSignature );
    }

    newIdentity->setSignature( signature );
    storeIdentity(newIdentity);
}

void OperaSettings::readGlobalAccount(const KConfigGroup &grp)
{
  //TODO
}
