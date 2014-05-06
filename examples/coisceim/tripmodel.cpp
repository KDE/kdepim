/*
    This file is part of Akonadi.

    Copyright (c) 2011 Stephen Kelly <steveire@gmail.com>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "tripmodel.h"

#include <QTimer>

#include <kicon.h>
#include <KGlobal>
#include <KConfigGroup>
#include <KConfig>

#include <AkonadiCore/Item>
#include <AkonadiCore/ChangeRecorder>
#include <KSharedConfig>

#include "trip.h"
#include "tripwidget.h"
#include "tripcomponentfactory.h"
#include "createtripwidget.h"

using namespace Akonadi;

TripModel::TripModel(Akonadi::ChangeRecorder* monitor, QObject* parent)
  : MixedTreeModel(monitor, parent), m_monitor(monitor)
{
  setCollectionFetchStrategy(MixedTreeModel::FetchNoCollections);

  TripComponentFactory factory;
  Trip *create = new Trip(createIndex(0, 0), monitor, &factory);
  m_createWidget = new CreateTripWidget(create, monitor);

  QTimer::singleShot(0, this, SLOT(repopulate()));

  connect(this, SIGNAL(modelReset()), SLOT(thisModelReset()));

  QHash<int, QByteArray> roleNames_ = roleNames();
  roleNames_.insert(TripRole, "trip");
  setRoleNames(roleNames_);
}

QModelIndex TripModel::index(int row, int column, const QModelIndex& parent) const
{
  if (row == rowCount() - 1)
    return createIndex(row, column);
  return MixedTreeModel::index(row, column, parent);
}

int TripModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent)
  return 1;
}

int TripModel::rowCount(const QModelIndex& parent) const
{
  return MixedTreeModel::rowCount(parent) + 1;
}

static const char* icons[] = {
  "akonadi",
  "nepomuk",
  "plasma"
};

QVariant TripModel::data(const QModelIndex& index, int role) const
{
  if (role == Qt::SizeHintRole && index.column() == 0)
    return QSize(180, 60);
  if (index.row() == rowCount() - 1) {
    if (role == Qt::DisplayRole)
      return QLatin1String("CREATE");
    if (role == WidgetRole) {
      return QVariant::fromValue<QWidget*>(m_createWidget);
    }
    if (role == Qt::DecorationRole) {
      return KIcon(QLatin1String("list-add-new"));
    }
    return QVariant();
  }
  if (role == WidgetRole) {
    const Item::Id id = MixedTreeModel::data(index, MixedTreeModel::ItemIdRole).toLongLong();
    if (!m_tripWidgets.contains(id))
      createWidget(index, id);
    return QVariant::fromValue<QWidget*>(m_tripWidgets.value(id));
  } else if (role == TripRole) {
    const Item::Id id = MixedTreeModel::data(index, MixedTreeModel::ItemIdRole).toLongLong();
    if (!m_trips.contains(id))
      createTrip(index, id);
    return QVariant::fromValue<QObject*>(m_trips.value(id));
  }

  if (role == Qt::DecorationRole) {
    const int iconsSize = sizeof(icons) / sizeof(*icons);
    return KIcon(QLatin1String(*(icons + (index.row() % iconsSize))));
  }

  return MixedTreeModel::data(index, role);
}

QModelIndex TripModel::parent(const QModelIndex& index) const
{
  Q_UNUSED(index);
  return QModelIndex();
}

Qt::ItemFlags TripModel::flags(const QModelIndex& index) const
{
  if (index.row() == rowCount() - 1)
    return QAbstractItemModel::flags( index );
  return MixedTreeModel::flags(index);
}

bool TripModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (index.row() == rowCount() - 1)
    return false;
  return MixedTreeModel::setData(index, value, role);
}

Q_DECLARE_METATYPE(QList<Collection::Id>)

Trip* TripModel::createTrip(const QModelIndex &index, Akonadi::Item::Id id) const
{
  if (m_trips.contains(id))
    return m_trips.value(id);

  TripComponentFactory factory;

  Trip *trip = new Trip(index, m_monitor, &factory);

  KSharedConfigPtr config = KSharedConfig::openConfig();

  KConfigGroup tripsGroup( config, "Trips" );

  QVariantList ids = tripsGroup.readEntry<QVariantList>(QString::number(id), QVariantList());

  if (ids.size() == 3) {
    trip->setCollection(Trip::MailCollectionRole, Collection(ids.takeFirst().toLongLong()));
    trip->setCollection(Trip::TodoCollectionRole, Collection(ids.takeFirst().toLongLong()));
    trip->setCollection(Trip::NotesCollectionRole, Collection(ids.takeFirst().toLongLong()));
  }

  m_trips.insert(id, trip);
  return trip;
}

void TripModel::createWidget(const QModelIndex &index, Akonadi::Item::Id id) const
{
  if (m_tripWidgets.contains(id))
    return;

  Trip *trip = createTrip(index, id);

  TripWidget *w = new TripWidget(trip);

  m_tripWidgets.insert(id, w);
}

bool TripModel::removeRows(int row, int count, const QModelIndex& parent)
{
  Q_UNUSED(count)
  const QModelIndex idx = index(row, 0, parent);
  const Item item = idx.data(MixedTreeModel::ItemRole).value<Item>();

  KSharedConfigPtr config = KSharedConfig::openConfig();

  KConfigGroup generalGroup( config, "General" );

  QVariantList trips = generalGroup.readEntry<QVariantList>("trips", QVariantList());

  QVariantList::iterator it = trips.begin();

  while (it != trips.end()) {
    const Item::Id eventId = it->toLongLong();
    if (eventId > 0 && eventId == item.id())
      trips.erase(it);
    else
      ++it;
  }
  generalGroup.writeEntry("trips", trips);
  generalGroup.sync();

  KConfigGroup tripsGroup(config, "Trips" );
  tripsGroup.deleteEntry(QString::number(item.id()));

  config->sync();

  m_monitor->setItemMonitored(item, false);
  return false;
}

void TripModel::repopulate()
{
  KSharedConfigPtr config = KSharedConfig::openConfig();

  KConfigGroup generalGroup( config, "General" );

  const QVariantList trips = generalGroup.readEntry<QVariantList>("trips", QVariantList());

  foreach(const QVariant &trip, trips) {
    const Item::Id eventId = trip.toLongLong();
    if (eventId > 0)
      m_monitor->setItemMonitored(Item(eventId), true);
  }

}

void TripModel::thisModelReset()
{
  qDeleteAll(m_trips);
  qDeleteAll(m_tripWidgets);
  m_trips.clear();
  m_tripWidgets.clear();
}

void TripModel::thisRowsRemoved(const QModelIndex& index, int start, int end)
{
  Q_UNUSED(index)
  Q_UNUSED(start)
  Q_UNUSED(end)
  qDeleteAll(m_trips);
  qDeleteAll(m_tripWidgets);
  m_trips.clear();
  m_tripWidgets.clear();
}
