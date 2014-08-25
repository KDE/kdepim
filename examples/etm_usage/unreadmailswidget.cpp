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

#include "unreadmailswidget.h"

#include <QTreeView>
#include <QHBoxLayout>
#include <QSplitter>

#include <kselectionproxymodel.h>

#include <AkonadiCore/entitytreemodel.h>
#include <AkonadiCore/entitymimetypefiltermodel.h>
#include <AkonadiCore/collection.h>
#include <AkonadiCore/collectionstatistics.h>

#include "entitytreewidget.h"
#include "itemviewerwidget.h"

using namespace Akonadi;

UnreadMailsTree::UnreadMailsTree(QObject *parent)
    : KRecursiveFilterProxyModel(parent)
{

}

bool UnreadMailsTree::acceptRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex idx = sourceModel()->index(sourceRow, 0, sourceParent);
    Collection col = idx.data(EntityTreeModel::CollectionRole).value<Collection>();
    qDebug() << sourceRow << sourceParent << col.statistics().unreadCount() << idx.data().toString();
    return col.statistics().unreadCount() > 0;
}

class UnreadMailsTreeWidget : public EntityTreeWidget
{
public:
    UnreadMailsTreeWidget(QWidget *parent = 0)
        : EntityTreeWidget(parent)
    {
    }

    /* reimp */ void connectTreeToModel(QTreeView *tree, Akonadi::EntityTreeModel *model)
    {
        m_collectionFilter = new Akonadi::EntityMimeTypeFilterModel(this);
        m_collectionFilter->addMimeTypeInclusionFilter(Akonadi::Collection::mimeType());
        m_collectionFilter->setSourceModel(model);
        m_collectionFilter->setHeaderGroup(Akonadi::EntityTreeModel::CollectionTreeHeaders);

        m_unreadFilter = new UnreadMailsTree(this);
        m_unreadFilter->setSourceModel(m_collectionFilter);

        tree->setModel(m_unreadFilter);
    }

    /* reimp */ QModelIndex mapToSource(const QModelIndex &idx)
    {
        return m_collectionFilter->mapToSource(m_unreadFilter->mapToSource(idx));
    }

private:
    Akonadi::EntityMimeTypeFilterModel *m_collectionFilter;
    UnreadMailsTree *m_unreadFilter;

};

UnreadMailsWidget::UnreadMailsWidget(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{

    QHBoxLayout *layout = new QHBoxLayout(this);
    QSplitter *splitter = new QSplitter(this);
    layout->addWidget(splitter);

    m_etw = new UnreadMailsTreeWidget(splitter);
    m_etw->init();

    QSplitter *rhsContainer = new QSplitter(Qt::Vertical, splitter);

    m_itemView = new QTreeView(rhsContainer);

    KSelectionProxyModel *selectionProxy = new KSelectionProxyModel(m_etw->view()->selectionModel(), this);
    selectionProxy->setFilterBehavior(KSelectionProxyModel::ChildrenOfExactSelection);
    selectionProxy->setSourceModel(m_etw->model());

    Akonadi::EntityMimeTypeFilterModel *itemFilter = new Akonadi::EntityMimeTypeFilterModel(this);
    itemFilter->setHeaderGroup(Akonadi::EntityTreeModel::ItemListHeaders);
    itemFilter->addMimeTypeExclusionFilter(Akonadi::Collection::mimeType());
    itemFilter->setSourceModel(selectionProxy);

    m_itemView->setModel(itemFilter);

    ItemViewerWidget *viewerWidget = new ItemViewerWidget(m_itemView->selectionModel(), rhsContainer);

}
