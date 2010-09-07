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

using namespace KCalCore;
using namespace CalendarSupport;
using namespace IncidenceEditors;

KOrganizerEditorConfig::KOrganizerEditorConfig()
  : EditorConfig()
{ }

KOrganizerEditorConfig::~KOrganizerEditorConfig()
{ }

KConfigSkeleton *KOrganizerEditorConfig::config() const
{
  return KCalPrefs::instance();
}

QString KOrganizerEditorConfig::fullName() const
{
  return KCalPrefs::instance()->fullName();
}

QString KOrganizerEditorConfig::email() const
{
  return KCalPrefs::instance()->email();
}

bool KOrganizerEditorConfig::thatIsMe( const QString &email ) const
{
  return KCalPrefs::instance()->thatIsMe(email);
}

QStringList KOrganizerEditorConfig::allEmails() const
{
  return KCalPrefs::instance()->allEmails();
}

QStringList KOrganizerEditorConfig::fullEmails() const
{
  return KCalPrefs::instance()->fullEmails();
}

bool KOrganizerEditorConfig::showTimeZoneSelectorInIncidenceEditor() const
{
  return KCalPrefs::instance()->showTimeZoneSelectorInIncidenceEditor();
}

QDateTime KOrganizerEditorConfig::defaultDuration() const
{
  return KCalPrefs::instance()->defaultDuration();
}

QDateTime KOrganizerEditorConfig::startTime() const
{
  return KCalPrefs::instance()->startTime();
}

bool KOrganizerEditorConfig::defaultAudioFileReminders() const
{
  return KCalPrefs::instance()->defaultAudioFileReminders();
}

KUrl KOrganizerEditorConfig::audioFilePath() const
{
  return KCalPrefs::instance()->audioFilePath();
}

int KOrganizerEditorConfig::reminderTime() const
{
  return KCalPrefs::instance()->reminderTime();
}

int KOrganizerEditorConfig::reminderTimeUnits() const
{
  return KCalPrefs::instance()->reminderTimeUnits();
}

bool KOrganizerEditorConfig::defaultTodoReminders() const
{
  return KCalPrefs::instance()->defaultTodoReminders();
}

bool KOrganizerEditorConfig::defaultEventReminders() const
{
  return KCalPrefs::instance()->defaultEventReminders();
}

QStringList KOrganizerEditorConfig::activeDesignerFields() const
{
  return KCalPrefs::instance()->activeDesignerFields();
}

QStringList &KOrganizerEditorConfig::templates( IncidenceBase::IncidenceType type )
{
  if ( type == IncidenceBase::TypeEvent ) {
    //TODO remove mEventTemplates+etc from Prefs::instance()
    return KCalPrefs::instance()->mEventTemplates;
  } if ( type == IncidenceBase::TypeTodo ) {
    return KCalPrefs::instance()->mTodoTemplates;
  } if ( type == IncidenceBase::TypeJournal ) {
    return KCalPrefs::instance()->mJournalTemplates;
  }
  return EditorConfig::templates( type );
}
