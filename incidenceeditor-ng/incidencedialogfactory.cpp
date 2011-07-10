/*
  Copyright (C) 2010 Bertjan Broeksema <broeksema@kde.org>
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

#include "incidencedialogfactory.h"
#include "eventortododialog.h"
#include "incidencedefaults.h"
#include "groupwareintegration.h"

#include <KCalCore/Event>
#include <KCalCore/Todo>
#include <Akonadi/Item>

using namespace IncidenceEditorNG;
using namespace KCalCore;

IncidenceDialog *IncidenceDialogFactory::create( bool needsSaving,
                                                 KCalCore::IncidenceBase::IncidenceType type,
                                                 QWidget *parent, Qt::WFlags flags )
{
  switch ( type ) {
  case KCalCore::IncidenceBase::TypeEvent: // Fall through
  case KCalCore::IncidenceBase::TypeTodo:
  case KCalCore::IncidenceBase::TypeJournal:
  {
    // TODO: rename EventOrTodoDialog to IncidenceDialog
    EventOrTodoDialog *dialog = new EventOrTodoDialog( parent, flags );

    // needs to be save to akonadi?, apply button should be turned on if so.
    dialog->setInitiallyDirty( needsSaving /* mInitiallyDirty */ );

    return dialog;
  }
  default:
    return 0;
  }
}

IncidenceDialog * IncidenceDialogFactory::createTodoEditor( const QString &summary,
                                                            const QString &description,
                                                            const QStringList &attachments,
                                                            const QStringList &attendees,
                                                            const QStringList &attachmentMimetypes,
                                                            const QStringList &attachmentLabels,
                                                            bool inlineAttachment,
                                                            Akonadi::Collection defaultCollection,
                                                            bool cleanupAttachmentTemporaryFiles,
                                                            QWidget *parent, Qt::WFlags flags )
{
  IncidenceDefaults defaults = IncidenceDefaults::minimalIncidenceDefaults( cleanupAttachmentTemporaryFiles );
  // if attach or attendee list is empty, these methods don't do anything, so
  // it's safe to call them in every case
  defaults.setAttachments( attachments, attachmentMimetypes, attachmentLabels, inlineAttachment );
  defaults.setAttendees( attendees );

  Todo::Ptr todo( new Todo );
  defaults.setDefaults( todo );

  todo->setSummary( summary );
  todo->setDescription( description );

  Akonadi::Item item;
  item.setPayload( todo );

  // Construct the groupware object, it'll take care of the IncidenceEditors::EditorConfig as well
  if ( !GroupwareIntegration::isActive() ) {
    GroupwareIntegration::activate();
  }

  IncidenceDialog *dialog = create( true, /* no need for, we're not editing an existing to-do */
                                    KCalCore::Incidence::TypeTodo,
                                    parent, flags );
  dialog->selectCollection( defaultCollection );
  dialog->load( item );
  return dialog;
}

IncidenceDialog * IncidenceDialogFactory::createEventEditor( const QString &summary,
                                                             const QString &description,
                                                             const QStringList &attachments,
                                                             const QStringList &attendees,
                                                             const QStringList &attachmentMimetypes,
                                                             const QStringList &attachmentLabels,
                                                             bool inlineAttachment,
                                                             Akonadi::Collection defaultCollection,
                                                             bool cleanupAttachmentTemporaryFiles,
                                                             QWidget *parent, Qt::WFlags flags )
{
  IncidenceDefaults defaults = IncidenceDefaults::minimalIncidenceDefaults( cleanupAttachmentTemporaryFiles );

  // if attach or attendee list is empty, these methods don't do anything, so
  // it's safe to call them in every case
  defaults.setAttachments( attachments, attachmentMimetypes, attachmentLabels, inlineAttachment );
  defaults.setAttendees( attendees );

  Event::Ptr event( new Event );
  defaults.setDefaults( event );

  event->setSummary( summary );
  event->setDescription( description );

  Akonadi::Item item;
  item.setPayload( event );

  // Construct the groupware object, it'll take care of the IncidenceEditors::EditorConfig as well
  if ( !GroupwareIntegration::isActive() ) {
    GroupwareIntegration::activate();
  }

  IncidenceDialog *dialog = create( false, /* no need for saving, we're not editing an existing event */
                                    KCalCore::Incidence::TypeEvent,
                                    parent, flags );
  dialog->selectCollection( defaultCollection );
  dialog->load( item );

  return dialog;
}
