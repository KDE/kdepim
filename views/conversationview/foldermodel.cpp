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

#include "foldermodel.h"


int FolderModel::rowCount(const QModelIndex &/*parent*/) const
{
  return backend->conversationCount();
}

QVariant FolderModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  if (index.row() < 0 || index.row() >= backend->conversationCount())
    return QVariant();

  if (index.row() < 0 || index.row() >= backend->conversationCount())
    return QVariant();

  if (role == Qt::DisplayRole) {
    switch (index.column()) {
      case 0: return backend->authorsInConversation(index.row()); break;
      case 1: return backend->subjectOfConversation(index.row()); break;
      case 2: return backend->previewOfConversation(index.row()); break;
      case 3: return backend->nbrOfMessagesInConversation(index.row()); break;
      case 4: return backend->dateTimeOfConversation(index.row()); break;
      case 5: return backend->unreadStatusOfConversation(index.row()); break;
      case 6: return backend->dateTimeOfConversationInText(index.row()); break;
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
    case 0: return QString(tr("Authors")); break;
    case 1: return QString(tr("Subject")); break;
    case 2: return QString(tr("Snippet")); break;
    case 3: return QString(tr("E-mails")); break;
    case 4: return QString(tr("Time")); break;
    default: return QVariant();
  }
}

DummyKonadiConversation* FolderModel::conversation(int conversationId) const
{
  return backend->conversation(conversationId);
}
