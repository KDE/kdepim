/*
    Copyright (c) 2009 Stephen Kelly <steveire@gmail.com>
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

#include "kdescendantsproxymodel_p.h"

#include <QtCore/QStringList>
#include <QtCore/QTimer>

#include "kdebug.h"

#define KDO(object) kDebug() << #object << object

#include "kbihash_p.h"

typedef KBiHash<QPersistentModelIndex, int> Mapping;

class KDescendantsProxyModelPrivate
{
  KDescendantsProxyModelPrivate(KDescendantsProxyModel * qq)
    : q_ptr(qq),
      m_rowCount(0),
      m_ignoreNextLayoutAboutToBeChanged(false),
      m_ignoreNextLayoutChanged(false),
      m_relayouting(false),
      m_displayAncestorData( false ),
      m_ancestorSeparator( QLatin1String( " / " ) )
  {
  }

  Q_DECLARE_PUBLIC(KDescendantsProxyModel)
  KDescendantsProxyModel * const q_ptr;

  mutable QVector<QPersistentModelIndex> m_pendingParents;

  void scheduleProcessPendingParents() const;
  void processPendingParents();

  QVector<QPersistentModelIndex> getParentItems(const QModelIndex &parent) const;
  void synchronousMappingRefresh();

  void updateInternalIndexes(int start, int offset);

  void resetInternalData();

  void sourceRowsAboutToBeInserted(const QModelIndex &, int, int);
  void sourceRowsInserted(const QModelIndex &, int, int);
  void sourceRowsAboutToBeRemoved(const QModelIndex &, int, int);
  void sourceRowsRemoved(const QModelIndex &, int, int);
  void sourceRowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int);
  void sourceRowsMoved(const QModelIndex &, int, int, const QModelIndex &, int);
  void sourceModelAboutToBeReset();
  void sourceModelReset();
  void sourceLayoutAboutToBeChanged();
  void sourceLayoutChanged();
  void sourceDataChanged(const QModelIndex &, const QModelIndex &);
  void sourceModelDestroyed();

  Mapping m_mapping;
  int m_rowCount;
  QPair<int, int> m_removePair;
  QPair<int, int> m_insertPair;

  bool m_ignoreNextLayoutAboutToBeChanged;
  bool m_ignoreNextLayoutChanged;
  bool m_relayouting;

  bool m_displayAncestorData;
  QString m_ancestorSeparator;

  QList<QPersistentModelIndex> m_layoutChangePersistentIndexes;
  QModelIndexList m_proxyIndexes;
};

void KDescendantsProxyModelPrivate::resetInternalData()
{
  m_rowCount = 0;
  m_mapping.clear();
  m_layoutChangePersistentIndexes.clear();
  m_proxyIndexes.clear();
}

QVector<QPersistentModelIndex> KDescendantsProxyModelPrivate::getParentItems(const QModelIndex& parent) const
{
  Q_Q(const KDescendantsProxyModel);
  const int rowCount = q->sourceModel()->rowCount(parent);
  QVector<QPersistentModelIndex> list;
  for (int row = 0; row < rowCount; ++row)
  {
    static const int column = 0;
    const QModelIndex idx = q->sourceModel()->index(row, column, parent);
    if (q->sourceModel()->hasChildren(idx))
      list << QPersistentModelIndex(idx);
  }
  return list;
}

void KDescendantsProxyModelPrivate::synchronousMappingRefresh()
{
  m_rowCount = 0;
  m_mapping.clear();
  m_pendingParents.clear();

  m_pendingParents.append(QModelIndex());

  m_relayouting = true;
  while (!m_pendingParents.isEmpty())
  {
    processPendingParents();
  }
  m_relayouting = false;
}

void KDescendantsProxyModelPrivate::scheduleProcessPendingParents() const
{
  Q_Q(const KDescendantsProxyModel);
  const_cast<KDescendantsProxyModelPrivate*>(this)->processPendingParents();
}

void KDescendantsProxyModelPrivate::processPendingParents()
{
  Q_Q(KDescendantsProxyModel);
  const QVector<QPersistentModelIndex>::iterator begin = m_pendingParents.begin();
  QVector<QPersistentModelIndex>::iterator it = begin;

  // Process chunkSize elements per invokation.
  static const int chunkSize = 30;

  const QVector<QPersistentModelIndex>::iterator end =
          /* (m_pendingParents.size() > chunkSize) ? begin + chunkSize : */ m_pendingParents.end();

  QVector<QPersistentModelIndex> newPendingParents;

  while (it != end && it != m_pendingParents.end()) {
    const QModelIndex sourceParent = *it;
    if (!sourceParent.isValid() && m_rowCount > 0)
    {
      // It was removed from the source model before it was inserted.
      it = m_pendingParents.erase(it);
      continue;
    }
    const int rowCount = q->sourceModel()->rowCount(sourceParent);

    Q_ASSERT(rowCount > 0);
    const QPersistentModelIndex sourceIndex = q->sourceModel()->index(rowCount - 1, 0, sourceParent);

    Q_ASSERT(sourceIndex.isValid());

    const QModelIndex proxyParent = q->mapFromSource(sourceParent);

    Q_ASSERT(sourceParent.isValid() == proxyParent.isValid());
    const int proxyEndRow = proxyParent.row() + rowCount;
    const int proxyStartRow = proxyEndRow - rowCount + 1;

    if (!m_relayouting)
      q->beginInsertRows(QModelIndex(), proxyStartRow, proxyEndRow);

    updateInternalIndexes(proxyStartRow, rowCount);
    m_mapping.insert(sourceIndex, proxyEndRow);
    it = m_pendingParents.erase(it);
    m_rowCount += rowCount;

    if (!m_relayouting)
      q->endInsertRows();

    for (int sourceRow = 0; sourceRow < rowCount; ++sourceRow ) {
      static const int column = 0;
      const QModelIndex child = q->sourceModel()->index(sourceRow, column, sourceParent);
      Q_ASSERT(child.isValid());

      if (q->sourceModel()->hasChildren(child))
        newPendingParents.append(child);
    }
  }
  m_pendingParents += newPendingParents;
  if (!m_pendingParents.isEmpty())
      processPendingParents();
