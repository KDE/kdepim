/*
 * foldermodel.cpp
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

#include <klocale.h>

#include "foldermodel.h"

FolderModel::~FolderModel() 
{ 
  Conversation *tmp;
  foreach (tmp, m_conversations) {
    delete tmp;
  }
  delete m_me; 
}

int FolderModel::rowCount(const QModelIndex &/*parent*/) const
{
  return m_conversations.count();
}

int FolderModel::columnCount(const QModelIndex &/*parent*/) const
{
  return 2;
}

QVariant FolderModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  if (index.row() < 0 || index.row() >= rowCount())
    return QVariant();

  if (role == Qt::DisplayRole) {
    switch (index.column()) {
      case 0: return m_conversations.at(index.row())->authors(); break;
      case 1: return m_conversations.at(index.row())->arrivalTime(); break;
      case 2: return m_conversations.at(index.row())->subject(); break;
      case 3: return m_conversations.at(index.row())->count(); break;
      case 4: return m_conversations.at(index.row())->snippet(); break;
      case 5: return m_conversations.at(index.row())->isUnread(); break;
      case 6: return m_conversations.at(index.row())->arrivalTimeInText(); break;
      default: return QVariant();
    }
  } else
    return QVariant();
}

QVariant FolderModel::headerData(int section, Qt::Orientation /*orientation*/, int role) const
{
  if (role != Qt::DisplayRole)
    return QVariant();

  switch (section) {
    case 0: return QString(i18n("Authors")); break;
    case 1: return QString(i18n("Conversation")); break;
    case 2: return QString(i18n("Time")); break;
    case 3: return QString(i18n("Snippets")); break;
    case 4: return QString(i18n("E-mails")); break;
    default: return QVariant();
  }
}

Conversation* FolderModel::conversation(int conversationId) const
{
  return m_conversations.at(conversationId);
}

void FolderModel::insertMessage(Message *m)
{
  Conversation *c = findConversation(m);
  if (c == 0) {
    emit layoutAboutToBeChanged();
    m_conversations.push_front(new Conversation(m_me, m));
    insertRows(0, 1);
    emit layoutChanged();
  } else {
    c->addMessage(m);
    int row = m_conversations.indexOf(c);
    emit dataChanged(createIndex(row, 0, row), createIndex(row, 1, row));
  }
  qDebug() << rowCount() << " conversations in folder";
}

Conversation* FolderModel::findConversation(const Message *m) const
{
  Conversation *tmp;
  foreach (tmp, m_conversations) {
    if (tmp->isRelated(m))
      return tmp;
  }
  return 0;
}

bool FolderModel::insertRows(int /*row*/, int /*count*/, const QModelIndex& parent)
{
  emit beginInsertRows(parent, 0, 0);
  emit endInsertRows();
  return true;
}

void FolderModel::markConversationAsRead(int id, bool read)
{
  m_conversations.at(id)->markAs(read);
  emit dataChanged(createIndex(id, 0, id), createIndex(id, 2, id));
}

QModelIndex FolderModel::index(int row, int column, const QModelIndex &/*parent*/) const 
{ 
  return createIndex(row, column, row); 
}

QModelIndex FolderModel::parent(const QModelIndex &/*parent*/) const 
{ 
  return QModelIndex(); 
}

#include "foldermodel.moc"
