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

#ifndef INCIDENCEEDITOR_INCIDENCEWHATWHERE_H
#define INCIDENCEEDITOR_INCIDENCEWHATWHERE_H

#include "incidenceeditors_ng_export.h"
#include "incidenceeditor-ng.h"

namespace Ui {
  class EventOrTodoDesktop;
}

namespace IncidenceEditorNG {

/**
 * The IncidenceGeneralEditor keeps track of the following Incidence parts:
 * - Summary
 * - Location
 * - Categories
 */
class INCIDENCEEDITORS_NG_EXPORT IncidenceWhatWhere : public IncidenceEditor
{
  Q_OBJECT
  public:
    explicit IncidenceWhatWhere( Ui::EventOrTodoDesktop *ui );

    virtual void load( const KCalCore::Incidence::Ptr &incidence );
    virtual void save( const KCalCore::Incidence::Ptr &incidence );
    virtual bool isDirty() const;
    virtual bool isValid() const;
    virtual void validate();

  private:
    Ui::EventOrTodoDesktop *mUi;
};

} // IncidenceEditorNG

#endif
