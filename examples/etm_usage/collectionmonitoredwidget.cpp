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

#include "collectionmonitoredwidget.h"

#include "entitytreewidget.h"
#include <QSplitter>
#include <QHBoxLayout>
#include <QItemSelection>

#include <AkonadiWidgets/EntityTreeView>
#include <AkonadiCore/EntityTreeModel>
#include <AkonadiCore/ChangeRecorder>
#include <AkonadiCore/CollectionFetchScope>

CollectionMonitoredWidget::CollectionMonitoredWidget(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    QSplitter *splitter = new QSplitter(this);
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(splitter);

    m_etw = new EntityTreeWidget(splitter);
    m_etw->init();
    connect(m_etw->view()->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), SLOT(selectionChanged(QItemSelection, QItemSelection)));

    m_oneCollectionChangeRecorder = new Akonadi::ChangeRecorder(this);

    Akonadi::EntityTreeModel *oneCollectionEtm = new Akonadi::EntityTreeModel(m_oneCollectionChangeRecorder, this);

    m_oneCollectionView = new Akonadi::EntityTreeView(splitter);
    m_oneCollectionView->setModel(oneCollectionEtm);
}

void CollectionMonitoredWidget::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QModelIndex index = selected.indexes().first();
    if (!index.isValid()) {
        return;
    }

    Akonadi::Collection col = index.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();

    if (!col.isValid()) {
        return;
    }

    foreach (const Akonadi::Collection &oldCol, m_oneCollectionChangeRecorder->collectionsMonitored()) {
        m_oneCollectionChangeRecorder->setCollectionMonitored(oldCol, false);
    }
    m_oneCollectionChangeRecorder->setCollectionMonitored(col, true);
}
