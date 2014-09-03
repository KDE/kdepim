/*
  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net,
    Author Tobias Koenig <tokoe@kdab.com>

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

#ifndef MAILCOMMON_FILTERCONTROLLER_H
#define MAILCOMMON_FILTERCONTROLLER_H

#include "mailcommon_export.h"

#include <QObject>

class QAbstractItemModel;
class QAction;
class QItemSelectionModel;

namespace MailCommon
{

class MAILCOMMON_EXPORT FilterController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QAction *addAction READ addAction)
    Q_PROPERTY(QAction *editAction READ editAction)
    Q_PROPERTY(QAction *removeAction READ removeAction)
    Q_PROPERTY(QAction *moveUpAction READ moveUpAction)
    Q_PROPERTY(QAction *moveDownAction READ moveDownAction)

public:
    /**
     * Creates a new filter controller.
     *
     * @param parent The parent object.
     */
    explicit FilterController(QObject *parent = 0);

    /**
     * Destroys the filter controller.
     */
    ~FilterController();

    /**
     * Returns the model that represents the list of filters.
     */
    QAbstractItemModel *model() const;

    /**
     * Returns the item selection model, which is used for adapting
     * the state of the actions.
     */
    QItemSelectionModel *selectionModel() const;

    /**
     * Returns the action for adding a new filter.
     */
    QAction *addAction() const;

    /**
     * Returns the action for editing the currently selected filter.
     */
    QAction *editAction() const;

    /**
     * Returns the action for removing the currently selected filter.
     */
    QAction *removeAction() const;

    /**
     * Returns the action for moving up the currently selected filter.
     */
    QAction *moveUpAction() const;

    /**
     * Returns the action for moving down the currently selected filter.
     */
    QAction *moveDownAction() const;

private:
    //@cond PRIVATE
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT(d, void selectionChanged())
    Q_PRIVATE_SLOT(d, void addFilter())
    Q_PRIVATE_SLOT(d, void editFilter())
    Q_PRIVATE_SLOT(d, void removeFilter())
    Q_PRIVATE_SLOT(d, void moveUpFilter())
    Q_PRIVATE_SLOT(d, void moveDownFilter())
    //@endcond
};

}

#endif
