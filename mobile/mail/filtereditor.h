/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Tobias Koenig <tokoe@kdab.com>

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

#ifndef FILTEREDITOR_H
#define FILTEREDITOR_H

#include <QtCore/QObject>

namespace MailCommon {
class FilterController;
}

class KActionCollection;
class QAbstractItemModel;
class QItemSelectionModel;

/**
 * @short The C++ part of the filter editor for mobile apps.
 *
 * This class encapsulates the logic of the filter viewing/editing
 * and the UI is provided by FilterEditor.qml.
 */
class FilterEditor : public QObject
{
  Q_OBJECT

  public:
    /**
     * Creates a new filter editor.
     *
     * @param actionCollection The action collection to register the manipulation
     *                         actions (e.g. add, edit, delete) at
     * @param parent The parent object.
     */
    explicit FilterEditor( KActionCollection *actionCollection, QObject *parent = 0 );

  public Q_SLOTS:
    /**
     * Returns the filter list model.
     */
    QAbstractItemModel* model() const;

    /**
     * Sets the row of the filter the user has selected in the UI.
     */
    void setRowSelected( int row );

  private:
    MailCommon::FilterController *mFilterController;
};

#endif
