/*
  Copyright (C) 2014 Sandro Knauß <knauss@kolabsys.com>

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

#ifndef INCIDENCEEDITOR_INCIDENCERESOURCE_H
#define INCIDENCEEDITOR_INCIDENCERESOURCE_H

#include "incidenceeditor-ng.h"
#include "attendeetablemodel.h"

#include <QModelIndex>
#include <QCompleter>

namespace Ui
{
class EventOrTodoDesktop;
class EventOrTodoMore;
}

namespace IncidenceEditorNG
{

class INCIDENCEEDITORS_NG_EXPORT IncidenceResource : public IncidenceEditor
{
    Q_OBJECT
public:
#ifdef KDEPIM_MOBILE_UI
    explicit IncidenceResource(Ui::EventOrTodoMore *ui);
#else
    explicit IncidenceResource(Ui::EventOrTodoDesktop *ui);
#endif

    void load(const KCalCore::Incidence::Ptr &incidence);
    void save(const KCalCore::Incidence::Ptr &incidence);
    bool isDirty() const;

    /** resturn the count of resources */
    int resourcesCount() const;

signals:
    /** is emitted it the count of the resources is changed.
     * @arg: new count of resources.
     */
    void resourceCountChanged(int);

private slots:
    void findResources();
    void bookResource();
    void layoutChanged();

private:
#ifdef KDEPIM_MOBILE_UI
    Ui::EventOrTodoMore *mUi;
#else
    Ui::EventOrTodoDesktop *mUi;
#endif

    /** completer for findResources */
    QCompleter *completer;

    /** used dataModel to rely on*/
    AttendeeTableModel *dataModel;
};

}

#endif