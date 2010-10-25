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

#include <calendarsupport/kcalprefs.h>

#include <KCalCore/Alarm>

#include <KGlobal>
#include <KLocale>

using namespace CalendarSupport;
using namespace KCalCore;

namespace IncidenceEditorNG {

namespace AlarmPresets {

// Don't use a map, because order matters
K_GLOBAL_STATIC( QStringList, sBeforeStartPresetNames )
K_GLOBAL_STATIC( QStringList, sBeforeEndPresetNames )
K_GLOBAL_STATIC( QList<KCalCore::Alarm::Ptr>, sBeforeStartPresets )
K_GLOBAL_STATIC( QList<KCalCore::Alarm::Ptr>, sBeforeEndPresets )

static int sDefaultPresetIndex = 0;
static int sDefaultAlarmOffset = 0; // We must save it, so we can detect that config changed.

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

  sDefaultAlarmOffset = KCalPrefs::instance()->reminderTime() > 0 ? KCalPrefs::instance()->reminderTime() :
                                                                    DEFAULT_REMINDER_OFFSET;

  if ( !hardcodedPresets.contains( sDefaultAlarmOffset ) ) {
    // Lets insert the user's favorite preset (and keep the list sorted):
    int index;
    for ( index = 0; index < hardcodedPresets.count(); ++index ) {
      if ( hardcodedPresets[index] > sDefaultAlarmOffset )
        break;
    }

    hardcodedPresets.insert( index, sDefaultAlarmOffset );
    sDefaultPresetIndex = index;
  } else {
    sDefaultPresetIndex = 2;
  }

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

void checkInitNeeded( When when )
{
  const int currentAlarmOffset = KCalPrefs::instance()->reminderTime() > 0 ? KCalPrefs::instance()->reminderTime() :
                                                                            DEFAULT_REMINDER_OFFSET;
  const bool configChanged = currentAlarmOffset != sDefaultAlarmOffset;

  switch ( when ) {
  case AlarmPresets::BeforeStart:
    if ( sBeforeStartPresetNames->isEmpty() || configChanged ) {
      sBeforeStartPresetNames->clear();
      initPresets( when );
    }
  case AlarmPresets::BeforeEnd:
    if ( sBeforeEndPresetNames->isEmpty() || configChanged ) {
      sBeforeEndPresetNames->clear();
      initPresets( when );
    }
  }
}

QStringList availablePresets( AlarmPresets::When when )
{
  checkInitNeeded( when );

  switch( when ) {
  case AlarmPresets::BeforeStart:
    return *sBeforeStartPresetNames;
  case AlarmPresets::BeforeEnd:
    return *sBeforeEndPresetNames;
  default:
    return QStringList();
  }
}

KCalCore::Alarm::Ptr preset( When when, const QString &name )
{
  checkInitNeeded( when );

  switch ( when ) {
  case AlarmPresets::BeforeStart:
  {
    Q_ASSERT( sBeforeStartPresetNames->count( name ) == 1 ); // The name should exists and only once

    return KCalCore::Alarm::Ptr(
      new KCalCore::Alarm( *sBeforeStartPresets->at( sBeforeStartPresetNames->indexOf( name ) ) ) );
  }
  case AlarmPresets::BeforeEnd:
  {
    Q_ASSERT( sBeforeEndPresetNames->count( name ) == 1 ); // The name should exists and only once

    return KCalCore::Alarm::Ptr(
      new KCalCore::Alarm( *sBeforeEndPresets->at( sBeforeEndPresetNames->indexOf( name ) ) ) );
  }
  default:
    return KCalCore::Alarm::Ptr();
  };
}

KCalCore::Alarm::Ptr defaultAlarm( When when )
{
  checkInitNeeded( when );

  switch ( when ) {
  case AlarmPresets::BeforeStart:
    return Alarm::Ptr( new Alarm( *sBeforeStartPresets->at( sDefaultPresetIndex ) ) );
  case AlarmPresets::BeforeEnd:
    return Alarm::Ptr( new Alarm( *sBeforeEndPresets->at( sDefaultPresetIndex ) ) );
  default:
    return Alarm::Ptr();
  };
}


int presetIndex( When when, const KCalCore::Alarm::Ptr &alarm )
{
  checkInitNeeded( when );
  const QStringList presets = availablePresets( when );

  for ( int i = 0; i < presets.size(); ++i ) {
    KCalCore::Alarm::Ptr presetAlarm( preset( when, presets.at( i ) ) );
    if ( presetAlarm == alarm ) {
      return i;
    }
  }

  return -1;
}

int defaultPresetIndex()
{
  // BeforeEnd would do too, index is the same.
  checkInitNeeded( AlarmPresets::BeforeStart );
  return sDefaultPresetIndex;
}

} // AlarmPresets

} // IncidenceEditorNG
