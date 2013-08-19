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

#ifndef INCIDENCEEDITOR_INCIDENCESECRECY_H
#define INCIDENCEEDITOR_INCIDENCESECRECY_H

#include "incidenceeditor-ng.h"

namespace Ui {
  class EventOrTodoDesktop;
  class EventOrTodoMore;
}

namespace IncidenceEditorNG {

class INCIDENCEEDITORS_NG_EXPORT IncidenceSecrecy : public IncidenceEditor
{
  Q_OBJECT
  public:
#ifdef KDEPIM_MOBILE_UI
    explicit IncidenceSecrecy( Ui::EventOrTodoMore *ui );
#else
    explicit IncidenceSecrecy( Ui::EventOrTodoDesktop *ui );
#endif

    virtual void load( const KCalCore::Incidence::Ptr &incidence );
    virtual void save( const KCalCore::Incidence::Ptr &incidence );
    virtual bool isDirty() const;

  private:
#ifdef KDEPIM_MOBILE_UI
    Ui::EventOrTodoMore *mUi;
#else
    Ui::EventOrTodoDesktop *mUi;
#endif
};

}

#endif
