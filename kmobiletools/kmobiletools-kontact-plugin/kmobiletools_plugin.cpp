/***************************************************************************
   Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>
   Copyright (C) 2007 Marco Gulino <marco@kmobiletools.org>
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "kmobiletools_plugin.h"

#include <kgenericfactory.h>
#include <kparts/componentfactory.h>
#include <kaboutdata.h>

#include <kontact/core.h>
#include <kontact/plugin.h>
#include <kapplication.h>
#include <dcopclient.h>
#include <kaction.h>


typedef KGenericFactory<KMmobileToolsPlugin, Kontact::Core> KMmobileToolsPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_kmobiletools,
                            KMmobileToolsPluginFactory( "kontact_kmobiletools" ) )

KMmobileToolsPlugin::KMmobileToolsPlugin( Kontact::Core *core, const char *, const QStringList& )
    : Kontact::Plugin( core, core, "KMmobileTools" ), partLoaded(false)
{
  kmtIface=0;
  setInstance( KMmobileToolsPluginFactory::instance() );
  insertNewAction( new KAction( i18n( "New SMS..." ), "newsms",
                   CTRL+SHIFT+Key_S, this, SLOT( slotNewSMS() ), actionCollection(),
                   "sms_new" ) );
  setExecutableName("kmobiletools_bin");
  kmtIface=new MainIFace_stub( "kmobiletools", "KMobileTools" );
}

KMmobileToolsPlugin::~KMmobileToolsPlugin()
{
}

bool KMmobileToolsPlugin::isRunningStandalone()
{
    return ( (!partLoaded) && kapp->dcopClient()->isApplicationRegistered("kmobiletools") );
}

KParts::ReadOnlyPart* KMmobileToolsPlugin::createPart()
{
  KParts::ReadOnlyPart* m_part=loadPart();
  partLoaded=(bool)m_part;
  return m_part;
}

void KMmobileToolsPlugin::slotNewSMS()
{
    if(kmtIface) kmtIface->newSMS();
}

#include "kmobiletools_plugin.moc"
