/*
  Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
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

#ifndef INCIDENCEEDITOR_INCIDENCECATEGORIES_H
#define INCIDENCEEDITOR_INCIDENCECATEGORIES_H

#include "incidenceeditor-ng.h"

namespace Ui {
  class EventOrTodoDesktop;
  class EventOrTodoMore;
}

namespace IncidenceEditorNG {

class INCIDENCEEDITORS_NG_EXPORT IncidenceCategories : public IncidenceEditor
{
  Q_OBJECT
  public:
#ifdef KDEPIM_MOBILE_UI
    explicit IncidenceCategories( Ui::EventOrTodoMore *ui );
#else
    explicit IncidenceCategories( Ui::EventOrTodoDesktop *ui );
#endif

    virtual void load( const KCalCore::Incidence::Ptr &incidence );
    virtual void save( const KCalCore::Incidence::Ptr &incidence );

    /**
     * Sets the currently selected categories.
     */
    void setCategories( const QStringList &categories );

    /**
     * Returns the list of currently selected categories.
     */
    QStringList categories() const;

    virtual bool isDirty() const;
    /**reimp*/
    void printDebugInfo() const;

  private slots:
    void selectCategories();
    void onSelectionChanged(const QStringList &);

  private:

    /** If the incidence comes from outside of KDE it can contain unknown categories.
     * KOrganizer usually checks for these, but it can happen that it checks before the
     * items are in the ETM, due to akonadi's async nature.
     * So we make the check inside the editor, and add new categories to config. This way
     * the editor can be used standalone too.
     * */
    void checkForUnknownCategories( const QStringList &categoriesToCheck );

    QStringList mSelectedCategories;
#ifdef KDEPIM_MOBILE_UI
    Ui::EventOrTodoMore *mUi;
#else
    Ui::EventOrTodoDesktop *mUi;
#endif
};

}

#endif
