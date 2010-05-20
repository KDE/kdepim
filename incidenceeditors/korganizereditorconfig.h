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

#ifndef KORGANIZEREDITORCONFIG_H
#define KORGANIZEREDITORCONFIG_H

#include "editorconfig.h"

namespace IncidenceEditors {

class INCIDENCEEDITORS_EXPORT KOrganizerEditorConfig : public IncidenceEditors::EditorConfig
{
  public:
    explicit KOrganizerEditorConfig();
    virtual ~KOrganizerEditorConfig();

    virtual KConfigSkeleton *config() const;
    virtual QString fullName() const;
    virtual QString email() const;
    virtual bool thatIsMe( const QString &email ) const;
    virtual QStringList allEmails() const;
    virtual QStringList fullEmails() const;
    virtual bool showTimeZoneSelectorInIncidenceEditor() const;
    virtual QDateTime defaultDuration() const;
    virtual QDateTime startTime() const;
    virtual int reminderTime() const;
    virtual int reminderTimeUnits() const;
    virtual bool defaultTodoReminders() const;
    virtual bool defaultEventReminders() const;
    virtual QStringList activeDesignerFields() const;
    virtual QStringList &templates( const QString &type );
};

} // IncidenceEditors

#endif // KORGANIZEREDITORCONFIG_H
