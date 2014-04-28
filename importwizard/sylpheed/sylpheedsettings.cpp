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

#include "sylpheedsettings.h"
#include "sylpheedsettingsutils.h"
#include <MailTransport/transportmanager.h>
#include "mailcommon/util/mailutil.h"

#include <KPIMIdentities/identity.h>
#include <KPIMIdentities/signature.h>

#include <KConfig>
#include <KConfigGroup>
#include <KDebug>

#include <QRegExp>
#include <QStringList>
#include <QFile>

SylpheedSettings::SylpheedSettings( ImportWizard *parent )
    :AbstractSettings( parent )
{
}

SylpheedSettings::~SylpheedSettings()
{
}

void SylpheedSettings::importSettings(const QString& filename, const QString& path)
{
    bool checkMailOnStartup = true;
    int intervalCheckMail = -1;
    const QString sylpheedrc = path + QLatin1String("/sylpheedrc");
    if (QFile( sylpheedrc ).exists()) {
        KConfig configCommon( sylpheedrc );
        if (configCommon.hasGroup("Common")) {
            KConfigGroup common = configCommon.group("Common");
            checkMailOnStartup = ( common.readEntry("check_on_startup",1) == 1 );

            if (common.readEntry(QLatin1String("autochk_newmail"),1) == 1 ) {
                intervalCheckMail = common.readEntry(QLatin1String("autochk_interval"),-1);
            }
            readGlobalSettings(common);
        }
    }
    KConfig config( filename );
    const QStringList accountList = config.groupList().filter( QRegExp( "Account: \\d+" ) );
    const QStringList::const_iterator end( accountList.constEnd() );
    for ( QStringList::const_iterator it = accountList.constBegin(); it!=end; ++it )
    {
        KConfigGroup group = config.group( *it );
        readAccount( group, checkMailOnStartup, intervalCheckMail );
        readIdentity( group );
    }
    const QString customheaderrc = path + QLatin1String("/customheaderrc");
    QFile customHeaderFile(customheaderrc);
    if (customHeaderFile.exists()) {
        if ( !customHeaderFile.open( QIODevice::ReadOnly ) ) {
            kDebug()<<" We can't open file"<<customheaderrc;
        } else {
            readCustomHeader(&customHeaderFile);
        }
    }
}

void SylpheedSettings::readCustomHeader(QFile *customHeaderFile)
{
    //In sylpheed we define custom header from account.
    //In kmail it's global
    QTextStream stream(customHeaderFile);
    QMap<QString, QString> header;
    while ( !stream.atEnd() ) {
        const QString line = stream.readLine();
        const QStringList lst = line.split(QLatin1Char(':'));
        if (lst.count() == 3) {
            QString str = lst.at(2);
            str.remove(0,1);
            header.insert(lst.at(1),str);
        }
    }
    if (!header.isEmpty()) {
        const int oldValue = readKmailSettings(QLatin1String("General"),QLatin1String("mime-header-count"));
        int newValue = header.count();
        if (oldValue!=-1) {
            newValue+=oldValue;
        }
        addKmailConfig( QLatin1String("General"),QLatin1String("mime-header-count"), newValue);
        int currentHeader = (oldValue>0) ? oldValue : 0;
        for (QMapIterator<QString, QString> it(header);  it.hasNext();  )
        {
            it.next();
            addComposerHeaderGroup(QString::fromLatin1("Mime #%1").arg(currentHeader), (it).key(), (it).value());
            ++currentHeader;
        }
    }
}

