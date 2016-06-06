/*
   Copyright (C) 2012-2016 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "abstractsettings.h"
#include "importwizard.h"
#include "importsettingpage.h"

#include <KIdentityManagement/kidentitymanagement/identitymanager.h>
#include <KIdentityManagement/kidentitymanagement/identity.h>
#include <MailTransport/mailtransport/transportmanager.h>

#include <KLocalizedString>
#include <KSharedConfig>

#include <AkonadiCore/agenttype.h>
#include <AkonadiCore/agentmanager.h>
#include <AkonadiCore/agentinstancecreatejob.h>

#include <QMetaMethod>

using namespace Akonadi;

AbstractSettings::AbstractSettings(ImportWizard *parent)
    : mImportWizard(parent)
{
    mManager = new KIdentityManagement::IdentityManager(false, this, "mIdentityManager");
    mKmailConfig = KSharedConfig::openConfig(QStringLiteral("kmail2rc"));
}

AbstractSettings::~AbstractSettings()
{
    syncKmailConfig();
    delete mManager;
}

KIdentityManagement::Identity *AbstractSettings::createIdentity(QString &name)
{
    name = uniqueIdentityName(name);
    KIdentityManagement::Identity *identity = &mManager->newFromScratch(name);
    addImportInfo(i18n("Setting up identity..."));
    return identity;
}

void AbstractSettings::storeIdentity(KIdentityManagement::Identity *identity)
{
    mManager->setAsDefault(identity->uoid());
    mManager->commit();
    addImportInfo(i18n("Identity set up."));
}

QString AbstractSettings::uniqueIdentityName(const QString &name)
{
    QString newName(name);
    int i = 0;
    while (!mManager->isUnique(newName)) {
        newName = QStringLiteral("%1_%2").arg(name).arg(i);
        ++i;
    }
    return newName;
}

MailTransport::Transport *AbstractSettings::createTransport()
{
    MailTransport::Transport *mt = MailTransport::TransportManager::self()->createTransport();
    addImportInfo(i18n("Setting up transport..."));
    return mt;
}

void AbstractSettings::storeTransport(MailTransport::Transport *mt, bool isDefault)
{
    mt->forceUniqueName();
    mt->save();
    MailTransport::TransportManager::self()->addTransport(mt);
    if (isDefault) {
        MailTransport::TransportManager::self()->setDefaultTransport(mt->id());
    }
    addImportInfo(i18n("Transport set up."));
}

void AbstractSettings::addImportInfo(const QString &log)
{
    mImportWizard->importSettingPage()->addImportInfo(log);
}

void AbstractSettings::addImportError(const QString &log)
{
    mImportWizard->importSettingPage()->addImportError(log);
}

void AbstractSettings::addCheckMailOnStartup(const QString &agentIdentifyName, bool loginAtStartup)
{
    if (agentIdentifyName.isEmpty()) {
        return;
    }
    const QString groupName = QStringLiteral("Resource %1").arg(agentIdentifyName);
    addKmailConfig(groupName, QStringLiteral("CheckOnStartup"), loginAtStartup);
}

void AbstractSettings::addToManualCheck(const QString &agentIdentifyName, bool manualCheck)
{
    if (agentIdentifyName.isEmpty()) {
        return;
    }
    const QString groupName = QStringLiteral("Resource %1").arg(agentIdentifyName);
    addKmailConfig(groupName, QStringLiteral("IncludeInManualChecks"), manualCheck);
}

void AbstractSettings::addComposerHeaderGroup(const QString &groupName, const QString &name, const QString &value)
{
    KConfigGroup group = mKmailConfig->group(groupName);
    group.writeEntry(QStringLiteral("name"), name);
    group.writeEntry(QStringLiteral("value"), value);
}

void AbstractSettings::addKmailConfig(const QString &groupName, const QString &key, const QString &value)
{
    KConfigGroup group = mKmailConfig->group(groupName);
    group.writeEntry(key, value);
}

void AbstractSettings::addKmailConfig(const QString &groupName, const QString &key, bool value)
{
    KConfigGroup group = mKmailConfig->group(groupName);
    group.writeEntry(key, value);
}

void AbstractSettings::addKmailConfig(const QString &groupName, const QString &key, int value)
{
    KConfigGroup group = mKmailConfig->group(groupName);
    group.writeEntry(key, value);
}

void AbstractSettings::syncKmailConfig()
{
    mKmailConfig->sync();
}

int AbstractSettings::readKmailSettings(const QString &groupName, const QString &key)
{
    KConfigGroup group = mKmailConfig->group(groupName);
    int value = group.readEntry(key, -1);
    return value;
}
