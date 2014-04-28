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

#include "trojitasettings.h"
#include "importwizardutil.h"

#include <MailTransport/transportmanager.h>

#include <KPIMIdentities/identity.h>
#include <KPIMIdentities/signature.h>

#include <KDebug>

#include <QSettings>

TrojitaSettings::TrojitaSettings(const QString& filename,ImportWizard *parent)
  :AbstractSettings( parent )
{
    settings = new QSettings(filename,QSettings::IniFormat,this);
    readImapAccount();
    readTransport();
    readIdentity();
    readGlobalSettings();
}

TrojitaSettings::~TrojitaSettings()
{
    delete settings;
}

void TrojitaSettings::readImapAccount()
{
    QMap<QString, QVariant> newSettings;
    QString name;

    if (settings->contains(QLatin1String("imap.host"))) {
        name = settings->value(QLatin1String("imap.host")).toString();
        newSettings.insert(QLatin1String("ImapServer"),name);
    }

    if (settings->contains(QLatin1String("imap.port"))) {
        int port = settings->value(QLatin1String("imap.port")).toInt();
        newSettings.insert( QLatin1String( "ImapPort" ), port );
    }

    if (settings->contains(QLatin1String("imap.starttls"))) {
        const bool useTLS = settings->value(QLatin1String("imap.starttls")).toBool();
        if (useTLS) {
            newSettings.insert( QLatin1String( "Safety" ), QLatin1String( "STARTTLS" ) );
        }
    }

    if (settings->contains(QLatin1String("imap.auth.user"))) {
        const QString userName = settings->value(QLatin1String("imap.auth.user")).toString();
        if (!userName.isEmpty()) {
            newSettings.insert( QLatin1String( "Username" ), userName );
        }
    }

    if (settings->contains(QLatin1String("imap.auth.pass"))) {
        const QString password = settings->value(QLatin1String("imap.auth.pass")).toString();
        if (!password.isEmpty())
            newSettings.insert( QLatin1String( "Password" ), password );
    }

    if (settings->contains(QLatin1String("imap.process"))) {
        //What's this ?
    }

    if (settings->contains(QLatin1String("imap.offline"))) {
        const bool offlineStatus = settings->value(QLatin1String("imap.offline")).toBool();
        //It's not a deconnected mode as imap disconnected #317023
        //Will implement soon.
        //TODO use akonadi cache.
    }

    if (settings->contains(QLatin1String("imap.enableId"))) {
        //Not supported by Akonadi.
    }

    if (settings->contains(QLatin1String("imap.ssl.pemCertificate"))) {
        //Not supported by akonadi.
    }

    if (settings->contains(QLatin1String("imap.capabilities.blacklist"))) {
        //Not supported by akonadi-imap-resource.
    }

    if (!name.isEmpty()) {
        const QString agentIdentifyName = AbstractBase::createResource( "akonadi_imap_resource", name, newSettings );
        //Check by default
        addCheckMailOnStartup(agentIdentifyName, true);
    }
}

void TrojitaSettings::readTransport()
{
    settings->beginGroup(QLatin1String("General"));
    const QString smtpMethod = settings->value(QLatin1String("msa.method")).toString();
    if (!smtpMethod.isEmpty()) {
        MailTransport::Transport *mt = createTransport();
        if (smtpMethod == QLatin1String("IMAP-SENDMAIL")) {
            //see http://tools.ietf.org/html/draft-kundrat-imap-submit-01
        } else if (smtpMethod == QLatin1String("SMTP") || smtpMethod == QLatin1String("SSMTP")) {
            if (settings->contains(QLatin1String("msa.smtp.host"))) {
                mt->setHost(settings->value(QLatin1String("msa.smtp.host")).toString());
            }
            if (settings->contains(QLatin1String("msa.smtp.port"))) {
                mt->setPort(settings->value(QLatin1String("msa.smtp.port")).toInt());
            }
            if (settings->contains(QLatin1String("msa.smtp.auth"))) {
                if (settings->value(QLatin1String("msa.smtp.auth")).toBool()) {
                    if (settings->contains(QLatin1String("msa.smtp.auth.user"))) {
                        mt->setUserName(settings->value(QLatin1String("msa.smtp.auth.user")).toString());
                    }
                    if (settings->contains(QLatin1String("msa.smtp.auth.pass"))) {
                        mt->setPassword(settings->value(QLatin1String("msa.smtp.auth.pass")).toString());
                    }
                }
            }

            if (settings->contains(QLatin1String("msa.smtp.starttls"))) {
                if (settings->value(QLatin1String("msa.smtp.starttls")).toBool()) {
                    mt->setEncryption( MailTransport::Transport::EnumEncryption::TLS );
                }
            }
            mt->setType(MailTransport::Transport::EnumType::SMTP);
        } else if (smtpMethod == QLatin1String("sendmail")) {
            mt->setType(MailTransport::Transport::EnumType::Sendmail);
            if (settings->contains(QLatin1String("msa.sendmail"))) {
                mt->setHost(settings->value(QLatin1String("msa.sendmail")).toString());
            }
        } else {
            qWarning()<<" smtpMethod unknown "<<smtpMethod;
        }
        storeTransport( mt, true ); //only one smtp for the moment
    }
    settings->endGroup();
}

void TrojitaSettings::readIdentity()
{
    const int size = settings->beginReadArray(QLatin1String("identities"));
    for (int i=0; i<size; ++i) {
        settings->setArrayIndex(i);
        QString realName = settings->value(QLatin1String("realName")).toString();
        KPIMIdentities::Identity* identity  = createIdentity(realName);
        identity->setFullName( realName );
        identity->setIdentityName( realName );
        const QString address = settings->value(QLatin1String("address")).toString();
        identity->setPrimaryEmailAddress(address);
        const QString organisation = settings->value(QLatin1String("organisation")).toString();
        identity->setOrganization(organisation);
        QString signatureStr = settings->value(QLatin1String("signature")).toString();
        if (!signatureStr.isEmpty()) {
            KPIMIdentities::Signature signature;
            signature.setType( KPIMIdentities::Signature::Inlined );
            signature.setText( signatureStr );
            identity->setSignature( signature );
        }
        kDebug()<<" realName :"<<realName<<" address : "<<address<<" organisation : "<<organisation<<" signature: "<<signatureStr;
        storeIdentity(identity);
    }
    settings->endArray();
}


void TrojitaSettings::readGlobalSettings()
{
    //TODO
}
