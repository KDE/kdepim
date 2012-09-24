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

#ifndef AKONADI_EVENTSFILTERPROXYMODEL_H
#define AKONADI_EVENTSFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

namespace Akonadi {

/**
 * @short A proxy model for \a EntityTreeModel based event models.
 *
 * This class provides a filter proxy model for an EntityTreeModel.
 * The list of shown events can be limited by setting a filter pattern.
 * Only events that contain this pattern as part of their data will be listed.
 *
 * Example:
 *
 * @code
 *
 * Akonadi::EventsFilterProxyModel *filter = new Akonadi::EventsFilterProxyModel;
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
class EventsFilterProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT

  public:
    /**
     * Creates a new events filter proxy model.
     *
     * @param parent The parent object.
     */
    explicit EventsFilterProxyModel( QObject *parent = 0 );

    /**
     * Destroys the events filter proxy model.
     */
    ~EventsFilterProxyModel();

  public Q_SLOTS:
    /**
     * Sets the @p filter that is used to filter for matching events.
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

#endif // AKONADI_EVENTSFILTERPROXYMODEL_H
