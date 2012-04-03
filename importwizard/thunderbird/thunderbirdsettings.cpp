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

#include "thunderbirdsettings.h"
#include <mailtransport/transportmanager.h>

#include <kpimidentities/identity.h>
#include <kpimidentities/signature.h>


#include <QTextStream>
#include <QStringList>
#include <QFile>
#include <QDebug>

ThunderbirdSettings::ThunderbirdSettings( const QString& filename, ImportWizard *parent )
    :AbstractSettings( parent )
{
  QFile file(filename);
  if ( !file.open( QIODevice::ReadOnly ) ) {
    kDebug()<<" We can't open file"<<filename;
    return;
  }
  QTextStream stream(&file);
  while ( !stream.atEnd() ) {
    const QString line = stream.readLine();
    if(line.startsWith(QLatin1String("user_pref"))) {
      if(line.contains(QLatin1String("mail.smtpserver")) ||
         line.contains(QLatin1String("mail.server.") ) ||
         line.contains(QLatin1String("mail.identity.")) ||
         line.contains(QLatin1String("mail.account.")) ||
         line.contains( QLatin1String( "mail.accountmanager." ) ) ) {
        insertIntoMap( line );
      }
    }
  }
  const QString mailAccountPreference = mHashConfig.value( QLatin1String( "mail.accountmanager.accounts" ) ).toString();
  if ( mailAccountPreference.isEmpty() )
    return;
  mAccountList = mailAccountPreference.split( QLatin1Char( ',' ) );
  readTransport();
  readAccount();
}

ThunderbirdSettings::~ThunderbirdSettings()
{
}

void ThunderbirdSettings::addAuth(QMap<QString, QVariant>& settings, const QString & argument, const QString &accountName )
{
  //TODO
  bool found = false;
  const int authMethod = mHashConfig.value( accountName + QLatin1String( ".authMethod" ) ).toInt(&found);
  if( found ) {
    switch( authMethod ) {
    case 0:
      break;
    default:
      qDebug()<<" ThunderbirdSettings::addAuth unknown :"<<authMethod;
      break;
    }
  }
  
#if 0
    if(authMethod==QLatin1String("PLAIN")) {
      settings.insert( argument, MailTransport::Transport::EnumAuthenticationType::PLAIN );
    } else if(authMethod==QLatin1String("NTLM")) {
      settings.insert( argument, MailTransport::Transport::EnumAuthenticationType::NTLM );
    } else if(authMethod==QLatin1String("DIGEST-MD5")) {
      settings.insert( argument, MailTransport::Transport::EnumAuthenticationType::DIGEST_MD5 );
    } else if(authMethod==QLatin1String("CRAM-MD5")) {
      settings.insert( argument, MailTransport::Transport::EnumAuthenticationType::CRAM_MD5 );
    } else if(authMethod==QLatin1String("LOGIN")) {
      settings.insert( argument, MailTransport::Transport::EnumAuthenticationType::LOGIN );
    } else if(authMethod==QLatin1String("POPB4SMTP")) {
      settings.insert( argument, MailTransport::Transport::EnumAuthenticationType::APOP ); //????
    } else {
      qDebug()<<" smtp auth method unknown "<<authMethod;
    }
#endif    
}

void ThunderbirdSettings::readAccount()
{
  Q_FOREACH( const QString&account, mAccountList )
  {
    const QString serverName = mHashConfig.value( QString::fromLatin1( "mail.account.%1" ).arg( account ) + QLatin1String( ".server" ) ).toString();
    const QString accountName = QString::fromLatin1( "mail.server.%1" ).arg( serverName );
    const QString host = mHashConfig.value( accountName + QLatin1String( ".hostname" ) ).toString();
    const QString userName = mHashConfig.value( accountName + QLatin1String( ".userName" ) ).toString();
    const QString name = mHashConfig.value( accountName + QLatin1String( ".name" ) ).toString();

    const QString type = mHashConfig.value( accountName + QLatin1String( ".type" ) ).toString();
    
    bool found = false;
    if( type == QLatin1String("imap")) {
      QMap<QString, QVariant> settings;
      settings.insert(QLatin1String("ImapServer"),serverName);
      settings.insert(QLatin1String("UserName"),userName);
      found = false;
      const int port = mHashConfig.value( accountName + QLatin1String( ".port" ) ).toInt( &found);
      if ( found ) {
        settings.insert( QLatin1String( "ImapPort" ), port );
      }
      addAuth( settings, QLatin1String( "Authentication" ), account );
      const QString offline = accountName + QLatin1String( ".offline_download" );
      if ( mHashConfig.contains( offline ) ) {
        const bool offlineStatus = mHashConfig.value( offline ).toBool();
        if ( offlineStatus ) {
          settings.insert( QLatin1String( "DisconnectedModeEnabled" ), offlineStatus );
        }
      }
      createResource( "akonadi_imap_resource", name,settings );
    } else if( type == QLatin1String("pop3")) {
      QMap<QString, QVariant> settings;
      settings.insert( QLatin1String( "Host" ), host );
      settings.insert( QLatin1String( "Login" ), userName );

      found = false;
      const int numberDayToLeave = mHashConfig.value( accountName + QLatin1String( ".num_days_to_leave_on_server")).toInt(&found);
      if ( found ) {
        settings.insert(QLatin1String("LeaveOnServer"),numberDayToLeave);
      }
      
      found = false;
      const int port = mHashConfig.value( accountName + QLatin1String( ".port" ) ).toInt( &found);
      if ( found ) {
        settings.insert( QLatin1String( "Port" ), port );
      }
      addAuth( settings, QLatin1String( "AuthenticationMethod" ),account );
      
      createResource( "akonadi_pop3_resource", name, settings );
    } else if ( type == QLatin1String( "none" ) ) {
      //TODO
      qDebug()<<" account type none!";
    } else {
      qDebug()<<" type unknown : "<<type;
    }

    const QString identityConfig = QString::fromLatin1( "mail.account.%1" ).arg( account ) + QLatin1String( ".identities" );
    if ( mHashConfig.contains( identityConfig ) )
    {
      readIdentity(mHashConfig.value(identityConfig).toString() );
    }
  }
}

