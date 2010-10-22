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

#include <KGlobal>
#include <KLocale>

namespace IncidenceEditorNG {

namespace AlarmPresets {

// Don't use a map, because order matters
K_GLOBAL_STATIC( QStringList, sBeforeStartPresetNames )
K_GLOBAL_STATIC( QStringList, sBeforeEndPresetNames )
K_GLOBAL_STATIC( QList<KCalCore::Alarm::Ptr>, sBeforeStartPresets )
K_GLOBAL_STATIC( QList<KCalCore::Alarm::Ptr>, sBeforeEndPresets )

void initPresets( AlarmPresets::When when )
{
  QList<int> hardcodedPresets;
  hardcodedPresets << 5           // 5 minutes
                   << 10
                   << 15
                   << 30
                   << 45
                   << 60          // 1 hour
                   << 2 * 60      // 2 hours
                   << 24 * 60     // 1 day
                   << 2 * 24 * 60 // 2 days
                   << 5 * 24 * 60;// 5 days

  switch ( when ) {
  case AlarmPresets::BeforeStart:

    for ( int i = 0; i < hardcodedPresets.count(); ++i ) {
      KCalCore::Alarm::Ptr alarm( new KCalCore::Alarm( 0 ) );
      alarm->setType( KCalCore::Alarm::Display );
      const int minutes = hardcodedPresets[i];
      alarm->setStartOffset( -minutes * 60 );
      alarm->setEnabled( true );
      if ( minutes < 60 ) {
        sBeforeStartPresetNames->append( i18nc( "@item:inlistbox", "%1 minutes before start", minutes ) );
      } else if ( minutes <= 2*60 ) {
        sBeforeStartPresetNames->append( i18nc( "@item:inlistbox", "%1 hours before start", minutes/60 ) );
      } else {
        sBeforeStartPresetNames->append( i18nc( "@item:inlistbox", "%1 days before start", minutes/(24*60) ) );
      }
      sBeforeStartPresets->append( alarm );
    }
    break;

  case AlarmPresets::BeforeEnd:
    for ( int i = 0; i < hardcodedPresets.count(); ++i ) {
      KCalCore::Alarm::Ptr alarm( new KCalCore::Alarm( 0 ) );
      alarm->setType( KCalCore::Alarm::Display );
      const int minutes = hardcodedPresets[i];
      alarm->setEndOffset( -minutes * 60 );
      alarm->setEnabled( true );
      if ( minutes < 60 ) {
        sBeforeEndPresetNames->append( i18nc( "@item:inlistbox", "%1 minutes before due", minutes ) );
      } else if ( minutes <= 2*60 ) {
        sBeforeEndPresetNames->append( i18nc( "@item:inlistbox", "%1 hours before due", minutes/60 ) );
      } else {
        sBeforeEndPresetNames->append( i18nc( "@item:inlistbox", "%1 days before due", minutes/(24*60) ) );
      }
      sBeforeEndPresets->append( alarm );
    }
    break;
  }
}

QStringList availablePresets( AlarmPresets::When when )
{
  switch ( when ) {
  case AlarmPresets::BeforeStart:
    if ( sBeforeStartPresetNames->isEmpty() ) {
      initPresets( when );
    }
    return *sBeforeStartPresetNames;
  case AlarmPresets::BeforeEnd:
    if ( sBeforeEndPresetNames->isEmpty() ) {
      initPresets( when );
    }
    return *sBeforeEndPresetNames;
  default:
    return QStringList();
  }
}

KCalCore::Alarm::Ptr preset( When when, const QString &name )
{
  switch ( when ) {
  case AlarmPresets::BeforeStart:
  {
    if ( sBeforeStartPresetNames->isEmpty() ) {
      initPresets( when );
    }
    Q_ASSERT( sBeforeStartPresetNames->count( name ) == 1 ); // The name should exists and only once

    return KCalCore::Alarm::Ptr(
      new KCalCore::Alarm( *sBeforeStartPresets->at( sBeforeStartPresetNames->indexOf( name ) ) ) );
  }
  case AlarmPresets::BeforeEnd:
  {
    if ( sBeforeEndPresetNames->isEmpty() ) {
      initPresets( when );
    }
    Q_ASSERT( sBeforeEndPresetNames->count( name ) == 1 ); // The name should exists and only once

    return KCalCore::Alarm::Ptr(
      new KCalCore::Alarm( *sBeforeEndPresets->at( sBeforeEndPresetNames->indexOf( name ) ) ) );
  }
  default:
    return KCalCore::Alarm::Ptr();
  };
}

int presetIndex( When when, const KCalCore::Alarm::Ptr &alarm )
{
  switch ( when ) {
  case AlarmPresets::BeforeStart:
    if ( sBeforeStartPresetNames->isEmpty() ) {
      initPresets( when );
    }
  case AlarmPresets::BeforeEnd:
    if ( sBeforeEndPresetNames->isEmpty() ) {
      initPresets( when );
    }
  }

  const QStringList presets = availablePresets( when );

  for ( int i = 0; i < presets.size(); ++i ) {
    KCalCore::Alarm::Ptr presetAlarm( preset( when, presets.at( i ) ) );
    if ( presetAlarm == alarm ) {
      return i;
    }
  }

  return -1;
}

} // AlarmPresets

} // IncidenceEditorNG