//   scheduleProcessPendingParents();
}

void KDescendantsProxyModelPrivate::updateInternalIndexes(int start, int offset)
{
  Q_ASSERT(start + offset >= 0);

  {
    Mapping::left_iterator it = m_mapping.leftBegin();

    for ( ; it != m_mapping.leftEnd(); ++it) {
      const int proxyRow = it.value();
      Q_ASSERT(proxyRow >= 0);

      if (proxyRow < start)
        continue;

      Q_ASSERT(proxyRow + offset >= 0);
      m_mapping.updateRight(it, proxyRow + offset);
    }
  }
}

KDescendantsProxyModel::KDescendantsProxyModel(QObject *parent)
  : QAbstractProxyModel(parent), d_ptr(new KDescendantsProxyModelPrivate(this))
{
}

KDescendantsProxyModel::~KDescendantsProxyModel()
{
  delete d_ptr;
}

void KDescendantsProxyModel::setRootIndex(const QModelIndex &index)
{
  Q_UNUSED(index)
}

QModelIndexList KDescendantsProxyModel::match(const QModelIndex &start, int role, const QVariant &value, int hits, Qt::MatchFlags flags) const
{
  return QAbstractProxyModel::match(start, role, value, hits, flags);
}

void KDescendantsProxyModel::setDisplayAncestorData( bool display )
{
  Q_D(KDescendantsProxyModel);
  d->m_displayAncestorData = display;
}

bool KDescendantsProxyModel::displayAncestorData() const
{
  Q_D(const KDescendantsProxyModel );
  return d->m_displayAncestorData;
}

