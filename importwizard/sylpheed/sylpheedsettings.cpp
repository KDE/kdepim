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

#include "sylpheedsettings.h"
#include <mailtransport/transportmanager.h>

#include <kpimidentities/identity.h>
#include <kpimidentities/signature.h>

#include <KConfig>
#include <KConfigGroup>

#include <QRegExp>
#include <QStringList>

SylpheedSettings::SylpheedSettings( const QString& filename, ImportWizard *parent )
    :AbstractSettings( parent )
{
  KConfig config( filename );
  const QStringList accountList = config.groupList().filter( QRegExp( "Account: \\d+" ) );
  const QStringList::const_iterator end( accountList.constEnd() );
  for ( QStringList::const_iterator it = accountList.constBegin(); it!=end; ++it )
  {
    KConfigGroup group = config.group( *it );
    readAccount( group );
    readIdentity( group );
  }
}

SylpheedSettings::~SylpheedSettings()
{
}

void SylpheedSettings::readSignature( const KConfigGroup& accountConfig, KPIMIdentities::Identity* identity )
{
  KPIMIdentities::Signature signature;
  const int signatureType = accountConfig.readEntry("signature_type", 0 );
  switch( signatureType ) {
  case 0: //File
    signature.setType( KPIMIdentities::Signature::FromFile );
    signature.setUrl( accountConfig.readEntry("signature_path" ),false );
    break;
  case 1: //Output
    signature.setType( KPIMIdentities::Signature::FromCommand );
    signature.setUrl( accountConfig.readEntry("signature_path" ),true );
    break;
  case 2: //Text
    signature.setType( KPIMIdentities::Signature::Inlined );
    signature.setText( accountConfig.readEntry("signature_text" ) );
    break;
  default:
    qDebug()<<" signature type unknow :"<<signatureType;
  }
  //TODO  const bool signatureBeforeQuote = ( accountConfig.readEntry( "signature_before_quote", 0 ) == 1 ); not implemented in kmail

  identity->setSignature( signature );
}

bool SylpheedSettings::readConfig( const QString& key, const KConfigGroup& accountConfig, int& value, bool remove_underscore )
{
  QString cleanedKey( key );
  if ( remove_underscore )
    cleanedKey.remove( QLatin1Char( '_' ) );
  const QString useKey = QLatin1String( "set_" )+ cleanedKey;
  if ( accountConfig.hasKey( useKey ) && ( accountConfig.readEntry( useKey, 0 ) == 1 ) ) {
    value = accountConfig.readEntry( key,0 );
    return true;
  }
  return false;
}


bool SylpheedSettings::readConfig( const QString& key, const KConfigGroup& accountConfig, QString& value, bool remove_underscore )
{
  QString cleanedKey( key );
  if ( remove_underscore )
    cleanedKey.remove( QLatin1Char( '_' ) );
  const QString useKey = QLatin1String( "set_" )+ cleanedKey;
  if ( accountConfig.hasKey( useKey ) && ( accountConfig.readEntry( useKey, 0 ) == 1 ) ) {
    value = accountConfig.readEntry( key );
    return true;
  }
  return false;
}

