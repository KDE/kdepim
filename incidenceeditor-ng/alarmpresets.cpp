/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
    Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "alarmpresets.h"

#include <QStringList>
#include <KLocale>
#include <kglobal.h>
using namespace KCalCore;

namespace IncidenceEditorsNG {

namespace AlarmPresets {

// Don't use a map, because order matters
K_GLOBAL_STATIC( QStringList, sBeforeStartPresetNames )
K_GLOBAL_STATIC( QStringList, sBeforeEndPresetNames )
K_GLOBAL_STATIC( QList<Alarm::Ptr>, sBeforeStartPresets )
K_GLOBAL_STATIC( QList<Alarm::Ptr>, sBeforeEndPresets )

void initPresets( AlarmPresets::When when )
{
  Alarm::Ptr alarm( new Alarm( 0 ) );
  switch ( when ) {
  case AlarmPresets::BeforeStart:
    alarm->setType( Alarm::Display );
    alarm->setStartOffset( -5 * 60 ); // 5 minutes before
    alarm->setEnabled( true );
    sBeforeStartPresetNames->append( i18nc( "@item:inlistbox", "5 minutes before start" ) );
    sBeforeStartPresets->append( alarm );

    alarm = Alarm::Ptr( new Alarm( 0 ) );
    alarm->setType( Alarm::Display );
    alarm->setStartOffset( -10 * 60 ); // 10 minutes before
    alarm->setEnabled( true );
    sBeforeStartPresetNames->append( i18nc( "@item:inlistbox", "10 minutes before start" ) );
    sBeforeStartPresets->append( alarm );

    alarm = Alarm::Ptr( new Alarm( 0 ) );
    alarm->setType( Alarm::Display );
    alarm->setStartOffset( -15 * 60 ); // 15 minutes before
    alarm->setEnabled( true );
    sBeforeStartPresetNames->append( i18nc( "@item:inlistbox", "15 minutes before start" ) );
    sBeforeStartPresets->append( alarm );

    alarm = Alarm::Ptr( new Alarm( 0 ) );
    alarm->setType( Alarm::Display );
    alarm->setStartOffset( -30 * 60 ); // 30 minutes before
    alarm->setEnabled( true );
    sBeforeStartPresetNames->append( i18nc( "@item:inlistbox", "30 minutes before start" ) );
    sBeforeStartPresets->append( alarm );

    alarm = Alarm::Ptr( new Alarm( 0 ) );
    alarm->setType( Alarm::Display );
    alarm->setStartOffset( -45 * 60 ); // 45 minutes before
    alarm->setEnabled( true );
    sBeforeStartPresetNames->append( i18nc( "@item:inlistbox", "45 minutes before start" ) );
    sBeforeStartPresets->append( alarm );

    alarm = Alarm::Ptr( new Alarm( 0 ) );
    alarm->setType( Alarm::Display );
    alarm->setStartOffset( -60 * 60 );
    alarm->setEnabled( true );
    sBeforeStartPresetNames->append( i18nc( "@item:inlistbox", "1 hour before start" ) );
    sBeforeStartPresets->append( alarm );

    alarm = Alarm::Ptr( new Alarm( 0 ) );
    alarm->setType( Alarm::Display );
    alarm->setStartOffset( -2 * 60 * 60 );
    alarm->setEnabled( true );
    sBeforeStartPresetNames->append( i18nc( "@item:inlistbox", "2 hours before start" ) );
    sBeforeStartPresets->append( alarm );

    alarm = Alarm::Ptr( new Alarm( 0 ) );
    alarm->setType( Alarm::Display );
    alarm->setStartOffset( -24 * 60 * 60 );
    alarm->setEnabled( true );
    sBeforeStartPresetNames->append( i18nc( "@item:inlistbox", "1 day before start" ) );
    sBeforeStartPresets->append( alarm );

    alarm = Alarm::Ptr( new Alarm( 0 ) );
    alarm->setType( Alarm::Display );
    alarm->setStartOffset( -2 * 24 * 60 * 60 );
    alarm->setEnabled( true );
    sBeforeStartPresetNames->append( i18nc( "@item:inlistbox", "2 days before start" ) );
    sBeforeStartPresets->append( alarm );

    alarm = Alarm::Ptr( new Alarm( 0 ) );
    alarm->setType( Alarm::Display );
    alarm->setStartOffset( -5 * 24 * 60 * 60 );
    alarm->setEnabled( true );
    sBeforeStartPresetNames->append( i18nc( "@item:inlistbox", "5 days before start" ) );
    sBeforeStartPresets->append( alarm );
    break;
  case AlarmPresets::BeforeEnd:
    alarm->setType( Alarm::Display );
    alarm->setEndOffset( -5 * 60 ); // 5 minutes before
    alarm->setEnabled( true );
    sBeforeEndPresetNames->append( i18nc( "@item:inlistbox", "5 minutes before due" ) );
    sBeforeEndPresets->append( alarm );

    alarm = Alarm::Ptr( new Alarm( 0 ) );
    alarm->setType( Alarm::Display );
    alarm->setEndOffset( -10 * 60 ); // 10 minutes before
    alarm->setEnabled( true );
    sBeforeEndPresetNames->append( i18nc( "@item:inlistbox", "10 minutes before due" ) );
    sBeforeEndPresets->append( alarm );

    alarm = Alarm::Ptr( new Alarm( 0 ) );
    alarm->setType( Alarm::Display );
    alarm->setEndOffset( -15 * 60 ); // 15 minutes before
    alarm->setEnabled( true );
    sBeforeEndPresetNames->append( i18nc( "@item:inlistbox", "15 minutes before due" ) );
    sBeforeEndPresets->append( alarm );

    alarm = Alarm::Ptr( new Alarm( 0 ) );
    alarm->setType( Alarm::Display );
    alarm->setEndOffset( -30 * 60 ); // 30 minutes before
    alarm->setEnabled( true );
    sBeforeEndPresetNames->append( i18nc( "@item:inlistbox", "30 minutes before due" ) );
    sBeforeEndPresets->append( alarm );

    alarm = Alarm::Ptr( new Alarm( 0 ) );
    alarm->setType( Alarm::Display );
    alarm->setEndOffset( -45 * 60 ); // 45 minutes before
    alarm->setEnabled( true );
    sBeforeEndPresetNames->append( i18nc( "@item:inlistbox", "45 minutes before due" ) );
    sBeforeEndPresets->append( alarm );

    alarm = Alarm::Ptr( new Alarm( 0 ) );
    alarm->setType( Alarm::Display );
    alarm->setEndOffset( -60 * 60 );
    alarm->setEnabled( true );
    sBeforeEndPresetNames->append( i18nc( "@item:inlistbox", "1 hour before due" ) );
    sBeforeEndPresets->append( alarm );

    alarm = Alarm::Ptr( new Alarm( 0 ) );
    alarm->setType( Alarm::Display );
    alarm->setEndOffset( -2 * 60 * 60 );
    alarm->setEnabled( true );
    sBeforeEndPresetNames->append( i18nc( "@item:inlistbox", "2 hours before due" ) );
    sBeforeEndPresets->append( alarm );

    alarm = Alarm::Ptr( new Alarm( 0 ) );
    alarm->setType( Alarm::Display );
    alarm->setEndOffset( -24 * 60 * 60 );
    alarm->setEnabled( true );
    sBeforeEndPresetNames->append( i18nc( "@item:inlistbox", "1 day before due" ) );
    sBeforeEndPresets->append( alarm );

    alarm = Alarm::Ptr( new Alarm( 0 ) );
    alarm->setType( Alarm::Display );
    alarm->setEndOffset( -2 * 24 * 60 * 60 );
    alarm->setEnabled( true );
    sBeforeEndPresetNames->append( i18nc( "@item:inlistbox", "2 days before due" ) );
    sBeforeEndPresets->append( alarm );

    alarm = Alarm::Ptr( new Alarm( 0 ) );
    alarm->setType( Alarm::Display );
    alarm->setEndOffset( -5 * 24 * 60 * 60 );
    alarm->setEnabled( true );
    sBeforeEndPresetNames->append( i18nc( "@item:inlistbox", "5 days before due" ) );
    sBeforeEndPresets->append( alarm );
    break;
  }
}

QStringList availablePresets( AlarmPresets::When when )
{
  switch ( when ) {
  case AlarmPresets::BeforeStart:
    if ( sBeforeStartPresetNames->isEmpty() )
      initPresets( when );
    return *sBeforeStartPresetNames;
  case AlarmPresets::BeforeEnd:
    if ( sBeforeEndPresetNames->isEmpty() )
      initPresets( when );
    return *sBeforeEndPresetNames;
  default:
    return QStringList();
  }
}


Alarm::Ptr preset( When when, const QString &name )
{
  switch ( when ) {
  case AlarmPresets::BeforeStart:
    {
      if ( sBeforeStartPresetNames->isEmpty() )
        initPresets( when );
      Q_ASSERT( sBeforeStartPresetNames->count( name ) == 1 ); // The name should exists and only once

      return Alarm::Ptr( new Alarm( *sBeforeStartPresets->at( sBeforeStartPresetNames->indexOf( name ) ) ) );
    }
  case AlarmPresets::BeforeEnd:
    {
      if ( sBeforeEndPresetNames->isEmpty() )
        initPresets( when );
      Q_ASSERT( sBeforeEndPresetNames->count( name ) == 1 ); // The name should exists and only once

      return Alarm::Ptr( new Alarm( *sBeforeEndPresets->at( sBeforeEndPresetNames->indexOf( name ) ) ) );
    }
  default:
    return Alarm::Ptr();
  };
}

int presetIndex( When when, const KCalCore::Alarm::Ptr &alarm )
{
  switch ( when ) {
  case AlarmPresets::BeforeStart:
    if ( sBeforeStartPresetNames->isEmpty() )
      initPresets( when );
  case AlarmPresets::BeforeEnd:
    if ( sBeforeEndPresetNames->isEmpty() )
      initPresets( when );
  }

  const QStringList presets = availablePresets( when );

  for ( int i = 0; i < presets.size(); ++i ) {
    Alarm::Ptr presetAlarm( preset( when, presets.at( i ) ) );
    if ( presetAlarm == alarm ) {
      return i;
    }
  }

  return -1;

}

} // AlarmPresets
} // IncidenceEditorsNG
