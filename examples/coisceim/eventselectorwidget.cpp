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

#include "eventselectorwidget.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QTextBrowser>
#include <QTreeView>
#include <KLocale>

#include <calendarsupport/next/incidenceviewer.h>

#include <AkonadiCore/ChangeRecorder>
#include <AkonadiCore/ItemFetchScope>

#include <KCalCore/Event>

#include "mixedtreemodel.h"

using namespace Akonadi;

EventSelectorDialog::EventSelectorDialog(QWidget* parent, Qt::WindowFlags f)
  : KDialog(parent, f)
{
  QHBoxLayout *layout = new QHBoxLayout(mainWidget());
  m_view = new QTreeView;
  layout->addWidget(m_view);

  ChangeRecorder *recorder = new ChangeRecorder;
  recorder->setMimeTypeMonitored(KCalCore::Event::eventMimeType());
  recorder->itemFetchScope().fetchFullPayload(true);
  MixedTreeModel *model = new MixedTreeModel(recorder);
  model->setCollectionFetchStrategy(MixedTreeModel::InvisibleCollectionFetch);

  m_view->setModel(model);

}

Item EventSelectorDialog::selectedItem()
{
  const QModelIndexList list = m_view->selectionModel()->selectedRows();
  if (!list.size() == 1)
    return Item();
  return list.first().data(MixedTreeModel::ItemRole).value<Item>();
}

EventSelectorWidget::EventSelectorWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  QHBoxLayout *hLayout = new QHBoxLayout;

  QPushButton *change = new QPushButton(i18n("Select trip"));
  connect(change, &QPushButton::clicked, this, &EventSelectorWidget::selectTrip);

  hLayout->addWidget(change);

  layout->addLayout(hLayout);

  m_browser = new CalendarSupport::IncidenceViewer;
  layout->addWidget(m_browser);
}

void EventSelectorWidget::selectTrip()
{
  EventSelectorDialog dlg;
  if (dlg.exec()) {
    Item item = dlg.selectedItem();
    if (item.isValid()) {
      emit selected(item);
      m_browser->setItem(item);
    }
  }
}

void EventSelectorWidget::clear()
{
  m_browser->setItem(Item());
}

