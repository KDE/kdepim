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
#include "mailcommon/mailutil.h"

#include <kpimidentities/identity.h>
#include <kpimidentities/signature.h>


#include <QTextStream>
#include <QStringList>
#include <QFile>
#include <KDebug>

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
         line.contains(QLatin1String("mail.accountmanager.")) ||
         line.contains(QLatin1String("mailnews."))||
         line.contains(QLatin1String("mail.compose."))||
         line.contains(QLatin1String("mail.spellcheck")) ||
         line.contains(QLatin1String("ldap_"))) {
        insertIntoMap( line );
      }
    } else {
        kDebug()<<" unstored line :"<<line;
    }
  }
  const QString mailAccountPreference = mHashConfig.value( QLatin1String( "mail.accountmanager.accounts" ) ).toString();
  if ( mailAccountPreference.isEmpty() )
    return;
  mAccountList = mailAccountPreference.split( QLatin1Char( ',' ) );
  readTransport();
  readAccount();
  readGlobalSettings();
  readLdapSettings();
}

ThunderbirdSettings::~ThunderbirdSettings()
{
}


void ThunderbirdSettings::readLdapSettings()
{
  //TODO: verify others variable
  //qDebug()<<" mLdapAccountList:"<<mLdapAccountList;
  Q_FOREACH(const QString& ldapAccountName, mLdapAccountList) {
    ldapStruct ldap;
    const QString ldapDescription = QString::fromLatin1("%1.description").arg(ldapAccountName);
    if(mHashConfig.contains(ldapDescription)) {
      ldap.description = mHashConfig.value(ldapDescription).toString();
    }
    const QString ldapAuthDn = QString::fromLatin1("%1.auth.dn").arg(ldapAccountName);
    if(mHashConfig.contains(ldapAuthDn)) {
      ldap.dn = mHashConfig.value(ldapAuthDn).toString();
    }
    const QString ldapAuthSaslMech = QString::fromLatin1("%1.auth.saslmech").arg(ldapAccountName);
    if(mHashConfig.contains(ldapAuthSaslMech)) {
      ldap.saslMech = mHashConfig.value(ldapAuthSaslMech).toString();
    }
    const QString ldapFilename = QString::fromLatin1("%1.filename").arg(ldapAccountName);
    if(mHashConfig.contains(ldapFilename)) {
      ldap.fileName = mHashConfig.value(ldapFilename).toString();
    }
    const QString ldapMaxHits = QString::fromLatin1("%1.maxHits").arg(ldapAccountName);
    if(mHashConfig.contains(ldapMaxHits)) {
      ldap.fileName = mHashConfig.value(ldapMaxHits).toInt();
    }
    const QString ldapUri = QString::fromLatin1("%1.uri").arg(ldapAccountName);
    if(mHashConfig.contains(ldapUri)) {
      ldap.ldapUrl = KUrl(mHashConfig.value(ldapUri).toString());
    }

    mergeLdap(ldap);
  }
}

void ThunderbirdSettings::mergeLdap(const ldapStruct &ldap)
{
  KSharedConfigPtr ldapConfig = KSharedConfig::openConfig( QLatin1String( "kabldaprc" ) );
  int numberOfLdapSelected = 0;
  KConfigGroup grp;
  if(ldapConfig->hasGroup(QLatin1String("LDAP"))) {
    grp = ldapConfig->group(QLatin1String("LDAP"));
    numberOfLdapSelected = grp.readEntry(QLatin1String("NumSelectedHosts"),0);
    grp.writeEntry(QLatin1String("NumSelectedHosts"),QString::number(numberOfLdapSelected+1));
  } else {
    grp = ldapConfig->group(QLatin1String("LDAP"));
    grp.writeEntry(QLatin1String("NumSelectedHosts"),QString::number(1));
  }
  const int port = ldap.ldapUrl.port();
  if(port!=-1)
    grp.writeEntry(QString::fromLatin1("SelectedPort%1").arg(numberOfLdapSelected),port);
  grp.writeEntry(QString::fromLatin1("SelectedPort%1").arg(numberOfLdapSelected),ldap.ldapUrl.host());
  grp.sync();
#if 0

    SelectedAuth0=Simple
    SelectedBase0=dc=kdab,dc=com
    SelectedBind0=uid=laurent,dc=kdab,dc=com
    SelectedHost0=mail.kdab.com
    SelectedMech0=DIGEST-MD5
    SelectedPageSize0=0
    SelectedPort0=636
    SelectedPwdBind0=
    SelectedSecurity0=SSL
    SelectedSizeLimit0=0
    SelectedTimeLimit0=0
    SelectedUser0=
    SelectedVersion0=3
#endif
}

