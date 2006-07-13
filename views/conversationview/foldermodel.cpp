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

#include <klocale.h>

#include "foldermodel.h"


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
      case 2: return m_conversations.at(index.row())->conversationTitle(); break;
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
  m_conversations << new Conversation(m_me, m);
}

#include "foldermodel.moc"
