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

#ifndef SEARCHMANAGER_H
#define SEARCHMANAGER_H

#include <akonadi/collection.h>

#include <QtCore/QObject>

class KJob;

/**
 * @short A class that manages the searches in mobile apps.
 *
 * A mobile app can have exactly one persistent search running
 * at a time. This class will create this search and remove it
 * on destruction or when a new search is started.
 *
 * @author Tobias Koenig <tokoe@kdab.com>
 */
class SearchManager : public QObject
{
  Q_OBJECT

  public:
    /**
     * Creates a new search manager.
     *
     * @param parent The parent object.
     */
    explicit SearchManager( QObject *parent = 0 );

    /**
     * Destroys the search manager.
     *
     * The running search will be stopped and its search collection removed.
     */
    ~SearchManager();

  public Q_SLOTS:
    /**
     * Starts a new search.
     * A previous search will be stopped and its search collection will be removed.
     *
     * @param query The Sparql or XESAM query of the search.
     */
    void startSearch( const QString &query );

    /**
     * Stops the currently running search and removes its search collection.
     */
    void stopSearch();

  Q_SIGNALS:
    /**
     * This signal is emitted whenever a new search has successfully been started.
     *
     * @param collection The search collection of this search.
     */
    void searchStarted( const Akonadi::Collection &collection );

    /**
     * This signal is emitted whenever a search has explicitly been stopped by
     * calling stopSearch().
     */
    void searchStopped();

  private Q_SLOTS:
    void result( KJob* );

  private:
    void cleanUpSearch();

    Akonadi::Collection::Id mCurrentSearchCollection;
};

#endif
