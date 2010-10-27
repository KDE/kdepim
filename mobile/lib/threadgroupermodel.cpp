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

#include "threadgroupermodel.h"

#include <KMime/Message>

#include <map>

struct ItemLessThanComparator
{
  ThreadGrouperModelPrivate const * m_grouper;
  ItemLessThanComparator(const ThreadGrouperModelPrivate * grouper);
  ItemLessThanComparator(const ItemLessThanComparator &other);
  ItemLessThanComparator &operator=(const ItemLessThanComparator &other);

  bool operator()(const Akonadi::Item &left, const Akonadi::Item &right) const;
};

typedef std::map<Akonadi::Item, QByteArray, ItemLessThanComparator> MessageMap;

class ThreadGrouperModelPrivate
{
public:
  ThreadGrouperModelPrivate(ThreadGrouperModel *qq)
    : q_ptr(qq), m_messageMap(this), m_order(ThreadGrouperModel::ThreadsWithNewRepliesOrder)
  {
  }
  Q_DECLARE_PUBLIC(ThreadGrouperModel)
  ThreadGrouperModel * const q_ptr;

  Akonadi::Item getThreadItem(const Akonadi::Item &item) const;

  Akonadi::Item threadRoot(const QModelIndex &index) const;

  KDateTime getMostRecentUpdate(KMime::Message::Ptr threadRoot) const;

  void populateThreadGrouperModel() const;

  mutable QHash<QByteArray, QSet<QByteArray> > m_threads;
  mutable MessageMap m_messageMap;
  mutable QHash<QByteArray, Akonadi::Item> m_threadItems;
  mutable QHash<QByteArray, Akonadi::Item> m_allItems;

  ThreadGrouperModel::OrderScheme m_order;
};

static QHash<QByteArray, QSet<QByteArray> >::const_iterator findValue(const QHash<QByteArray, QSet<QByteArray> > &container, const QByteArray &target)
{
  QHash<QByteArray, QSet<QByteArray> >::const_iterator it = container.constBegin();
  const QHash<QByteArray, QSet<QByteArray> >::const_iterator end = container.constEnd();

  for ( ; it != end; ++it)
  {
    if (it.value().contains(target)) {
      return it;
    }
  }
  return end;
}

Akonadi::Item ThreadGrouperModelPrivate::getThreadItem(const Akonadi::Item& item) const
{
  Q_ASSERT(item.hasPayload<KMime::Message::Ptr>());
  const KMime::Message::Ptr message = item.payload<KMime::Message::Ptr>();
  const QByteArray identifier = message->messageID()->identifier();

  if (m_threadItems.contains(identifier))
    return m_threadItems.value(identifier);

  QHash<QByteArray, QSet<QByteArray> >::const_iterator it = findValue(m_threads, identifier);
  Q_ASSERT(it != m_threads.constEnd());
  Q_ASSERT(m_threadItems.value(it.key()).isValid());
  return m_threadItems.value(it.key());
}

KDateTime ThreadGrouperModelPrivate::getMostRecentUpdate(KMime::Message::Ptr threadRoot) const
{
  QSet<QByteArray> messages = m_threads[threadRoot->messageID()->identifier()];

  KDateTime newest = threadRoot->date()->dateTime();

  if (messages.isEmpty())
    return newest;

  foreach(const QByteArray &ba, messages) {
    const Akonadi::Item item = m_allItems.value(ba);
    Q_ASSERT(item.isValid());
    Q_ASSERT(item.hasPayload<KMime::Message::Ptr>());
    KMime::Message::Ptr message = item.payload<KMime::Message::Ptr>();
    KDateTime messageDateTime = message->date()->dateTime();
    if (messageDateTime > newest)
      newest = messageDateTime;
  }

  return newest;
}

ItemLessThanComparator::ItemLessThanComparator(const ThreadGrouperModelPrivate * grouper)
  : m_grouper(grouper)
{

}

ItemLessThanComparator::ItemLessThanComparator(const ItemLessThanComparator &other)
{
  m_grouper = other.m_grouper;
}

ItemLessThanComparator &ItemLessThanComparator::operator=(const ItemLessThanComparator &other)
{
  m_grouper = other.m_grouper;
  return *this;
}

