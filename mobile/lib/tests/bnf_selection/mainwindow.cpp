
#include "mainwindow.h"

#include "breadcrumbnavigationcontext.h"

MainWindow::MainWindow(QWidget *parent)
  : QWidget(parent)
{
  QStandardItemModel *model = new QStandardItemModel(this);

  int counter = 0;
  QStandardItem *parentItem = model->invisibleRootItem();
  for (int i = 0; i < 4; ++i) {
    QStandardItem *topItem = new QStandardItem(QString("item %0").arg(++counter));

    parentItem->appendRow(topItem);
    for (int i = 0; i < 4; ++i) {
      QStandardItem *childItem = new QStandardItem(QString("item %0").arg(++counter));
      topItem->appendRow(childItem);
      for (int i = 0; i < 4; ++i) {
        QStandardItem *grandChildItem = new QStandardItem(QString("item %0").arg(++counter));
        childItem->appendRow(grandChildItem);
      }
    }
  }

  QHBoxLayout *layout = new QHBoxLayout(this);

  QTreeView *tree = new QTreeView;
  layout->addWidget(tree);

  tree->setModel(model);

  m_bnf = new KBreadcrumbNavigationFactory(this);

  m_bnf->createBreadcrumbContext(model);

  tree->setSelectionModel(m_bnf->selectionModel());

  tree->expandAll();
  tree->setSelectionMode(QTreeView::ExtendedSelection);

  QVBoxLayout *vLayout = new QVBoxLayout;

  QListView *breadcrumbList = new QListView;
  vLayout->addWidget(breadcrumbList);
  breadcrumbList->setModel(m_bnf->breadcrumbItemModel());
  QListView *childList = new QListView;
  vLayout->addWidget(childList);
  childList->setModel(m_bnf->childItemModel());

  layout->addLayout(vLayout);



}

