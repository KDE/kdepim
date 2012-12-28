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
    QMap<QString, QVariant> settings;
    QString name;
    if (!name.isEmpty()) {
        const QString agentIdentifyName = AbstractBase::createResource( "akonadi_imap_resource", name, settings );
    }
}

void TrojitaSettings::readTransport()
{
    settings->beginGroup(QLatin1String("General"));
    const QString smtpMethod = settings->value(QLatin1String("msa.method")).toString();

    if (smtpMethod == QLatin1String("IMAP-SENDMAIL")) {

    } else if (smtpMethod == QLatin1String("SMTP")) {

    } else if (smtpMethod == QLatin1String("SSMTP")) {

    } else if (smtpMethod == QLatin1String("sendmail")) {

    } else {
        qWarning()<<" smtpMethod unknown "<<smtpMethod;
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
        const QString signature = settings->value(QLatin1String("signature")).toString();
        qDebug()<<" realName :"<<realName<<" address : "<<address<<" organisation : "<<organisation<<" signature: "<<signature;
        storeIdentity(identity);
    }
    settings->endArray();
}


void TrojitaSettings::readGlobalSettings()
{
    //TODO
}
