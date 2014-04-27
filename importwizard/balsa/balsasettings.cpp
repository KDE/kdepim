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

#include "balsasettings.h"


#include <mailtransport/transportmanager.h>
#include "mailcommon/util/mailutil.h"
#include "messageviewer/header/kxface.h"

#include <kpimidentities/identity.h>
#include <kpimidentities/signature.h>


#include <KConfig>
#include <KConfigGroup>
#include <QImage>

BalsaSettings::BalsaSettings(const QString &filename, ImportWizard *parent)
    :AbstractSettings( parent )
{
    KConfig config(filename);

    bool autoCheck = false;
    int autoCheckDelay = -1;
    if (config.hasGroup(QLatin1String("MailboxChecking"))) {
        KConfigGroup grp = config.group(QLatin1String("MailboxChecking"));
        autoCheck = grp.readEntry(QLatin1String("Auto"),false);
        autoCheckDelay = grp.readEntry(QLatin1String("AutoDelay"),-1);
    }

    const QStringList mailBoxList = config.groupList().filter( QRegExp( "mailbox-\\+d" ) );
    Q_FOREACH (const QString& mailBox,mailBoxList) {
        KConfigGroup grp = config.group(mailBox);
        readAccount(grp,autoCheck,autoCheckDelay);
    }

    const QStringList smtpList = config.groupList().filter( QRegExp( "smtp-server-" ) );
    Q_FOREACH (const QString& smtp,smtpList) {
        KConfigGroup grp = config.group(smtp);
        readTransport(grp);
    }
    readGlobalSettings(config);
}

BalsaSettings::~BalsaSettings()
{

}

void BalsaSettings::readAccount(const KConfigGroup &grp, bool autoCheck, int autoDelay)
{
    Q_UNUSED( autoDelay );
    const QString type = grp.readEntry(QLatin1String("Type"));
    bool check = grp.readEntry(QLatin1String("Check"), false);
    if (type == QLatin1String("LibBalsaMailboxPOP3")) {
        QMap<QString, QVariant> settings;
        const QString server = grp.readEntry(QLatin1String("Server"));
        settings.insert( QLatin1String( "Host" ), server );
        const QString name = grp.readEntry(QLatin1String("Name"));

        const bool apop = grp.readEntry(QLatin1String("DisableApop"),false);
        Q_UNUSED( apop );
        const QString agentIdentifyName = AbstractBase::createResource( "akonadi_pop3_resource", name,settings );

        addCheckMailOnStartup(agentIdentifyName,autoCheck);
        addToManualCheck(agentIdentifyName,check);

    } else if (type == QLatin1String("LibBalsaMailboxImap")) {
        QMap<QString, QVariant> settings;
        const QString server = grp.readEntry(QLatin1String("Server"));
        settings.insert(QLatin1String("ImapServer"),server);
        const QString name = grp.readEntry(QLatin1String("Name"));
        const QString agentIdentifyName = AbstractBase::createResource( "akonadi_imap_resource", name,settings );
        addCheckMailOnStartup(agentIdentifyName,autoCheck);
        addToManualCheck(agentIdentifyName,check);
    } else {
        qDebug()<<" unknown account type :"<<type;
    }
}

void BalsaSettings::readIdentity(const KConfigGroup &grp)
{
    QString name = grp.readEntry(QLatin1String("FullName"));
    KPIMIdentities::Identity* newIdentity = createIdentity(name);
    newIdentity->setFullName(name);
    //QT5 newIdentity->setEmailAddr(grp.readEntry(QLatin1String("Address")));
    newIdentity->setReplyToAddr(grp.readEntry(QLatin1String("ReplyTo")));
    newIdentity->setBcc(grp.readEntry(QLatin1String("Bcc")));
    const QString smtp = grp.readEntry(QLatin1String("SmtpServer"));
    if (!smtp.isEmpty() && mHashSmtp.contains(smtp)) {
        newIdentity->setTransport(mHashSmtp.value(smtp));
    }

    const QString signaturePath = grp.readEntry(QLatin1String("SignaturePath"));
    if (!signaturePath.isEmpty()) {
        KPIMIdentities::Signature signature;
        if (grp.readEntry(QLatin1String("SigExecutable"),false)) {
            signature.setUrl(signaturePath, true );
            signature.setType( KPIMIdentities::Signature::FromCommand );
        } else {
            signature.setType( KPIMIdentities::Signature::FromFile );
        }
        newIdentity->setSignature( signature );
    }

    const QString xfacePathStr = grp.readEntry(QLatin1String("XFacePath"));
    if (!xfacePathStr.isEmpty()) {
        newIdentity->setXFaceEnabled(true);
        MessageViewer::KXFace xf;
        newIdentity->setXFace(xf.fromImage( QImage( xfacePathStr ) ));
    }
#if 0
    Domain=
            ReplyString=Re :
            ForwardString=Fwd :
            SendMultipartAlternative=false
            SigSending=true
            SigForward=true
            SigReply=true
            SigSeparator=true
            SigPrepend=false
            RequestMDN=false
            GpgSign=false
            GpgEncrypt=false
            GpgTrustAlways=false
            GpgWarnSendPlain=true
            CryptProtocol=8
            ForceKeyID=
        #endif


            storeIdentity(newIdentity);
}

