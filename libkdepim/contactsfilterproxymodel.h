/*
    This file is part of Akonadi Contact.

    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

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

#ifndef AKONADI_CONTACTSFILTERMODEL_H
#define AKONADI_CONTACTSFILTERMODEL_H

#include "kdepim_export.h"

#include <QtGui/QSortFilterProxyModel>

namespace Akonadi {

/**
 * @short A proxy model for \a ContactsTreeModel models.
 *
 * @author Tobias Koenig <tokoe@kde.org>
 * @since 4.4
 */
class KDEPIM_EXPORT ContactsFilterModel : public QSortFilterProxyModel
{
  Q_OBJECT

  public:
    /**
     * Creates a new contacts filter model.
     *
     * @param parent The parent object.
     */
    ContactsFilterModel( QObject *parent );

    /**
     * Destroys the contacts filter model.
     */
    ~ContactsFilterModel();

  public Q_SLOTS:
    /**
     * Sets the @p filter that is used to filter for matching contacts
     * and contact groups.
     */
    void setFilterString( const QString &filter );

  protected:
    virtual bool filterAcceptsRow( int row, const QModelIndex &parent ) const;
    virtual bool lessThan( const QModelIndex &left, const QModelIndex &right ) const;

  private:
    class Private;
    Private* const d;
};

}

#endif
