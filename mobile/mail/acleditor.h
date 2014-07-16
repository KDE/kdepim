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

#ifndef ACLEDITOR_H
#define ACLEDITOR_H

#include <akonadi/collection.h>

#include <QtCore/QObject>

namespace PimCommon {
class AclManager;
}

class KActionCollection;
class QAbstractItemModel;
class QAction;

/**
 * @short The C++ part of the ACL editor for mobile apps.
 *
 * This class encapsulates the logic of the acl viewing/editing
 * and the UI is provided by AclEditor.qml.
 */
class AclEditor : public QObject
{
  Q_OBJECT

  Q_PROPERTY( QString collectionName READ collectionName NOTIFY collectionChanged )
  Q_PROPERTY( bool collectionHasAcls READ collectionHasAcls NOTIFY collectionChanged )

  public:
    /**
     * Creates a new ACL editor.
     *
     * @param actionCollection The action collection to register the manipulation
     *                         actions (e.g. add, edit, delete) at
     * @param parent The parent object.
     */
    explicit AclEditor( KActionCollection *actionCollection, QObject *parent = 0 );

    /**
     * Sets the @p collection whose ACLs shall be edited.
     *
     * @note This does not load the collection into the editor, load
     *       must be called explicitly.
     */
    void setCollection( const Akonadi::Collection &collection );

    /**
     * Returns the name of the current collection.
     */
    QString collectionName() const;

    /**
     * Returns whether the current collection provides ACLs.
     *
     * That's mostly only true for collections that represent IMAP folders.
     */
    bool collectionHasAcls() const;

  public Q_SLOTS:
    /**
     * Loads the current collection into the editor.
     */
    void load();

    /**
     * Saves changes to the ACLs back to the current collection.
     */
    void save();

    /**
     * Returns the ACL list model of the current collection.
     */
    QAbstractItemModel* model() const;

    /**
     * Sets the row of the ACL the user has selected in the UI.
     */
    void setRowSelected( int row );

  Q_SIGNALS:
    /**
     * This signal is emitted whenever the current collection has been
     * changed.
     *
     * @param collection The new current collection.
     */
    void collectionChanged( const Akonadi::Collection &collection );

  private:
    PimCommon::AclManager *mAclManager;
    Akonadi::Collection mCollection;
};

#endif