bool ItemLessThanComparator::operator()(const Akonadi::Item &leftItem, const Akonadi::Item &rightItem) const
{
  Q_ASSERT(leftItem.isValid());
  Q_ASSERT(rightItem.isValid());

  const Akonadi::Item leftThreadRootItem = m_grouper->getThreadItem(leftItem);
  const Akonadi::Item rightThreadRootItem = m_grouper->getThreadItem(rightItem);

  Q_ASSERT(rightThreadRootItem.isValid());
  Q_ASSERT(leftThreadRootItem.isValid());

  if (leftThreadRootItem != rightThreadRootItem) {
    Q_ASSERT(leftThreadRootItem.hasPayload<KMime::Message::Ptr>());
    Q_ASSERT(rightThreadRootItem.hasPayload<KMime::Message::Ptr>());

    const KMime::Message::Ptr leftThreadRootMessage = leftThreadRootItem.payload<KMime::Message::Ptr>();
    const KMime::Message::Ptr rightThreadRootMessage = rightThreadRootItem.payload<KMime::Message::Ptr>();

    if (m_grouper->m_order == ThreadGrouperModel::ThreadsWithNewRepliesOrder) {
      const KDateTime leftNewest = m_grouper->getMostRecentUpdate(leftThreadRootMessage);
      const KDateTime rightNewest = m_grouper->getMostRecentUpdate(rightThreadRootMessage);

      if (leftNewest != rightNewest) {
        return leftNewest > rightNewest;
      }
    } else {
      const KDateTime leftThreadRootDateTime = leftThreadRootMessage->date()->dateTime();
      const KDateTime rightThreadRootDateTime = rightThreadRootMessage->date()->dateTime();
      if (leftThreadRootDateTime != rightThreadRootDateTime) {
        return leftThreadRootDateTime > rightThreadRootDateTime;
      }
    }

    return leftThreadRootItem.id() < rightThreadRootItem.id();
  }

  if (leftThreadRootItem == leftItem)
    return true;

  if (rightThreadRootItem == rightItem)
    return false;

  Q_ASSERT(leftItem.hasPayload<KMime::Message::Ptr>());
  Q_ASSERT(rightItem.hasPayload<KMime::Message::Ptr>());

  const KMime::Message::Ptr leftMessage = leftItem.payload<KMime::Message::Ptr>();
  const KMime::Message::Ptr rightMessage = rightItem.payload<KMime::Message::Ptr>();

  const KDateTime leftDateTime = leftMessage->date()->dateTime();
  const KDateTime rightDateTime = rightMessage->date()->dateTime();

  // Messages in the same thread are ordered most recent last.
  if (leftDateTime != rightDateTime) {
    return leftDateTime < rightDateTime;
  }

  return leftItem.id() < rightItem.id();
}

static QVector<QByteArray> getRemovableItems(const QHash<QByteArray, QSet<QByteArray> > &container, const QByteArray &value)
{
  QVector<QByteArray> items;

  foreach(const QByteArray &ba, container[value]) {
    items.push_back(ba);
    items += getRemovableItems(container, ba);
  }

  return items;
}


static void addToPending(QHash<QByteArray, QSet<QByteArray> > &pendingThreads, const QByteArray& inReplyTo, const QByteArray& identifier)
{
  QSet<QByteArray> existingResponses = pendingThreads.take(identifier);
  pendingThreads[inReplyTo].insert(identifier);
  pendingThreads[inReplyTo].unite(existingResponses);
}

void ThreadGrouperModelPrivate::populateThreadGrouperModel() const
{
  Q_Q(const ThreadGrouperModel);
  m_threads.clear();
  m_messageMap.clear();

  QHash<QByteArray, QSet<QByteArray> > pendingThreads;

  if (!q->sourceModel())
    return;
  const int rowCount = q->sourceModel()->rowCount();

  for (int row = 0; row < rowCount; ++row) {
    const QModelIndex idx = q->sourceModel()->index(row, 0);
    Q_ASSERT(idx.isValid());
    const Akonadi::Item item = idx.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
    Q_ASSERT(item.isValid());
    Q_ASSERT(item.hasPayload<KMime::Message::Ptr>());
    const KMime::Message::Ptr message = item.payload<KMime::Message::Ptr>();
    const QByteArray identifier = message->messageID()->identifier();
    if (!message->inReplyTo()->isEmpty()) {
      QByteArray _inReplyTo = message->inReplyTo()->as7BitString(false);
      const QByteArray inReplyTo = _inReplyTo.mid(1, _inReplyTo.size() -2 );

      const QHash<QByteArray, QSet<QByteArray> >::const_iterator it = findValue(m_threads, inReplyTo);
      const QHash<QByteArray, QSet<QByteArray> >::const_iterator end = m_threads.constEnd();
      if (it == end) {
        addToPending(pendingThreads, inReplyTo, identifier);
        m_threadItems[identifier] = item;
        m_allItems[identifier] = item;
        m_messageMap[item] = identifier;
        continue;
      }
      m_threadItems.remove(identifier);
      m_threads[it.key()].insert(identifier);
    } else {
      foreach(const QByteArray &ba, getRemovableItems(pendingThreads, identifier)) {
        m_threadItems.remove(ba);
        pendingThreads.remove(ba);
        m_threads[identifier].insert(ba);
      }
      m_allItems[identifier] = item;
      m_threadItems[identifier] = item;
      m_messageMap[item] = identifier;
    }
  }

  QHash<QByteArray, QSet<QByteArray> >::const_iterator pendingIt = pendingThreads.constBegin();
  const QHash<QByteArray, QSet<QByteArray> >::const_iterator pendingEnd = pendingThreads.constEnd();
  for ( ; pendingIt != pendingEnd; ++pendingIt) {
    const QByteArray inReplyTo = pendingIt.key();
    QSet<QByteArray> pendingItems = pendingIt.value();
    if (m_threads.contains(inReplyTo)) {
      foreach(const QByteArray &ba, pendingItems) {
        m_threadItems.remove(ba);
        m_threads[inReplyTo].unite(m_threads[ba]);
        m_threads.remove(ba);
      }
      m_threads[inReplyTo].unite(pendingItems);
    } else {
      foreach(const QByteArray &ba, pendingItems) {
        static const QSet<QByteArray> staticEmptySet;
        if ( !m_threads.contains( ba ) )
          m_threads.insert(ba, staticEmptySet);
      }
    }
  }
}