void SylpheedSettings::readGlobalSettings(const KConfigGroup& group)
{
    const bool showTrayIcon = (group.readEntry("show_trayicon", 0) == 1 );
    addKmailConfig(QLatin1String("General"), QLatin1String("SystemTrayEnabled"), showTrayIcon);

    const bool cleanTrashOnExit = (group.readEntry("clean_trash_on_exit", 0) == 1 );
    addKmailConfig(QLatin1String("General"), QLatin1String("empty-trash-on-exit"), cleanTrashOnExit);

    const bool alwaysMarkReadOnShowMsg = (group.readEntry("always_mark_read_on_show_msg", 0) == 1 );
    if (alwaysMarkReadOnShowMsg) {
        addKmailConfig(QLatin1String("Behaviour"), QLatin1String("DelayedMarkAsRead"), true);
        addKmailConfig(QLatin1String("Behaviour"), QLatin1String("DelayedMarkTime"), 0);
    }

    if (group.readEntry("enable_autosave", 0) == 1 ) {
        const int autosaveInterval = group.readEntry("autosave_interval",5);
        addKmailConfig(QLatin1String("Composer"), QLatin1String("autosave"), autosaveInterval);
    }
    const bool checkAttach = (group.readEntry("check_attach", 0) == 1 );
    addKmailConfig(QLatin1String("Composer"), QLatin1String("showForgottenAttachmentWarning"), checkAttach);

    const QString attachStr = group.readEntry("check_attach_str");
    if (!attachStr.isEmpty()) {
        addKmailConfig(QLatin1String("Composer"), QLatin1String("attachment-keywords"), attachStr);
    }

    const int lineWrap = group.readEntry("linewrap_length", 80);
    addKmailConfig(QLatin1String("Composer"), QLatin1String("break-at"), lineWrap);
    addKmailConfig(QLatin1String("Composer"), QLatin1String("word-wrap"), true);


    if (group.readEntry(QLatin1String("recycle_quote_colors"), 0)==1) {
        addKmailConfig(QLatin1String("Reader"), QLatin1String("RecycleQuoteColors"), true);
    }

    if (group.readEntry(QLatin1String("auto_signature")) == 0) {
        addKmailConfig(QLatin1String("Composer"), QLatin1String("signature"), QLatin1String("manual"));
    }

    if (group.readEntry(QLatin1String("auto_ext_editor"),-1) == 1) {
        addKmailConfig(QLatin1String("General"), QLatin1String("use-external-editor"), true);

        const QString externalEditor = group.readEntry(QLatin1String("mime_open_command"));
        if (!externalEditor.isEmpty()) {
            addKmailConfig(QLatin1String("General"), QLatin1String("external-editor"), externalEditor);
        }
    }

    readSettingsColor(group);
    readTemplateFormat(group);
    readTagColor(group);
    readDateFormat(group);
}

void SylpheedSettings::readTemplateFormat(const KConfigGroup& group)
{
    const QString replyQuote = group.readEntry(QLatin1String("reply_quote_mark"));
    if (!replyQuote.isEmpty()) {
        addKmailConfig(QLatin1String("TemplateParser"), QLatin1String("QuoteString"), replyQuote);
    }
    const QString forwardQuote = group.readEntry(QLatin1String("forward_quote_mark"));
    if (!forwardQuote.isEmpty()) {
        //Not implemented in kmail
    }
    const QString replyQuoteFormat = group.readEntry(QLatin1String("reply_quote_format"));
    if (!replyQuoteFormat.isEmpty()) {
        addKmailConfig(QLatin1String("TemplateParser"), QLatin1String("TemplateReply"), convertToKmailTemplate(replyQuoteFormat));
    }
    const QString forwardQuoteFormat = group.readEntry(QLatin1String("forward_quote_format"));
    if (!forwardQuoteFormat.isEmpty()) {
        addKmailConfig(QLatin1String("TemplateParser"), QLatin1String("TemplateForward"), convertToKmailTemplate(forwardQuoteFormat));
    }
}

void SylpheedSettings::readDateFormat(const KConfigGroup& group)
{
    const QString dateFormat = group.readEntry(QLatin1String("date_format"));
    if (!dateFormat.isEmpty()) {
        addKmailConfig(QLatin1String("General"), QLatin1String("customDateFormat"), dateFormat);
    }
}

void SylpheedSettings::readTagColor(const KConfigGroup& group)
{
    Q_UNUSED( group );
    //TODO
}

void SylpheedSettings::readSettingsColor(const KConfigGroup& group)
{
    const bool enableColor = group.readEntry("enable_color", false);
    if (enableColor) {
        const int colorLevel1 = group.readEntry("quote_level1_color", -1);
        if (colorLevel1!=-1) {
            //[Reader]  QuotedText1
        }
        const int colorLevel2 = group.readEntry("quote_level2_color", -1);
        if (colorLevel2!=-1) {
            //[Reader]  QuotedText2
        }
        const int colorLevel3 = group.readEntry("quote_level3_color", -1);
        if (colorLevel3!=-1) {
            //[Reader]  QuotedText3
        }
    }
}