void SylpheedSettings::readPop3Account( const KConfigGroup& accountConfig )
{
  QMap<QString, QVariant> settings;
  const QString host = accountConfig.readEntry("receive_server");
  settings.insert( QLatin1String( "Host" ), host );
  
  const QString name = accountConfig.readEntry( QLatin1String( "name" ) );
  const QString inbox = adaptFolder(accountConfig.readEntry(QLatin1String("inbox")));
  settings.insert(QLatin1String("TargetCollection"), inbox);
  int port = 0;
  if ( readConfig( QLatin1String( "pop_port" ), accountConfig, port, true ) )
    settings.insert( QLatin1String( "Port" ), port );
  if ( accountConfig.hasKey( QLatin1String( "ssl_pop" ) ) ) {
    const int sslPop = accountConfig.readEntry( QLatin1String( "ssl_pop" ), 0 );
    switch(sslPop) {
      case 0:
        //Nothing
        break;
      case 1:
        settings.insert( QLatin1String( "UseSSL" ), true );
        break;
      case 2:
        settings.insert( QLatin1String( "UseTLS" ), true );
        break;
      default:
        qDebug()<<" unknown ssl_pop value "<<sslPop;
    }
  }
  if ( accountConfig.hasKey( QLatin1String( "remove_mail" ) ) ){
    const bool removeMail = (accountConfig.readEntry( QLatin1String( "remove_mail" ), 1)==1);
    settings.insert(QLatin1String("LeaveOnServer"),removeMail);
  }

  if ( accountConfig.hasKey( QLatin1String( "message_leave_time" ) ) ){
    settings.insert( QLatin1String( "LeaveOnServerDays" ), accountConfig.readEntry( QLatin1String( "message_leave_time" ) ) );
  }
  const QString user = accountConfig.readEntry( QLatin1String( "user_id" ) );
  settings.insert( QLatin1String( "Login" ), user );

  const QString password = accountConfig.readEntry( QLatin1String( "password" ) );
  settings.insert( QLatin1String( "Password" ), password );
  
  //use_apop_auth
  if ( accountConfig.hasKey( QLatin1String( "use_apop_auth" ) ) ){
    const bool useApop = (accountConfig.readEntry( QLatin1String( "use_apop_auth" ), 1)==1);
    if(useApop) {
      settings.insert(QLatin1String( "AuthenticationMethod" ), MailTransport::Transport::EnumAuthenticationType::APOP);
    }
  }


  createResource( "akonadi_pop3_resource", name, settings );
}

void SylpheedSettings::readImapAccount( const KConfigGroup& accountConfig )
{
  QMap<QString, QVariant> settings;
  const QString name = accountConfig.readEntry( QLatin1String( "name" ) );


  const int sslimap = accountConfig.readEntry( QLatin1String( "ssl_imap" ), 0);
  switch(sslimap) {
    //TODO
  case 0:
    //None
    settings.insert( QLatin1String( "Safety" ), QLatin1String( "NONE" ) );
    break;
  case 1:
    //SSL
    settings.insert( QLatin1String( "Safety" ), QLatin1String( "SSL" ) );
    break;
  case 2:
    settings.insert( QLatin1String( "Safety" ), QLatin1String( "STARTTLS" ) );
    //TLS
  default:
    qDebug()<<" sslimap unknown "<<sslimap;
    break;
  }

  int port = 0;
  if ( readConfig( QLatin1String( "imap_port" ), accountConfig, port, true ) )
    settings.insert( QLatin1String( "ImapPort" ), port );

  QString trashFolder;
  if ( readConfig( QLatin1String( "trash_folder" ), accountConfig, trashFolder, false ) )
    settings.insert( QLatin1String( "TrashCollection" ), adaptFolderId( trashFolder ) );

  const int auth = accountConfig.readEntry(QLatin1String("imap_auth_method"),0);
  switch(auth) {
    case 0:
      break;
    case 1: //Login
      settings.insert(QLatin1String("Authentication"), MailTransport::Transport::EnumAuthenticationType::LOGIN);
      break;
    case 2: //Cram-md5
      settings.insert(QLatin1String("Authentication"),MailTransport::Transport::EnumAuthenticationType::CRAM_MD5);
      break;
    case 4: //Plain
      settings.insert(QLatin1String("Authentication"),MailTransport::Transport::EnumAuthenticationType::PLAIN);
    default:
      qDebug()<<" imap auth unknown "<<auth;
      break;
  }
  const QString password = accountConfig.readEntry( QLatin1String( "password" ) );
  settings.insert( QLatin1String( "Password" ), password );

  createResource( "akonadi_imap_resource", name,settings );
}


void SylpheedSettings::readAccount( const KConfigGroup& accountConfig )
{
  if ( accountConfig.hasKey( QLatin1String( "protocol" ) ) )
  {
    const int protocol = accountConfig.readEntry( QLatin1String( "protocol" ), 0 );
    switch( protocol )
    {
      case 0:
        readPop3Account( accountConfig );
        break;
      case 3:
        //imap
        readImapAccount(accountConfig);
        break;
      case 4:
        qDebug()<<" Add it when nntp resource will implemented";
        //news
        break;
      case 5:
        //local
        break;
      default:
        qDebug()<<" protocol not defined"<<protocol;
    }
  }  
}
  
