/*
    This file is part of Akonadi.

    Copyright (c) 2011 Stephen Kelly <steveire@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "coisceimwidget.h"
// #include "coisceimwidgetadaptor.h"

#include <QVBoxLayout>
#include <QSplitter>

#include <QListView>
#include <QTreeView>

#include <AkonadiCore/ChangeRecorder>
#include <AkonadiCore/ItemFetchScope>

#include "tripmodel.h"
#include "stackedwidgetview.h"
#include <KGlobal>
#include "createtripwidget.h"


using namespace Akonadi;

CoisceimWidget::CoisceimWidget(QWidget *parent)
  : QWidget(parent)
{
//   new CoisceimWidgetAdaptor(this);
  QHBoxLayout *layout = new QHBoxLayout(this);
  QSplitter *splitter = new QSplitter;
  layout->addWidget(splitter);

  ChangeRecorder *tripRec = new ChangeRecorder(this);
  tripRec->itemFetchScope().fetchFullPayload(true);
  m_tripModel = new TripModel(tripRec, this);

  QListView *list = new QListView;
  list->setViewMode(QListView::IconMode);
  list->setFlow(QListView::TopToBottom);
  list->setUniformItemSizes(true);
  list->setModel(m_tripModel);

  splitter->addWidget(list);

  StackedWidgetView *stack = new StackedWidgetView(TripModel::WidgetRole);
  splitter->addWidget(stack);

  stack->setModel(m_tripModel);
  stack->setSelectionModel(list->selectionModel());
}

void CoisceimWidget::createTrip()
{
  qDebug() << "CREATE";
}

