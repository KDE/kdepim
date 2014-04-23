/*
  Copyright (C) 2010 Bertjan Broeksema <broeksema@kde.org>
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

#ifndef INCIDENCEEDITOR_INCIDENCEDIALOGFACTORY_H
#define INCIDENCEEDITOR_INCIDENCEDIALOGFACTORY_H

#include "incidenceeditors-ng_export.h"

#include <KCalCore/IncidenceBase>
#include <Collection>

namespace Akonadi {
  class IncidenceChanger;
}

namespace IncidenceEditorNG {

class IncidenceDialog;

namespace IncidenceDialogFactory
{
  /**
   * Creates a new IncidenceDialog for given type. Returns 0 for unsupported types.
   *
   * @param needsSaving If true, the editor will be initialy dirty, and needs saving.
   *                    Apply button will be turned on. This is used for example when
   *                    we fill the editor with data that's not yet in akonadi, like
   *                    the "Create To-do/Reminder" in KMail.
   * @param type   The Incidence type for which to create a dialog.
   * @param parent The parent widget of the dialog
   * @param flags  The window flags for the dialog.
   *
   * TODO: Implement support for Journals.
   * NOTE: There is no editor for Incidence::TypeFreeBusy
   */
  INCIDENCEEDITORS_NG_EXPORT IncidenceDialog *create(
    bool needsSaving,
    KCalCore::IncidenceBase::IncidenceType type,
    Akonadi::IncidenceChanger *changer,
    QWidget *parent = 0, Qt::WindowFlags flags = 0 );

  INCIDENCEEDITORS_NG_EXPORT IncidenceDialog *createTodoEditor(
    const QString &summary,
    const QString &description,
    const QStringList &attachments,
    const QStringList &attendees,
    const QStringList &attachmentMimetypes,
    const QStringList &attachmentLabels,
    bool inlineAttachment,
    Akonadi::Collection defaultCollection,
    bool cleanupAttachmentTemp,
    QWidget *parent = 0, Qt::WindowFlags flags = 0 );

  INCIDENCEEDITORS_NG_EXPORT IncidenceDialog *createEventEditor(
    const QString &summary,
    const QString &description,
    const QStringList &attachments,
    const QStringList &attendees,
    const QStringList &attachmentMimetypes,
    const QStringList &attachmentLabels,
    bool inlineAttachment,
    Akonadi::Collection defaultCollection,
    bool cleanupAttachmentTempFiles,
    QWidget *parent = 0, Qt::WindowFlags flags = 0 );

} // namespace IncidenceDialogFactory

} // namespace IncidenceEditorNG

#endif
