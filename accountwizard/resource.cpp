/*
    Copyright (c) 2009 Volker Krause <vkrause@kde.org>

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

#include "resource.h"

#include <agenttype.h>
#include <agentmanager.h>
#include <agentinstancecreatejob.h>

#include "accountwizard_debug.h"
#include <KLocalizedString>

#include <QMetaMethod>
#include <QVariant>
#include <QtDBus/qdbusinterface.h>
#include <QtDBus/qdbusreply.h>

using namespace Akonadi;

static QVariant::Type argumentType(const QMetaObject *mo, const QString &method)
{
    QMetaMethod m;
    for (int i = 0; i < mo->methodCount(); ++i) {
        const QString signature = QLatin1String(mo->method(i).methodSignature());
        if (signature.contains(method + QLatin1Char('('))) {
            m = mo->method(i);
            break;
        }
    }

    if (m.methodSignature().isEmpty()) {
        qCWarning(ACCOUNTWIZARD_LOG) << "Did not find D-Bus method: " << method << " available methods are:";
        for (int i = 0; i < mo->methodCount(); ++i) {
            qCWarning(ACCOUNTWIZARD_LOG) << mo->method(i).methodSignature();
        }
        return QVariant::Invalid;
    }

    const QList<QByteArray> argTypes = m.parameterTypes();
    if (argTypes.count() != 1) {
        return QVariant::Invalid;
    }

    return QVariant::nameToType(argTypes.first().constData());
}

Resource::Resource(const QString &type, QObject *parent)
    : SetupObject(parent)
    , m_typeIdentifier(type)
    , m_editMode(false)
{
}

void Resource::setOption(const QString &key, const QVariant &value)
{
    m_settings.insert(key, value);
}

void Resource::setName(const QString &name)
{
    m_name = name;
}

void Resource::create()
{
    const AgentType type = AgentManager::self()->type(m_typeIdentifier);
    if (!type.isValid()) {
        Q_EMIT error(i18n("Resource type '%1' is not available.", m_typeIdentifier));
        return;
    }

    // check if unique instance already exists
    qCDebug(ACCOUNTWIZARD_LOG) << type.capabilities();
    if (type.capabilities().contains(QStringLiteral("Unique"))) {
        foreach (const AgentInstance &instance, AgentManager::self()->instances()) {
            qCDebug(ACCOUNTWIZARD_LOG) << instance.type().identifier() << (instance.type() == type);
            if (instance.type() == type) {
                if (m_editMode) {
                    edit();
                }
                Q_EMIT finished(i18n("Resource '%1' is already set up.", type.name()));
                return;
            }
        }
    }

    Q_EMIT info(i18n("Creating resource instance for '%1'...", type.name()));
    AgentInstanceCreateJob *job = new AgentInstanceCreateJob(type, this);
    connect(job, &AgentInstanceCreateJob::result, this, &Resource::instanceCreateResult);
    job->start();
}

void Resource::instanceCreateResult(KJob *job)
{
    if (job->error()) {
        Q_EMIT error(i18n("Failed to create resource instance: %1", job->errorText()));
        return;
    }

    m_instance = qobject_cast<AgentInstanceCreateJob *>(job)->instance();

    if (!m_settings.isEmpty()) {
        Q_EMIT info(i18n("Configuring resource instance..."));
        QDBusInterface iface(QStringLiteral("org.freedesktop.Akonadi.Resource.") + m_instance.identifier(), QStringLiteral("/Settings"));
        if (!iface.isValid()) {
            Q_EMIT error(i18n("Unable to configure resource instance."));
            return;
        }

        // configure resource
        if (!m_name.isEmpty()) {
            m_instance.setName(m_name);
        }
        QMap<QString, QVariant>::const_iterator end(m_settings.constEnd());
        for (QMap<QString, QVariant>::const_iterator it = m_settings.constBegin(); it != end; ++it) {
            qCDebug(ACCOUNTWIZARD_LOG) << "Setting up " << it.key() << " for agent " << m_instance.identifier();
            const QString methodName = QStringLiteral("set%1").arg(it.key());
            QVariant arg = it.value();
            const QVariant::Type targetType = argumentType(iface.metaObject(), methodName);
            if (!arg.canConvert(targetType)) {
                Q_EMIT error(i18n("Could not convert value of setting '%1' to required type %2.", it.key(), QLatin1String(QVariant::typeToName(targetType))));
                return;
            }
            arg.convert(targetType);
            QDBusReply<void> reply = iface.call(methodName, arg);
            if (!reply.isValid()) {
                Q_EMIT error(i18n("Could not set setting '%1': %2", it.key(), reply.error().message()));
                return;
            }
        }
        m_instance.reconfigure();
    }

    if (m_editMode) {
        edit();
    }
    Q_EMIT finished(i18n("Resource setup completed."));
}

void Resource::edit()
{
    if (m_instance.isValid()) {
        m_instance.configure();
    }
}

void Resource::destroy()
{
    if (m_instance.isValid()) {
        AgentManager::self()->removeInstance(m_instance);
        Q_EMIT info(i18n("Removed resource instance for '%1'.", m_instance.type().name()));
    }
}

QString Resource::identifier()
{
    return m_instance.identifier();
}

void Resource::reconfigure()
{
    m_instance.reconfigure();
}

void Resource::setEditMode(const bool editMode)
{
    m_editMode = editMode;
}
