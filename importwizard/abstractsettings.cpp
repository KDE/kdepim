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
#include "abstractsettings.h"
#include "importwizard.h"

#include <KDebug>

#include <akonadi/agenttype.h>
#include <akonadi/agentmanager.h>
#include <akonadi/agentinstancecreatejob.h>

using namespace Akonadi;

AbstractSettings::AbstractSettings(ImportWizard *parent)
{
}

AbstractSettings::~AbstractSettings()
{

}

void AbstractSettings::createResource( const QString& resources )
{
  const AgentType type = AgentManager::self()->type( resources );
  if ( !type.isValid() ) {
    //emit error( i18n( "Resource type '%1' is not available.", resources ) );
    return;
  }

  // check if unique instance already exists
  kDebug() << type.capabilities();
  if ( type.capabilities().contains( QLatin1String( "Unique" ) ) ) {
    foreach ( const AgentInstance &instance, AgentManager::self()->instances() ) {
      kDebug() << instance.type().identifier() << (instance.type() == type);
      if ( instance.type() == type ) {
        //emit finished( i18n( "Resource '%1' is already set up.", type.name() ) );
        return;
      }
    }
  }

  //emit info( i18n( "Creating resource instance for '%1'...", type.name() ) );
  AgentInstanceCreateJob *job = new AgentInstanceCreateJob( type, this );
  connect( job, SIGNAL(result(KJob*)), SLOT(instanceCreateResult(KJob*)) );
  job->start();

}

void AbstractSettings::instanceCreateResult(KJob* job)
{
#if 0  
  if ( job->error() ) {
    //emit error( i18n( "Failed to create resource instance: %1", job->errorText() ) );
    return;
  }

  m_instance = qobject_cast<AgentInstanceCreateJob*>( job )->instance();

  if ( !m_settings.isEmpty() ) {
    //emit info( i18n( "Configuring resource instance..." ) );
    QDBusInterface iface( "org.freedesktop.Akonadi.Resource." + m_instance.identifier(), "/Settings" );
    if ( !iface.isValid() ) {
      emit error( i18n( "Unable to configure resource instance." ) );
      return;
    }

    // configure resource
    if ( !m_name.isEmpty() )
      m_instance.setName( m_name );
    QMap<QString, QVariant>::const_iterator end( m_settings.constEnd());
    for ( QMap<QString, QVariant>::const_iterator it = m_settings.constBegin(); it != end; ++it ) {
      kDebug() << "Setting up " << it.key() << " for agent " << m_instance.identifier();
      const QString methodName = QString::fromLatin1("set%1").arg( it.key() );
      QVariant arg = it.value();
      const QVariant::Type targetType = argumentType( iface.metaObject(), methodName );
      if ( !arg.canConvert( targetType ) ) {
        emit error( i18n( "Could not convert value of setting '%1' to required type %2.", it.key(), QVariant::typeToName( targetType ) ) );
        return;
      }
      arg.convert( targetType );
      QDBusReply<void> reply = iface.call( methodName, arg );
      if ( !reply.isValid() ) {
        emit error( i18n( "Could not set setting '%1': %2", it.key(), reply.error().message() ) );
        return;
      }
    }
    m_instance.reconfigure();
  }

  emit finished( i18n( "Resource setup completed." ) );
#endif
}
