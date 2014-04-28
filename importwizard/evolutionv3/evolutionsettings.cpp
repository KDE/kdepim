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

#include "evolutionsettings.h"
#include "evolutionutil.h"
#include "mailcommon/util/mailutil.h"
#include "importwizardutil.h"

#include <KPIMIdentities/identity.h>

#include <MailTransport/transportmanager.h>

#include <KConfig>
#include <KDebug>

#include <QFile>
#include <QDir>
#include <QDomDocument>
#include <QDomElement>

EvolutionSettings::EvolutionSettings( ImportWizard *parent )
    :AbstractSettings( parent )
{
}


EvolutionSettings::~EvolutionSettings()
{
}

void EvolutionSettings::loadAccount(const QString& filename)
{
    //Read gconf file
    QFile file(filename);
    if ( !file.open( QIODevice::ReadOnly ) ) {
        kDebug()<<" We can't open file"<<filename;
        return;
    }
    QDomDocument doc;
    if ( !EvolutionUtil::loadInDomDocument( &file, doc ) )
        return;
    QDomElement config = doc.documentElement();

    if ( config.isNull() ) {
        kDebug() << "No config found in filename "<<filename;
        return;
    }
    for ( QDomElement e = config.firstChildElement(); !e.isNull(); e = e.nextSiblingElement() ) {
        const QString tag = e.tagName();
        if ( tag == QLatin1String( "entry" ) ) {
            if ( e.hasAttribute( "name" ) ) {
                const QString attr = e.attribute("name");
                if ( attr == QLatin1String( "accounts" ) ) {
                    readAccount(e);
                } else if ( attr == QLatin1String( "signatures" ) ) {
                    readSignatures( e );
                } else if( attr == QLatin1String("send_recv_all_on_start")) {
                    //TODO: implement it.
                } else if( attr == QLatin1String("send_recv_on_start")) {
                    //TODO: implement it.
                } else {
                    kDebug()<<" attr unknown "<<attr;
                }
            }
        }
    }
}

void EvolutionSettings::loadLdap(const QString& filename)
{
    QFile file(filename);
    if ( !file.open( QIODevice::ReadOnly ) ) {
        kDebug()<<" We can't open file"<<filename;
        return;
    }
    QDomDocument doc;
    if ( !EvolutionUtil::loadInDomDocument( &file, doc ) )
        return;
    QDomElement ldapConfig = doc.documentElement();
    for ( QDomElement e = ldapConfig.firstChildElement(); !e.isNull(); e = e.nextSiblingElement() ) {
        const QString tag = e.tagName();
        if ( tag == QLatin1String( "entry" ) ) {
            for ( QDomElement serverConfig = e.firstChildElement(); !serverConfig.isNull(); serverConfig = serverConfig.nextSiblingElement() ) {
                if(serverConfig.tagName() == QLatin1String("li")) {
                    QDomElement ldapValue = serverConfig.firstChildElement();
                    readLdap(ldapValue.text());
                }
            }
        }
    }
}