void KDescendantsProxyModel::setAncestorSeparator( const QString &separator )
{
  Q_D(KDescendantsProxyModel);
  d->m_ancestorSeparator = separator;
}

QString KDescendantsProxyModel::ancestorSeparator() const
{
  Q_D(const KDescendantsProxyModel );
  return d->m_ancestorSeparator;
}


void KDescendantsProxyModel::setSourceModel(QAbstractItemModel *_sourceModel)
{
  beginResetModel();

  if (_sourceModel) {
    disconnect(_sourceModel, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)),
               this, SLOT(sourceRowsAboutToBeInserted(const QModelIndex &, int, int)));
    disconnect(_sourceModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
               this, SLOT(sourceRowsInserted(const QModelIndex &, int, int)));
    disconnect(_sourceModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
               this, SLOT(sourceRowsAboutToBeRemoved(const QModelIndex &, int, int)));
    disconnect(_sourceModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
               this, SLOT(sourceRowsRemoved(const QModelIndex &, int, int)));
//     disconnect(_sourceModel, SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
//             this, SLOT(sourceRowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
//     disconnect(_sourceModel, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
//             this, SLOT(sourceRowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
    disconnect(_sourceModel, SIGNAL(modelAboutToBeReset()),
               this, SLOT(sourceModelAboutToBeReset()));
    disconnect(_sourceModel, SIGNAL(modelReset()),
               this, SLOT(sourceModelReset()));
    disconnect(_sourceModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
               this, SLOT(sourceDataChanged(const QModelIndex &, const QModelIndex &)));
    disconnect(_sourceModel, SIGNAL(layoutAboutToBeChanged()),
               this, SLOT(sourceLayoutAboutToBeChanged()));
    disconnect(_sourceModel, SIGNAL(layoutChanged()),
               this, SLOT(sourceLayoutChanged()));
    disconnect(_sourceModel, SIGNAL(destroyed()),
               this, SLOT(sourceModelDestroyed()));
  }

  QAbstractProxyModel::setSourceModel(_sourceModel);

  if (_sourceModel) {
    connect(_sourceModel, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)),
            SLOT(sourceRowsAboutToBeInserted(const QModelIndex &, int, int)));
    connect(_sourceModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
            SLOT(sourceRowsInserted(const QModelIndex &, int, int)));
    connect(_sourceModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
            SLOT(sourceRowsAboutToBeRemoved(const QModelIndex &, int, int)));
    connect(_sourceModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
            SLOT(sourceRowsRemoved(const QModelIndex &, int, int)));
//     connect(_sourceModel, SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
//             SLOT(sourceRowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
//     connect(_sourceModel, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
//             SLOT(sourceRowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
    connect(_sourceModel, SIGNAL(modelAboutToBeReset()),
            SLOT(sourceModelAboutToBeReset()));
    connect(_sourceModel, SIGNAL(modelReset()),
            SLOT(sourceModelReset()));
    connect(_sourceModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
            SLOT(sourceDataChanged(const QModelIndex &, const QModelIndex &)));
    connect(_sourceModel, SIGNAL(layoutAboutToBeChanged()),
            SLOT(sourceLayoutAboutToBeChanged()));
    connect(_sourceModel, SIGNAL(layoutChanged()),
            SLOT(sourceLayoutChanged()));
    connect(_sourceModel, SIGNAL(destroyed()),
            SLOT(sourceModelDestroyed()));
  }

  endResetModel();
}

QModelIndex KDescendantsProxyModel::parent(const QModelIndex &index) const
{
  Q_UNUSED(index)
  return QModelIndex();
}

bool KDescendantsProxyModel::hasChildren(const QModelIndex &parent) const
{
  Q_D(const KDescendantsProxyModel);
  return !(d->m_mapping.isEmpty() || parent.isValid());
}

