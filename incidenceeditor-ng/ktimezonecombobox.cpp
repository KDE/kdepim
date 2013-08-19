/*
  Copyright (C) 2007 Bruno Virlet <bruno.virlet@gmail.com>
  Copyright 2008-2009,2013 Allen Winter <winter@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "ktimezonecombobox.h"

#include <KCalCore/ICalTimeZones>

#include <KGlobal>
#include <KLocale>
#include <KSystemTimeZone>

using namespace IncidenceEditorNG;

class KTimeZoneComboBox::Private
{
  public:
    Private( KTimeZoneComboBox *parent )
      : mParent( parent ), mAdditionalZones( 0 )
    {}

    void fillComboBox();
    KTimeZoneComboBox *const mParent;
    QStringList mZones;
    const KCalCore::ICalTimeZones *mAdditionalZones;
};

void KTimeZoneComboBox::Private::fillComboBox()
{
  mParent->clear();
  mZones.clear();

  // Read all system time zones
  const KTimeZones::ZoneMap timezones = KSystemTimeZones::zones();
  for ( KTimeZones::ZoneMap::ConstIterator it=timezones.begin(); it != timezones.end(); ++it ) {
    mZones.append( it.key().toUtf8() );
  }
  mZones.sort();

  // Prepend the list of additional timezones
  if ( mAdditionalZones ) {
    const KCalCore::ICalTimeZones::ZoneMap calzones = mAdditionalZones->zones();
    for ( KCalCore::ICalTimeZones::ZoneMap::ConstIterator it=calzones.begin();
          it != calzones.end(); ++it ) {
      mZones.prepend( it.key().toUtf8() );
    }
  }
  // Prepend Local, UTC and Floating, for convenience
  mZones.prepend( "UTC" );      // do not use i18n here  index=2
  mZones.prepend( "Floating" ); // do not use i18n here  index=1
  mZones.prepend( KSystemTimeZones::local().name() );  // index=0

  // Put translated zones into the combobox
  foreach ( const QString &z, mZones ) {
    mParent->addItem( i18n( z.toUtf8() ).replace( '_', ' ' ) );
  }
}

KTimeZoneComboBox::KTimeZoneComboBox( QWidget *parent )
  : KComboBox( parent ), d( new KTimeZoneComboBox::Private( this ) )
{
  KGlobal::locale()->insertCatalog( "timezones4" ); // for translated timezones
  d->fillComboBox();
}

KTimeZoneComboBox::KTimeZoneComboBox( const KCalCore::ICalTimeZones *zones, QWidget *parent )
  : KComboBox( parent ), d( new KTimeZoneComboBox::Private( this ) )
{
  d->mAdditionalZones = zones;
  KGlobal::locale()->insertCatalog( "timezones4" ); // for translated timezones
  d->fillComboBox();
}

void KTimeZoneComboBox::setAdditionalTimeZones( const KCalCore::ICalTimeZones *zones )
{
  d->mAdditionalZones = zones;
  d->fillComboBox();
}

KTimeZoneComboBox::~KTimeZoneComboBox()
{
  delete d;
}

void KTimeZoneComboBox::selectTimeSpec( const KDateTime::Spec &spec )
{
  int nCurrentlySet = -1;

  int i = 0;
  foreach ( const QString &z, d->mZones ) {
    if ( z == spec.timeZone().name() ) {
      nCurrentlySet = i;
      break;
    }
    ++i;
  }

  if ( nCurrentlySet == -1 ) {
    if ( spec.isUtc() ) {
      setCurrentIndex( 2 ); // UTC
    } else if ( spec.isLocalZone() ) {
      setCurrentIndex( 0 ); // Local
    } else {
      setCurrentIndex( 1 ); // Floating event
    }
  } else {
    setCurrentIndex( nCurrentlySet );
  }
}

KDateTime::Spec KTimeZoneComboBox::selectedTimeSpec() const
{
  KDateTime::Spec spec;
  if ( currentIndex() >= 0 ) {
    if ( currentIndex() == 0 ) { // Local
      spec = KDateTime::Spec( KDateTime::LocalZone );
    } else if ( currentIndex() == 1 ) { // Floating event
      spec = KDateTime::Spec( KDateTime::ClockTime );
    } else if ( currentIndex() == 2 ) { // UTC
      spec.setType( KDateTime::UTC );
    } else {
      const KTimeZone systemTz = KSystemTimeZones::zone( d->mZones[currentIndex()] );
      // If it's not valid, then it's an additional Tz
      if ( systemTz.isValid() ) {
        spec.setType( systemTz );
      } else {
        const KCalCore::ICalTimeZone additionalTz =
          d->mAdditionalZones->zone( d->mZones[currentIndex()] );
        spec.setType( additionalTz );
      }
    }
  }

  return spec;
}

void KTimeZoneComboBox::selectLocalTimeSpec()
{
  selectTimeSpec( KDateTime::Spec( KSystemTimeZones::local() ) );
}

void KTimeZoneComboBox::setFloating( bool floating, const KDateTime::Spec &spec )
{
  if ( floating ) {
    selectTimeSpec( KDateTime::Spec( KDateTime::ClockTime ) );
  } else {
    if ( spec.isValid() ) {
      selectTimeSpec( spec );
    } else {
      selectLocalTimeSpec();
    }
  }
}