void EvolutionSettings::readLdap(const QString &ldapStr)
{
    qDebug()<<" ldap "<<ldapStr;
    QDomDocument ldap;
    if ( !EvolutionUtil::loadInDomDocument( ldapStr, ldap ) )
        return;

    QDomElement domElement = ldap.documentElement();

    if ( domElement.isNull() ) {
        kDebug() << "ldap not found";
        return;
    }
    //Ldap server
    if(domElement.attribute(QLatin1String("base_uri")) == QLatin1String("ldap://")) {
        for ( QDomElement e = domElement.firstChildElement(); !e.isNull(); e = e.nextSiblingElement() ) {
            //const QString name = e.attribute( QLatin1String( "name" ) ); We don't use it in kmail

            ldapStruct ldap;
            const QString relative_uri = e.attribute( QLatin1String( "relative_uri" ) );
            const QString uri = e.attribute( QLatin1String( "uri" ) );
            KUrl url(uri);
            ldap.port = url.port();
            ldap.ldapUrl = url;
            qDebug()<<" relative_uri"<<relative_uri;

            QDomElement propertiesElement = e.firstChildElement();
            if(!propertiesElement.isNull()) {
                for ( QDomElement property = propertiesElement.firstChildElement(); !property.isNull(); property = property.nextSiblingElement() ) {
                    const QString propertyTag = property.tagName();
                    if(propertyTag == QLatin1String("property")) {
                        if(property.hasAttribute(QLatin1String("name"))) {
                            const QString propertyName = property.attribute(QLatin1String("name"));
                            if(propertyName == QLatin1String("timeout")) {
                                ldap.timeout = property.attribute(QLatin1String("value")).toInt();
                            } else if(propertyName == QLatin1String("ssl")) {
                                const QString value = property.attribute(QLatin1String("value"));
                                if(value == QLatin1String("always")) {
                                    ldap.useSSL = true;
                                } else if(value == QLatin1String("whenever_possible")) {
                                    ldap.useTLS = true;
                                } else {
                                    qDebug()<<" ssl attribute unknown"<<value;
                                }
                            } else if(propertyName == QLatin1String("limit")) {
                                ldap.limit = property.attribute(QLatin1String("value")).toInt();
                            } else if(propertyName == QLatin1String("binddn")) {
                                ldap.dn = property.attribute(QLatin1String("value"));
                            } else if(propertyName == QLatin1String("auth")) {
                                const QString value = property.attribute(QLatin1String("value"));
                                if(value == QLatin1String("ldap/simple-email")) {
                                    //TODO:
                                } else if( value == QLatin1String("none")) {
                                    //TODO:
                                } else if( value == QLatin1String("ldap/simple-binddn")) {
                                    //TODO:
                                } else {
                                    qDebug()<<" Unknown auth value "<<value;
                                }
                                qDebug()<<" auth"<<value;
                            } else {
                                qDebug()<<" property unknown :"<<propertyName;
                            }
                        }
                    } else {
                        qDebug()<<" tag unknown :"<<propertyTag;
                    }
                }
                ImportWizardUtil::mergeLdap(ldap);
            }
        }
    }
}

void EvolutionSettings::readSignatures(const QDomElement &account)
{
    for ( QDomElement signatureConfig = account.firstChildElement(); !signatureConfig.isNull(); signatureConfig = signatureConfig.nextSiblingElement() ) {
        if(signatureConfig.tagName() == QLatin1String("li")) {
            QDomElement stringValue = signatureConfig.firstChildElement();
            extractSignatureInfo(stringValue.text());
        }
    }
}

void EvolutionSettings::extractSignatureInfo( const QString&info )
{
    //kDebug()<<" signature info "<<info;
    QDomDocument signature;
    if ( !EvolutionUtil::loadInDomDocument( info, signature ) )
        return;

    QDomElement domElement = signature.documentElement();

    if ( domElement.isNull() ) {
        kDebug() << "Signature not found";
        return;
    }
    for ( QDomElement e = domElement.firstChildElement(); !e.isNull(); e = e.nextSiblingElement() ) {
        KPIMIdentities::Signature signature;

        const QString tag = e.tagName();
        const QString uid = e.attribute( QLatin1String( "uid" ) );
        const QString signatureName = e.attribute( QLatin1String( "name" ) ); //Use it ?
        const QString format = e.attribute( QLatin1String( "text" ) );
        const bool automatic = ( e.attribute( QLatin1String( "auto" ) ) == QLatin1String( "true" ) );
        if(automatic) {
            //TODO:
        } else {
            if ( format == QLatin1String( "text/html" ) ) {
                signature.setInlinedHtml( true );
            } else if ( format == QLatin1String( "text/plain" ) ) {
                signature.setInlinedHtml( false );
            }

            if ( tag == QLatin1String( "filename" ) ) {
                if ( e.hasAttribute( QLatin1String( "script" ) ) && e.attribute( QLatin1String( "script" ) ) == QLatin1String( "true" ) ){
                    signature.setUrl( e.text(), true );
                    signature.setType( KPIMIdentities::Signature::FromCommand );
                } else {
                    signature.setUrl( QDir::homePath() + QLatin1String( ".local/share/evolution/signatures/" ) + e.text(), false );
                    signature.setType( KPIMIdentities::Signature::FromFile );
                }
            }
        }
        mMapSignature.insert( uid, signature );
        
        kDebug()<<" signature tag :"<<tag;
    }
}

