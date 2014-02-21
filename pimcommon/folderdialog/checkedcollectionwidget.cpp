/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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


#include "checkedcollectionwidget.h"
#include <Akonadi/RecursiveCollectionFilterProxyModel>
#include <Akonadi/CollectionFilterProxyModel>

#include <Akonadi/ChangeRecorder>
#include <Akonadi/EntityTreeModel>
#include <Akonadi/EntityRightsFilterModel>
#include <KRecursiveFilterProxyModel>


#include <KCheckableProxyModel>
#include <KLineEdit>
#include <KLocalizedString>

#include <QVBoxLayout>
#include <QTreeView>

using namespace PimCommon;

CheckedCollectionWidget::CheckedCollectionWidget(const QString &mimetype, QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setMargin(0);
    setLayout(vbox);

    // Create a new change recorder.
    Akonadi::ChangeRecorder *changeRecorder = new Akonadi::ChangeRecorder( this );
    changeRecorder->fetchCollection( true );
    changeRecorder->setAllMonitored( true );
    changeRecorder->setMimeTypeMonitored( mimetype );
    connect(changeRecorder, SIGNAL(collectionAdded(Akonadi::Collection,Akonadi::Collection)), SIGNAL(collectionAdded(Akonadi::Collection,Akonadi::Collection)));
    connect(changeRecorder, SIGNAL(collectionRemoved(Akonadi::Collection)), SIGNAL(collectionRemoved(Akonadi::Collection)));

    mEntityTreeModel = new Akonadi::EntityTreeModel( changeRecorder, this );
    // Set the model to show only collections, not items.
    mEntityTreeModel->setItemPopulationStrategy( Akonadi::EntityTreeModel::NoItemPopulation );

    Akonadi::CollectionFilterProxyModel *mimeTypeProxy = new Akonadi::CollectionFilterProxyModel( this );
    mimeTypeProxy->setExcludeVirtualCollections( true );
    mimeTypeProxy->addMimeTypeFilters( QStringList() << mimetype );
    mimeTypeProxy->setSourceModel( mEntityTreeModel );

    // Create the Check proxy model.
    mSelectionModel = new QItemSelectionModel( mimeTypeProxy );
    mCheckProxy = new KCheckableProxyModel( this );
    mCheckProxy->setSelectionModel( mSelectionModel );
    mCheckProxy->setSourceModel( mimeTypeProxy );

    mCollectionFilter = new KRecursiveFilterProxyModel(this);
    mCollectionFilter->setSourceModel(mCheckProxy);
    mCollectionFilter->setDynamicSortFilter(true);
    mCollectionFilter->setFilterCaseSensitivity(Qt::CaseInsensitive);

    KLineEdit *searchLine = new KLineEdit(this);
    searchLine->setPlaceholderText(i18n("Search..."));
    searchLine->setClearButtonShown(true);
    connect(searchLine, SIGNAL(textChanged(QString)),
            this, SLOT(slotSetCollectionFilter(QString)));

    vbox->addWidget(searchLine);

    mFolderView = new QTreeView;
    mFolderView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mFolderView->setAlternatingRowColors(true);
    mFolderView->setModel(mCollectionFilter);

    vbox->addWidget(mFolderView);
}

CheckedCollectionWidget::~CheckedCollectionWidget()
{
}

Akonadi::EntityTreeModel *CheckedCollectionWidget::entityTreeModel() const
{
    return mEntityTreeModel;
}

QTreeView *CheckedCollectionWidget::folderTreeView() const
{
    return mFolderView;
}

QItemSelectionModel *CheckedCollectionWidget::selectionModel() const
{
    return mSelectionModel;
}

KCheckableProxyModel *CheckedCollectionWidget::checkableProxy() const
{
    return mCheckProxy;
}

void CheckedCollectionWidget::slotSetCollectionFilter(const QString &filter)
{
    mCollectionFilter->setFilterWildcard(filter);
    mFolderView->expandAll();
}

