/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Tobias Koenig <tokoe@kdab.com>

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

#include "acleditor.h"

#include <AkonadiCore/attributefactory.h>
#include <AkonadiCore/entitydisplayattribute.h>
#include <kactioncollection.h>
#include <pimcommon/acl/aclmanager.h>
#include <pimcommon/acl/imapaclattribute.h>

#include <QtCore/QAbstractItemModel>
#include <QItemSelectionModel>

AclEditor::AclEditor(KActionCollection *actionCollection, QObject *parent)
    : QObject(parent), mAclManager(new PimCommon::AclManager(this))
{
    Akonadi::AttributeFactory::registerAttribute<PimCommon::ImapAclAttribute>();

    actionCollection->addAction(QLatin1String("acleditor_add"), mAclManager->addAction());
    actionCollection->addAction(QLatin1String("acleditor_edit"), mAclManager->editAction());
    actionCollection->addAction(QLatin1String("acleditor_delete"), mAclManager->deleteAction());
}

void AclEditor::setCollection(const Akonadi::Collection &collection)
{
    mCollection = collection;

    emit collectionChanged(mCollection);
}

QString AclEditor::collectionName() const
{
    if (mCollection.isValid()) {
        return mCollection.displayName();
    } else {
        return QString();
    }
}

bool AclEditor::collectionHasAcls() const
{
    if (!mCollection.isValid()) {
        return false;
    }

    return mCollection.hasAttribute<PimCommon::ImapAclAttribute>();
}

void AclEditor::load()
{
    mAclManager->setCollection(mCollection);
}

void AclEditor::save()
{
    mAclManager->save();
}

QAbstractItemModel *AclEditor::model() const
{
    return mAclManager->model();
}

void AclEditor::setRowSelected(int row)
{
    const QAbstractItemModel *model = mAclManager->model();
    QItemSelectionModel *selectionModel = mAclManager->selectionModel();

    Q_ASSERT(row >= 0 && row < model->rowCount());

    selectionModel->select(model->index(row, 0), QItemSelectionModel::ClearAndSelect);
}

