/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2008 Will Stephenson <wtephenson@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "kabc_resourcegroupwise.h"
#include "kabc_resourcegroupwiseconfig.h"
#include "kresources_export.h"

#include <kglobal.h>
#include <klocale.h>

using namespace KABC;

class ResourceGroupwiseFactory : public KRES::PluginFactory<ResourceGroupwise, ResourceGroupwiseConfig>
{
  public:
    ResourceGroupwiseFactory()
      : KRES::PluginFactory<ResourceGroupwise, ResourceGroupwiseConfig>()
    {
      KGlobal::locale()->insertCatalog( QLatin1String( "kres_groupwise" ) );
    }
};

K_EXPORT_PLUGIN(ResourceGroupwiseFactory)