QString SylpheedSettings::convertToKmailTemplate(const QString& templateStr)
{
    QString newTemplate = templateStr;
    newTemplate.replace(QLatin1String("%date"),QLatin1String("%DATE"));
    newTemplate.replace(QLatin1String("%d"),QLatin1String("%DATE"));
    newTemplate.replace(QLatin1String("%from"),QLatin1String("%OTONAME"));
    newTemplate.replace(QLatin1String("%f"),QLatin1String("%OTONAME"));
    newTemplate.replace(QLatin1String("%to"),QLatin1String("%TONAME"));
    newTemplate.replace(QLatin1String("%t"),QLatin1String("%TONAME"));
    newTemplate.replace(QLatin1String("%cc"),QLatin1String("%CCNAME"));
    newTemplate.replace(QLatin1String("%c"),QLatin1String("%CCNAME"));

    newTemplate.replace(QLatin1String("%email"),QLatin1String("%OFROMNAME"));
    newTemplate.replace(QLatin1String("%A"),QLatin1String("%OFROMNAME"));

    newTemplate.replace(QLatin1String("%cursor"),QLatin1String("%CURSOR"));
    newTemplate.replace(QLatin1String("%X"),QLatin1String("%CURSOR"));

    newTemplate.replace(QLatin1String("%msg"),QLatin1String("%TEXT"));
    newTemplate.replace(QLatin1String("%M"),QLatin1String("%TEXT"));

    newTemplate.replace(QLatin1String("%quoted_msg"),QLatin1String("%QUOTE"));
    newTemplate.replace(QLatin1String("%Q"),QLatin1String("%QUOTE"));

    newTemplate.replace(QLatin1String("%subject"),QLatin1String("%OFULLSUBJECT"));
    newTemplate.replace(QLatin1String("%s"),QLatin1String("%OFULLSUBJECT"));

    newTemplate.replace(QLatin1String("%messageid"),QLatin1String("%MSGID"));
    newTemplate.replace(QLatin1String("%i"),QLatin1String("%MSGID"));

    newTemplate.replace(QLatin1String("%firstname"),QLatin1String("%OFROMNAME"));
    newTemplate.replace(QLatin1String("%F"),QLatin1String("%OFROMNAME"));

    newTemplate.replace(QLatin1String("%lastname"),QLatin1String("%OFROMLNAME"));
    newTemplate.replace(QLatin1String("%L"),QLatin1String("%OFROMLNAME"));

    newTemplate.replace(QLatin1String("%fullname"),QLatin1String("%OFROMFNAME"));
    newTemplate.replace(QLatin1String("%N"),QLatin1String("%OFROMFNAME"));
    //TODO add more variable
    return newTemplate;
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
    const int signatureEnabled = accountConfig.readEntry("auto_signature", -1 );
    switch(signatureEnabled) {
    case -1:
        break;
    case 0:
        signature.setEnabledSignature(false);
        break;
    case 1:
        signature.setEnabledSignature(true);
        break;
    default:
        qDebug()<<" auto_signature undefined "<<signatureEnabled;
    }

    //TODO  const bool signatureBeforeQuote = ( accountConfig.readEntry( "signature_before_quote", 0 ) == 1 ); not implemented in kmail

    identity->setSignature( signature );
}

void SylpheedSettings::readPop3Account( const KConfigGroup& accountConfig, bool checkMailOnStartup, int intervalCheckMail )
{
    QMap<QString, QVariant> settings;
    const QString host = accountConfig.readEntry("receive_server");
    settings.insert( QLatin1String( "Host" ), host );

    const QString name = accountConfig.readEntry( QLatin1String( "name" ) );
    const QString inbox = MailCommon::Util::convertFolderPathToCollectionStr(accountConfig.readEntry(QLatin1String("inbox")));
    settings.insert(QLatin1String("TargetCollection"), inbox);
    int port = 0;
    if ( SylpheedSettingsUtils::readConfig( QLatin1String( "pop_port" ), accountConfig, port, true ) )
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
            break;
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
        if (useApop) {
            settings.insert(QLatin1String( "AuthenticationMethod" ), MailTransport::Transport::EnumAuthenticationType::APOP);
        }
    }
    if (intervalCheckMail != -1) {
        settings.insert(QLatin1String("IntervalCheckEnabled"), true);
        settings.insert(QLatin1String("IntervalCheckInterval"), intervalCheckMail);
    }


    const QString agentIdentifyName = AbstractBase::createResource( "akonadi_pop3_resource", name, settings );
    addCheckMailOnStartup(agentIdentifyName,checkMailOnStartup);
    const bool enableManualCheck = (accountConfig.readEntry( QLatin1String( "receive_at_get_all" ), 0) ==1 );
    addToManualCheck(agentIdentifyName,enableManualCheck);
}

