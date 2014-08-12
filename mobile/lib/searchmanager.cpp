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

#include "searchmanager.h"

#include <AkonadiCore/collectiondeletejob.h>
#include <AkonadiCore/searchcreatejob.h>
#include <klocale.h>
#include <QDebug>
#include <QtCore/QUuid>

SearchManager::SearchManager( QObject *parent )
  : QObject( parent ),
    mCurrentSearchCollection( -1 )
{
}

SearchManager::~SearchManager()
{
  cleanUpSearch();
}

void SearchManager::startSearch( const QString &query )
{
  cleanUpSearch();
#if 0 //QT5
  const QString searchName = i18n( "Search Results" ) + QLatin1String( "                                      " ) + QUuid::createUuid().toString();
  Akonadi::SearchCreateJob *job = new Akonadi::SearchCreateJob( searchName, query );
  connect( job, SIGNAL(result(KJob*)), this, SLOT(result(KJob*)) );
#endif
}

void SearchManager::stopSearch()
{
  cleanUpSearch();

  emit searchStopped();
}

void SearchManager::result( KJob *job )
{
  if ( job->error() ) {
    qWarning() << "Unable to create search collection:" << job->errorText();
    return;
  }

  const Akonadi::SearchCreateJob *searchJob = qobject_cast<Akonadi::SearchCreateJob*>( job );

  const Akonadi::Collection collection = searchJob->createdCollection();
  mCurrentSearchCollection = collection.id();

  emit searchStarted( collection );
}

void SearchManager::cleanUpSearch()
{
  // cleanup search collection
  if ( mCurrentSearchCollection != -1 )
    new Akonadi::CollectionDeleteJob( Akonadi::Collection( mCurrentSearchCollection ) );

  mCurrentSearchCollection = -1;
}