void ThunderbirdSettings::readGlobalSettings()
{
  const QString markMessageReadStr = QLatin1String("mailnews.mark_message_read.delay");
  if(mHashConfig.contains(markMessageReadStr)) {
    const bool markMessageRead = mHashConfig.value(markMessageReadStr).toBool();
    addKmailConfig(QLatin1String("Behaviour"), QLatin1String("DelayedMarkAsRead"), markMessageRead);
  } else {
    //Default value
    addKmailConfig(QLatin1String("Behaviour"), QLatin1String("DelayedMarkAsRead"), true);
  }
  const QString markMessageReadIntervalStr = QLatin1String("mailnews.mark_message_read.delay.interval");
  if(mHashConfig.contains(markMessageReadIntervalStr)) {
    bool found = false;
    const int markMessageReadInterval = mHashConfig.value(markMessageReadIntervalStr).toInt(&found);
    if(found) {
      addKmailConfig(QLatin1String("Behaviour"), QLatin1String("DelayedMarkTime"), markMessageReadInterval);
    }
  } else {
    //Default 5 seconds
    addKmailConfig(QLatin1String("Behaviour"), QLatin1String("DelayedMarkTime"), 5);
  }

  const QString mailComposeAttachmentReminderStr = QLatin1String("mail.compose.attachment_reminder");
  if(mHashConfig.contains(mailComposeAttachmentReminderStr)) {
    const bool mailComposeAttachmentReminder = mHashConfig.value(mailComposeAttachmentReminderStr).toBool();
    addKmailConfig(QLatin1String("Composer"), QLatin1String("showForgottenAttachmentWarning"), mailComposeAttachmentReminder);
  } else {
    addKmailConfig(QLatin1String("Composer"), QLatin1String("showForgottenAttachmentWarning"), true);
  }

  const QString mailComposeAttachmentReminderKeywordsStr = QLatin1String("mail.compose.attachment_reminder_keywords");
  if(mHashConfig.contains(mailComposeAttachmentReminderKeywordsStr)) {
    const QString mailComposeAttachmentReminderKeywords = mHashConfig.value(mailComposeAttachmentReminderKeywordsStr).toString();
    addKmailConfig(QLatin1String("Composer"), QLatin1String("attachment-keywords"), mailComposeAttachmentReminderKeywords);
  } //not default value keep kmail use one default value

  const QString mailComposeAutosaveStr = QLatin1String("mail.compose.autosave");
  if(mHashConfig.contains(mailComposeAutosaveStr)) {
    const bool mailComposeAutosave = mHashConfig.value(mailComposeAutosaveStr).toBool();
    if(mailComposeAutosave) {
      const QString mailComposeAutosaveintervalStr = QLatin1String("mail.compose.autosaveinterval");
      if(mHashConfig.contains(mailComposeAutosaveintervalStr)) {
        bool found = false;
        const int mailComposeAutosaveinterval = mHashConfig.value(mailComposeAutosaveintervalStr).toInt(&found);
        if(found) {
          addKmailConfig(QLatin1String("Composer"), QLatin1String("autosave"), mailComposeAutosaveinterval);
        } else {
          addKmailConfig(QLatin1String("Composer"), QLatin1String("autosave"), 5);
        }
      } else {
        //Default value
        addKmailConfig(QLatin1String("Composer"), QLatin1String("autosave"), 5);
      }
    } else {
      //Don't autosave
      addKmailConfig(QLatin1String("Composer"), QLatin1String("autosave"), 0);
    }
  }

  const QString mailSpellCheckInlineStr = QLatin1String("mail.spellcheck.inline");
  if(mHashConfig.contains(mailSpellCheckInlineStr)) {
    const bool mailSpellCheckInline = mHashConfig.value(mailSpellCheckInlineStr).toBool();
    addKmailConfig(QLatin1String("Spelling"),QLatin1String("backgroundCheckerEnabled"),mailSpellCheckInline);
  } else {
    addKmailConfig(QLatin1String("Spelling"),QLatin1String("backgroundCheckerEnabled"),false);
  }
}

