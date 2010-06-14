/*
    Copyright (C) 2010  Bertjan Broeksema b.broeksema@home.nl

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/
#include "alarmpresets.h"

#include <boost/shared_ptr.hpp>

#include <KCal/Alarm>
#include <KLocale>

using namespace KCal;

typedef boost::shared_ptr<Alarm> Ptr;

namespace IncidenceEditorsNG {

namespace AlarmPresets {

// Don't use a map, because order matters
static QStringList sPresetNames;
static QList<Ptr>  sPresets = QList<Ptr>();

void initPresets()
{
  Ptr alarm( new Alarm( 0 ) );
  alarm->setType( Alarm::Display );
  alarm->setStartOffset( -5 * 60 ); // 5 minutes before
  alarm->setEnabled( true );
  sPresetNames.append( i18nc( "@item:inlistbox", "5 minutes before" ) );
  sPresets.append( alarm );

  alarm = Ptr( new Alarm( 0 ) );
  alarm->setType( Alarm::Display );
  alarm->setStartOffset( -10 * 60 ); // 10 minutes before
  alarm->setEnabled( true );
  sPresetNames.append( i18nc( "@item:inlistbox", "10 minutes before" ) );
  sPresets.append( alarm );

  alarm = Ptr( new Alarm( 0 ) );
  alarm->setType( Alarm::Display );
  alarm->setStartOffset( -15 * 60 ); // 15 minutes before
  alarm->setEnabled( true );
  sPresetNames.append( i18nc( "@item:inlistbox", "15 minutes before" ) );
  sPresets.append( alarm );

  alarm = Ptr( new Alarm( 0 ) );
  alarm->setType( Alarm::Display );
  alarm->setStartOffset( -30 * 60 ); // 30 minutes before
  alarm->setEnabled( true );
  sPresetNames.append( i18nc( "@item:inlistbox", "30 minutes before" ) );
  sPresets.append( alarm );

  alarm = Ptr( new Alarm( 0 ) );
  alarm->setType( Alarm::Display );
  alarm->setStartOffset( -45 * 60 ); // 45 minutes before
  alarm->setEnabled( true );
  sPresetNames.append( i18nc( "@item:inlistbox", "45 minutes before" ) );
  sPresets.append( alarm );

  alarm = Ptr( new Alarm( 0 ) );
  alarm->setType( Alarm::Display );
  alarm->setStartOffset( -60 * 60 );
  alarm->setEnabled( true );
  sPresetNames.append( i18nc( "@item:inlistbox", "1 hour before" ) );
  sPresets.append( alarm );

  alarm = Ptr( new Alarm( 0 ) );
  alarm->setType( Alarm::Display );
  alarm->setStartOffset( -2 * 60 * 60 );
  alarm->setEnabled( true );
  sPresetNames.append( i18nc( "@item:inlistbox", "2 hours before" ) );
  sPresets.append( alarm );

  alarm = Ptr( new Alarm( 0 ) );
  alarm->setType( Alarm::Display );
  alarm->setStartOffset( -24 * 60 * 60 );
  alarm->setEnabled( true );
  sPresetNames.append( i18nc( "@item:inlistbox", "1 day before" ) );
  sPresets.append( alarm );

  alarm = Ptr( new Alarm( 0 ) );
  alarm->setType( Alarm::Display );
  alarm->setStartOffset( -2 * 24 * 60 * 60 );
  alarm->setEnabled( true );
  sPresetNames.append( i18nc( "@item:inlistbox", "2 days before" ) );
  sPresets.append( alarm );

  alarm = Ptr( new Alarm( 0 ) );
  alarm->setType( Alarm::Display );
  alarm->setStartOffset( -5 * 24 * 60 * 60 );
  alarm->setEnabled( true );
  sPresetNames.append( i18nc( "@item:inlistbox", "5 days before" ) );
  sPresets.append( alarm );
}

QStringList availablePresets()
{
  if ( sPresetNames.isEmpty() )
    initPresets();

  return sPresetNames;
}


Alarm *preset( const QString &name )
{
  if ( sPresetNames.isEmpty() )
    initPresets();

  Q_ASSERT( sPresetNames.count( name ) == 1 ); // The name should exists and only once

  Alarm *alarm = new Alarm( *sPresets.at( sPresetNames.indexOf( name ) ) );
  return alarm;
}

int presetIndex( const KCal::Alarm &alarm )
{
  if ( sPresetNames.isEmpty() )
    initPresets();

  const QStringList presets = availablePresets();

  for ( int i = 0; i < presets.size(); ++i  ) {
    QScopedPointer<Alarm> presetAlarm( preset( presets.at( i ) ) );
    if ( *presetAlarm == alarm )
      return i;
  }

  return -1;

}

} // AlarmPresets
} // IncidenceEditorsNG
