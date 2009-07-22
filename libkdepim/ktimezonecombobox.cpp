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
    Private( Calendar *calendar, KTimeZoneComboBox *parent )
      : mParent( parent ), mCalendar( calendar )
    {}

    void fillComboBox();
    KTimeZoneComboBox *mParent;
    Calendar *mCalendar;
    QStringList mZones;
};

void KPIM::KTimeZoneComboBox::Private::fillComboBox()
{
  // Read all system time zones

  const KTimeZones::ZoneMap timezones = KSystemTimeZones::zones();
  for ( KTimeZones::ZoneMap::ConstIterator it=timezones.begin(); it != timezones.end(); ++it ) {
    mZones.append( it.key().toUtf8() );
  }
  mZones.sort();

  // Prepend the list of timezones from the Calendar
  if ( mCalendar ) {
    const ICalTimeZones::ZoneMap calzones = mCalendar->timeZones()->zones();
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

KTimeZoneComboBox::KTimeZoneComboBox( Calendar *calendar, QWidget *parent )
  : KComboBox( parent ), d( new KPIM::KTimeZoneComboBox::Private( calendar, this ) )
{
  KGlobal::locale()->insertCatalog( "timezones4" ); // for translated timezones
  d->fillComboBox();
}

void KTimeZoneComboBox::setCalendar( Calendar *calendar )
{
  d->mCalendar = calendar;
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

KDateTime::Spec KTimeZoneComboBox::selectedTimeSpec()
{
  KDateTime::Spec spec;
  spec.setType( KSystemTimeZones::zone( d->mZones[currentIndex()] ) );
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