void ThunderbirdSettings::addAuth(QMap<QString, QVariant>& settings, const QString & argument, const QString &accountName )
{
  bool found = false;
  if ( mHashConfig.contains( accountName + QLatin1String( ".authMethod" ) ) ) {
    const int authMethod = mHashConfig.value( accountName + QLatin1String( ".authMethod" ) ).toInt(&found);
    if( found ) {
      switch( authMethod ) {
      case 0:
        break;
      case 4: //Encrypted password ???
        settings.insert( argument, MailTransport::Transport::EnumAuthenticationType::LOGIN ); //????
        kDebug()<<" authmethod == encrypt password";
        break;
      case 5: //GSSAPI
        settings.insert( argument, MailTransport::Transport::EnumAuthenticationType::GSSAPI );
        break;
      case 6: //NTLM
        settings.insert( argument, MailTransport::Transport::EnumAuthenticationType::NTLM );
        break;
      case 7: //TLS
        kDebug()<<" authmethod method == TLS"; //????
        break;
      default:
        kDebug()<<" ThunderbirdSettings::addAuth unknown :"<<authMethod;
        break;
      }
    }
  }
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
    //TODO use it ?
    const QString directory = mHashConfig.value( accountName + QLatin1String( ".directory" ) ).toString();

    const QString loginAtStartupStr = accountName + QLatin1String( ".login_at_startup" );
    bool loginAtStartup = true; //Default for thunderbird;
    if ( mHashConfig.contains( loginAtStartupStr ) ) {
      loginAtStartup = mHashConfig.value( loginAtStartupStr ).toBool();
    }
    bool found = false;
    if( type == QLatin1String("imap")) {
      QMap<QString, QVariant> settings;
      settings.insert(QLatin1String("ImapServer"),host);
      settings.insert(QLatin1String("UserName"),userName);
      const int port = mHashConfig.value( accountName + QLatin1String( ".port" ) ).toInt( &found);
      if ( found ) {
        settings.insert( QLatin1String( "ImapPort" ), port );
      }
      addAuth( settings, QLatin1String( "Authentication" ), accountName );
      const QString offline = accountName + QLatin1String( ".offline_download" );
      if ( mHashConfig.contains( offline ) ) {
        const bool offlineStatus = mHashConfig.value( offline ).toBool();
        if ( offlineStatus ) {
          settings.insert( QLatin1String( "DisconnectedModeEnabled" ), offlineStatus );
        }
      } else {
        //default value == true
        settings.insert( QLatin1String( "DisconnectedModeEnabled" ), true );
      }

      found = false;
      const int socketType = mHashConfig.value( accountName + QLatin1String( ".socketType" ) ).toInt( &found);
      if(found) {
        switch(socketType) {
          case 0:
            //None
            settings.insert( QLatin1String( "Safety" ), QLatin1String("None") );
            break;
          case 2:
            //STARTTLS
            settings.insert( QLatin1String( "Safety" ), QLatin1String("STARTTLS") );
            break;
          case 3:
            //SSL/TLS
            settings.insert( QLatin1String( "Safety" ), QLatin1String("SSL") );
            break;
          default:
            kDebug()<<" socketType "<<socketType;
        }
      }
      const QString checkNewMailStr = accountName + QLatin1String( ".check_new_mail" );
      if(mHashConfig.contains(checkNewMailStr)) {
        const bool checkNewMail = mHashConfig.value(checkNewMailStr).toBool();
        settings.insert(QLatin1String("IntervalCheckEnabled"), checkNewMail);
      }

      const QString checkTimeStr = accountName + QLatin1String( ".check_time" );
      if(mHashConfig.contains(checkTimeStr)) {
        found = false;
        const int checkTime = mHashConfig.value( checkTimeStr ).toInt( &found);
        if(found) {
          settings.insert(QLatin1String("IntervalCheckTime"),checkTime);
        }
      } else {
        //Default value from thunderbird
        settings.insert(QLatin1String("IntervalCheckTime"), 10 );
      }
      const QString trashFolderStr = accountName + QLatin1String( ".trash_folder_name" );
      if(mHashConfig.contains(trashFolderStr)) {
        settings.insert(QLatin1String("TrashCollection"),MailCommon::Util::convertFolderPathToCollectionId(mHashConfig.value(trashFolderStr).toString()));
      }

      const QString agentIdentifyName = AbstractBase::createResource( "akonadi_imap_resource", name,settings );
      addCheckMailOnStartup(agentIdentifyName,loginAtStartup);
    } else if( type == QLatin1String("pop3")) {
      QMap<QString, QVariant> settings;
      settings.insert( QLatin1String( "Host" ), host );
      settings.insert( QLatin1String( "Login" ), userName );

      const bool leaveOnServer = mHashConfig.value( accountName + QLatin1String( ".leave_on_server")).toBool();
      if(leaveOnServer) {
        settings.insert(QLatin1String("LeaveOnServer"),leaveOnServer);
      }

      found = false;
      const int numberDayToLeave = mHashConfig.value( accountName + QLatin1String( ".num_days_to_leave_on_server")).toInt(&found);
      if ( found ) {
        settings.insert(QLatin1String("LeaveOnServerDays"),numberDayToLeave);
      }
      
      found = false;
      const int port = mHashConfig.value( accountName + QLatin1String( ".port" ) ).toInt( &found);
      if ( found ) {
        settings.insert( QLatin1String( "Port" ), port );
      }

      found = false;
      const int socketType = mHashConfig.value( accountName + QLatin1String( ".socketType" ) ).toInt( &found);
      if(found) {
        switch(socketType) {
          case 0:
            //None
            //nothing
            break;
          case 2:
            //STARTTLS
            settings.insert( QLatin1String( "UseTLS" ), true );
            break;
          case 3:
            //SSL/TLS
            settings.insert( QLatin1String( "UseSSL" ), true );
            break;
          default:
            kDebug()<<" socketType "<<socketType;
        }
      }
      addAuth( settings, QLatin1String( "AuthenticationMethod" ),accountName );
      const QString checkNewMailStr = accountName + QLatin1String( ".check_new_mail" );
      if(mHashConfig.contains(checkNewMailStr)) {
        const bool checkNewMail = mHashConfig.value(checkNewMailStr).toBool();
        settings.insert(QLatin1String("IntervalCheckEnabled"), checkNewMail);
      }
      const QString checkTimeStr = accountName + QLatin1String( ".check_time" );
      if(mHashConfig.contains(checkTimeStr)) {
        found = false;
        const int checkTime = mHashConfig.value( checkTimeStr ).toInt( &found);
        if(found) {
          settings.insert(QLatin1String("IntervalCheckInterval"),checkTime);
        }
      } else {
        //Default value from thunderbird
        settings.insert(QLatin1String("IntervalCheckInterval"), 10 );
      }

      const QString agentIdentifyName = AbstractBase::createResource( "akonadi_pop3_resource", name, settings );
      addCheckMailOnStartup(agentIdentifyName,loginAtStartup);
    } else if ( type == QLatin1String( "none" ) ) {
      //FIXME look at if we can implement it
      kDebug()<<" account type none!";
      continue;
    } else if (type == QLatin1String("movemail")) {
      kDebug()<<" movemail accound found and not implemented in importthunderbird";
      continue;
      //TODO
    } else if (type == QLatin1String("rss")) {
      //TODO when akregator2 will merge in kdepim
      kDebug()<<" rss resource needs to be implemented";
      continue;
    } else if (type == QLatin1String("nntp")) {
      //TODO add config directly to knode
      //TODO when knode will merge in kdepim
      kDebug()<<" nntp resource need to be implemented";
      continue;
    } else {
      kDebug()<<" type unknown : "<<type;
      continue;
    }

    const QString identityConfig = QString::fromLatin1( "mail.account.%1" ).arg( account ) + QLatin1String( ".identities" );
    if ( mHashConfig.contains( identityConfig ) )
    {
      const QStringList idList = mHashConfig.value(identityConfig).toString().split(QLatin1Char(','));
      Q_FOREACH(const QString& id, idList) {
        readIdentity( id );
      }
    }
  }
}

