/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "agentconfigmodel.h"

#include <KDebug>

#include <QDBusInterface>
#include <QMetaMethod>
#include <KLocalizedString>

AgentConfigModel::AgentConfigModel(QObject* parent): QAbstractTableModel(parent), m_interface( 0 )
{
}

AgentConfigModel::~AgentConfigModel()
{
  delete m_interface;
}

void AgentConfigModel::setAgentInstance(const Akonadi::AgentInstance& instance)
{
  m_settings.clear();
  reset();

  m_interface = new QDBusInterface(
    QString::fromLatin1("org.freedesktop.Akonadi.Agent.%1").arg( instance.identifier() ),
    "/Settings" );
  if ( !m_interface->isValid() ) {
    kError() << "Unable to obtain KConfigXT D-Bus interface of agent" << instance.identifier();
    delete m_interface;
    return;
  }

  reload();
}

void AgentConfigModel::reload()
{
  m_settings.clear();
  for ( int i = 0; i < m_interface->metaObject()->methodCount(); ++i ) {
    const QMetaMethod method = m_interface->metaObject()->method( i );
    if ( QByteArray( method.typeName() ).isEmpty() ) // returns void
      continue;
#if 0 //QT5
    const QByteArray signature( method.signature() );
    if ( signature.isEmpty() )
      continue;
    if ( signature.startsWith( "set" ) || !signature.contains( "()" ) ) // setter or takes parameters // krazy:exclude=strings
      continue;
    if ( signature.startsWith( "Introspect" ) ) // D-Bus stuff // krazy:exclude=strings
      continue;
    const QString methodName = QString::fromLatin1( signature.left( signature.indexOf( '(' ) ) );
    const QDBusMessage reply = m_interface->call( methodName );
    if ( !reply.arguments().count() == 1 ) {
      kError() << "call to method" << signature << "failed: " << reply.arguments() << reply.errorMessage();
      continue;
    }
    const QString settingName = methodName.at( 0 ).toUpper() + methodName.mid( 1 );
    m_settings.append( qMakePair( settingName, reply.arguments().at( 0 ) ) );
#endif
  }
  reset();
}


int AgentConfigModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED( parent );
  return 3;
}

int AgentConfigModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED( parent );
  return m_settings.size();
}

QVariant AgentConfigModel::data(const QModelIndex& index, int role) const
{
  if ( index.row() < 0 || index.row() >= m_settings.size() )
    return QVariant();
  QPair<QString, QVariant> setting = m_settings.at( index.row() );
  if ( role == Qt::DisplayRole || role == Qt::EditRole ) {
    if ( index.column() == 0 )
      return setting.first;
    else if ( index.column() == 1 )
      return setting.second;
    else if ( index.column() == 2 )
      return setting.second.typeName();
  }
  return QVariant();
}

bool AgentConfigModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if ( index.column() == 1 && role == Qt::EditRole && index.row() >= 0 && index.row() < m_settings.size() ) {
    const QPair<QString, QVariant> setting = m_settings.at( index.row() );
    if ( setting.second != value ) {
      m_interface->call( QString::fromLatin1( "set%1" ).arg( setting.first ), value );
      reload();
    }
  }
  return QAbstractItemModel::setData(index, value, role);
}

QVariant AgentConfigModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole ) {
    if ( section == 0 )
      return i18n( "Setting" );
    else if ( section == 1 )
      return i18n( "Value" );
    else if ( section == 2 )
      return i18n( "Type" );
  }
  return QAbstractItemModel::headerData(section, orientation, role);
}

Qt::ItemFlags AgentConfigModel::flags(const QModelIndex& index) const
{
  if ( index.column() == 1 )
    return QAbstractItemModel::flags( index ) | Qt::ItemIsEditable;
  return QAbstractItemModel::flags(index);
}

void AgentConfigModel::writeConfig()
{
  m_interface->call( "writeConfig" );
}

