/*
  This file is part of libkdepim.

  Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

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

#include "kpimprefs.h"

#include <KConfigGroup>
#include <KConfigSkeleton>

#include <QString>

using namespace KPIM;

KPimPrefs::KPimPrefs( const QString &name )
  : KConfigSkeleton( name )
{
}

KPimPrefs::KPimPrefs( KPimPrefs const &other )
  : KConfigSkeleton( other.objectName() )
  , mCustomCategories( other.mCustomCategories )
{
}

KPimPrefs::~KPimPrefs()
{
}

void KPimPrefs::usrSetDefaults()
{
  setCategoryDefaults();
}

void KPimPrefs::usrReadConfig()
{
  KConfigGroup group( config(), "General" );
  mCustomCategories = group.readEntry( "Custom Categories", QStringList() );
  if ( mCustomCategories.isEmpty() ) {
    setCategoryDefaults();
  }
  mCustomCategories.sort();
}

void KPimPrefs::usrWriteConfig()
{
  KConfigGroup group( config(), "General" );
  group.writeEntry( "Custom Categories", mCustomCategories );

  KConfigSkeleton::usrWriteConfig();
}

const QString KPimPrefs::categorySeparator( ":" );
