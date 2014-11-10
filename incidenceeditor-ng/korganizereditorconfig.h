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

#ifndef INCIDENCEEDITOR_KORGANIZEREDITORCONFIG_H
#define INCIDENCEEDITOR_KORGANIZEREDITORCONFIG_H

#include "editorconfig.h"

#include <KCalCore/IncidenceBase>

namespace IncidenceEditorNG
{

class INCIDENCEEDITORS_NG_EXPORT KOrganizerEditorConfig : public IncidenceEditorNG::EditorConfig
{
public:
    KOrganizerEditorConfig();
    virtual ~KOrganizerEditorConfig();

    virtual KConfigSkeleton *config() const;
    virtual QString fullName() const;
    virtual QString email() const;
    virtual bool thatIsMe(const QString &email) const;
    virtual QStringList allEmails() const;
    virtual QStringList fullEmails() const;
    virtual bool showTimeZoneSelectorInIncidenceEditor() const;
    virtual QDateTime defaultDuration() const;
    virtual QDateTime startTime() const;
    virtual bool defaultAudioFileReminders() const;
    virtual QUrl audioFilePath() const;
    virtual int reminderTime() const;
    virtual int reminderTimeUnits() const;
    virtual bool defaultTodoReminders() const;
    virtual bool defaultEventReminders() const;
    virtual QStringList activeDesignerFields() const;
    virtual QStringList &templates(KCalCore::IncidenceBase::IncidenceType type);
};

} // IncidenceEditors

#endif