void SylpheedSettings::readImapAccount( const KConfigGroup& accountConfig, bool checkMailOnStartup, int intervalCheckMail )
{
    QMap<QString, QVariant> settings;
    const QString host = accountConfig.readEntry("receive_server");
    settings.insert(QLatin1String("ImapServer"), host);

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
        //TLS
        settings.insert( QLatin1String( "Safety" ), QLatin1String( "STARTTLS" ) );
        break;
    default:
        kDebug()<<" sslimap unknown "<<sslimap;
        break;
    }

    int port = 0;
    if ( SylpheedSettingsUtils::readConfig( QLatin1String( "imap_port" ), accountConfig, port, true ) )
        settings.insert( QLatin1String( "ImapPort" ), port );

    QString trashFolder;
    if ( SylpheedSettingsUtils::readConfig( QLatin1String( "trash_folder" ), accountConfig, trashFolder, false ) )
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
        break;
    default:
        kDebug()<<" imap auth unknown "<<auth;
        break;
    }

    if (intervalCheckMail != -1) {
        settings.insert(QLatin1String("IntervalCheckEnabled"), true);
        settings.insert(QLatin1String("IntervalCheckTime"),intervalCheckMail);
    }


    const QString password = accountConfig.readEntry( QLatin1String( "password" ) );
    settings.insert( QLatin1String( "Password" ), password );

    const QString agentIdentifyName = AbstractBase::createResource( "akonadi_imap_resource", name,settings );
    addCheckMailOnStartup(agentIdentifyName,checkMailOnStartup);

    const bool enableManualCheck = (accountConfig.readEntry( QLatin1String( "receive_at_get_all" ), 0) ==1 );
    addToManualCheck(agentIdentifyName,enableManualCheck);
}


void SylpheedSettings::readAccount(const KConfigGroup& accountConfig , bool checkMailOnStartup, int intervalCheckMail)
{
    if ( accountConfig.hasKey( QLatin1String( "protocol" ) ) )
    {
        const int protocol = accountConfig.readEntry( QLatin1String( "protocol" ), 0 );
        switch( protocol )
        {
        case 0:
            readPop3Account( accountConfig, checkMailOnStartup, intervalCheckMail );
            break;
        case 3:
            //imap
            readImapAccount(accountConfig, checkMailOnStartup, intervalCheckMail);
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
    QString name = accountConfig.readEntry( QLatin1String( "name" ) );
    KPIMIdentities::Identity* identity  = createIdentity(name);

    identity->setFullName( name );
    identity->setIdentityName( name );
    const QString organization = accountConfig.readEntry( QLatin1String( "organization" ), QString() );
    identity->setOrganization(organization);
    const QString email = accountConfig.readEntry( QLatin1String( "address" ) );
    identity->setPrimaryEmailAddress(email);

    QString value;
    if ( SylpheedSettingsUtils::readConfig( QLatin1String("auto_bcc") , accountConfig, value, true ) )
        identity->setBcc(value);
    if ( SylpheedSettingsUtils::readConfig( QLatin1String("auto_cc") , accountConfig, value, true ) )
        identity->setCc(value);
    if ( SylpheedSettingsUtils::readConfig( QLatin1String("auto_replyto") , accountConfig, value, true ) )
        identity->setReplyToAddr(value);

    if ( SylpheedSettingsUtils::readConfig( QLatin1String("daft_folder") , accountConfig, value, false ) ) {
        //QT5 identity->setDrafts(MailCommon::Util::convertFolderPathToCollectionStr(value));
    }

    if ( SylpheedSettingsUtils::readConfig( QLatin1String("sent_folder") , accountConfig, value, false ) ) {
        //QT5 identity->setFcc(MailCommon::Util::convertFolderPathToCollectionStr(value));
    }

    const QString transportId = readTransport(accountConfig);
    if (!transportId.isEmpty())
    {
        identity->setTransport(transportId);
    }
    readSignature( accountConfig, identity );
    storeIdentity(identity);
}

QString SylpheedSettings::readTransport( const KConfigGroup& accountConfig )
{
    const QString smtpserver = accountConfig.readEntry("smtp_server");

    if (!smtpserver.isEmpty()) {
        MailTransport::Transport *mt = createTransport();
        mt->setName( smtpserver );
        mt->setHost(smtpserver);
        int port = 0;
        if ( SylpheedSettingsUtils::readConfig( QLatin1String( "smtp_port" ), accountConfig, port, true ) )
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
                kDebug()<<" smtp authentication unknown :"<<authMethod;
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
        if ( SylpheedSettingsUtils::readConfig( QLatin1String( "domain" ), accountConfig, domainName, false ) )
            mt->setLocalHostname( domainName );

        storeTransport( mt, true );
        return QString::number(mt->id());
    }
    return QString();
}