int KDescendantsProxyModel::rowCount(const QModelIndex &parent) const
{
  Q_D(const KDescendantsProxyModel);
  if (d->m_pendingParents.contains(parent) || parent.isValid() || !sourceModel())
    return 0;

  if (d->m_mapping.isEmpty() && sourceModel()->hasChildren())
  {
    const_cast<KDescendantsProxyModelPrivate*>(d)->synchronousMappingRefresh();
  }
  return d->m_rowCount;
}

QModelIndex KDescendantsProxyModel::index(int row, int column, const QModelIndex &parent) const
{
  if (parent.isValid())
    return QModelIndex();

  if (!hasIndex(row, column, parent))
    return QModelIndex();

  return createIndex(row, column);
}

QModelIndex KDescendantsProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
  Q_D(const KDescendantsProxyModel);
  if (d->m_mapping.isEmpty() || !proxyIndex.isValid() || !sourceModel())
    return QModelIndex();

  Mapping::left_const_iterator it = d->m_mapping.leftConstBegin();
  const Mapping::left_const_iterator end = d->m_mapping.leftConstEnd();

  Mapping::left_const_iterator result = end;
  for ( ; it != end; ++it) {
    if (it.value() == proxyIndex.row()) {
      Q_ASSERT(it.key().isValid());
      return it.key().sibling(it.key().row(), proxyIndex.column());
    } else if (it.value() > proxyIndex.row()) {
      if (result == end || it.value() < result.value())
        result = it;
    }
  }
  Q_ASSERT(result != d->m_mapping.leftEnd());

  const int proxyLastRow = result.value();
  const QModelIndex sourceLastChild = result.key();
  Q_ASSERT(sourceLastChild.isValid());

  // proxyLastRow is greater than proxyIndex.row().
  // sourceLastChild is vertically below the result we're looking for
  // and not necessarily in the correct parent.
  // We travel up through its parent hierarchy until we are in the
  // right parent, then return the correct sibling.

  // Source:           Proxy:    Row
  // - A               - A       - 0
  // - B               - B       - 1
  // - C               - C       - 2
  // - D               - D       - 3
  // - - E             - E       - 4
  // - - F             - F       - 5
  // - - G             - G       - 6
  // - - H             - H       - 7
  // - - I             - I       - 8
  // - - - J           - J       - 9
  // - - - K           - K       - 10
  // - - - L           - L       - 11
  // - - M             - M       - 12
  // - - N             - N       - 13
  // - O               - O       - 14

  // Note that L, N and O are lastChildIndexes, and therefore have a mapping. If we
  // are trying to map G from the proxy to the source, We at this point have an iterator
  // pointing to (L -> 11). The proxy row of G is 6. (proxyIndex.row() == 6). We seek the
  // sourceIndex which is vertically above L by the distance proxyLastRow - proxyIndex.row().
  // In this case the verticalDistance is 5.

  int verticalDistance = proxyLastRow - proxyIndex.row();

  // We traverse the ancestors of L, until we can index the desired row in the source.

  QModelIndex ancestor = sourceLastChild;
  while (ancestor.isValid())
  {
    const int ancestorRow = ancestor.row();
    if (verticalDistance <= ancestorRow)
    {
      return ancestor.sibling(ancestorRow - verticalDistance, proxyIndex.column());
    }
    verticalDistance -= (ancestorRow + 1);
    ancestor = ancestor.parent();
  }
  Q_ASSERT(!"Didn't find target row.");
  return QModelIndex();
}

