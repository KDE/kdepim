/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
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

#ifndef EVENTORTODODIALOGNG_H
#define EVENTORTODODIALOGNG_H

#include <KDialog>

#include "incidenceeditors-ng_export.h"

namespace Akonadi {
class Item;
}

class EventOrTodoDialogNGPrivate;

namespace IncidenceEditorsNG {

class INCIDENCEEDITORS_NG_EXPORT EventOrTodoDialogNG : public KDialog
{
  Q_OBJECT
public:
  EventOrTodoDialogNG();
  ~EventOrTodoDialogNG();

  /**
   * Loads the @param item into the dialog.
   *
   * To create a new Incidence pass an invalid item with either an
   * KCal::Event:Ptr or a KCal::Todo:Ptr set as payload.
   *
   * When the item has is valid it will fetch the payload when this is not
   * set.
   */
  void load( const Akonadi::Item &item );

private:
  EventOrTodoDialogNGPrivate * const d_ptr;
  Q_DECLARE_PRIVATE( EventOrTodoDialogNG )
  Q_DISABLE_COPY( EventOrTodoDialogNG )

  Q_PRIVATE_SLOT(d_ptr, void updateButtonStatus(bool))
};

}

#endif // EVENTORTODODIALOGNG_H