void ThunderbirdSettings::readTransport()
{
  const QString mailSmtpServer = mHashConfig.value( QLatin1String( "mail.smtpservers" ) ).toString();
  if ( mailSmtpServer.isEmpty() )
    return;
  QStringList smtpList = mailSmtpServer.split( QLatin1Char( ',' ) );
  const QString defaultSmtp = mHashConfig.value( QLatin1String( "mail.smtp.defaultserver" ) ).toString();
  Q_FOREACH( const QString &smtp, smtpList )
  {
    const QString smtpName = QString::fromLatin1( "mail.smtpserver.%1" ).arg( smtp );
    MailTransport::Transport *mt = createTransport();
    const QString name = mHashConfig.value( smtpName + QLatin1String( ".description" ) ).toString();
    mt->setName(name);
    const QString hostName = mHashConfig.value( smtpName + QLatin1String( ".hostname" ) ).toString();
    mt->setHost( hostName );
    
    const int port = mHashConfig.value( smtpName + QLatin1String( ".port" ) ).toInt();
    if ( port > 0 )
      mt->setPort( port );
    
    const int authMethod = mHashConfig.value( smtpName + QLatin1String( ".authMethod" ) ).toInt();
    switch(authMethod) {
      case 0:
        break;
      case 1: //No authentification
        mt->setAuthenticationType(MailTransport::Transport::EnumAuthenticationType::PLAIN); //????
        break;
      case 3: //Uncrypted password
        mt->setAuthenticationType(MailTransport::Transport::EnumAuthenticationType::CLEAR); //???
        break;
      case 4: //crypted password
        mt->setAuthenticationType(MailTransport::Transport::EnumAuthenticationType::LOGIN); //???
        break;
      case 5: //GSSAPI
        mt->setAuthenticationType(MailTransport::Transport::EnumAuthenticationType::GSSAPI);
        break;
      case 6: //NTLM
        mt->setAuthenticationType(MailTransport::Transport::EnumAuthenticationType::NTLM);
        break;
      default:
        qDebug()<<" authMethod unknown :"<<authMethod;
    }

    const int trySsl = mHashConfig.value( smtpName + QLatin1String( ".try_ssl" ) ).toInt();
    switch(trySsl) {
      case 0:
        mt->setEncryption( MailTransport::Transport::EnumEncryption::None );
        break;
      case 2:
        mt->setEncryption( MailTransport::Transport::EnumEncryption::TLS );
        break;
      case 3:
        mt->setEncryption( MailTransport::Transport::EnumEncryption::SSL );
        break;
      default:
        qDebug()<<" trySsl unknown :"<<trySsl;
    }

    const QString userName = mHashConfig.value( smtpName + QLatin1String( ".username" ) ).toString();
    if ( !userName.isEmpty() ) {
      mt->setUserName( userName );
      mt->setRequiresAuthentication( true );
    }

    storeTransport( mt, ( smtp == defaultSmtp ) );
    mHashSmtp.insert( smtp, QString::number( mt->id() ) ); 
  }
}