void ThunderbirdSettings::readTransport()
{
  const QString mailSmtpServer = mHashConfig.value( QLatin1String( "mail.smtpservers" ) ).toString();
  if ( mailSmtpServer.isEmpty() )
    return;
  QStringList smtpList = mailSmtpServer.split( QLatin1Char( ',' ) );
  QString defaultSmtp = mHashConfig.value( QLatin1String( "mail.smtp.defaultserver" ) ).toString();
  if(smtpList.count() == 1 && defaultSmtp.isEmpty())
  {
    //Be sure to define default smtp
    defaultSmtp = smtpList.at(0);
  }

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
        kDebug()<<" authMethod unknown :"<<authMethod;
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
        kDebug()<<" trySsl unknown :"<<trySsl;
    }

    const QString userName = mHashConfig.value( smtpName + QLatin1String( ".username" ) ).toString();
    if ( !userName.isEmpty() ) {
      mt->setUserName( userName );
      if(authMethod > 1) {
        mt->setRequiresAuthentication( true );
      }
    }

    storeTransport( mt, ( smtp == defaultSmtp ) );
    mHashSmtp.insert( smtp, QString::number( mt->id() ) ); 
  }
}

QString convertThunderbirdPath(const QString& path)
{
  QString newPath;
  KUrl url(path);
  newPath = url.path();
  newPath.remove(0,1);
  return MailCommon::Util::convertFolderPathToCollectionStr(newPath);
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
  newIdentity->setIdentityName( fullName );

  const QString organization = mHashConfig.value(identity + QLatin1String(".organization")).toString();
  newIdentity->setOrganization(organization);

  bool doBcc = mHashConfig.value(identity + QLatin1String(".doBcc")).toBool();
  if(doBcc) {
    const QString bcc = mHashConfig.value(identity + QLatin1String(".doBccList")).toString();
    newIdentity->setBcc( bcc );
  }

  bool doCc = mHashConfig.value(identity + QLatin1String(".doCc")).toBool();
  if(doCc) {
    const QString cc = mHashConfig.value(identity + QLatin1String(".doCcList")).toString();
    newIdentity->setCc( cc );
  }

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
      const QString draftFolder = convertThunderbirdPath( mHashConfig.value( identity + QLatin1String( ".draft_folder" ) ).toString() );
      newIdentity->setDrafts( draftFolder );
    }
  }

  //fcc_reply_follows_parent not implemented in kmail
  if ( mHashConfig.contains( identity + QLatin1String( ".fcc_folder_picker_mode" ) ) )
  {
    const int useSpecificFccFolder = mHashConfig.value(  identity + QLatin1String( ".fcc_folder_picker_mode" ) ).toInt();
    if ( useSpecificFccFolder == 1 )
    {
      const QString fccFolder = convertThunderbirdPath( mHashConfig.value( identity + QLatin1String( ".fcc_folder" ) ).toString() );
      newIdentity->setFcc( fccFolder );
    }
  }

  if ( mHashConfig.contains( identity + QLatin1String( ".tmpl_folder_picker_mode" ) ) )
  {
    const int useSpecificTemplateFolder = mHashConfig.value(  identity + QLatin1String( ".tmpl_folder_picker_mode" ) ).toInt();
    if ( useSpecificTemplateFolder == 1 )
    {
      const QString templateFolder = convertThunderbirdPath( mHashConfig.value( identity + QLatin1String( ".stationery_folder" ) ).toString() );
      newIdentity->setTemplates( templateFolder );
    }
  }

  
  const QString attachVcardStr( identity + QLatin1String( ".attach_vcard" ) );
  if ( mHashConfig.contains( attachVcardStr ) ) {
    const bool attachVcard = mHashConfig.value( attachVcardStr ).toBool();
    if ( attachVcard ) {
      const QString vcardContent = mHashConfig.value( identity + QLatin1String( ".escapedVCard" ) ).toString();
      //TODO: not implemented in kmail
    }
  }
  const QString composeHtmlStr( identity + QLatin1String( ".compose_html" ) );
  //TODO: implement it in kmail

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
    const int pos(valueStr.length()-1);
    if ( valueStr.at( pos ) == QLatin1Char( '"' ) )
      valueStr.remove( pos, 1 );
    //Store as String
    mHashConfig.insert( key, valueStr );
  } else {
    if ( valueStr == QLatin1String( "true" ) ) {
      mHashConfig.insert( key, true );
    } else if ( valueStr == QLatin1String( "false" ) ) {
      mHashConfig.insert( key, false );
    } else { 
      //Store as integer
      const int value = valueStr.toInt();
      mHashConfig.insert( key, value );
    }
  }
  if(key.contains(QLatin1String("ldap_")) && key.endsWith(QLatin1String(".description"))) {
    QString ldapAccountName = key;
    mLdapAccountList.append(ldapAccountName.remove(QLatin1String(".description")));
  }
}
