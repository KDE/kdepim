
#include "mainwindow.h"

#include "breadcrumbnavigationcontext.h"

MainWindow::MainWindow(QWidget *parent)
  : QWidget(parent)
{
  m_model = new QStandardItemModel(this);

  int counter = 0;
  QStandardItem *parentItem = m_model->invisibleRootItem();
  for (int i = 0; i < 4; ++i) {
    QStandardItem *topItem = new QStandardItem(QString::fromLatin1("item %1").arg(++counter));

    parentItem->appendRow(topItem);
    for (int i = 0; i < 4; ++i) {
      QStandardItem *childItem = new QStandardItem(QString::fromLatin1("item %1").arg(++counter));
      topItem->appendRow(childItem);
      for (int i = 0; i < 4; ++i) {
        QStandardItem *grandChildItem = new QStandardItem(QString::fromLatin1("item %1").arg(++counter));
        childItem->appendRow(grandChildItem);
      }
    }
  }

  QHBoxLayout *layout = new QHBoxLayout(this);

  QTreeView *tree = new QTreeView;
  layout->addWidget(tree);

  tree->setModel(m_model);

  m_bnf = new KBreadcrumbNavigationFactory(this);

  m_bnf->createBreadcrumbContext(m_model);

  tree->setSelectionModel(m_bnf->selectionModel());

  tree->expandAll();
  tree->setSelectionMode(QTreeView::ExtendedSelection);

  QVBoxLayout *vLayout = new QVBoxLayout;

  QListView *breadcrumbList = new QListView;
  vLayout->addWidget(breadcrumbList);
  breadcrumbList->setModel(m_bnf->breadcrumbItemModel());
  QListView *selectedList = new QListView;
  vLayout->addWidget(selectedList);
  selectedList->setModel(m_bnf->selectedItemModel());
  QListView *childList = new QListView;
  vLayout->addWidget(childList);
  childList->setModel(m_bnf->childItemModel());

  layout->addLayout(vLayout);

  QTimer::singleShot(2000, this, SLOT(doSelects()));
}

void MainWindow::doSelects()
{
  QModelIndex first = m_model->index(0, 0, m_model->index(0, 0));
  QModelIndex second = m_model->index(0, 0, m_model->index(1, 0));
  m_bnf->selectionModel()->select( first, QItemSelectionModel::Select );
  m_bnf->selectionModel()->select(second, QItemSelectionModel::Select );
}
