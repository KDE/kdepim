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

#include "trojitasettings.h"
#include "importwizardutil.h"

#include <mailtransport/transportmanager.h>

#include <kpimidentities/identity.h>
#include <kpimidentities/signature.h>

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
    //TODO
    QMap<QString, QVariant> newSettings;
    QString name;

    if (settings->contains(QLatin1String("imap.host"))) {

    }

    if (settings->contains(QLatin1String("imap.port"))) {

    }

    if (settings->contains(QLatin1String("imap.starttls"))) {

    }

    if (settings->contains(QLatin1String("imap.auth.user"))) {

    }

    if (settings->contains(QLatin1String("imap.auth.pass"))) {

    }

    if (settings->contains(QLatin1String("imap.process"))) {

    }

    if (settings->contains(QLatin1String("imap.offline"))) {

    }

    if (settings->contains(QLatin1String("imap.enableId"))) {

    }

    if (settings->contains(QLatin1String("imap.ssl.pemCertificate"))) {

    }

    if (settings->contains(QLatin1String("imap.capabilities.blacklist"))) {

    }

    if (!name.isEmpty()) {
        const QString agentIdentifyName = AbstractBase::createResource( "akonadi_imap_resource", name, newSettings );
    }
}

void TrojitaSettings::readTransport()
{
    settings->beginGroup(QLatin1String("General"));
    const QString smtpMethod = settings->value(QLatin1String("msa.method")).toString();
    if (!smtpMethod.isEmpty()) {
        MailTransport::Transport *mt = createTransport();
#if 0
        QString smtpHostKey = QLatin1String("msa.smtp.host");
        QString smtpPortKey = QLatin1String("msa.smtp.port");
        QString smtpAuthKey = QLatin1String("msa.smtp.auth");
        QString smtpStartTlsKey = QLatin1String("msa.smtp.starttls");
        QString smtpUserKey = QLatin1String("msa.smtp.auth.user");
        QString smtpPassKey = QLatin1String("msa.smtp.auth.pass");
        QString sendmailKey = QLatin1String("msa.sendmail");
        QString sendmailDefaultCmd = QLatin1String("sendmail -bm -oi");

#endif

        if (smtpMethod == QLatin1String("IMAP-SENDMAIL")) {

        } else if (smtpMethod == QLatin1String("SMTP")) {

        } else if (smtpMethod == QLatin1String("SSMTP")) {

        } else if (smtpMethod == QLatin1String("sendmail")) {
            mt->setType(MailTransport::Transport::EnumType::Sendmail);
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
        KPIMIdentities::Identity* identity  = createIdentity();
        const QString realName = settings->value(QLatin1String("realName")).toString();
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
        qDebug()<<" realName :"<<realName<<" address : "<<address<<" organisation : "<<organisation<<" signature: "<<signatureStr;
        storeIdentity(identity);
    }
    settings->endArray();
}


void TrojitaSettings::readGlobalSettings()
{
    //TODO
}
