/*
 * conversationview.h
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
#ifndef CONVERSATIONVIEW_H
#define CONVERSATIONVIEW_H

#include <QTreeView>

#include "conversationdelegate.h"

class ConversationView : public QTreeView
{
  Q_OBJECT
public:
  ConversationView(FolderModel *folderModel, QSortFilterProxyModel *proxyModel, QWidget *parent = 0);
  ~ConversationView();

public slots:
  void updateWidth(int width, int /*nouse*/);
  void swapSort(int column);

private:
  ConversationDelegate *cDelegate;
};

#endif
