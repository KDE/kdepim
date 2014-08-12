/*
  Copyright (c) 2010 Kevin Ottens <ervin@kde.org>

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

#include "korganizereditorconfig.h"

#include <calendarsupport/kcalprefs.h>

using namespace IncidenceEditorNG;

KOrganizerEditorConfig::KOrganizerEditorConfig()
  : EditorConfig()
{
}

KOrganizerEditorConfig::~KOrganizerEditorConfig()
{
}

KConfigSkeleton *KOrganizerEditorConfig::config() const
{
  return CalendarSupport::KCalPrefs::instance();
}

QString KOrganizerEditorConfig::fullName() const
{
  return CalendarSupport::KCalPrefs::instance()->fullName();
}

QString KOrganizerEditorConfig::email() const
{
  return CalendarSupport::KCalPrefs::instance()->email();
}

bool KOrganizerEditorConfig::thatIsMe( const QString &email ) const
{
  return CalendarSupport::KCalPrefs::instance()->thatIsMe(email);
}

QStringList KOrganizerEditorConfig::allEmails() const
{
  return CalendarSupport::KCalPrefs::instance()->allEmails();
}

QStringList KOrganizerEditorConfig::fullEmails() const
{
  return CalendarSupport::KCalPrefs::instance()->fullEmails();
}

bool KOrganizerEditorConfig::showTimeZoneSelectorInIncidenceEditor() const
{
  return CalendarSupport::KCalPrefs::instance()->showTimeZoneSelectorInIncidenceEditor();
}

QDateTime KOrganizerEditorConfig::defaultDuration() const
{
  return CalendarSupport::KCalPrefs::instance()->defaultDuration();
}

QDateTime KOrganizerEditorConfig::startTime() const
{
  return CalendarSupport::KCalPrefs::instance()->startTime();
}

bool KOrganizerEditorConfig::defaultAudioFileReminders() const
{
  return CalendarSupport::KCalPrefs::instance()->defaultAudioFileReminders();
}

QUrl KOrganizerEditorConfig::audioFilePath() const
{
  return CalendarSupport::KCalPrefs::instance()->audioFilePath();
}

int KOrganizerEditorConfig::reminderTime() const
{
  return CalendarSupport::KCalPrefs::instance()->reminderTime();
}

int KOrganizerEditorConfig::reminderTimeUnits() const
{
  return CalendarSupport::KCalPrefs::instance()->reminderTimeUnits();
}

bool KOrganizerEditorConfig::defaultTodoReminders() const
{
  return CalendarSupport::KCalPrefs::instance()->defaultTodoReminders();
}

bool KOrganizerEditorConfig::defaultEventReminders() const
{
  return CalendarSupport::KCalPrefs::instance()->defaultEventReminders();
}

QStringList KOrganizerEditorConfig::activeDesignerFields() const
{
  return CalendarSupport::KCalPrefs::instance()->activeDesignerFields();
}

QStringList &KOrganizerEditorConfig::templates( KCalCore::IncidenceBase::IncidenceType type )
{
  if ( type == KCalCore::IncidenceBase::TypeEvent ) {
    //TODO remove mEventTemplates+etc from Prefs::instance()
    return CalendarSupport::KCalPrefs::instance()->mEventTemplates;
  } if ( type == KCalCore::IncidenceBase::TypeTodo ) {
    return CalendarSupport::KCalPrefs::instance()->mTodoTemplates;
  } if ( type == KCalCore::IncidenceBase::TypeJournal ) {
    return CalendarSupport::KCalPrefs::instance()->mJournalTemplates;
  }
  return EditorConfig::templates( type );
}
