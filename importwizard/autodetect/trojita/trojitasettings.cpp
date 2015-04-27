/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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

#include <MailTransport/mailtransport/transportmanager.h>

#include <KIdentityManagement/kidentitymanagement/identity.h>
#include <KIdentityManagement/kidentitymanagement/signature.h>

#include "importwizard_debug.h"

#include <QSettings>

TrojitaSettings::TrojitaSettings(const QString &filename, ImportWizard *parent)
    : AbstractSettings(parent)
{
    settings = new QSettings(filename, QSettings::IniFormat, this);
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

    if (settings->contains(QStringLiteral("imap.host"))) {
        name = settings->value(QStringLiteral("imap.host")).toString();
        newSettings.insert(QStringLiteral("ImapServer"), name);
    }

    if (settings->contains(QStringLiteral("imap.port"))) {
        int port = settings->value(QStringLiteral("imap.port")).toInt();
        newSettings.insert(QStringLiteral("ImapPort"), port);
    }

    if (settings->contains(QStringLiteral("imap.starttls"))) {
        const bool useTLS = settings->value(QStringLiteral("imap.starttls")).toBool();
        if (useTLS) {
            newSettings.insert(QStringLiteral("Safety"), QStringLiteral("STARTTLS"));
        }
    }

    if (settings->contains(QStringLiteral("imap.auth.user"))) {
        const QString userName = settings->value(QStringLiteral("imap.auth.user")).toString();
        if (!userName.isEmpty()) {
            newSettings.insert(QStringLiteral("Username"), userName);
        }
    }

    if (settings->contains(QStringLiteral("imap.auth.pass"))) {
        const QString password = settings->value(QStringLiteral("imap.auth.pass")).toString();
        if (!password.isEmpty()) {
            newSettings.insert(QStringLiteral("Password"), password);
        }
    }

    if (settings->contains(QStringLiteral("imap.process"))) {
        //What's this ?
    }

    if (settings->contains(QStringLiteral("imap.offline"))) {
        const bool offlineStatus = settings->value(QStringLiteral("imap.offline")).toBool();
        //It's not a deconnected mode as imap disconnected #317023
        //Will implement soon.
        //TODO use akonadi cache.
    }

    if (settings->contains(QStringLiteral("imap.enableId"))) {
        //Not supported by Akonadi.
    }

    if (settings->contains(QStringLiteral("imap.ssl.pemCertificate"))) {
        //Not supported by akonadi.
    }

    if (settings->contains(QStringLiteral("imap.capabilities.blacklist"))) {
        //Not supported by akonadi-imap-resource.
    }

    if (!name.isEmpty()) {
        const QString agentIdentifyName = AbstractBase::createResource("akonadi_imap_resource", name, newSettings);
        //Check by default
        addCheckMailOnStartup(agentIdentifyName, true);
    }
}

void TrojitaSettings::readTransport()
{
    settings->beginGroup(QStringLiteral("General"));
    const QString smtpMethod = settings->value(QStringLiteral("msa.method")).toString();
    if (!smtpMethod.isEmpty()) {
        MailTransport::Transport *mt = createTransport();
        if (smtpMethod == QStringLiteral("IMAP-SENDMAIL")) {
            //see http://tools.ietf.org/html/draft-kundrat-imap-submit-01
        } else if (smtpMethod == QStringLiteral("SMTP") || smtpMethod == QStringLiteral("SSMTP")) {
            if (settings->contains(QStringLiteral("msa.smtp.host"))) {
                mt->setHost(settings->value(QStringLiteral("msa.smtp.host")).toString());
            }
            if (settings->contains(QStringLiteral("msa.smtp.port"))) {
                mt->setPort(settings->value(QStringLiteral("msa.smtp.port")).toInt());
            }
            if (settings->contains(QStringLiteral("msa.smtp.auth"))) {
                if (settings->value(QStringLiteral("msa.smtp.auth")).toBool()) {
                    if (settings->contains(QStringLiteral("msa.smtp.auth.user"))) {
                        mt->setUserName(settings->value(QStringLiteral("msa.smtp.auth.user")).toString());
                    }
                    if (settings->contains(QStringLiteral("msa.smtp.auth.pass"))) {
                        mt->setPassword(settings->value(QStringLiteral("msa.smtp.auth.pass")).toString());
                    }
                }
            }

            if (settings->contains(QStringLiteral("msa.smtp.starttls"))) {
                if (settings->value(QStringLiteral("msa.smtp.starttls")).toBool()) {
                    mt->setEncryption(MailTransport::Transport::EnumEncryption::TLS);
                }
            }
            mt->setType(MailTransport::Transport::EnumType::SMTP);
        } else if (smtpMethod == QStringLiteral("sendmail")) {
            mt->setType(MailTransport::Transport::EnumType::Sendmail);
            if (settings->contains(QStringLiteral("msa.sendmail"))) {
                mt->setHost(settings->value(QStringLiteral("msa.sendmail")).toString());
            }
        } else {
            qCWarning(IMPORTWIZARD_LOG) << " smtpMethod unknown " << smtpMethod;
        }
        storeTransport(mt, true);   //only one smtp for the moment
    }
    settings->endGroup();
}

void TrojitaSettings::readIdentity()
{
    const int size = settings->beginReadArray(QStringLiteral("identities"));
    for (int i = 0; i < size; ++i) {
        settings->setArrayIndex(i);
        QString realName = settings->value(QStringLiteral("realName")).toString();
        KIdentityManagement::Identity *identity  = createIdentity(realName);
        identity->setFullName(realName);
        identity->setIdentityName(realName);
        const QString address = settings->value(QStringLiteral("address")).toString();
        identity->setPrimaryEmailAddress(address);
        const QString organisation = settings->value(QStringLiteral("organisation")).toString();
        identity->setOrganization(organisation);
        QString signatureStr = settings->value(QStringLiteral("signature")).toString();
        if (!signatureStr.isEmpty()) {
            KIdentityManagement::Signature signature;
            signature.setType(KIdentityManagement::Signature::Inlined);
            signature.setText(signatureStr);
            identity->setSignature(signature);
        }
        qCDebug(IMPORTWIZARD_LOG) << " realName :" << realName << " address : " << address << " organisation : " << organisation << " signature: " << signatureStr;
        storeIdentity(identity);
    }
    settings->endArray();
}

void TrojitaSettings::readGlobalSettings()
{
    //TODO
}