QModelIndex KDescendantsProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
  Q_D(const KDescendantsProxyModel);

  if (!sourceModel())
    return QModelIndex();

  if (d->m_mapping.isEmpty())
    return QModelIndex();


  {
    // Can do this faster with KHash2Map.

    Mapping::left_const_iterator it = d->m_mapping.leftConstBegin();
    const Mapping::left_const_iterator end = d->m_mapping.leftConstEnd();
    const QModelIndex sourceParent = sourceIndex.parent();
    Mapping::left_const_iterator result = end;

    for ( ; it != end; ++it )
    {
      QModelIndex index = it.key();
      while (index.isValid())
      {
        const QModelIndex ancestor = index.parent();
        if (ancestor == sourceParent)
        {
          if (result == end || (*it < *result && index.row() >= sourceIndex.row()))
          {
            result = it;
          }
        }
        index = ancestor;
      }
    }
    Q_ASSERT(result != end);
    const QModelIndex sourceLastChild = result.key();
    int proxyRow = result.value();
    QModelIndex index = sourceLastChild;
    while (index.isValid())
    {
      const QModelIndex ancestor = index.parent();
      if (ancestor == sourceParent)
      {
        return createIndex(proxyRow - (index.row() - sourceIndex.row()), sourceIndex.column());
      }
      proxyRow -= (ancestor.row() + 1);
      index = ancestor;
    }
    Q_ASSERT(!"Didn't find valid proxy mapping.");
    return QModelIndex();
  }

}

int KDescendantsProxyModel::columnCount(const QModelIndex &parent) const
{
  if (parent.isValid() /* || rowCount(parent) == 0 */ || !sourceModel())
    return 0;

  return sourceModel()->columnCount();
}

QVariant KDescendantsProxyModel::data(const QModelIndex &index, int role) const
{
  if (!sourceModel())
    return QVariant();
  return sourceModel()->data(mapToSource(index), role);
}

Qt::ItemFlags KDescendantsProxyModel::flags(const QModelIndex &index) const
{
  if (!index.isValid() || !sourceModel())
    return QAbstractProxyModel::flags(index);

  const QModelIndex srcIndex = mapToSource(index);
  Q_ASSERT(srcIndex.isValid());
  return sourceModel()->flags(srcIndex);
}

void KDescendantsProxyModelPrivate::sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
  Q_Q(KDescendantsProxyModel);

  if (!q->sourceModel()->hasChildren(parent))
  {
    // parent was not a parent before.
    return;
  }

  int proxyStart = -1;
  if (q->sourceModel()->hasChildren(parent) && start > 0)
  {
    const QModelIndex aboveStart = q->sourceModel()->index(start - 1, 0, parent);
    proxyStart = q->mapFromSource(aboveStart).row() + 1;
  } else {
    proxyStart = q->mapFromSource(parent).row() + 1;
  }
  const int proxyEnd = proxyStart + (end - start);

  m_insertPair = qMakePair(proxyStart, proxyEnd);
  q->beginInsertRows(QModelIndex(), proxyStart, proxyEnd);
}

void KDescendantsProxyModelPrivate::sourceRowsInserted(const QModelIndex &parent, int start, int end)
{
  Q_Q(KDescendantsProxyModel);

  const QModelIndex sourceStart = q->sourceModel()->index(start, 0, parent);
  Q_ASSERT(sourceStart.isValid());

  const int rowCount = q->sourceModel()->rowCount(parent);
  Q_ASSERT(rowCount > 0);

  const int difference = end - start + 1;

  if (rowCount == difference)
  {
    // @p parent was not a parent before.
    m_pendingParents.append(parent);
    scheduleProcessPendingParents();
    return;
  }

  const int proxyStart = m_insertPair.first;

  Q_ASSERT(proxyStart >= 0);

  updateInternalIndexes(proxyStart, difference);

  if (rowCount - 1 == end)
  {
    // The previously last row (the mapped one) is no longer the last.
    // For example,

    // - A            - A           0
    // - - B          - B           1
    // - - C          - C           2
    // - - - D        - D           3
    // - - - E   ->   - E           4
    // - - F          - F           5
    // - - G     ->   - G           6
    // - H            - H           7
    // - I       ->   - I           8

    // As last children, E, F and G have mappings.
    // Consider that 'J' is appended to the children of 'C', below 'E'.

    // - A            - A           0
    // - - B          - B           1
    // - - C          - C           2
    // - - - D        - D           3
    // - - - E   ->   - E           4
    // - - - J        - ???         5
    // - - F          - F           6
    // - - G     ->   - G           7
    // - H            - H           8
    // - I       ->   - I           9

    // The updateInternalIndexes call above will have updated the F and G mappings correctly because proxyStart is 5.
    // That means that E -> 4 was not affected by the updateInternalIndexes call.
    // Now the mapping for E -> 4 needs to be updated so that it's a mapping for J -> 5.

    Q_ASSERT(!m_mapping.isEmpty());
    static const int column = 0;
    const QModelIndex oldIndex = q->sourceModel()->index(rowCount - 1 - difference, column, parent);
    Q_ASSERT(m_mapping.leftContains(oldIndex));

    // oldIndex is E in the source. proxyRow is 4.
    const int proxyRow = m_mapping.takeLeft(oldIndex);
    const QModelIndex newIndex = q->sourceModel()->index(rowCount - 1, column, parent);

    // newIndex is J. (proxyRow + difference) is 5.
    m_mapping.insert(newIndex, proxyRow + difference);
  }

  for (int row = start; row <= end; ++row)
  {
    static const int column = 0;
    const QModelIndex idx = q->sourceModel()->index(row, column, parent);
    Q_ASSERT(idx.isValid());
    if (q->sourceModel()->hasChildren(idx))
      m_pendingParents.append(idx);
  }

  m_rowCount += difference;

  scheduleProcessPendingParents();
  q->endInsertRows();
}

