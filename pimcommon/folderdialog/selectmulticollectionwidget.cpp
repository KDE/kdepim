/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "selectmulticollectionwidget.h"
#include "pimcommon/folderdialog/checkedcollectionwidget.h"

#include <Akonadi/RecursiveCollectionFilterProxyModel>
#include <Akonadi/CollectionFilterProxyModel>

#include <Akonadi/ChangeRecorder>
#include <Akonadi/EntityTreeModel>
#include <Akonadi/EntityRightsFilterModel>

#include <KCheckableProxyModel>

#include <QVBoxLayout>
#include <QTreeView>

using namespace PimCommon;
SelectMultiCollectionWidget::SelectMultiCollectionWidget(const QString &mimetype, QWidget *parent)
    : QWidget(parent)
{
    initialize(mimetype);
}


SelectMultiCollectionWidget::SelectMultiCollectionWidget(const QString &mimetype, const QList<Akonadi::Collection::Id> &selectedCollection, QWidget *parent)
    : QWidget(parent),
      mListCollection(selectedCollection)
{
    initialize(mimetype);
}

SelectMultiCollectionWidget::~SelectMultiCollectionWidget()
{
}

void SelectMultiCollectionWidget::initialize(const QString &mimetype)
{
    QVBoxLayout *vbox = new QVBoxLayout;
    setLayout(vbox);

    mCheckedCollectionWidget = new PimCommon::CheckedCollectionWidget(mimetype);
    connect(mCheckedCollectionWidget->entityTreeModel(), SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(slotCollectionsInserted(QModelIndex,int,int)));

    vbox->addWidget(mCheckedCollectionWidget);
}

void SelectMultiCollectionWidget::updateStatus(const QModelIndex &parent)
{
    const int nbCol = mCheckedCollectionWidget->checkableProxy()->rowCount( parent );
    for ( int i = 0; i < nbCol; ++i ) {
        const QModelIndex child = mCheckedCollectionWidget->checkableProxy()->index( i, 0, parent );

        const Akonadi::Collection col =
                mCheckedCollectionWidget->checkableProxy()->data( child, Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();

        if (mListCollection.contains(col.id())) {
            mCheckedCollectionWidget->checkableProxy()->setData( child, Qt::Checked, Qt::CheckStateRole );
        }
        updateStatus( child );
    }
}

void SelectMultiCollectionWidget::slotCollectionsInserted(const QModelIndex &, int, int)
{
    if (!mListCollection.isEmpty()) {
        updateStatus(QModelIndex());
    }
    mCheckedCollectionWidget->folderTreeView()->expandAll();
}

QList<Akonadi::Collection> SelectMultiCollectionWidget::selectedCollection(const QModelIndex &parent) const
{
    QList<Akonadi::Collection> lst;

    const int nbCol = mCheckedCollectionWidget->checkableProxy()->rowCount( parent );
    for ( int i = 0; i < nbCol; ++i ) {
      const QModelIndex child = mCheckedCollectionWidget->checkableProxy()->index( i, 0, parent );

      const Akonadi::Collection col =
        mCheckedCollectionWidget->checkableProxy()->data( child, Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();

      if (mCheckedCollectionWidget->checkableProxy()->data( child, Qt::CheckStateRole ).value<int>())
          lst << col;
      lst << selectedCollection( child );
    }
    return lst;
}


