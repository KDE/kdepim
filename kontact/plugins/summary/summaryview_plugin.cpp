/*
*   Copyright 2010 Ryan Rix <ry@n.rix.si>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "summaryview_plugin.h"

#include <plasmakpart.h>

#include <kontactinterface/core.h>
#include <kontactinterface/plugin.h>

#include <kactioncollection.h>
#include <kgenericfactory.h>
#include <kicon.h>
#include <kparts/componentfactory.h>

EXPORT_KONTACT_PLUGIN( SummaryPlugin, summary )

SummaryPlugin::SummaryPlugin( KontactInterface::Core *core, const QVariantList & )
    : KontactInterface::Plugin( core, core, "Summary" )
{
    setComponentData( KontactPluginFactory::componentData() );
}

SummaryPlugin::~SummaryPlugin()
{
}

KParts::ReadOnlyPart *SummaryPlugin::createPart()
{
KParts::ReadOnlyPart *part = loadPart();

connect( part, SIGNAL(showPart()), this, SLOT(showPart()) );

return part;
}

void SummaryPlugin::readProperties( const KConfigGroup &config )
{
}

void SummaryPlugin::saveProperties( KConfigGroup &config )
{
}

void SummaryPlugin::showPart()
{
core()->selectPlugin( this );
}


#include "summaryview_plugin.moc"