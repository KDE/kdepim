/*
  Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
  Author: Kevin Krammer <krake@kdab.com>

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

#ifndef INCIDENCEEDITOR_INCIDENCECOMPLETIONPRIORITY_H
#define INCIDENCEEDITOR_INCIDENCECOMPLETIONPRIORITY_H

#include "incidenceeditor-ng.h"

namespace Ui
{
class EventOrTodoDesktop;
}

namespace IncidenceEditorNG
{

class INCIDENCEEDITORS_NG_EXPORT IncidenceCompletionPriority : public IncidenceEditor
{
    Q_OBJECT
public:
    explicit IncidenceCompletionPriority(Ui::EventOrTodoDesktop *ui);
    ~IncidenceCompletionPriority();

    void load(const KCalCore::Incidence::Ptr &incidence);
    void save(const KCalCore::Incidence::Ptr &incidence);
    bool isDirty() const;

private:
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT(d, void sliderValueChanged(int))
};

}

#endif
