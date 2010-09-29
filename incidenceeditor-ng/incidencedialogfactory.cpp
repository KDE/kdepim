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

#include <KCalCore/Todo>
#include <Akonadi/Item>

using namespace IncidenceEditorNG;
using namespace KCalCore;

IncidenceDialog *IncidenceDialogFactory::create( KCalCore::IncidenceBase::IncidenceType type,
                                                 QWidget *parent, Qt::WFlags flags )
{
  switch ( type ) {
  case KCalCore::IncidenceBase::TypeEvent: // Fall through
  case KCalCore::IncidenceBase::TypeTodo:
  case KCalCore::IncidenceBase::TypeJournal:
    // TODO: rename EventOrTodoDialog to IncidenceDialog
    return new EventOrTodoDialog( parent, flags );
  default:
    return 0;
  }
}

IncidenceDialog * IncidenceDialogFactory::createTodoEditor( const QString &summary,
                                                            const QString &description,
                                                            const QStringList &attachments,
                                                            const QStringList &attendees,
                                                            const QStringList &attachmentMimetypes,
                                                            bool inlineAttachment,
                                                            Akonadi::Collection defaultCollection,
                                                            QWidget *parent, Qt::WFlags flags )
{
  IncidenceDefaults defaults = IncidenceDefaults::minimalIncidenceDefaults();
  // if attach or attendee list is empty, these methods don't do anything, so
  // it's safe to call them in every case
  defaults.setAttachments( attachments, attachmentMimetypes, inlineAttachment );
  defaults.setAttendees( attendees );

  Todo::Ptr todo( new Todo );
  defaults.setDefaults( todo );

  todo->setSummary( summary );
  todo->setDescription( description );

  Akonadi::Item item;
  item.setPayload( todo );

  IncidenceEditorNG::IncidenceDialog *dialog = create( KCalCore::Incidence::TypeTodo,
                                                       parent, flags );
  dialog->selectCollection( defaultCollection );
  dialog->load( item );
}
