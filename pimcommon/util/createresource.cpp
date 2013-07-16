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

#include "createresource.h"

#include <KDebug>
#include <KLocale>

#include <akonadi/agenttype.h>
#include <akonadi/agentmanager.h>
#include <akonadi/agentinstancecreatejob.h>

#include <QDBusReply>
#include <QDBusInterface>
#include <QMetaMethod>

using namespace Akonadi;

using namespace PimCommon;

CreateResource::CreateResource()
{
}

CreateResource::~CreateResource()
{
}

//code from accountwizard
static QVariant::Type argumentType( const QMetaObject *mo, const QString &method )
{
    QMetaMethod m;
    const int numberOfMethod( mo->methodCount() );
    for ( int i = 0; i < numberOfMethod; ++i ) {
        const QString signature = QString::fromLatin1( mo->method( i ).signature() );
        if ( signature.contains(method + QLatin1Char('(') )) {
            m = mo->method( i );
            break;
        }
    }

    if ( !m.signature() ) {
        kWarning() << "Did not find D-Bus method: " << method << " available methods are:";
        const int numberOfMethod(mo->methodCount());
        for ( int i = 0; i < numberOfMethod; ++ i )
            kWarning() << mo->method( i ).signature();
        return QVariant::Invalid;
    }

    const QList<QByteArray> argTypes = m.parameterTypes();
    if ( argTypes.count() != 1 )
        return QVariant::Invalid;

    return QVariant::nameToType( argTypes.first() );
}

QString CreateResource::createResource( const QString& resources, const QString& name, const QMap<QString, QVariant>& settings )
{
    const AgentType type = AgentManager::self()->type( resources );
    if ( !type.isValid() ) {
        Q_EMIT createResourceError( i18n( "Resource type '%1' is not available.", resources ) );
        return QString();
    }

    // check if unique instance already exists
    kDebug() << type.capabilities();
    if ( type.capabilities().contains( QLatin1String( "Unique" ) ) ) {
        Q_FOREACH ( const AgentInstance &instance, AgentManager::self()->instances() ) {
            kDebug() << instance.type().identifier() << (instance.type() == type);
            if ( instance.type() == type ) {
                Q_EMIT createResourceInfo(i18n( "Resource '%1' is already set up.", type.name() ) );
                return QString();
            }
        }
    }

    Q_EMIT createResourceInfo( i18n( "Creating resource instance for '%1'...", type.name() ) );
    AgentInstanceCreateJob *job = new AgentInstanceCreateJob( type, this );
    if (job->exec()) {
        Akonadi::AgentInstance instance = job->instance();

        if ( !settings.isEmpty() ) {
            Q_EMIT createResourceInfo( i18n( "Configuring resource instance..." ) );
            QDBusInterface iface( QLatin1String("org.freedesktop.Akonadi.Resource.") + instance.identifier(), QLatin1String("/Settings") );
            if ( !iface.isValid() ) {
                Q_EMIT createResourceError( i18n( "Unable to configure resource instance." ) );
                return QString();
            }

            // configure resource
            if ( !name.isEmpty() )
                instance.setName( name );
            QMap<QString, QVariant>::const_iterator end( settings.constEnd());
            for ( QMap<QString, QVariant>::const_iterator it = settings.constBegin(); it != end; ++it ) {
                kDebug() << "Setting up " << it.key() << " for agent " << instance.identifier();
                const QString methodName = QString::fromLatin1("set%1").arg( it.key() );
                QVariant arg = it.value();
                const QVariant::Type targetType = argumentType( iface.metaObject(), methodName );
                if ( !arg.canConvert( targetType ) ) {
                    Q_EMIT createResourceError( i18n( "Could not convert value of setting '%1' to required type %2.", it.key(), QString::fromLatin1(QVariant::typeToName( targetType ) )) );
                    return QString();
                }
                arg.convert( targetType );
                QDBusReply<void> reply = iface.call( methodName, arg );
                if ( !reply.isValid() ) {
                    Q_EMIT createResourceError( i18n( "Could not set setting '%1': %2", it.key(), reply.error().message() ) );
                    return QString();
                }
            }
            instance.reconfigure();
        }

        Q_EMIT createResourceInfo( i18n( "Resource setup completed." ) );
        return instance.identifier();
    } else {
        if ( job->error() ) {
            Q_EMIT createResourceError( i18n( "Failed to create resource instance: %1", job->errorText() ) );
        }
    }
    return QString();
}

#include "createresource.moc"
