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

#ifndef AKONADI_COLLECTIONFETCHWATCHER_H
#define AKONADI_COLLECTIONFETCHWATCHER_H

#include "mobileui_export.h"

#include <QtCore/QObject>

class QAbstractItemModel;
class QModelIndex;

namespace AkonadiFuture {

/**
 * @short A class that encapsulates logic to wait for an collection to be completely loaded into a model.
 *
 * @author Tobias Koenig <tokoe@kde.org>
 */
class MOBILEUI_EXPORT CollectionFetchWatcher : public QObject
{
  Q_OBJECT

  public:
    /**
     * Creates a new collection fetch watcher.
     *
     * @param index The model index of the collection to wait for.
     * @param model The model to work on.
     * @param parent The parent object.
     */
    CollectionFetchWatcher( const QModelIndex &index, const QAbstractItemModel *model, QObject *parent = 0 );

    /**
     * Destroys the collection fetch watcher.
     */
    ~CollectionFetchWatcher();

    /**
     * Starts watching the collection.
     */
    void start();

  Q_SIGNALS:
    /**
     * This signal is emitted when the watched collection at @p index has been loaded completely into
     * the model.
     */
    void collectionFetched( const QModelIndex &index );

  private:
    //@cond PRIVATE
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void dataChanged( const QModelIndex&, const QModelIndex& ) )
    //@endcond
};

}

#endif
