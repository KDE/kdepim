/*
    This file is part of KAddressbook.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <kapplication.h>
#include <kconfig.h>
#include <kdeversion.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <ktoolinvocation.h>

#include "kabprefs.h"
#include "locationmap.h"

class LocationMapHelper {
  public:
    LocationMapHelper() : q( 0 ) {}
    ~LocationMapHelper() { delete q; }
    LocationMap *q;
};

K_GLOBAL_STATIC( LocationMapHelper, s_globalLocationMap )

LocationMap::LocationMap()
{
  Q_ASSERT( !s_globalLocationMap->q );
  s_globalLocationMap->q = this;
}

LocationMap::~LocationMap()
{
}

LocationMap *LocationMap::instance()
{
  if (!s_globalLocationMap->q) {
    new LocationMap;
  }

  return s_globalLocationMap->q;
}

void LocationMap::showAddress( const KABC::Address &addr )
{
  KUrl url( createUrl( addr ) );
  if ( url.isEmpty() )
    return;

  KToolInvocation::invokeBrowser( url.url() );
}

QString LocationMap::createUrl( const KABC::Address &addr )
{
  /**
    This method makes substitutions for the following place holders:
      %s street
      %r region
      %l locality
      %z zip code
      %n country name
      %c country (in ISO format)
   */

  QString urlTemplate = KABPrefs::instance()->locationMapURL().arg( KGlobal::locale()->country() );
  if ( urlTemplate.isEmpty() ) {
    KMessageBox::error( 0, i18n( "No service provider available for map lookup!\nPlease add one in the configuration dialog." ) );
    return QString();
  }

  return urlTemplate.replace( "%s", addr.street() ).
                     replace( "%r", addr.region() ).
                     replace( "%l", addr.locality() ).
                     replace( "%z", addr.postalCode() ).
                     replace( "%n", addr.country() ).
                     replace( "%c", addr.countryToISO( addr.country() ) );
}

#include "locationmap.moc"
