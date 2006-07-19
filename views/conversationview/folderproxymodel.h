/*
 * folderproxymodel.h
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

#ifndef FOLDERPROXYMODEL_H
#define FOLDERPROXYMODEL_H

#include <QVariant>
#include <QModelIndex>
#include <QSortFilterProxyModel>
#include <QStringList>
#include <QRegExp>
#include <QHeaderView>

#include "dummykonadiadapter.h"
#include "conversation.h"
#include "foldermodel.h"

class FolderProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT
public:
  FolderProxyModel(FolderModel *model, QObject *parent = 0);

  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  Conversation* conversation(const QModelIndex &index) const;
  void setFilter(const QString &filter);
  void setHeader(QHeaderView *header);
  bool resortable() const;
  void setResortable(bool enable);
  void markConversationAsRead(const QModelIndex& index, bool read = true);

public slots:
  void toggleFilterUnread();
  void resort();

protected:
  bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

private:
  FolderModel* m_model;
  bool m_filterUnread, m_resortable;
  QRegExp m_filter;
  QHeaderView *m_header;
};

#endif
