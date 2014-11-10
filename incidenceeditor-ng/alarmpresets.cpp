/*
  Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

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

#include <KLocalizedString>

#include <QDebug>

using namespace CalendarSupport;
using namespace KCalCore;

namespace IncidenceEditorNG
{

namespace AlarmPresets
{

// Don't use a map, because order matters
Q_GLOBAL_STATIC(QStringList, sBeforeStartPresetNames)
Q_GLOBAL_STATIC(QStringList, sBeforeEndPresetNames)
Q_GLOBAL_STATIC(QList<KCalCore::Alarm::Ptr>, sBeforeStartPresets)
Q_GLOBAL_STATIC(QList<KCalCore::Alarm::Ptr>, sBeforeEndPresets)

static int sDefaultPresetIndex = 0;
static int sDefaultAlarmOffset = 0; // We must save it, so we can detect that config changed.

int configuredReminderTimeInMinutes()
{
    QList<int> units;
    units << 1 << 60 << (24 * 60);

    const int configuredUnits = KCalPrefs::instance()->reminderTimeUnits();
    const int unitsToUse = configuredUnits >= 0 && configuredUnits <= 2 ? configuredUnits : 0;

    const int configuredReminderTime = KCalPrefs::instance()->reminderTime();
    const int reminderTimeToUse =  configuredReminderTime > 0 ?
                                   configuredReminderTime :
                                   DEFAULT_REMINDER_OFFSET;

    return reminderTimeToUse * units[unitsToUse];
}

void initPresets(AlarmPresets::When when)
{
    QList<int> hardcodedPresets;
    hardcodedPresets << 0           // at start/due
                     << 5           // 5 minutes
                     << 10
                     << 15
                     << 30
                     << 45
                     << 60          // 1 hour
                     << 2 * 60      // 2 hours
                     << 24 * 60     // 1 day
                     << 2 * 24 * 60 // 2 days
                     << 5 * 24 * 60;// 5 days

    sDefaultAlarmOffset = configuredReminderTimeInMinutes();

    if (!hardcodedPresets.contains(sDefaultAlarmOffset)) {
        // Lets insert the user's favorite preset (and keep the list sorted):
        int index;
        for (index = 0; index < hardcodedPresets.count(); ++index) {
            if (hardcodedPresets[index] > sDefaultAlarmOffset) {
                break;
            }
        }
        hardcodedPresets.insert(index, sDefaultAlarmOffset);
        sDefaultPresetIndex = index;
    } else {
        sDefaultPresetIndex = hardcodedPresets.indexOf(sDefaultAlarmOffset);
    }

    switch (when) {
    case AlarmPresets::BeforeStart:

        for (int i = 0; i < hardcodedPresets.count(); ++i) {
            KCalCore::Alarm::Ptr alarm(new KCalCore::Alarm(0));
            alarm->setType(KCalCore::Alarm::Display);
            const int minutes = hardcodedPresets[i];
            alarm->setStartOffset(-minutes * 60);
            alarm->setEnabled(true);
            if (minutes == 0) {
                sBeforeStartPresetNames->append(i18nc("@item:inlistbox",
                                                      "At start"));
            } else if (minutes < 60) {
                sBeforeStartPresetNames->append(i18ncp("@item:inlistbox",
                                                       "%1 minute before start",
                                                       "%1 minutes before start", minutes));
            } else if (minutes < 24 * 60) {
                sBeforeStartPresetNames->append(i18ncp("@item:inlistbox",
                                                       "%1 hour before start",
                                                       "%1 hours before start", minutes / 60));
            } else {
                sBeforeStartPresetNames->append(i18ncp("@item:inlistbox",
                                                       "%1 day before start",
                                                       "%1 days before start", minutes / (24 * 60)));
            }
            sBeforeStartPresets->append(alarm);
        }
        break;

    case AlarmPresets::BeforeEnd:
        for (int i = 0; i < hardcodedPresets.count(); ++i) {
            KCalCore::Alarm::Ptr alarm(new KCalCore::Alarm(0));
            alarm->setType(KCalCore::Alarm::Display);
            const int minutes = hardcodedPresets[i];
            alarm->setEndOffset(-minutes * 60);
            alarm->setEnabled(true);
            if (minutes == 0) {
                sBeforeEndPresetNames->append(i18nc("@item:inlistbox", "When due"));
            } else if (minutes < 60) {
                sBeforeEndPresetNames->append(i18ncp("@item:inlistbox",
                                                     "%1 minute before due",
                                                     "%1 minutes before due", minutes));
            } else if (minutes < 24 * 60) {
                sBeforeEndPresetNames->append(i18ncp("@item:inlistbox",
                                                     "%1 hour before due",
                                                     "%1 hours before due", minutes / 60));
            } else {
                sBeforeEndPresetNames->append(i18ncp("@item:inlistbox",
                                                     "%1 day before due",
                                                     "%1 days before due", minutes / (24 * 60)));
            }
            sBeforeEndPresets->append(alarm);
        }
        break;
    }
}

void checkInitNeeded(When when)
{
    const int currentAlarmOffset = configuredReminderTimeInMinutes();
    const bool configChanged = currentAlarmOffset != sDefaultAlarmOffset;

    switch (when) {
    case AlarmPresets::BeforeStart:
        if (sBeforeStartPresetNames->isEmpty() || configChanged) {
            sBeforeStartPresetNames->clear();
            sBeforeStartPresets->clear();
            initPresets(when);
        }
        break;
    case AlarmPresets::BeforeEnd:
        if (sBeforeEndPresetNames->isEmpty() || configChanged) {
            sBeforeEndPresetNames->clear();
            sBeforeEndPresets->clear();
            initPresets(when);
        }
        break;
    default:
        Q_ASSERT_X(false, "checkInitNeeded", "Unknown preset type");
    }
}

QStringList availablePresets(AlarmPresets::When when)
{
    checkInitNeeded(when);

    switch (when) {
    case AlarmPresets::BeforeStart:
        return *sBeforeStartPresetNames;
    case AlarmPresets::BeforeEnd:
        return *sBeforeEndPresetNames;
    default:
        return QStringList();
    }
}

KCalCore::Alarm::Ptr preset(When when, const QString &name)
{
    checkInitNeeded(when);

    switch (when) {
    case AlarmPresets::BeforeStart: {
        // The name should exists and only once
        if (sBeforeStartPresetNames->count(name) != 1) {
            // print some debug info before crashing
            qDebug() << " name = " << name << "; when = " << when
                     << "; count for name = " << sBeforeStartPresetNames->count(name)
                     <<  "; global count = " << sBeforeStartPresetNames->count();
            Q_ASSERT_X(false, "preset", "Number of presets should be one");
        }

        return KCalCore::Alarm::Ptr(
                   new KCalCore::Alarm(*sBeforeStartPresets->at(sBeforeStartPresetNames->indexOf(name))));
    }
    case AlarmPresets::BeforeEnd: {
        Q_ASSERT(sBeforeEndPresetNames->count(name) == 1);     // The name should exists and only once

        return KCalCore::Alarm::Ptr(
                   new KCalCore::Alarm(*sBeforeEndPresets->at(sBeforeEndPresetNames->indexOf(name))));
    }
    default:
        return KCalCore::Alarm::Ptr();
    };
}

KCalCore::Alarm::Ptr defaultAlarm(When when)
{
    checkInitNeeded(when);

    switch (when) {
    case AlarmPresets::BeforeStart:
        return Alarm::Ptr(new Alarm(*sBeforeStartPresets->at(sDefaultPresetIndex)));
    case AlarmPresets::BeforeEnd:
        return Alarm::Ptr(new Alarm(*sBeforeEndPresets->at(sDefaultPresetIndex)));
    default:
        return Alarm::Ptr();
    };
}

int presetIndex(When when, const KCalCore::Alarm::Ptr &alarm)
{
    checkInitNeeded(when);
    const QStringList presets = availablePresets(when);

    for (int i = 0; i < presets.size(); ++i) {
        KCalCore::Alarm::Ptr presetAlarm(preset(when, presets.at(i)));
        if (presetAlarm == alarm) {
            return i;
        }
    }

    return -1;
}

int defaultPresetIndex()
{
    // BeforeEnd would do too, index is the same.
    checkInitNeeded(AlarmPresets::BeforeStart);
    return sDefaultPresetIndex;
}

} // AlarmPresets

} // IncidenceEditorNG
