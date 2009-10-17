/*
    Copyright (C) 2009    Frank Osterfeld <osterfeld@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KRSS_FEEDITEMMODEL_H
#define KRSS_FEEDITEMMODEL_H

#include "krss_export.h"
#include "item.h"

#include <Akonadi/EntityTreeModel>

class KJob;

namespace KRss {

class FeedItemModelPrivate;

class KRSS_EXPORT FeedItemModel : public Akonadi::EntityTreeModel
{
    Q_OBJECT

public:

    enum ItemColumn {
        ItemTitleColumn = 0,
        AuthorsColumn,
        DateColumn,
        FeedTitleColumn,
        ItemColumnCount
    };

    enum Role {
        ItemRole = Qt::UserRole,
        SortRole,
        IsNewRole,
        IsUnreadRole,
        IsReadRole,
        IsDeletedRole, //PENDING(frank) transitional Akregator compat, review
        IsImportantRole, //PENDING(frank) transitional Akregator compat, review
        LinkRole  //PENDING(frank) transitional Akregator compat, review
    };

public:

    explicit FeedItemModel( Akonadi::Session *session, Akonadi::ChangeRecorder* monitor, QObject* parent = 0 );
    ~FeedItemModel();

    /* reimp */ QVariant getData( const Akonadi::Item& item, int column, int role=Qt::DisplayRole ) const;

    /* reimp */ QVariant getData( const Akonadi::Collection& collection, int column, int role=Qt::DisplayRole ) const;

    /* reimp */ int columnCount( const QModelIndex& index = QModelIndex() ) const;

    /* reimp */ QVariant getHeaderData( int section, Qt::Orientation orientation, int role, int headerSet ) const;

private:
    friend class ::KRss::FeedItemModelPrivate;
    FeedItemModelPrivate* const d;
};

} // namespace KRss

Q_DECLARE_METATYPE(KRss::Item)

#endif // KRSS_FEEDITEMMODEL_H
