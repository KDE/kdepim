/*
    This file is part of Akregator2.

        Copyright (C) 2007 Frank Osterfeld <osterfeld@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef AKREGATOR2_SELECTIONCONTROLLER_H
#define AKREGATOR2_SELECTIONCONTROLLER_H

#include "abstractselectioncontroller.h"

#include <QtCore/QMap>
#include <QtCore/QPointer>

class QModelIndex;
class QPoint;
class QItemSelection;
class QItemSelectionModel;
class QTimer;

class KJob;

namespace Akonadi {
    class Item;
    class Session;
}

namespace KRss {
    class FeedItemModel;
}

namespace Akregator2
{

class TotalUnreadCountWatcher : public QObject {
    Q_OBJECT
public:
    explicit TotalUnreadCountWatcher( QObject* parent=0 );

    void setModel( QAbstractItemModel* model );

    int unreadCount() const;

Q_SIGNALS:
    void unreadCountChanged( int count );

private Q_SLOTS:
    void dataChanged( const QModelIndex& topLeft, const QModelIndex&  bottomRight );
    void updateUnreadCount();

private:
    QPointer<QAbstractItemModel> m_model;
    int m_unreadCount;
    QTimer* m_timer;
};

class SelectionController : public AbstractSelectionController
{
    Q_OBJECT

public:

    explicit SelectionController( Akonadi::Session* session, QObject* parent = 0 );

    //impl
    void setFeedSelector( QAbstractItemView* feedSelector ) ;

    //impl
    void setArticleLister( Akregator2::ArticleLister* lister );

    //impl
    Akonadi::Item currentItem() const;

    //impl
    Akonadi::Item::List selectedItems() const;

    //impl
    void setSingleArticleDisplay( Akregator2::SingleArticleDisplay* display );

    QModelIndex selectedCollectionIndex() const;

    Akonadi::Collection selectedCollection() const;

    Akonadi::Collection::List resourceRootCollections() const;

    //impl
    void setFolderExpansionHandler( Akregator2::FolderExpansionHandler* handler );

Q_SIGNALS:
    void totalUnreadCountChanged( int );

public Q_SLOTS:

    //impl
    void setFilters( const std::vector<boost::shared_ptr<const Akregator2::Filters::AbstractMatcher> >& );

    //impl
    void forceFilterUpdate();

private Q_SLOTS:
    void itemSelectionChanged();
    void fullItemFetched( KJob* );
    void itemIndexDoubleClicked( const QModelIndex& index );
    void subscriptionContextMenuRequested( const QPoint& point );
    void feedSelectionChanged ( const QItemSelection & selected, const QItemSelection & deselected );

private:
    void init();

private:
    QPointer<QAbstractItemView> m_feedSelector;
    Akregator2::ArticleLister* m_articleLister;
    Akregator2::SingleArticleDisplay* m_singleDisplay;
    Akregator2::FolderExpansionHandler* m_folderExpansionHandler;
    KRss::FeedItemModel* m_itemModel;
    QItemSelectionModel* m_feedSelectionResolved;
    QMap<Akonadi::Collection, QPoint> m_scrollBarPositions;
    Akonadi::Session* m_session;
    QAbstractItemModel* m_collectionFilterModel;
    TotalUnreadCountWatcher* m_unreadWatcher;
};

} // namespace Akregator2

#endif // AKREGATOR2_SELECTIONCONTROLLER_H