void ThunderbirdSettings::readIdentity( const QString& account )
{
  KPIMIdentities::Identity* newIdentity = createIdentity();
  const QString identity = QString::fromLatin1( "mail.identity.%1" ).arg( account );
  
  const QString fcc = mHashConfig.value( identity + QLatin1String( ".fcc_folder" ) ).toString();

  const QString smtpServer = mHashConfig.value( identity + QLatin1String( ".smtpServer" ) ).toString();
  if(!smtpServer.isEmpty() && mHashSmtp.contains(smtpServer))
  {
    newIdentity->setTransport(mHashSmtp.value(smtpServer));
  }

  const QString userEmail = mHashConfig.value( identity + QLatin1String( ".useremail" ) ).toString();
  newIdentity->setPrimaryEmailAddress(userEmail);

  const QString fullName = mHashConfig.value( identity + QLatin1String( ".fullName" ) ).toString();
  newIdentity->setFullName( fullName );

  const QString organization = mHashConfig.value(identity + QLatin1String(".organization")).toString();
  newIdentity->setOrganization(organization);

  bool doBcc = mHashConfig.value(identity + QLatin1String(".doBcc")).toBool();
  if(doBcc) {
    const QString bcc = mHashConfig.value(identity + QLatin1String(".doBccList")).toString();
  }

  bool doCc = mHashConfig.value(identity + QLatin1String(".doCc")).toBool();
  if(doCc) {
    const QString cc = mHashConfig.value(identity + QLatin1String(".doCcList")).toString();
  }

  const QString draft = adaptFolder(mHashConfig.value(identity + QLatin1String(".draft_folder")).toString());
  newIdentity->setDrafts(draft);

  const QString replyTo = mHashConfig.value(identity + QLatin1String( ".reply_to")).toString();
  newIdentity->setReplyToAddr( replyTo );

  KPIMIdentities::Signature signature;
  const bool signatureHtml = mHashConfig.value(identity + QLatin1String( ".htmlSigFormat" )).toBool();
  if(signatureHtml) {
      signature.setInlinedHtml( true );
  }

  const bool attachSignature = mHashConfig.value(identity + QLatin1String( ".attach_signature" )).toBool();
  if ( attachSignature ) {
    const QString fileSignature = mHashConfig.value(identity + QLatin1String( ".sig_file")).toString();
    signature.setType( KPIMIdentities::Signature::FromFile );
    signature.setUrl( fileSignature,false );
  }
  else {
    const QString textSignature = mHashConfig.value(identity + QLatin1String( ".htmlSigText" ) ).toString();
    signature.setType( KPIMIdentities::Signature::Inlined );
    signature.setText( textSignature );
  }


  if ( mHashConfig.contains( identity + QLatin1String( ".drafts_folder_picker_mode" ) ) )
  {
    const int useSpecificDraftFolder = mHashConfig.value(  identity + QLatin1String( ".drafts_folder_picker_mode" ) ).toInt();
    if ( useSpecificDraftFolder == 1 )
    {
      const QString draftFolder = adaptFolder( mHashConfig.value( identity + QLatin1String( ".draft_folder" ) ).toString() );
      newIdentity->setDrafts( draftFolder );
    }
  }

  if ( mHashConfig.contains( identity + QLatin1String( ".fcc_folder_picker_mode" ) ) )
  {
    const int useSpecificTemplateFolder = mHashConfig.value(  identity + QLatin1String( ".fcc_folder_picker_mode" ) ).toInt();
    if ( useSpecificTemplateFolder == 1 )
    {
      const QString templateFolder = adaptFolder( mHashConfig.value( identity + QLatin1String( ".fcc_folder" ) ).toString() );
      newIdentity->setTemplates( templateFolder );
    }
  }

  
  const QString attachVcardStr( identity + QLatin1String( ".attach_vcard" ) );
  if ( mHashConfig.contains( attachVcardStr ) ) {
    const bool attachVcard = mHashConfig.value( attachVcardStr ).toBool();
    if ( attachVcard ) {
      //TODO mail.identity.id1.escapedVCard
    }
  }
    
  newIdentity->setSignature( signature );

  storeIdentity(newIdentity);
}

void ThunderbirdSettings::insertIntoMap( const QString& line )
{
  QString newLine = line;
  newLine.remove( QLatin1String( "user_pref(\"" ) );
  newLine.remove( QLatin1String( ");" ) );
  const int pos = newLine.indexOf( QLatin1Char( ',' ) );
  QString key = newLine.left( pos );
  key.remove( key.length() -1, 1 );
  QString valueStr = newLine.right( newLine.length() - pos -2);
  if ( valueStr.at( 0 ) == QLatin1Char( '"' ) ) {
    valueStr.remove( 0, 1 );
    if ( valueStr.at( valueStr.length()-1 ) == QLatin1Char( '"' ) )
      valueStr.remove( valueStr.length()-1, 1 );
    //Store as String
    mHashConfig.insert( key, valueStr );
  } else {
    if ( valueStr == QLatin1String( "true" ) ) {
      bool b = true;
      mHashConfig.insert( key, b );
    } else if ( valueStr == QLatin1String( "false" ) ) {
      bool b = false;
      mHashConfig.insert( key, b );
    } else { 
      //Store as integer
      const int value = valueStr.toInt();
      mHashConfig.insert( key, value );
    }
  }
}
