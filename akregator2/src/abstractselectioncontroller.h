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
#ifndef AKREGATOR2_ABSTRACTSELECTIONCONTROLLER_H
#define AKREGATOR2_ABSTRACTSELECTIONCONTROLLER_H

#include <QObject>
#include <Akonadi/Collection>
#include <Akonadi/Item>

#include <boost/shared_ptr.hpp>
#include <vector>

class QAbstractItemModel;
class QAbstractItemView;
class QItemSelectionModel;
class QModelIndex;
class QPoint;

namespace Akonadi {
    class Collection;
}
namespace Akregator2 {

class FolderExpansionHandler;

namespace Filters {
    class AbstractMatcher;
}

class ArticleLister
{
public:

    virtual ~ArticleLister() {}

    virtual void setItemModel( QAbstractItemModel* model ) = 0;

    virtual QItemSelectionModel* articleSelectionModel() const = 0;

    virtual void setIsAggregation( bool isAggregation ) = 0;

    virtual void setFilters( const std::vector<boost::shared_ptr<const Filters::AbstractMatcher> >& ) = 0;

    virtual void forceFilterUpdate() = 0;

    virtual QPoint scrollBarPositions() const = 0;

    virtual void setScrollBarPositions( const QPoint& p ) = 0;

    virtual const QAbstractItemView* itemView() const = 0;

    virtual QAbstractItemView* itemView() = 0;
};

class SingleArticleDisplay
{
public:
    virtual ~SingleArticleDisplay() {}

    virtual void showItem( const Akonadi::Item& article ) = 0;
};

class AbstractSelectionController : public QObject
{
    Q_OBJECT

public:
    explicit AbstractSelectionController( QObject* parent = 0 );
    virtual ~AbstractSelectionController();

    virtual void setFeedSelector( QAbstractItemView* feedSelector ) = 0;

    virtual void setArticleLister( Akregator2::ArticleLister* lister ) = 0;

    virtual void setFolderExpansionHandler( Akregator2::FolderExpansionHandler* handler ) = 0;

    virtual void setSingleArticleDisplay( Akregator2::SingleArticleDisplay* display ) = 0;

    virtual Akonadi::Item currentItem() const = 0;

    virtual Akonadi::Item::List selectedItems() const = 0;

    virtual QModelIndex selectedCollectionIndex() const = 0;

    virtual Akonadi::Collection selectedCollection() const = 0;

    virtual Akonadi::Collection::List resourceRootCollections() const = 0;

public Q_SLOTS:

    virtual void setFilters( const std::vector<boost::shared_ptr<const Akregator2::Filters::AbstractMatcher> >& ) = 0;

    virtual void forceFilterUpdate() = 0;

Q_SIGNALS:
    void currentCollectionChanged( const Akonadi::Collection& );

    void currentItemChanged( const Akonadi::Item& );

    void itemDoubleClicked( const Akonadi::Item& );
};

} // namespace Akregator2

#endif // AKREGATOR2_ABSTRACTSELECTIONCONTROLLER_H
