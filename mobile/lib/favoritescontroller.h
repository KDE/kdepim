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

#ifndef FAVORITESCONTROLLER_H
#define FAVORITESCONTROLLER_H

#include <ksharedconfig.h>

#include <QtCore/QObject>

class QAbstractItemModel;
class QAction;
class QItemSelectionModel;

class FavoritesController : public QObject
{
  Q_OBJECT

  Q_PROPERTY( QAbstractItemModel* model READ model )
  Q_PROPERTY( QItemSelectionModel* selectionModel READ selectionModel )

  Q_PROPERTY( QAction* removeAction READ removeAction )
  Q_PROPERTY( QAction* moveUpAction READ moveUpAction )
  Q_PROPERTY( QAction* moveDownAction READ moveDownAction )

  public:
    /**
     * Creates a new favorites controller.
     *
     * @param parent The parent object.
     */
    explicit FavoritesController( const KSharedConfig::Ptr &config, QObject *parent = 0 );

    /**
     * Destroys the favorites controller.
     */
    ~FavoritesController();

    /**
     * Returns the model that represents the list of favoritess.
     */
    QAbstractItemModel* model() const;

    /**
     * Returns the item selection model, which is used for adapting
     * the state of the actions.
     */
    QItemSelectionModel* selectionModel() const;

    /**
     * Sets the collection selection @p model the loading and saving of favorites
     * shall be applied on.
     */
    void setCollectionSelectionModel( QItemSelectionModel *model );

    /**
     * Returns the action for removing the currently selected favorites.
     */
    QAction* removeAction() const;

    /**
     * Returns the action for moving up the currently selected favorites.
     */
    QAction* moveUpAction() const;

    /**
     * Returns the action for moving down the currently selected favorites.
     */
    QAction* moveDownAction() const;

  public Q_SLOTS:
    /**
     * Applies the favorite with the given @p name on the collection selection model.
     */
    void loadFavorite( const QString &name ) const;

    /**
     * Saves the current collection selection as favorite with the given @p name.
     */
    void saveFavorite( const QString &name );

  private:
    //@cond PRIVATE
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void selectionChanged() )
    Q_PRIVATE_SLOT( d, void removeFavorite() )
    Q_PRIVATE_SLOT( d, void moveUpFavorite() )
    Q_PRIVATE_SLOT( d, void moveDownFavorite() )
    //@endcond
};

#endif
