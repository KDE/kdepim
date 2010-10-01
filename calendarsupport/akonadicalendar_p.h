/*
  Copyright (c) 2009 KDAB
  Authors: Sebastian Sauer <sebsauer@kdab.net>
           Till Adam <till@kdab.net>
           Frank Osterfeld <frank@kdab.net>
           Sérgio Martins <sergio.martins@kdab.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef CALENDARSUPPORT_AKONADICALENDAR_P_H
#define CALENDARSUPPORT_AKONADICALENDAR_P_H

#include "akonadicalendar.h"
#include "calendarmodel.h"

#include <KCalCore/Alarm>

#include <Akonadi/Session>
#include <Akonadi/ChangeRecorder>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/EntityDisplayAttribute>

namespace CalendarSupport {

// Rename when Calendar is deleted.
struct UnseenItem2
{
  Akonadi::Entity::Id collection;
  QString uid;

  bool operator<( const UnseenItem2 &other ) const
  {
    if ( collection != other.collection ) {
      return collection < other.collection;
    }
    return uid < other.uid;
  }
};

class AkonadiCalendar::Private : public QObject
{
  Q_OBJECT
  public:

    enum UpdateMode {
      DontCare,
      AssertExists,
      AssertNew
    };

    Private( AkonadiCalendar *qq );

    ~Private();
    void updateItem( const Akonadi::Item &item, UpdateMode mode );
    void itemChanged( const Akonadi::Item &item );
    void load();
    void clear();
    void readFromModel();
    KCalCore::Alarm::List alarmsTo( const KDateTime &to );

    // Out ETM model
    CalendarModel *mCalendarModel;

    // akonadi id to items
    QHash<Akonadi::Item::Id, Akonadi::Item> m_itemMap;

    // akonadi id to collection
    QHash<Akonadi::Entity::Id, Akonadi::Collection> m_collectionMap;

    // child to parent map, for already cached parents
    QHash<Akonadi::Item::Id, Akonadi::Item::Id> m_childToParent;

    //parent to children map for alread cached children
    QHash<Akonadi::Item::Id, QList<Akonadi::Item::Id> > m_parentToChildren;

    QMap<QString, Akonadi::Item::Id> mUidToItemId;

    QMap<UnseenItem2, Akonadi::Item::Id> mUnseenToItemId;

    // child to parent map, unknown/not cached parent items
    QHash<Akonadi::Item::Id, UnseenItem2> m_childToUnseenParent;

    QMap<UnseenItem2, QList<Akonadi::Item::Id> > m_unseenParentToChildren;

    // on start dates/due dates of non-recurring, single-day Incidences
    QMultiHash<QString, Akonadi::Item::Id> m_itemIdsForDate;

    QHash<Akonadi::Item::Id, QString> m_itemDateForItemId;

  public Q_SLOTS:
    void dataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight );
    void itemsAdded( const Akonadi::Item::List &items );
    void itemsRemoved( const Akonadi::Item::List &items );
    void rowsInserted( const QModelIndex &index, int start, int end );
    void rowsAboutToBeRemoved( const QModelIndex &index, int start, int end );
    void layoutChanged();
    void modelReset();

  private:
    void removeItemFromMaps( const Akonadi::Item &item );
    AkonadiCalendar *q;
};

} // end namespace CalendarSupport

#endif
