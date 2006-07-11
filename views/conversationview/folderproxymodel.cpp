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

int FolderProxyModel::rowCount(const QModelIndex &/*parent*/) const
{
  return sourceModel->rowCount();
}

int FolderProxyModel::columnCount(const QModelIndex &/*parent*/) const
{
  return 2;
}

QVariant FolderProxyModel::data(const QModelIndex &index, int role) const
{
  return sourceModel->data(index, role);
}

QVariant FolderProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  return sourceModel->headerData(section, orientation, role);
}

Conversation* FolderProxyModel::conversation(const QModelIndex &index) const
{
  return sourceModel->conversation(mapToSource(index).row());
}

void FolderProxyModel::toggleFilterUnread()
{
  qDebug() << "Proxy reached!";
  filterUnread = !filterUnread;
  qDebug() << "filterUnread: " << filterUnread;
  clear();
}

bool FolderProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
  qDebug() << "row: " << sourceRow << ", filterUnreadStatus: " << (filterUnread ? sourceModel->conversation(sourceRow)->isUnread() : true);
  return (sourceModel->conversation(sourceRow)->authors().contains(filterRegExp())
    || sourceModel->conversation(sourceRow)->conversationTitle().contains(filterRegExp()))
    && (filterUnread ? sourceModel->conversation(sourceRow)->isUnread() : true);
}

#include "folderproxymodel.moc"
