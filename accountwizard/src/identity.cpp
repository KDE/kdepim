/*
    Copyright (c) 2010 Laurent Montel <montel@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "identity.h"
#include "transport.h"

#include <kidentitymanagement/identitymanager.h>
#include <kidentitymanagement/identity.h>

#include <KLocalizedString>

Identity::Identity(QObject *parent)
    : SetupObject(parent),
{
    m_manager = new KIdentityManagement::IdentityManager(false, this, "mIdentityManager");
    m_identity = &m_manager->newFromScratch(QString());
    Q_ASSERT(m_identity != 0);
}

Identity::~Identity()
{
    delete m_manager;
}

void Identity::create()
{
    Q_EMIT info(i18n("Setting up identity..."));

    // store identity information
    m_identity->setIdentityName(identityName());

    m_manager->setAsDefault(m_identity->uoid());
    m_manager->commit();

    Q_EMIT finished(i18n("Identity set up."));
}

QString Identity::identityName() const
{
    // create identity name
    QString name(m_identityName);
    if (name.isEmpty()) {
        name = i18nc("Default name for new email accounts/identities.", "Unnamed");

        QString idName = m_identity->primaryEmailAddress();
        int pos = idName.indexOf(QLatin1Char('@'));
        if (pos != -1) {
            name = idName.mid(0, pos);
        }

        // Make the name a bit more human friendly
        name.replace(QLatin1Char('.'), QLatin1Char(' '));
        pos = name.indexOf(QLatin1Char(' '));
        if (pos != 0) {
            name[ pos + 1 ] = name[ pos + 1 ].toUpper();
        }
        name[ 0 ] = name[ 0 ].toUpper();
    }

    if (!m_manager->isUnique(name)) {
        name = m_manager->makeUnique(name);
    }
    return name;
}

void Identity::destroy()
{
    m_manager->removeIdentityForced(m_identity->identityName());
    m_manager->commit();
    m_identity = 0;
    Q_EMIT info(i18n("Identity removed."));
}

void Identity::setIdentityName(const QString &name)
{
    m_identityName = name;
}

void Identity::setRealName(const QString &name)
{
    m_identity->setFullName(name);
}

void Identity::setOrganization(const QString &org)
{
    m_identity->setOrganization(org);
}

void Identity::setEmail(const QString &email)
{
    m_identity->setPrimaryEmailAddress(email);
}

uint Identity::uoid() const
{
    return m_identity->uoid();
}

void Identity::setTransport(QObject *transport)
{
    if (transport) {
        m_identity->setTransport(QString::number(qobject_cast<Transport*>(transport)->transportId()));
    } else {
        m_identity->setTransport(QString());
    }
    setDependsOn(qobject_cast<SetupObject *>(transport));
}

void Identity::setSignature(const QString &sig)
{
    if (!sig.isEmpty()) {
        const KIdentityManagement::Signature signature(sig);
        m_identity->setSignature(signature);
    } else {
        m_identity->setSignature(KIdentityManagement::Signature());
    }
}

void Identity::setPreferredCryptoMessageFormat(const QString &format)
{
    m_identity->setPreferredCryptoMessageFormat(format);
}

void Identity::setXFace(const QString &xface)
{
    m_identity->setXFaceEnabled(!xface.isEmpty());
    m_identity->setXFace(xface);
}

