/*
 * Copyright (c) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
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
 *
 */

#ifndef SEARCHCOLLECTIONINDEXINGWARNING_H
#define SEARCHCOLLECTIONINDEXINGWARNING_H

#include <KMessageWidget>
#include <Akonadi/Collection>

class KJob;

namespace Akonadi {
class CollectionFetchJob;
}

class SearchCollectionIndexingWarning : public KMessageWidget
{
    Q_OBJECT

public:
    explicit SearchCollectionIndexingWarning(QWidget *parent = 0);
    ~SearchCollectionIndexingWarning();

    void setCollection(const Akonadi::Collection &collection);

private Q_SLOTS:
    void queryRootCollectionFetchFinished(KJob *job);
    void queryCollectionFetchFinished(KJob *job);
    void indexerStatsFetchFinished(KJob *job);

private:
    Akonadi::CollectionFetchJob *fetchCollections(const Akonadi::Collection::List &cols,
                                                  bool recursive);
    void queryIndexerStatus();

    Akonadi::Collection mCollection;
    Akonadi::Collection::List mCollections;
};

#endif // SEARCHCOLLECTIONINDEXINGWARNING_H