void KDescendantsProxyModelPrivate::sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
  Q_Q(KDescendantsProxyModel);

  const int proxyStart = q->mapFromSource(q->sourceModel()->index(start, 0, parent)).row();
  const int proxyEnd = q->mapFromSource(q->sourceModel()->index(end, 0, parent)).row();

  m_removePair = qMakePair(proxyStart, proxyEnd);

  q->beginRemoveRows(QModelIndex(), proxyStart, proxyEnd);
}

void KDescendantsProxyModelPrivate::sourceRowsRemoved(const QModelIndex &parent, int start, int end)
{
  Q_Q(KDescendantsProxyModel);
  Q_UNUSED(end)

  const int rowCount = q->sourceModel()->rowCount(parent);

  Mapping::left_iterator it = m_mapping.leftBegin();

  while (it != m_mapping.leftEnd())
  {
    if (!it.key().isValid())
      it = m_mapping.eraseLeft(it);
    else
      ++it;
  }

  const int proxyStart = m_removePair.first;
  const int proxyEnd = m_removePair.second;

  const int difference = proxyEnd - proxyStart + 1;
  m_removePair = qMakePair(-1, -1);
  m_rowCount -= difference;
  Q_ASSERT(m_rowCount >= 0);

  updateInternalIndexes(proxyStart, -1 * difference);

  if (rowCount == start && rowCount != 0)
  {
    static const int column = 0;
    const QModelIndex newIndex = q->sourceModel()->index(rowCount - 1, column, parent);
    m_mapping.insert(newIndex, proxyStart - 1);
  }

  q->endRemoveRows();
}

void KDescendantsProxyModelPrivate::sourceRowsAboutToBeMoved(const QModelIndex &srcParent, int srcStart, int srcEnd, const QModelIndex &destParent, int destStart)
{
  Q_UNUSED(srcParent)
  Q_UNUSED(srcStart)
  Q_UNUSED(srcEnd)
  Q_UNUSED(destParent)
  Q_UNUSED(destStart)
  Q_Q(KDescendantsProxyModel);
  q->beginResetModel();
}

void KDescendantsProxyModelPrivate::sourceRowsMoved(const QModelIndex &srcParent, int srcStart, int srcEnd, const QModelIndex &destParent, int destStart)
{
  Q_UNUSED(srcParent)
  Q_UNUSED(srcStart)
  Q_UNUSED(srcEnd)
  Q_UNUSED(destParent)
  Q_UNUSED(destStart)
  Q_Q(KDescendantsProxyModel);
  resetInternalData();
  q->endResetModel();
}

