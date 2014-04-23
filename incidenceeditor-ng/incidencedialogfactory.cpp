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

#include "incidencedialogfactory.h"
#include "incidencedialog.h"
#include "incidencedefaults.h"

#include <KCalCore/Event>
#include <KCalCore/Todo>
#include <Item>
#include <Calendar/IncidenceChanger>

using namespace IncidenceEditorNG;
using namespace KCalCore;

IncidenceDialog *IncidenceDialogFactory::create( bool needsSaving,
                                                 KCalCore::IncidenceBase::IncidenceType type,
                                                 Akonadi::IncidenceChanger *changer,
                                                 QWidget *parent, Qt::WindowFlags flags )
{
  switch ( type ) {
  case KCalCore::IncidenceBase::TypeEvent: // Fall through
  case KCalCore::IncidenceBase::TypeTodo:
  case KCalCore::IncidenceBase::TypeJournal:
  {
    IncidenceDialog *dialog = new IncidenceDialog( changer, parent, flags );

    // needs to be save to akonadi?, apply button should be turned on if so.
    dialog->setInitiallyDirty( needsSaving/* mInitiallyDirty */ );

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
                                                            bool cleanupAttachmentTempFiles,
                                                            QWidget *parent, Qt::WindowFlags flags )
{
  IncidenceDefaults defaults =
    IncidenceDefaults::minimalIncidenceDefaults( cleanupAttachmentTempFiles );

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

  IncidenceDialog *dialog = create( true, /* no need for, we're not editing an existing to-do */
                                    KCalCore::Incidence::TypeTodo,
                                    0,
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
                                                             bool cleanupAttachmentTempFiles,
                                                             QWidget *parent, Qt::WindowFlags flags )
{
  IncidenceDefaults defaults =
    IncidenceDefaults::minimalIncidenceDefaults( cleanupAttachmentTempFiles );

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

  IncidenceDialog *dialog =
    create( false, // not needed for saving, as we're not editing an existing incidence
            KCalCore::Incidence::TypeEvent,
            0,
            parent, flags );

  dialog->selectCollection( defaultCollection );
  dialog->load( item );

  return dialog;
}