ThreadGrouperModel::ThreadGrouperModel(QObject* parent)
  : QSortFilterProxyModel(parent), d_ptr(new ThreadGrouperModelPrivate(this))
{
  setDynamicSortFilter(true);
  sort(0, Qt::AscendingOrder);
}

ThreadGrouperModel::~ThreadGrouperModel()
{
  delete d_ptr;
}

void ThreadGrouperModel::setThreadOrder(ThreadGrouperModel::OrderScheme order)
{
  Q_D(ThreadGrouperModel);
  d->m_order = order;
}

ThreadGrouperModel::OrderScheme ThreadGrouperModel::threadOrder() const
{
  Q_D(const ThreadGrouperModel);
  return d->m_order;
}

Akonadi::Item ThreadGrouperModelPrivate::threadRoot(const QModelIndex &index) const
{
  const Akonadi::Item item = index.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
  Q_ASSERT(item.isValid());
  return getThreadItem(item);
}

QVariant ThreadGrouperModel::data(const QModelIndex& index, int role) const
{
  Q_D(const ThreadGrouperModel);
  if (!index.isValid())
    return QVariant();
  if (role == ThreadIdRole) {
    return d->threadRoot(index).id();
  }
  if (role == Qt::DisplayRole) {
    Akonadi::Item item = QSortFilterProxyModel::data(index, Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
    Q_ASSERT(item.isValid());
    Q_ASSERT(item.hasPayload<KMime::Message::Ptr>());
    KMime::Message::Ptr message = item.payload<KMime::Message::Ptr>();
    return  QSortFilterProxyModel::data(index, role).toString() + message->subject()->asUnicodeString() + " - " + message->date()->asUnicodeString();
  }

  return QSortFilterProxyModel::data(index, role);
}

void ThreadGrouperModel::setSourceModel(QAbstractItemModel* sourceModel)
{
  Q_D(ThreadGrouperModel);
  d->populateThreadGrouperModel();
  QSortFilterProxyModel::setSourceModel(sourceModel);

  disconnect(sourceModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
      this, SIGNAL(_q_sourceRowsAboutToBeInserted(QModelIndex,int,int)));

  disconnect(sourceModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
      this, SLOT(_q_sourceRowsInserted(QModelIndex,int,int)));

  disconnect(sourceModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
      this, SLOT(_q_sourceRowsAboutToBeRemoved(QModelIndex,int,int)));

  disconnect(sourceModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
      this, SLOT(_q_sourceRowsRemoved(QModelIndex,int,int)));

  disconnect(sourceModel, SIGNAL(layoutAboutToBeChanged()),
      this, SLOT(_q_sourceLayoutAboutToBeChanged()));

  disconnect(sourceModel, SIGNAL(layoutChanged()),
      this, SLOT(_q_sourceLayoutChanged()));

  connect(sourceModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
      this, SLOT(_q_sourceAboutToBeReset()));

  connect(sourceModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
      this, SLOT(populateThreadGrouperModel()));

  connect(sourceModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
      this, SLOT(_q_sourceReset()));

  connect(sourceModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
      this, SLOT(_q_sourceAboutToBeReset()));

  connect(sourceModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
      this, SLOT(populateThreadGrouperModel()));

  connect(sourceModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
      this, SLOT(_q_sourceReset()));

  connect(sourceModel, SIGNAL(layoutAboutToBeChanged()),
      this, SLOT(_q_sourceAboutToBeReset()));

  connect(sourceModel, SIGNAL(layoutChanged()),
      this, SLOT(populateThreadGrouperModel()));

  connect(sourceModel, SIGNAL(layoutChanged()),
      this, SLOT(_q_sourceReset()));

}

bool ThreadGrouperModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
  Q_D(const ThreadGrouperModel);
  static ItemLessThanComparator lt(d);
  const Akonadi::Item leftItem = left.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
  const Akonadi::Item rightItem = right.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
  return lt(leftItem, rightItem);
}

#include "threadgroupermodel.moc"
