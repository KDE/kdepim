/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

// READ THE README FILE

#include "unreadmailsincollectionswidget.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QSplitter>
#include <QApplication>

#include <kglobal.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include <akonadi/entitytreemodel.h>
#include <akonadi/entitytreeview.h>

#include <akonadi_next/checkableitemproxymodel.h>
#include <akonadi_next/etmstatesaver.h>
#include <akonadi_next/mailmodel.h>

#include <Akonadi/ChangeRecorder>
#include <Akonadi/ItemFetchScope>
#include <akonadi/entitymimetypefiltermodel.h>
#include <akonadi/selectionproxymodel.h>

#include <KMime/Message>
#include <messagecore/messagestatus.h>

#include "itemviewerwidget.h"


#define VIEW(model) \
  { \
  Akonadi::EntityTreeView *view = new Akonadi::EntityTreeView; \
  view->setModel(model); \
  view->setWindowTitle(#model); \
  view->show(); \
  }

UnreadMailsInCollectionsProxy::UnreadMailsInCollectionsProxy(QObject* parent)
  : QSortFilterProxyModel(parent)
{

}

bool UnreadMailsInCollectionsProxy::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
  static const int column = 0;
  const QModelIndex index = sourceModel()->index(source_row, column, source_parent);
  Q_ASSERT(index.isValid());
  const Akonadi::Item item = index.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
  Q_ASSERT(item.isValid());
  KPIM::MessageStatus messageStatus;
  messageStatus.setStatusFromFlags(item.flags());

  // Or messageStatus.isImportant();
  return messageStatus.isUnread();
}

UnreadMailsInCollectionsWidget::UnreadMailsInCollectionsWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  QPushButton *configureButton = new QPushButton("Configure");
  connect( configureButton, SIGNAL(clicked(bool)), SLOT(configure()));
  layout->addWidget(configureButton);

  QSplitter *splitter = new QSplitter;
  layout->addWidget(splitter);

  m_changeRecorder = new Akonadi::ChangeRecorder(this);
  m_changeRecorder->itemFetchScope().fetchFullPayload(true);
  m_changeRecorder->setCollectionMonitored(Akonadi::Collection::root());
  m_changeRecorder->setMimeTypeMonitored( KMime::Message::mimeType() );

  m_etm = new MailModel(m_changeRecorder, this);

  VIEW(m_etm);

  Akonadi::EntityMimeTypeFilterModel *collectionFilter = new Akonadi::EntityMimeTypeFilterModel(this);

  collectionFilter->setSourceModel(m_etm);
  collectionFilter->setHeaderGroup(Akonadi::EntityTreeModel::CollectionTreeHeaders);
  collectionFilter->addMimeTypeInclusionFilter(Akonadi::Collection::mimeType());

  m_checkedItemModel = new QItemSelectionModel(collectionFilter);

  m_checkableProxy = new CheckableItemProxyModel(this);
  m_checkableProxy->setSelectionModel(m_checkedItemModel);
  m_checkableProxy->setSourceModel(collectionFilter);

  KSelectionProxyModel *selectionProxy = new KSelectionProxyModel(m_checkedItemModel, this);
  selectionProxy->setFilterBehavior(KSelectionProxyModel::ChildrenOfExactSelection);
  selectionProxy->setSourceModel(m_etm);

  Akonadi::EntityMimeTypeFilterModel *itemFilter = new Akonadi::EntityMimeTypeFilterModel(this);
  itemFilter->addMimeTypeExclusionFilter(Akonadi::Collection::mimeType());
  itemFilter->setHeaderGroup(Akonadi::EntityTreeModel::ItemListHeaders);
  itemFilter->setSourceModel(selectionProxy);

  UnreadMailsInCollectionsProxy *unreadMailsProxy = new UnreadMailsInCollectionsProxy(this);
  unreadMailsProxy->setSourceModel(itemFilter);

  Akonadi::EntityTreeView *emailView = new Akonadi::EntityTreeView;
  emailView->setModel(unreadMailsProxy);

  splitter->addWidget(emailView);

  ItemViewerWidget *itemViewer = new ItemViewerWidget(emailView->selectionModel());

  splitter->addWidget(itemViewer);

  connect( m_etm, SIGNAL(modelAboutToBeReset()), SLOT(saveState()) );
  connect( m_etm, SIGNAL(modelReset()), SLOT(restoreState()) );
  connect( qApp, SIGNAL(aboutToQuit()), SLOT(saveState()) );

  restoreCheckState();
}

UnreadMailsInCollectionsWidget::~UnreadMailsInCollectionsWidget()
{
  saveCheckState();
}

// We could use another ETMStateSaver to save and restore state in the configure dialog
// Ie expanded state of the collections, and scroll position.

void UnreadMailsInCollectionsWidget::saveCheckState()
{

  ETMStateSaver saver;
  saver.setSelectionModel(m_checkedItemModel);

  KConfigGroup cfg( KGlobal::config(), "CheckState" );
  saver.saveState(cfg);
  cfg.sync();
}

void UnreadMailsInCollectionsWidget::restoreCheckState()
{
  ETMStateSaver *restorer = new ETMStateSaver;
  restorer->setSelectionModel(m_checkedItemModel);

  KConfigGroup cfg( KGlobal::config(), "CheckState" );
  restorer->restoreState(cfg);
}

void UnreadMailsInCollectionsWidget::configure()
{
  Akonadi::EntityTreeView *configureView = new Akonadi::EntityTreeView;
  configureView->setModel(m_checkableProxy);
  configureView->show();
}
