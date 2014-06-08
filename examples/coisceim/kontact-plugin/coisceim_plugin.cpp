/*
    This file is part of Akonadi.

    Copyright (c) 2011 Stephen Kelly <steveire@gmail.com>

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

#include "coisceim_plugin.h"

#include <KontactInterface/Core>

#include <QAction>
#include <KActionCollection>
#include <KCmdLineArgs>
#include <KIcon>
#include <KLocale>
#include <qdebug.h>

EXPORT_KONTACT_PLUGIN( CoisceimPlugin, coisceim )

CoisceimPlugin::CoisceimPlugin( KontactInterface::Core *core, const QVariantList & )
  : KontactInterface::Plugin( core, core, "coisceim" ), m_interface( 0 )
{
  setComponentData( KontactPluginFactory::componentData() );

  QAction *action =
    new QAction( KIcon( "byobu" ),
                 i18nc( "@action:inmenu", "New Trip" ), this );
  actionCollection()->addAction( "new_trip", action );
  //action->setHelpText(
  //  i18nc( "@info:status", "Create a new trip" ) );
  connect( action, SIGNAL(triggered(bool)), SLOT(createTrip()) );
  insertNewAction( action );

  mUniqueAppWatcher = new KontactInterface::UniqueAppWatcher(
    new KontactInterface::UniqueAppHandlerFactory<CoisceimUniqueAppHandler>(), this );
}

CoisceimPlugin::~CoisceimPlugin()
{
  delete m_interface;
  m_interface = 0;
}

bool CoisceimPlugin::isRunningStandalone() const
{
  return mUniqueAppWatcher->isRunningStandalone();
}

KParts::ReadOnlyPart *CoisceimPlugin::createPart()
{
  KParts::ReadOnlyPart *part = loadPart();
  if ( !part ) {
    return 0;
  }

  m_interface = new org::kde::coisceim::CoisceimWidget(
    "org.kde.coisceim", "/CoisceimWidget", QDBusConnection::sessionBus() );

  return part;
}

org::kde::coisceim::CoisceimWidget *CoisceimPlugin::interface()
{
  if ( !m_interface ) {
    part();
  }
  Q_ASSERT( m_interface );
  return m_interface;
}

void CoisceimPlugin::createTrip()
{
  qDebug() << "CALL CREATE";
  core()->selectPlugin( this );
  interface()->createTrip();
}

void CoisceimUniqueAppHandler::loadCommandLineOptions()
{
  //  No command line args to load.
  KCmdLineArgs::addCmdLineOptions( KCmdLineOptions() );
}

int CoisceimUniqueAppHandler::newInstance()
{
  // Ensure part is loaded
  (void)plugin()->part();
  return KontactInterface::UniqueAppHandler::newInstance();
}

