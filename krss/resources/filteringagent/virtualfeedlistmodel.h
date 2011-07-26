/*
    Copyright (C) 2009    Dmitry Ivanov <vonami@gmail.com>

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

#ifndef RSSFILTERINGAGENT_VIRTUALFEEDLISTMODEL_H
#define RSSFILTERINGAGENT_VIRTUALFEEDLISTMODEL_H

#include <krss/virtualfeedcollection.h>

#include <Akonadi/Collection>
#include <QAbstractListModel>
#include <QList>

class VirtualFeedListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Role {
        FeedIdRole = Qt::UserRole,
        UserRole = Qt::UserRole + 100
    };

    explicit VirtualFeedListModel( const QList<KRss::VirtualFeedCollection>& virtualFeeds,
                                   QObject* parent = 0 );

    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
    int rowCount( const QModelIndex& parent = QModelIndex() ) const;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    int rowForFeed( const Akonadi::Collection::Id& id );

private Q_SLOTS:
    void slotCollectionAdded( const Akonadi::Collection& collection, const Akonadi::Collection& parent );
    void slotCollectionChanged( const Akonadi::Collection& collection );
    void slotCollectionRemoved( const Akonadi::Collection& collection );

private:
    QList<KRss::VirtualFeedCollection> m_virtualFeeds;
};

#endif // RSSFILTERINGAGENT_VIRTUALFEEDLISTMODEL_H