void SylpheedSettings::readIdentity( const KConfigGroup& accountConfig )
{
  KPIMIdentities::Identity* identity  = createIdentity();
  const QString name = accountConfig.readEntry( QLatin1String( "name" ) );
  identity->setFullName( name );
  identity->setIdentityName( name );
  const QString organization = accountConfig.readEntry( QLatin1String( "organization" ), QString() );
  identity->setOrganization(organization);
  const QString email = accountConfig.readEntry( QLatin1String( "address" ) );
  identity->setPrimaryEmailAddress(email);

  QString value;
  if ( readConfig( QLatin1String("auto_bcc") , accountConfig, value, true ) )
    identity->setBcc(value);
  if ( readConfig( QLatin1String("auto_cc") , accountConfig, value, true ) )
    identity->setCc(value);
  if ( readConfig( QLatin1String("auto_replyto") , accountConfig, value, true ) )
    identity->setReplyToAddr(value);
  
  if ( readConfig( QLatin1String("daft_folder") , accountConfig, value, false ) )
    identity->setDrafts(adaptFolder(value));

  if ( readConfig( QLatin1String("sent_folder") , accountConfig, value, false ) )
    identity->setFcc(adaptFolder(value));

  const QString transportId = readTransport(accountConfig);
  if(!transportId.isEmpty())
  {
    identity->setTransport(transportId);
  }
  readSignature( accountConfig, identity );
  storeIdentity(identity);
}
  
QString SylpheedSettings::readTransport( const KConfigGroup& accountConfig )
{
  const QString smtpserver = accountConfig.readEntry("smtp_server");
  
  if(!smtpserver.isEmpty()) {
    MailTransport::Transport *mt = createTransport();
    mt->setName( smtpserver );
    mt->setHost(smtpserver);
    int port = 0;
    if ( readConfig( QLatin1String( "smtp_port" ), accountConfig, port, true ) )
      mt->setPort( port );
    const QString user = accountConfig.readEntry( QLatin1String( "smtp_user_id" ) );
    
    if ( !user.isEmpty() ) {
      mt->setUserName( user );
      mt->setRequiresAuthentication( true );
    }
    const QString password = accountConfig.readEntry( QLatin1String( "smtp_password" ) );
    if ( !password.isEmpty() ) {
      mt->setStorePassword( true );
      mt->setPassword( password );
    }
    if ( accountConfig.readEntry( QLatin1String( "use_smtp_auth" ), 0 )==1 ) {
      const int authMethod = accountConfig.readEntry( QLatin1String( "smtp_auth_method" ), 0 );
      switch( authMethod ) {
      case 0: //Automatic:
        mt->setAuthenticationType(MailTransport::Transport::EnumAuthenticationType::PLAIN); //????
        break;
      case 1: //Login
        mt->setAuthenticationType(MailTransport::Transport::EnumAuthenticationType::LOGIN);
        break;
      case 2: //Cram-MD5
        mt->setAuthenticationType(MailTransport::Transport::EnumAuthenticationType::CRAM_MD5);
        break;
      case 8: //Plain
        mt->setAuthenticationType(MailTransport::Transport::EnumAuthenticationType::PLAIN);
        break;
      default:
        qDebug()<<" smtp authentification unknown :"<<authMethod;
      }
    }
    const int sslSmtp = accountConfig.readEntry( QLatin1String( "ssl_smtp" ), 0 );
    switch( sslSmtp ) {
    case 0:
      mt->setEncryption( MailTransport::Transport::EnumEncryption::None ); 
      break;
    case 1:
      mt->setEncryption( MailTransport::Transport::EnumEncryption::SSL );
      break;
    case 2:
      mt->setEncryption( MailTransport::Transport::EnumEncryption::TLS );
      break;
    default:
      qDebug()<<" smtp ssl config unknown :"<<sslSmtp;
        
    }
    QString domainName;
    if ( readConfig( QLatin1String( "domain" ), accountConfig, domainName, false ) )
      mt->setLocalHostname( domainName );

    storeTransport( mt, true );
    return QString::number(mt->id()); //TODO verify
  }
  return QString();
}
