/*
    Copyright (c) 2010 Tobias Koenig <tokoe@kde.org>

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

#ifndef EMAILSFILTERPROXYMODEL_H
#define EMAILSFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

namespace Akonadi {

/**
 * @short A proxy model for \a EntityTreeModel based email models.
 *
 * This class provides a filter proxy model for a EntityTreeModel.
 * The list of shown emails can be limited by settings a filter pattern.
 * Only emails that contain this pattern as part of their data will be listed.
 *
 * Example:
 *
 * @code
 *
 * Akonadi::EmailsFilterProxyModel *filter = new Akonadi::EmailsFilterProxyModel;
 * filter->setSourceModel( model );
 *
 * Akonadi::EntityTreeView *view = new Akonadi::EntityTreeView;
 * view->setModel( filter );
 *
 * QLineEdit *filterEdit = new QLineEdit;
 * connect( filterEdit, SIGNAL( textChanged( const QString& ) ),
 *          filter, SLOT( setFilterString( const QString& ) ) );
 *
 * @endcode
 *
 * @author Tobias Koenig <tokoe@kde.org>
 */
class EmailsFilterProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT

  public:
    /**
     * Creates a new emails filter proxy model.
     *
     * @param parent The parent object.
     */
    explicit EmailsFilterProxyModel( QObject *parent = 0 );

    /**
     * Destroys the emails filter proxy model.
     */
    ~EmailsFilterProxyModel();

  public Q_SLOTS:
    /**
     * Sets the @p filter that is used to filter for matching emails.
     */
    void setFilterString( const QString &filter );

  protected:
    //@cond PRIVATE
    virtual bool filterAcceptsRow( int row, const QModelIndex &parent ) const;
    //@endcond

  private:
    //@cond PRIVATE
    class Private;
    Private* const d;
    //@endcond
};

}

#endif
