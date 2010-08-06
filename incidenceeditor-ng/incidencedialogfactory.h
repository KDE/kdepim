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

#ifndef INCIDENCEDIALOGFACTORY_H
#define INCIDENCEDIALOGFACTORY_H

#include <KCalCore/Incidence>

#include "incidenceeditors-ng_export.h"

namespace IncidenceEditorsNG
{

class IncidenceDialog;

namespace IncidenceDialogFactory
{
  /**
    Creates a new IncidenceDialog for given type. Returns 0 for unsupported types.

    @param type   The Incidence type for which to create a dialog.
    @param parent The parent widget of the dialog
    @param flags  The window flags for the dialog.

    TODO: Implement support for Journals.
    NOTE: There is no editor for Incidence::TypeFreeBusy
   */
  INCIDENCEEDITORS_NG_EXPORT IncidenceDialog *create( KCalCore::Incidence::IncidenceType type,
                                                      QWidget *parent = 0,
                                                      Qt::WFlags flags = 0);
}

} // IncidenceEditorsNG

#endif // INCIDENCEDIALOGFACTORY_H
