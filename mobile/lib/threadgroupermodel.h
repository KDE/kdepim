/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

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

#ifndef THREADGROUPERMODEL_H
#define THREADGROUPERMODEL_H

#include "mobileui_export.h"

#include <AkonadiCore/EntityTreeModel>

#include <QItemSelectionModel>
#include <QSortFilterProxyModel>

class ThreadGrouperModelPrivate;

/**
 * @short A base class for custom comperators, used by ThreadGrouperModel for sorting.
 */
class MOBILEUI_EXPORT ThreadGrouperComparator
{
  public:
    /**
     * Creates a thread grouper comparator.
     */
    ThreadGrouperComparator();

    /**
     * Destroys the thread grouper comparator.
     */
    virtual ~ThreadGrouperComparator();

    /**
     * Reimplement to return the unique identifier for the given @p item.
     */
    virtual QByteArray identifierForItem( const Akonadi::Item &item ) const = 0;

    /**
     * Reimplement to return the parent identifier for the given @p item.
     */
    virtual QByteArray parentIdentifierForItem( const Akonadi::Item &item ) const = 0;

    /**
     * Reimplement to return if the @p left item is smaller than the @p right item.
     */
    virtual bool lessThan( const Akonadi::Item &left, const Akonadi::Item &right ) const = 0;

    /**
     * Returns the grouper string for the given @p item.
     */
    virtual QString grouperString( const Akonadi::Item &item ) const;

  protected:
    /**
     * Returns the thread item for @p item.
     */
    Akonadi::Item threadItem( const Akonadi::Item &item ) const;

    /**
     * Returns the item for the given item @p identifier.
     */
    Akonadi::Item itemForIdentifier( const QByteArray &identifier ) const;

    /**
     * Returns the set of descendants identifiers for the given thread @p identifier.
     */
    QSet<QByteArray> threadDescendants( const QByteArray &identifier ) const;

    /**
     * Invalidates the ThreadGrouperModel to trigger a refresh.
     */
    void invalidate();

    /**
     * This method is called whenever the comparator should reset its caches.
     */
    virtual void resetCaches();

  private:
    Q_DISABLE_COPY( ThreadGrouperComparator )
    ThreadGrouperModelPrivate *m_grouper;

    friend class ThreadGrouperModelPrivate;
};

class MOBILEUI_EXPORT ThreadGrouperModel : public QSortFilterProxyModel
{
  Q_OBJECT

  public:
    enum CustomRoles {
      // FIXME Fix custom role handling in proxies.
      ThreadIdRole = Akonadi::EntityTreeModel::UserRole + 30,
      GrouperRole
    };

    /**
     * Creates a new thread grouper model.
     *
     * @param comparator The comparator object, which abstracts type specific comparison.
     * @param parent The parent object.
     *
     * @note The model does not take ownership of the comparator.
     */
    explicit ThreadGrouperModel( ThreadGrouperComparator *comparator, QObject* parent = 0 );

    virtual ~ThreadGrouperModel();

    virtual void setSourceModel( QAbstractItemModel *sourceModel );

    virtual bool lessThan( const QModelIndex &left, const QModelIndex &right ) const;
    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;

    /**
     * Set whether threading will be @p enabled.
     */
    void setThreadingEnabled( bool enabled );

    /**
     * Returns whether threading is enabled.
     */
    bool threadingEnabled() const;

    /**
     * Sets whether the model shall be repopulated on data change.
     *
     * @note Must be called before setSourceModel() to have any effect.
     */
    void setDynamicModelRepopulation( bool enabled );

  private:
    Q_DECLARE_PRIVATE( ThreadGrouperModel )
    ThreadGrouperModelPrivate* const d_ptr;
    Q_PRIVATE_SLOT( d_func(), void populateThreadGrouperModel() )
    Q_PRIVATE_SLOT( d_func(), void resort() )
};

#endif