void EvolutionSettings::readAccount(const QDomElement &account)
{
    for ( QDomElement accountConfig = account.firstChildElement(); !accountConfig.isNull(); accountConfig = accountConfig.nextSiblingElement() ) {
        if(accountConfig.tagName() == QLatin1String("li")) {
            QDomElement stringValue = accountConfig.firstChildElement();
            extractAccountInfo(stringValue.text());
        }
    }
}

void EvolutionSettings::extractAccountInfo(const QString& info)
{
    kDebug()<<" info "<<info;
    //Read QDomElement
    QDomDocument account;
    if ( !EvolutionUtil::loadInDomDocument( info, account ) )
        return;

    QDomElement domElement = account.documentElement();

    if ( domElement.isNull() ) {
        kDebug() << "Account not found";
        return;
    }

    QString name;
    if(domElement.hasAttribute(QLatin1String("name"))) {
        name = domElement.attribute(QLatin1String("name"));
    }

    KPIMIdentities::Identity* newIdentity = createIdentity(name);

    const bool enableManualCheck = (domElement.attribute(QLatin1String("enabled"))== QLatin1String( "true" ));

    for ( QDomElement e = domElement.firstChildElement(); !e.isNull(); e = e.nextSiblingElement() ) {
        const QString tag = e.tagName();
        if ( tag == QLatin1String( "identity" ) )
        {
            for ( QDomElement identity = e.firstChildElement(); !identity.isNull(); identity = identity.nextSiblingElement() ) {
                const QString identityTag = identity.tagName();
                if ( identityTag == QLatin1String( "name" ) )
                {
                    const QString fullName( identity.text() );
                    newIdentity->setIdentityName( fullName );
                    newIdentity->setFullName( fullName );
                }
                else if ( identityTag == QLatin1String( "addr-spec" ) )
                {
                    newIdentity->setPrimaryEmailAddress(identity.text());
                }
                else if ( identityTag == QLatin1String( "organization" ) )
                {
                    newIdentity->setOrganization(identity.text());
                }
                else if ( identityTag == QLatin1String( "signature" ) )
                {
                    if ( identity.hasAttribute( "uid" ) ) {
                        newIdentity->setSignature( mMapSignature.value( identity.attribute( "uid" ) ) );
                    }
                }
                else if ( identityTag == QLatin1String( "reply-to" ) )
                {
                    newIdentity->setReplyToAddr( identity.text() );
                }
                else
                {
                    kDebug()<<" tag identity not found :"<<identityTag;
                }
            }
        }
        else if ( tag == QLatin1String( "source" ) )
        {
            if(e.hasAttribute(QLatin1String("save-passwd"))&& e.attribute( "save-passwd" ) == QLatin1String( "true" ) ) {
                //TODO
            }
            int interval = -1;
            bool intervalCheck = false;
            if(e.hasAttribute(QLatin1String("auto-check"))) {
                intervalCheck = ( e.attribute(QLatin1String("auto-check")) == QLatin1String( "true" ) );
            }
            if(e.hasAttribute(QLatin1String("auto-check-timeout"))) {
                interval = e.attribute(QLatin1String("auto-check-timeout")).toInt();
            }
            for ( QDomElement server = e.firstChildElement(); !server.isNull(); server = server.nextSiblingElement() ) {
                const QString serverTag = server.tagName();
                if ( serverTag == QLatin1String( "url" ) ) {
                    kDebug()<<" server.text() :"<<server.text();
                    QUrl serverUrl( server.text() );
                    const QString scheme = serverUrl.scheme();
                    QMap<QString, QVariant> settings;
                    const int port = serverUrl.port();

                    const QString path = serverUrl.path();
                    kDebug()<<" path !"<<path;
                    const QString userName = serverUrl.userInfo();

                    const QStringList listArgument = path.split(QLatin1Char(';'));

                    //imapx://name@pop3.xx.org:993/;security-method=ssl-on-alternate-port;namespace;shell-command=ssh%20-C%20-l%20%25u%20%25h%20exec%20/usr/sbin/imapd%20;use-shell-command=true
                    if(scheme == QLatin1String("imap") || scheme == QLatin1String("imapx")) {
                        if( port > 0 )
                            settings.insert(QLatin1String("ImapPort"),port);
                        //Perhaps imapx is specific don't know
                        if ( intervalCheck ) {
                            settings.insert( QLatin1String( "IntervalCheckEnabled" ), true );
                        }
                        if ( interval > -1 ) {
                            settings.insert(QLatin1String("IntervalCheckTime" ), interval );
                        }

                        bool found = false;
                        const QString securityMethod = getSecurityMethod( listArgument, found );
                        if( found ) {
                            if( securityMethod == QLatin1String("none")) {
                                settings.insert( QLatin1String( "Safety" ), QLatin1String("None") );
                                //Nothing
                            } else if(securityMethod == QLatin1String("ssl-on-alternate-port")){
                                settings.insert( QLatin1String( "Safety" ), QLatin1String("SSL") );
                            } else {
                                kDebug()<<" security method unknown : "<<path;
                            }
                        } else {
                            settings.insert( QLatin1String( "Safety" ), QLatin1String("STARTTLS") );
                        }

                        addAuth(settings, QLatin1String( "Authentication" ), userName);
                        const QString agentIdentifyName = AbstractBase::createResource( "akonadi_imap_resource", name,settings );
                        //By default
                        addCheckMailOnStartup(agentIdentifyName,enableManualCheck);
                        addToManualCheck(agentIdentifyName,enableManualCheck);
                    } else if(scheme == QLatin1String("pop")) {
                        if( port > 0 )
                            settings.insert(QLatin1String("Port"),port);
                        bool found = false;
                        const QString securityMethod = getSecurityMethod( listArgument, found );
                        if( found ) {
                            if( securityMethod == QLatin1String("none")) {
                                //Nothing
                            } else if(securityMethod == QLatin1String("ssl-on-alternate-port")){
                                settings.insert( QLatin1String( "UseSSL" ), true );
                            } else {
                                kDebug()<<" security method unknown : "<<path;
                            }
                        } else {
                            settings.insert( QLatin1String( "UseTLS" ), true );
                        }

                        if ( intervalCheck ) {
                            settings.insert( QLatin1String( "IntervalCheckEnabled" ), true );
                        }
                        if ( interval > -1 ) {
                            settings.insert(QLatin1String("IntervalCheckInterval" ), interval );
                        }
                        if(e.hasAttribute(QLatin1String("keep-on-server"))&& e.attribute(QLatin1String("keep-on-server") ) == QLatin1String( "true" ) ) {
                            settings.insert(QLatin1String("LeaveOnServer"),true);
                        }
                        addAuth(settings, QLatin1String( "AuthenticationMethod" ), userName);
                        const QString agentIdentifyName = AbstractBase::createResource( "akonadi_pop3_resource", name, settings );
                        //By default
                        addCheckMailOnStartup(agentIdentifyName,enableManualCheck);
                        addToManualCheck(agentIdentifyName,enableManualCheck);
                    } else if( scheme == QLatin1String("spool") || scheme == QLatin1String("mbox") ) {
                        //mbox file
                        settings.insert(QLatin1String("Path"),path);
                        settings.insert(QLatin1String("DisplayName"),name);
                        AbstractBase::createResource( "akonadi_mbox_resource", name, settings );

                    } else if( scheme == QLatin1String("maildir") ||scheme == QLatin1String( "spooldir" ) ) {
                        settings.insert(QLatin1String("Path"),path);
                        AbstractBase::createResource( "akonadi_maildir_resource", name, settings );
                    } else if( scheme == QLatin1String("nntp")) {
                        //FIXME in the future
                        kDebug()<<" For the moment we can't import nntp resource";
                    } else {
                        kDebug()<<" unknown scheme "<<scheme;
                    }
                } else {
                    kDebug()<<" server tag unknow :"<<serverTag;
                }
            }
        }
        else if ( tag == QLatin1String( "transport" ) )
        {
            if ( e.hasAttribute( "save-passwd" ) && e.attribute( "save-passwd" ) == QLatin1String( "true" ) )
            {
                //TODO save to kwallet ?
            }

            MailTransport::Transport *transport = createTransport();
            for ( QDomElement smtp = e.firstChildElement(); !smtp.isNull(); smtp = smtp.nextSiblingElement() ) {
                const QString smtpTag = smtp.tagName();
                if ( smtpTag == QLatin1String( "url" ) ) {
                    kDebug()<<" smtp.text() :"<<smtp.text();
                    QUrl smtpUrl( smtp.text() );
                    const QString scheme = smtpUrl.scheme();
                    if(scheme == QLatin1String("sendmail")) {
                        transport->setType(MailTransport::Transport::EnumType::Sendmail);
                    } else {
                        transport->setHost( smtpUrl.host() );
                        transport->setName( smtpUrl.host() );
                        //TODO setUserName :
                        //transport->setRequiresAuthentication(true);
                        //transport->setUserName(....);
                        const int port = smtpUrl.port();
                        if ( port > 0 )
                            transport->setPort( port );

                        const QString userName = smtpUrl.userInfo();
                        bool found = false;
                        const QString authMethod = getAuthMethod(userName, found);
                        if( found ) {
                            if(authMethod==QLatin1String("PLAIN")) {
                                transport->setAuthenticationType(MailTransport::Transport::EnumAuthenticationType::PLAIN);
                            } else if(authMethod==QLatin1String("NTLM")) {
                                transport->setAuthenticationType(MailTransport::Transport::EnumAuthenticationType::NTLM);
                            } else if(authMethod==QLatin1String("DIGEST-MD5")) {
                                transport->setAuthenticationType(MailTransport::Transport::EnumAuthenticationType::DIGEST_MD5);
                            } else if(authMethod==QLatin1String("CRAM-MD5")) {
                                transport->setAuthenticationType(MailTransport::Transport::EnumAuthenticationType::CRAM_MD5);
                            } else if(authMethod==QLatin1String("LOGIN")) {
                                transport->setAuthenticationType(MailTransport::Transport::EnumAuthenticationType::LOGIN);
                            } else if(authMethod==QLatin1String("GSSAPI")) {
                                transport->setAuthenticationType(MailTransport::Transport::EnumAuthenticationType::GSSAPI);
                            } else if(authMethod==QLatin1String("POPB4SMTP")) {
                                transport->setAuthenticationType(MailTransport::Transport::EnumAuthenticationType::APOP); //????
                            } else {
                                kDebug()<<" smtp auth method unknown "<<authMethod;
                            }
                        }

                        const QString path = smtpUrl.path();
                        found = false;
                        const QStringList listArgument = path.split(QLatin1Char(';'));
                        const QString securityMethod = getSecurityMethod( listArgument, found );
                        if( found ) {
                            if( securityMethod == QLatin1String("none")) {
                                transport->setEncryption( MailTransport::Transport::EnumEncryption::None );

                            } else if(securityMethod == QLatin1String("ssl-on-alternate-port")){
                                transport->setEncryption( MailTransport::Transport::EnumEncryption::SSL );
                            } else {
                                kDebug()<<" security method unknown : "<<path;
                            }
                        } else {
                            transport->setEncryption( MailTransport::Transport::EnumEncryption::TLS );
                        }
                    }
                } else {
                    kDebug()<<" smtp tag unknow :"<<smtpTag;
                }
            }
            storeTransport(transport, true );
        }
        else if ( tag == QLatin1String( "drafts-folder" ) )
        {
            const QString selectedFolder = MailCommon::Util::convertFolderPathToCollectionStr( e.text().remove( QLatin1String( "folder://" ) ) );
            //QT5 newIdentity->setDrafts(selectedFolder);
        }
        else if ( tag == QLatin1String( "sent-folder" ) )
        {
            const QString selectedFolder = MailCommon::Util::convertFolderPathToCollectionStr( e.text().remove( QLatin1String( "folder://" ) ) );
            //QT5 newIdentity->setFcc(selectedFolder);
        }
        else if ( tag == QLatin1String( "auto-cc" ) )
        {
            if ( e.hasAttribute( "always" ) && ( e.attribute( "always" ) == QLatin1String( "true" ) ) )
            {
                QDomElement recipient = e.firstChildElement();
                const QString text = recipient.text();
                newIdentity->setCc(text);
            }
        }
        else if( tag == QLatin1String("reply-to"))
        {
            newIdentity->setReplyToAddr(e.text());
        }
        else if ( tag == QLatin1String( "auto-bcc" ) )
        {
            if ( e.hasAttribute( "always" ) && ( e.attribute( "always" ) == QLatin1String( "true" ) ) )
            {
                QDomElement recipient = e.firstChildElement();
                const QString text = recipient.text();
                newIdentity->setBcc(text);
            }
        }
        else if ( tag == QLatin1String( "receipt-policy" ) )
        {
            if ( e.hasAttribute( QLatin1String( "policy" ) ) ) {
                const QString policy = e.attribute( QLatin1String( "policy" ) );
                //TODO
            }
        }
        else if ( tag == QLatin1String( "pgp" ) )
        {
            if ( e.hasAttribute( QLatin1String( "encrypt-to-self" ) ) &&
                 ( e.attribute( QLatin1String( "encrypt-to-self" ) ) == QLatin1String( "true" ) ) ) {
                //TODO
            }
            if ( e.hasAttribute( QLatin1String( "always-trust" ) ) &&
                 ( e.attribute( QLatin1String( "always-trust" ) ) == QLatin1String( "true" ) ) ) {
                //TODO
            }
            if ( e.hasAttribute( QLatin1String( "always-sign" ) ) &&
                 ( e.attribute( QLatin1String( "always-sign" ) ) == QLatin1String( "true" ) ) ) {
                //TODO
            }
            if ( e.hasAttribute( QLatin1String( "no-imip-sign" ) ) &&
                 ( e.attribute( QLatin1String( "no-imip-sign" ) ) == QLatin1String( "true" ) ) ) {
                //TODO
            }
        }
        else if ( tag == QLatin1String( "smime" ) )
        {
            if ( e.hasAttribute( QLatin1String( "sign-default" ) ) &&
                 ( e.attribute( QLatin1String( "sign-default" ) ) == QLatin1String( "true" ) ) ) {
                //TODO
            }
            if ( e.hasAttribute( QLatin1String( "encrypt-default" ) ) &&
                 ( e.attribute( QLatin1String( "encrypt-default" ) ) == QLatin1String( "true" ) ) ) {
                //TODO
            }
            if ( e.hasAttribute( QLatin1String( "encrypt-to-self" ) ) &&
                 ( e.attribute( QLatin1String( "encrypt-to-self" ) ) == QLatin1String( "true" ) ) ) {
                //TODO
            }
            //TODO
        }
        else
            kDebug()<<" tag not know :"<<tag;

    }
    storeIdentity(newIdentity);
}

QString EvolutionSettings::getSecurityMethod( const QStringList& listArgument, bool & found )
{
    found = false;
    if(listArgument.isEmpty())
        return QString();
    Q_FOREACH ( const QString& str, listArgument ) {
        if(str.contains(QLatin1String("security-method="))) {
            const int index = str.indexOf(QLatin1String("security-method="));
            if(index != -1) {
                const QString securityMethod = str.right(str.length() - index - 16 /*security-method=*/);
                found = true;
                return securityMethod;
            }
        }
    }
    return QString();
}

QString EvolutionSettings::getAuthMethod( const QString& path, bool & found)
{
    const int index = path.indexOf(QLatin1String("auth="));
    if(index != -1) {
        const QString securityMethod = path.right(path.length() - index - 5 /*auth=*/);
        found = true;
        return securityMethod;
    }
    found = false;
    return QString();
}

void EvolutionSettings::addAuth(QMap<QString, QVariant>& settings, const QString & argument, const QString& userName)
{
    bool found = false;
    const QString authMethod = getAuthMethod(userName, found);
    if( found ) {
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
            kDebug()<<" smtp auth method unknown "<<authMethod;
        }
    }
}
