/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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

#include "kabc_resourcegroupware.h"
#include "kabc_resourcegroupwareconfig.h"

#include <kglobal.h>
#include <klocale.h>

using namespace KABC;

typedef KRES::PluginFactory<ResourceGroupware, ResourceGroupwareConfig> GroupwareFactory;

extern "C"
{
  void *init_kabc_groupware()
  {
    KGlobal::locale()->insertCatalogue( "libkcal" );
    KGlobal::locale()->insertCatalogue( "kres_groupware" );
    return new ResourceGroupware;
  }
}
