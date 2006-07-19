/*
 * folderproxymodel.cpp
 *
 * copyright (c) Aron Bostrom <Aron.Bostrom at gmail.com>, 2006 
 *
 * this library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <QtDebug>

#include "folderproxymodel.h"

FolderProxyModel::FolderProxyModel(FolderModel *model, QObject *parent) : QSortFilterProxyModel(parent), m_model(model)
{
  setSourceModel(m_model);
  m_filterUnread = false;
  QObject::connect(m_model, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(resort()));
  QObject::connect(m_model, SIGNAL(layoutChanged()), this, SLOT(resort()));
  m_resortable = false;
  m_header = 0;
}

void FolderProxyModel::setHeader(QHeaderView *header)
{
  m_header = header;
}

bool FolderProxyModel::resortable() const
{
  return m_resortable;
}

void FolderProxyModel::setResortable(bool enable)
{
  m_resortable = enable;
}

int FolderProxyModel::rowCount(const QModelIndex &/*parent*/) const
{
  return m_model->rowCount();
}

int FolderProxyModel::columnCount(const QModelIndex &/*parent*/) const
{
  return 2;
}

QVariant FolderProxyModel::data(const QModelIndex &index, int role) const
{
  return m_model->data(index, role);
}

QVariant FolderProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  return m_model->headerData(section, orientation, role);
}

Conversation* FolderProxyModel::conversation(const QModelIndex &index) const
{
  return m_model->conversation(mapToSource(index).row());
}

void FolderProxyModel::toggleFilterUnread()
{
  m_filterUnread = !m_filterUnread;
//  clear();
  filterChanged();
}

bool FolderProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
{
  Conversation *c = m_model->conversation(sourceRow);
  return (c->authors().contains(m_filter) || c->subject().contains(m_filter))
    && (m_filterUnread ? c->isUnread() : true);
}

void FolderProxyModel::setFilter( const QString &filter)
{
  m_filter = QRegExp(filter, Qt::CaseInsensitive, QRegExp::FixedString);
}

void FolderProxyModel::resort()
{
//   filterChanged();
  if (m_resortable && m_header != 0)
    sort(m_header->sortIndicatorSection(), m_header->sortIndicatorOrder());
}

void FolderProxyModel:: markConversationAsRead(const QModelIndex& index, bool read)
{
  m_model->markConversationAsRead(mapToSource(index), read);
}

#include "folderproxymodel.moc"
