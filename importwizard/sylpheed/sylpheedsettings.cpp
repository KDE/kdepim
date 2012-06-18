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
#include "mailcommon/mailutil.h"

#include <kpimidentities/identity.h>
#include <kpimidentities/signature.h>

#include <KConfig>
#include <KConfigGroup>

#include <QRegExp>
#include <QStringList>
#include <QFile>

SylpheedSettings::SylpheedSettings( const QString& filename, const QString& path, ImportWizard *parent )
    :AbstractSettings( parent )
{
  bool checkMailOnStartup = true;
  const QString sylpheedrc = path + QLatin1String("/sylpheedrc");
  if(QFile( sylpheedrc ).exists()) {
    KConfig configCommon( sylpheedrc );
    if(configCommon.hasGroup("Common")) {
      KConfigGroup common = configCommon.group("Common");
      checkMailOnStartup = ( common.readEntry("check_on_startup",1) == 1 );
      readGlobalSettings(common);
    }
  }
  KConfig config( filename );
  const QStringList accountList = config.groupList().filter( QRegExp( "Account: \\d+" ) );
  const QStringList::const_iterator end( accountList.constEnd() );
  for ( QStringList::const_iterator it = accountList.constBegin(); it!=end; ++it )
  {
    KConfigGroup group = config.group( *it );
    readAccount( group, checkMailOnStartup );
    readIdentity( group );
  }
  const QString customheaderrc = path + QLatin1String("/customheaderrc");
  QFile customHeaderFile(customheaderrc);
  if(customHeaderFile.exists()) {
    if ( !customHeaderFile.open( QIODevice::ReadOnly ) ) {
      kDebug()<<" We can't open file"<<customheaderrc;
    } else {
      readCustomHeader(&customHeaderFile);
    }
  }
}

SylpheedSettings::~SylpheedSettings()
{
}

void SylpheedSettings::readCustomHeader(QFile *customHeaderFile)
{
  QTextStream stream(customHeaderFile);
  QMap<QString, QString> header;
  while ( !stream.atEnd() ) {
    const QString line = stream.readLine();
    QStringList lst = line.split(QLatin1Char(':'));
    if(lst.count() == 3) {
      QString str = lst.at(2);
      str.remove(0,1);
      header.insert(lst.at(1),str);
    }
  }
  if(!header.isEmpty()) {
    //TODO
  }
}

