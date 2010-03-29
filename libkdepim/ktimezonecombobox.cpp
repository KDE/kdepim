/*
  Copyright (C) 2007 Bruno Virlet <bruno.virlet@gmail.com>
  Copyright 2008-2009 Allen Winter <winter@kde.org>

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

#include <kcal/calendar.h>
#include <kcal/icaltimezones.h>

#include <KDebug>
#include <KGlobal>
#include <KLocale>
#include <KSystemTimeZone>

using namespace KPIM;
using namespace KCal;

class KPIM::KTimeZoneComboBox::Private
{
  public:
    Private( KTimeZoneComboBox *parent )
      : mParent( parent ), mAdditionalZones( 0 )
    {}

    void fillComboBox();
    KTimeZoneComboBox * const mParent;
    QStringList mZones;
    const ICalTimeZones* mAdditionalZones;
};

void KPIM::KTimeZoneComboBox::Private::fillComboBox()
{
  mParent->clear();
  // Read all system time zones

  const KTimeZones::ZoneMap timezones = KSystemTimeZones::zones();
  for ( KTimeZones::ZoneMap::ConstIterator it=timezones.begin(); it != timezones.end(); ++it ) {
    mZones.append( it.key().toUtf8() );
  }
  mZones.sort();

  // Prepend the list of additional timezones
  if ( mAdditionalZones ) {
    const ICalTimeZones::ZoneMap calzones = mAdditionalZones->zones();
    for ( ICalTimeZones::ZoneMap::ConstIterator it=calzones.begin(); it != calzones.end(); ++it ) {
      kDebug() << "Prepend timezone " << it.key().toUtf8();
      mZones.prepend( it.key().toUtf8() );
    }
  }
  // Prepend UTC and Floating, for convenience
  mZones.prepend( "UTC" );      // do not use i18n here
  mZones.prepend( "Floating" ); // do not use i18n here

  // Put translated zones into the combobox
  foreach( const QString &z, mZones ) {
    mParent->addItem( i18n( z.toUtf8() ).replace( '_', ' ' ) );
  }
}

KTimeZoneComboBox::KTimeZoneComboBox( QWidget *parent )
  : KComboBox( parent ), d( new KPIM::KTimeZoneComboBox::Private( this ) )
{
  KGlobal::locale()->insertCatalog( "timezones4" ); // for translated timezones
  d->fillComboBox();
}


KTimeZoneComboBox::KTimeZoneComboBox( const ICalTimeZones* zones, QWidget *parent )
  : KComboBox( parent ), d( new KPIM::KTimeZoneComboBox::Private( this ) )
{
  d->mAdditionalZones = zones;
  KGlobal::locale()->insertCatalog( "timezones4" ); // for translated timezones
  d->fillComboBox();
}

void KTimeZoneComboBox::setAdditionalTimeZones( const ICalTimeZones* zones )
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
  foreach( const QString &z, d->mZones ) {
    if ( z == spec.timeZone().name() ) {
      nCurrentlySet = i;
      break;
    }
    i++;
  }

  if ( nCurrentlySet == -1 ) {
    if ( spec.isUtc() ) {
      setCurrentIndex( 1 ); // UTC
    } else {
      setCurrentIndex( 0 ); // Floating event
    }
  } else {
    setCurrentIndex( nCurrentlySet );
  }
}

KDateTime::Spec KTimeZoneComboBox::selectedTimeSpec() const
{
  KDateTime::Spec spec;
  if ( currentIndex() == 0 ) { // Floating event
    spec = KDateTime::Spec( KDateTime::ClockTime );
  }
  else if ( currentIndex() == 1 ) { // UTC
    spec.setType( KDateTime::UTC );
  }
  else {
    spec.setType( KSystemTimeZones::zone( d->mZones[currentIndex()] ) );
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
    selectTimeSpec( KDateTime::ClockTime );
  } else {
    if ( spec.isValid() ) {
      selectTimeSpec( spec );
    } else {
      selectLocalTimeSpec();
    }
  }
}
