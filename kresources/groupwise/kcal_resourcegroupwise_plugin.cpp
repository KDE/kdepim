/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2008 Will Stephenson <wtephenson@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kcal_resourcegroupwise.h"
#include "kcal_resourcegroupwiseconfig.h"
#include "kresources_export.h"

#include <kglobal.h>
#include <klocale.h>

using namespace KCal;

class ResourceGroupwiseKCalFactory : public KRES::PluginFactory<ResourceGroupwise, ResourceGroupwiseConfig>
{
  public:
    ResourceGroupwiseKCalFactory()
      : KRES::PluginFactory<ResourceGroupwise, ResourceGroupwiseConfig>()
    {
      KGlobal::locale()->insertCatalog( QLatin1String( "kcal_groupwise" ) );
    }
};

K_EXPORT_PLUGIN(ResourceGroupwiseKCalFactory)
