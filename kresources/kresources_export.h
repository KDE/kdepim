/*  This file is part of the KDE project
    Copyright (C) 2008 Jarosław Staniek <staniek@kde.org>

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

#ifndef KRESOURCES_KDEPIM_EXPORT_H
#define KRESOURCES_KDEPIM_EXPORT_H

/** Exports a function returning kresources plugin factory, for resource class @a resourceclass, 
 *  @a resourceconfigclass config class and @a catalog catalog.
 */
#define EXPORT_KRESOURCES_PLUGIN( resourceclass, resourceconfigclass, catalog ) \
  typedef KRES::PluginFactory< resourceclass, resourceconfigclass > FactoryBase; \
  class Factory : public FactoryBase { \
    public: Factory() { KGlobal::locale()->insertCatalog(catalog); } \
  }; \
  K_EXPORT_PLUGIN( Factory )

/** Like EXPORT_KRESOURCES_PLUGIN but allows to specify two catalogs.
 */
#define EXPORT_KRESOURCES_PLUGIN2( resourceclass, resourceconfigclass, catalog1, catalog2 ) \
  typedef KRES::PluginFactory< resourceclass, resourceconfigclass > FactoryBase; \
  class Factory : public FactoryBase { \
    public: Factory() { KGlobal::locale()->insertCatalog(catalog1); \
      KGlobal::locale()->insertCatalog(catalog2); } \
  }; \
  K_EXPORT_PLUGIN( Factory )

#endif
