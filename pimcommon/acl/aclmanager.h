/*
 * Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
 * Copyright (c) 2010 Tobias Koenig <tokoe@kdab.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef MAILCOMMON_ACLMANAGER_H
#define MAILCOMMON_ACLMANAGER_H

#include "pimcommon_export.h"

#include <Collection>

#include <QObject>

class QAbstractItemModel;
class QAction;
class QItemSelectionModel;

namespace PimCommon {

class PIMCOMMON_EXPORT AclManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY( Akonadi::Collection collection READ collection WRITE setCollection NOTIFY collectionChanged )
    Q_PROPERTY( QAbstractItemModel *model READ model )
    Q_PROPERTY( QItemSelectionModel *selectionModel READ selectionModel )
    Q_PROPERTY( QAction *addAction READ addAction )
    Q_PROPERTY( QAction *editAction READ editAction )
    Q_PROPERTY( QAction *deleteAction READ deleteAction )

public:
    /**
     * Creates a new ACL manager.
     *
     * @param parent The parent object.
     */
    explicit AclManager( QObject *parent = 0 );

    /**
     * Destroys the ACL manager.
     */
    ~AclManager();

    /**
     * Sets the @p collection whose ACL will be managed.
     */
    void setCollection( const Akonadi::Collection &collection );

    /**
     * Sets the @p collection whose ACL are managed.
     */
    Akonadi::Collection collection() const;

    /**
     * Returns the model that represents the ACL of the managed collection.
     */
    QAbstractItemModel *model() const;

    /**
     * Returns the selection model that is used by the manager to select the
     * ACL entry to work on.
     */
    QItemSelectionModel *selectionModel() const;

    /**
     * Returns the action that handles adding new ACL entries.
     */
    QAction *addAction() const;

    /**
     * Returns the action that handles editing the currently selected ACL entry.
     */
    QAction *editAction() const;

    /**
     * Returns the action that handles deleting the currently selected ACL entry.
     */
    QAction *deleteAction() const;

public Q_SLOTS:
    /**
     * Saves the changes of the ACL back to the collection.
     */
    void save();

Q_SIGNALS:
    /**
     * This signal is emitted whenever the collection whose ACL will
     * be managed has changed.
     */
    void collectionChanged( const Akonadi::Collection &collection );

private:
    //@cond PRIVATE
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT( d, void selectionChanged() )
    Q_PRIVATE_SLOT( d, void addAcl() )
    Q_PRIVATE_SLOT( d, void editAcl() )
    Q_PRIVATE_SLOT( d, void deleteAcl() )
    //@endcond
};

}

#endif
