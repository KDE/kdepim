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

#ifndef FAVORITESEDITOR_H
#define FAVORITESEDITOR_H

#include <ksharedconfig.h>

#include <QtCore/QObject>

class FavoritesController;
class KActionCollection;
class QAbstractItemModel;
class QItemSelectionModel;

/**
 * @short The C++ part of the favorites editor for mobile apps.
 *
 * This class encapsulates the logic of the favorites viewing/editing
 * and the UI is provided by FavoritesEditor.qml.
 */
class FavoritesEditor : public QObject
{
  Q_OBJECT

  public:
    /**
     * Creates a new favorites editor.
     *
     * @param actionCollection The action collection to register the manipulation
     *                         actions (e.g. remove, moveUp, moveDown) at.
     * @param config The config object to store the favorites.
     * @param parent The parent object.
     */
    FavoritesEditor( KActionCollection *actionCollection, const KSharedConfig::Ptr &config, QObject *parent = 0 );

    /**
     * Returns the model that represents the list of favoritess.
     */
    QAbstractItemModel* model() const;

    /**
     * Sets the collection selection @p model the loading and saving of favorites
     * shall be applied on.
     */
    void setCollectionSelectionModel( QItemSelectionModel *model );

    /**
     * Applies the favorite with the given @p name on the collection selection model.
     */
    void loadFavorite( const QString &name ) const;

    /**
     * Saves the current collection selection as favorite with the given @p name.
     */
    void saveFavorite( const QString &name );

  public Q_SLOTS:
    /**
     * Sets the row of the favorite the user has selected in the UI.
     */
    void setRowSelected( int row );

  private:
    FavoritesController *mFavoritesController;
};

#endif
