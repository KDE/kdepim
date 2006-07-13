/*
 * foldermodel.h
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

#ifndef FOLDERMODEL_H
#define FOLDERMODEL_H

#include <QVariant>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QStringList>

#include "conversation.h"

class FolderModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  /**
   * Constructs an empty FolderModel.
   * @param listOfMe is a set of aliases for the user. This model takes ownership of this parameter.
   */
  FolderModel(QStringList *listOfMe, QObject *parent = 0) : QAbstractItemModel(parent), m_me(listOfMe) {}
  ~FolderModel() { delete m_me; }

  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  Conversation* conversation(int conversationId) const;
  QModelIndex index(int row, int column, const QModelIndex &/*parent*/) const { return createIndex(row, column, row); }
  QModelIndex parent(const QModelIndex &/*parent*/) const { return QModelIndex(); }

private:
  QList<Conversation*> m_conversations;
  QStringList *m_me;
};

#endif