void BalsaSettings::readTransport(const KConfigGroup &grp)
{
    MailTransport::Transport *mt = createTransport();
    const QString smtp = grp.name().remove(QLatin1String("smtp-server-"));
    const QString server = grp.readEntry(QLatin1String("Server"));
    mt->setHost(server);

    const int tlsMode = grp.readEntry(QLatin1String("TLSMode"),-1);
    //TODO
    switch(tlsMode) {
    case 0:
        break;
    case 1:
        mt->setEncryption( MailTransport::Transport::EnumEncryption::TLS );
        break;
    case 2:
        break;
    default:
        break;
    }

    const QString ssl = grp.readEntry(QLatin1String("SSL"));
    if (ssl == QLatin1String("true")) {
        mt->setEncryption( MailTransport::Transport::EnumEncryption::SSL );
    } else if (ssl == QLatin1String("false")) {
        mt->setEncryption( MailTransport::Transport::EnumEncryption::None );
    } else {
        qDebug()<<" unknown ssl value :"<<ssl;
    }

    const QString anonymous = grp.readEntry(QLatin1String("Anonymous"));

    //TODO
    storeTransport( mt, /*( smtp == defaultSmtp )*/true ); //FIXME
    mHashSmtp.insert( smtp, QString::number( mt->id() ) );

    //TODO
    /*
    Server=localhost:25
    Anonymous=false
    RememberPasswd=false
    SSL=false
    TLSMode=1
    BigMessage=0
*/
}

void BalsaSettings::readGlobalSettings(const KConfig &config)
{
    if (config.hasGroup(QLatin1String("Compose"))) {
        KConfigGroup compose = config.group(QLatin1String("Compose"));
        if (compose.hasKey(QLatin1String("QuoteString"))) {
            const QString quote = compose.readEntry(QLatin1String("QuoteString"));
            if (!quote.isEmpty())
                addKmailConfig( QLatin1String("TemplateParser"), QLatin1String("QuoteString"), quote);
        }
    }
    if (config.hasGroup(QLatin1String("MessageDisplay"))) {
        KConfigGroup messageDisplay = config.group(QLatin1String("MessageDisplay"));
        if (messageDisplay.hasKey(QLatin1String("WordWrap"))) {
            bool wordWrap = messageDisplay.readEntry(QLatin1String("WordWrap"),false);
            Q_UNUSED( wordWrap );
            //TODO not implemented in kmail.
        }
        if (messageDisplay.hasKey(QLatin1String("WordWrapLength"))) {
            const int wordWrapLength = messageDisplay.readEntry(QLatin1String("WordWrapLength"),-1);
            Q_UNUSED( wordWrapLength );
            //TODO not implemented in kmail
        }
        if (messageDisplay.hasKey(QLatin1String("DateFormat"))) {
            const QString dateFormat = messageDisplay.readEntry(QLatin1String("DateFormat"));
            if (!dateFormat.isEmpty()) {
                addKmailConfig(QLatin1String("General"), QLatin1String("customDateFormat"), dateFormat);
            }
        }
    }

    if (config.hasGroup(QLatin1String("Sending"))) {
        KConfigGroup sending = config.group(QLatin1String("Sending"));
        if (sending.hasKey(QLatin1String("WordWrap"))) {
            const bool wordWrap = sending.readEntry(QLatin1String("WordWrap"),false);
            addKmailConfig( QLatin1String("Composer"), QLatin1String("word-wrap"), wordWrap);
        }
        if (sending.hasKey(QLatin1String("break-at"))) {
            const int wordWrapLength = sending.readEntry(QLatin1String("break-at"),-1);
            if (wordWrapLength!=-1) {
                addKmailConfig( QLatin1String("Composer"), QLatin1String("break-at"),wordWrapLength);
            }
        }
    }
    if (config.hasGroup(QLatin1String("Global"))) {
        KConfigGroup global = config.group(QLatin1String("Global"));
        if (global.hasKey(QLatin1String("EmptyTrash"))) {
            const bool emptyTrash = global.readEntry(QLatin1String("EmptyTrash"),false);
            addKmailConfig( QLatin1String("General"), QLatin1String("empty-trash-on-exit"),emptyTrash);
        }
    }
    if (config.hasGroup(QLatin1String("Spelling"))) {
        KConfigGroup spellChecking = config.group(QLatin1String("Spelling"));
        if (spellChecking.hasKey(QLatin1String("SpellCheckActive"))) {
            const bool active = spellChecking.readEntry(QLatin1String("SpellCheckActive"),false);
            addKmailConfig( QLatin1String("Spelling"), QLatin1String("backgroundCheckerEnabled"),active);
            addKmailConfig( QLatin1String("Spelling"), QLatin1String("checkerEnabledByDefault"),active);
        }
        if (spellChecking.hasKey(QLatin1String("SpellCheckLanguage"))) {
            const QString spellCheck = spellChecking.readEntry(QLatin1String("defaultLanguage"));
            addKmailConfig( QLatin1String("Spelling"), QLatin1String("defaultLanguage"),spellCheck);
        }
    }
}