void SylpheedSettings::readGlobalSettings(const KConfigGroup& group)
{
  const bool showTrayIcon = (group.readEntry("show_trayicon", 0) == 1 );
  addKmailConfig(QLatin1String("General"), QLatin1String("SystemTrayEnabled"), showTrayIcon);

  const bool cleanTrashOnExit = (group.readEntry("clean_trash_on_exit", 0) == 1 );
  addKmailConfig(QLatin1String("General"), QLatin1String("empty-trash-on-exit"), cleanTrashOnExit);

  const bool alwaysMarkReadOnShowMsg = (group.readEntry("always_mark_read_on_show_msg", 0) == 1 );
  if(alwaysMarkReadOnShowMsg) {
    addKmailConfig(QLatin1String("Behaviour"), QLatin1String("DelayedMarkAsRead"), true);
    addKmailConfig(QLatin1String("Behaviour"), QLatin1String("DelayedMarkTime"), 0);
  }

  if(group.readEntry("enable_autosave", 0) == 1 ) {
    const int autosaveInterval = group.readEntry("autosave_interval",5);
    addKmailConfig(QLatin1String("Composer"), QLatin1String("autosave"), autosaveInterval);
  }
  const bool checkAttach = (group.readEntry("check_attach", 0) == 1 );
  addKmailConfig(QLatin1String("Composer"), QLatin1String("showForgottenAttachmentWarning"), checkAttach);

  const QString attachStr = group.readEntry("check_attach_str");
  if(!attachStr.isEmpty()) {
    addKmailConfig(QLatin1String("Composer"), QLatin1String("attachment-keywords"), attachStr);
  }

  const int lineWrap = group.readEntry("linewrap_length", 80);
  addKmailConfig(QLatin1String("Composer"), QLatin1String("break-at"), lineWrap);
  addKmailConfig(QLatin1String("Composer"), QLatin1String("word-wrap"), true);

  const bool enableColor = group.readEntry("enable_color", false);
  if(enableColor) {
    const int colorLevel1 = group.readEntry("quote_level1_color", -1);
    if(colorLevel1!=-1) {
      //[Reader]  QuotedText1
    }
    const int colorLevel2 = group.readEntry("quote_level2_color", -1);
    if(colorLevel2!=-1) {
      //[Reader]  QuotedText2

    }
    const int colorLevel3 = group.readEntry("quote_level3_color", -1);
    if(colorLevel3!=-1) {
      //[Reader]  QuotedText3

    }

  }
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
    kDebug()<<" signature type unknow :"<<signatureType;
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

void SylpheedSettings::readPop3Account( const KConfigGroup& accountConfig, bool checkMailOnStartup )
{
  QMap<QString, QVariant> settings;
  const QString host = accountConfig.readEntry("receive_server");
  settings.insert( QLatin1String( "Host" ), host );
  
  const QString name = accountConfig.readEntry( QLatin1String( "name" ) );
  const QString inbox = MailCommon::Util::convertFolderPathToCollectionStr(accountConfig.readEntry(QLatin1String("inbox")));
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
        kDebug()<<" unknown ssl_pop value "<<sslPop;
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

  const QString agentIdentifyName = AbstractBase::createResource( "akonadi_pop3_resource", name, settings );
  addCheckMailOnStartup(agentIdentifyName,checkMailOnStartup);
}

void SylpheedSettings::readImapAccount( const KConfigGroup& accountConfig, bool checkMailOnStartup )
{
  QMap<QString, QVariant> settings;
  const QString name = accountConfig.readEntry( QLatin1String( "name" ) );
  const int sslimap = accountConfig.readEntry( QLatin1String( "ssl_imap" ), 0);
  switch(sslimap) {
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
    kDebug()<<" sslimap unknown "<<sslimap;
    break;
  }

  int port = 0;
  if ( readConfig( QLatin1String( "imap_port" ), accountConfig, port, true ) )
    settings.insert( QLatin1String( "ImapPort" ), port );

  QString trashFolder;
  if ( readConfig( QLatin1String( "trash_folder" ), accountConfig, trashFolder, false ) )
    settings.insert( QLatin1String( "TrashCollection" ), MailCommon::Util::convertFolderPathToCollectionId( trashFolder ) );

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
      kDebug()<<" imap auth unknown "<<auth;
      break;
  }
  const QString password = accountConfig.readEntry( QLatin1String( "password" ) );
  settings.insert( QLatin1String( "Password" ), password );

  const QString agentIdentifyName = AbstractBase::createResource( "akonadi_imap_resource", name,settings );
  addCheckMailOnStartup(agentIdentifyName,checkMailOnStartup);
}


void SylpheedSettings::readAccount(const KConfigGroup& accountConfig , bool checkMailOnStartup)
{
  if ( accountConfig.hasKey( QLatin1String( "protocol" ) ) )
  {
    const int protocol = accountConfig.readEntry( QLatin1String( "protocol" ), 0 );
    switch( protocol )
    {
      case 0:
        readPop3Account( accountConfig, checkMailOnStartup );
        break;
      case 3:
        //imap
        readImapAccount(accountConfig, checkMailOnStartup);
        break;
      case 4:
        kDebug()<<" Add it when nntp resource will implemented";
        //news
        break;
      case 5:
        //local
        break;
      default:
        kDebug()<<" protocol not defined"<<protocol;
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
    identity->setDrafts(MailCommon::Util::convertFolderPathToCollectionStr(value));

  if ( readConfig( QLatin1String("sent_folder") , accountConfig, value, false ) )
    identity->setFcc(MailCommon::Util::convertFolderPathToCollectionStr(value));

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
        kDebug()<<" smtp authentification unknown :"<<authMethod;
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
      kDebug()<<" smtp ssl config unknown :"<<sslSmtp;
        
    }
    QString domainName;
    if ( readConfig( QLatin1String( "domain" ), accountConfig, domainName, false ) )
      mt->setLocalHostname( domainName );

    storeTransport( mt, true );
    return QString::number(mt->id());
  }
  return QString();
}