void KDescendantsProxyModelPrivate::sourceModelAboutToBeReset()
{
  Q_Q(KDescendantsProxyModel);
  q->beginResetModel();
}

void KDescendantsProxyModelPrivate::sourceModelReset()
{
  Q_Q(KDescendantsProxyModel);
  resetInternalData();
  if (q->sourceModel()->hasChildren())
  {
    m_pendingParents.append(QModelIndex());
    scheduleProcessPendingParents();
  }
  q->endResetModel();
}

void KDescendantsProxyModelPrivate::sourceLayoutAboutToBeChanged()
{
  Q_Q(KDescendantsProxyModel);

  if (m_ignoreNextLayoutChanged) {
      m_ignoreNextLayoutChanged = false;
      return;
  }

  if (m_mapping.isEmpty())
    return;

  QPersistentModelIndex srcPersistentIndex;
  foreach(const QPersistentModelIndex &proxyPersistentIndex, q->persistentIndexList()) {
      m_proxyIndexes << proxyPersistentIndex;
      Q_ASSERT(proxyPersistentIndex.isValid());
      srcPersistentIndex = q->mapToSource(proxyPersistentIndex);
      Q_ASSERT(srcPersistentIndex.isValid());
      m_layoutChangePersistentIndexes << srcPersistentIndex;
  }

  q->layoutAboutToBeChanged();
}

void KDescendantsProxyModelPrivate::sourceLayoutChanged()
{
  Q_Q(KDescendantsProxyModel);

  if (m_ignoreNextLayoutAboutToBeChanged) {
      m_ignoreNextLayoutAboutToBeChanged = false;
      return;
  }

  if (m_mapping.isEmpty())
    return;

  m_rowCount = 0;

  synchronousMappingRefresh();

  for (int i = 0; i < m_proxyIndexes.size(); ++i) {
      q->changePersistentIndex(m_proxyIndexes.at(i), q->mapFromSource(m_layoutChangePersistentIndexes.at(i)));
  }

  m_layoutChangePersistentIndexes.clear();
  m_proxyIndexes.clear();

  q->layoutChanged();
}

void KDescendantsProxyModelPrivate::sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
  Q_Q(KDescendantsProxyModel);

  const int topRow = topLeft.row();
  const int bottomRow = bottomRight.row();

  for(int i = topRow; i <= bottomRow; ++i)
  {
    const QModelIndex sourceTopLeft = q->sourceModel()->index(i, topLeft.column(), topLeft.parent());
    const QModelIndex proxyTopLeft = q->mapFromSource(sourceTopLeft);
    // TODO. If an index does not have any descendants, then we can emit in blocks of rows.
    // As it is we emit once for each row.
    const QModelIndex sourceBottomRight = q->sourceModel()->index(i, bottomRight.column(), bottomRight.parent());
    const QModelIndex proxyBottomRight = q->mapFromSource(sourceBottomRight);
    emit q->dataChanged(proxyTopLeft, proxyBottomRight);
  }
}

void KDescendantsProxyModelPrivate::sourceModelDestroyed()
{
  Q_Q(KDescendantsProxyModel);
  resetInternalData();
  q->endResetModel();
}

QMimeData* KDescendantsProxyModel::mimeData( const QModelIndexList & indexes ) const
{
  Q_ASSERT(sourceModel());
  QModelIndexList sourceIndexes;
  foreach(const QModelIndex& index, indexes)
    sourceIndexes << mapToSource(index);
  return sourceModel()->mimeData(sourceIndexes);
}

QStringList KDescendantsProxyModel::mimeTypes() const
{
  Q_ASSERT(sourceModel());
  return sourceModel()->mimeTypes();
}

Qt::DropActions KDescendantsProxyModel::supportedDropActions() const
{
  Q_ASSERT(sourceModel());
  return sourceModel()->supportedDropActions();
}

#include "moc_kdescendantsproxymodel_p.cpp"
