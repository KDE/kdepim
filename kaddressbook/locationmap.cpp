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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

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
#include <kstaticdeleter.h>
#include <kurl.h>

#include "kabprefs.h"
#include "locationmap.h"

LocationMap *LocationMap::mSelf = 0;
static KStaticDeleter<LocationMap> locationMapDeleter;

LocationMap::LocationMap()
{
}

LocationMap::~LocationMap()
{
}

LocationMap *LocationMap::instance()
{
  if ( !mSelf )
    locationMapDeleter.setObject( mSelf, new LocationMap );

  return mSelf;
}

void LocationMap::showAddress( const KABC::Address &addr )
{
  KURL url( createUrl( addr ) );
  if ( url.isEmpty() )
    return;

  kapp->invokeBrowser( url.url() );
}

QString LocationMap::createUrl( const KABC::Address &addr )
{
  /**
    This method makes substitutions for the following place holders:
      %s street
      %r region
      %l locality
      %z zip code
      %c country (in ISO format)
   */

  QString urlTemplate = KABPrefs::instance()->locationMapURL().arg( KGlobal::locale()->country() );
  if ( urlTemplate.isEmpty() ) {
    KMessageBox::error( 0, i18n( "No service provider available for map lookup!\nPlease add one in the configuration dialog." ) );
    return QString::null;
  }

#if KDE_VERSION >= 319
  return urlTemplate.replace( "%s", addr.street() ).
                     replace( "%r", addr.region() ).
                     replace( "%l", addr.locality() ).
                     replace( "%z", addr.postalCode() ).
                     replace( "%c", addr.countryToISO( addr.country() ) );
#else
  return urlTemplate.replace( "%s", addr.street() ).
                     replace( "%r", addr.region() ).
                     replace( "%l", addr.locality() ).
                     replace( "%z", addr.postalCode() ).
                     replace( "%c", "" );
#endif
}

#include "locationmap.moc"
