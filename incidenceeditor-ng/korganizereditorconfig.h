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

    KConfigSkeleton *config() const Q_DECL_OVERRIDE;
    QString fullName() const Q_DECL_OVERRIDE;
    QString email() const Q_DECL_OVERRIDE;
    bool thatIsMe(const QString &email) const Q_DECL_OVERRIDE;
    QStringList allEmails() const Q_DECL_OVERRIDE;
    QStringList fullEmails() const Q_DECL_OVERRIDE;
    bool showTimeZoneSelectorInIncidenceEditor() const Q_DECL_OVERRIDE;
    QDateTime defaultDuration() const Q_DECL_OVERRIDE;
    QDateTime startTime() const Q_DECL_OVERRIDE;
    bool defaultAudioFileReminders() const Q_DECL_OVERRIDE;
    QUrl audioFilePath() const Q_DECL_OVERRIDE;
    int reminderTime() const Q_DECL_OVERRIDE;
    int reminderTimeUnits() const Q_DECL_OVERRIDE;
    bool defaultTodoReminders() const Q_DECL_OVERRIDE;
    bool defaultEventReminders() const Q_DECL_OVERRIDE;
    QStringList activeDesignerFields() const Q_DECL_OVERRIDE;
    QStringList &templates(KCalCore::IncidenceBase::IncidenceType type) Q_DECL_OVERRIDE;
};

} // IncidenceEditors

#endif
