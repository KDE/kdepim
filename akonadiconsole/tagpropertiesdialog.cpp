/*
 * Copyright (C) 2014  Daniel Vrátil <dvratil@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "tagpropertiesdialog.h"
#include "dbaccess.h"

#include <AkonadiCore/AttributeFactory>
#include <QSqlQuery>
#include <QSqlError>
#include <KConfigGroup>
#include <QDialogButtonBox>

using namespace Akonadi;

TagPropertiesDialog::TagPropertiesDialog(QWidget *parent)
    : QDialog(parent)
    , mChanged(false)
{
    setupUi();
}

TagPropertiesDialog::TagPropertiesDialog(const Akonadi::Tag &tag, QWidget *parent)
    : QDialog(parent)
    , mTag(tag)
    , mChanged(false)
{
    setupUi();
}

TagPropertiesDialog::~TagPropertiesDialog()
{
}

Tag TagPropertiesDialog::tag() const
{
    return mTag;
}

bool TagPropertiesDialog::changed() const
{
    return mChanged;
}

void TagPropertiesDialog::setupUi()
{
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &TagPropertiesDialog::slotAccept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &TagPropertiesDialog::reject);

    QWidget *widget = new QWidget(this);
    ui.setupUi(widget);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(widget);
    mainLayout->addWidget(buttonBox);

    connect(ui.addAttrButton, &QPushButton::clicked, this, &TagPropertiesDialog::addAttributeClicked);
    connect(ui.deleteAttrButton, &QPushButton::clicked, this, &TagPropertiesDialog::deleteAttributeClicked);

    connect(ui.addRIDButton, &QPushButton::clicked, this, &TagPropertiesDialog::addRIDClicked);
    connect(ui.deleteRIDButton, &QPushButton::clicked, this, &TagPropertiesDialog::deleteRIDClicked);

    Attribute::List attributes = mTag.attributes();
    mAttributesModel = new QStandardItemModel(attributes.size(), 2, this);
    connect(mAttributesModel, &QStandardItemModel::itemChanged, this, &TagPropertiesDialog::attributeChanged);
    QStringList labels;
    labels << i18n("Attribute") << i18n("Value");
    mAttributesModel->setHorizontalHeaderLabels(labels);

    mRemoteIdsModel = new QStandardItemModel(this);
    connect(mRemoteIdsModel, &QStandardItemModel::itemChanged, this, &TagPropertiesDialog::remoteIdChanged);
    mRemoteIdsModel->setColumnCount(2);
    labels.clear();
    labels << i18n("Resource") << i18n("Remote ID");
    mRemoteIdsModel->setHorizontalHeaderLabels(labels);
    ui.ridsView->setModel(mRemoteIdsModel);

    if (mTag.isValid()) {
        ui.idLabel->setText(QString::number(mTag.id()));
        ui.typeEdit->setText(QString::fromLatin1(mTag.type()));
        ui.gidEdit->setText(QString::fromLatin1(mTag.gid()));
        ui.parentIdLabel->setText(QString::number(mTag.parent().id()));

        for (int i = 0; i < attributes.count(); ++i) {
            QModelIndex index = mAttributesModel->index(i, 0);
            Q_ASSERT(index.isValid());
            mAttributesModel->setData(index, QString::fromLatin1(attributes[i]->type()));
            mAttributesModel->item(i, 0)->setEditable(false);
            index = mAttributesModel->index(i, 1);
            Q_ASSERT(index.isValid());
            mAttributesModel->setData(index, QString::fromLatin1(attributes[i]->serialized()));
            mAttributesModel->item(i, 1)->setEditable(true);
        }

        {
            // There is (intentionally) no way to retrieve Tag RID for another
            // resource than in the current context. Since Akonadi Console has
            // not resource context at all, we need to retrieve the IDs the hard way
            QSqlQuery query(DbAccess::database());
            query.prepare(QLatin1String("SELECT ResourceTable.name, TagRemoteIdResourceRelationTable.remoteId "
                                        "FROM TagRemoteIdResourceRelationTable "
                                        "LEFT JOIN ResourceTable ON ResourceTable.id = TagRemoteIdResourceRelationTable.resourceId "
                                        "WHERE TagRemoteIdResourceRelationTable.tagid = ?"));
            query.addBindValue(mTag.id());
            if (query.exec()) {
                while (query.next()) {
                    QList<QStandardItem*> items;
                    QStandardItem *item = new QStandardItem(query.value(0).toString());
                    item->setEditable(false);
                    items << item;
                    item = new QStandardItem(query.value(1).toString());
                    item->setEditable(true);
                    items << item;
                    mRemoteIdsModel->appendRow(items);
                }
            } else {
                qCritical() << query.executedQuery();
                qCritical() << query.lastError().text();
            }
        }
    } else {
        ui.idLabel->setVisible(false);
        ui.idLabelBuddy->setVisible(false);
        if (mTag.parent().isValid()) {
            ui.parentIdLabel->setText(QString::number(mTag.parent().id()));
        } else {
            ui.parentIdLabel->setVisible(false);
            ui.parentIdLabelBuddy->setVisible(false);
        }
        // Since we are using direct SQL to update RIDs, we cannot do this
        // when creating a new Tag, because the tag is created by caller after
        // this dialog is closed
        ui.tabWidget->setTabEnabled(2, false);
    }

    ui.attrsView->setModel(mAttributesModel);
}

void TagPropertiesDialog::addAttributeClicked()
{
    const QString newType = ui.newAttrEdit->text();
    if (newType.isEmpty()) {
        return;
    }
    ui.newAttrEdit->clear();

    mChangedAttrs.insert(newType);
    mRemovedAttrs.remove(newType);
    mChanged = true;

    const int row = mAttributesModel->rowCount();
    mAttributesModel->insertRow(row);
    const QModelIndex index = mAttributesModel->index(row, 0);
    Q_ASSERT(index.isValid());
    mAttributesModel->setData(index, newType);
    mAttributesModel->item(row, 0)->setEditable(false);
    mAttributesModel->setItem(row, 1, new QStandardItem);
    mAttributesModel->item(row, 1)->setEditable(true);
}

void TagPropertiesDialog::deleteAttributeClicked()
{
    const QModelIndexList selection = ui.attrsView->selectionModel()->selectedRows();
    if (selection.count() != 1) {
        return;
    }
    const QString attr = selection.first().data().toString();
    mChangedAttrs.remove(attr);
    mRemovedAttrs.insert(attr);
    mChanged = true;
    mAttributesModel->removeRow(selection.first().row());
}

void TagPropertiesDialog::attributeChanged(QStandardItem *item)
{
    const QString attr = mAttributesModel->data(mAttributesModel->index(item->row(), 0)).toString();
    mRemovedAttrs.remove(attr);
    mChangedAttrs.insert(attr);
    mChanged = true;
}

void TagPropertiesDialog::addRIDClicked()
{
    const QString newResource = ui.newRIDEdit->text();
    if (newResource.isEmpty()) {
        return;
    }
    ui.newRIDEdit->clear();

    mChangedRIDs.insert(newResource);
    mRemovedRIDs.remove(newResource);
    // Don't change mChanged here, we will handle this internally

    const int row = mRemoteIdsModel->rowCount();
    mRemoteIdsModel->insertRow(row);
    const QModelIndex index = mRemoteIdsModel->index(row, 0);
    Q_ASSERT(index.isValid());
    mRemoteIdsModel->setData(index, newResource);
    mRemoteIdsModel->item(row, 0)->setEditable(false);
    mRemoteIdsModel->setItem(row, 1, new QStandardItem);
    mRemoteIdsModel->item(row, 1)->setEditable(true);
}

void TagPropertiesDialog::deleteRIDClicked()
{
    const QModelIndexList selection = ui.ridsView->selectionModel()->selectedRows();
    if (selection.count() != 1) {
        return;
    }
    const QString res = selection.first().data().toString();
    mChangedRIDs.remove(res);
    mRemovedRIDs.insert(res);
    // Don't change mChanged here, we will handle this internally
    mRemoteIdsModel->removeRow(selection.first().row());
}

void TagPropertiesDialog::remoteIdChanged(QStandardItem *item)
{
    const QString res = mRemoteIdsModel->data(mRemoteIdsModel->index(item->row(), 0)).toString();
    mRemovedRIDs.remove(res);
    mChangedRIDs.insert(res);
    // Don't change mChanged here, we will handle this internally
}


void TagPropertiesDialog::slotAccept()
{
    mChanged |= (mTag.type() != ui.typeEdit->text().toLatin1());
    mChanged |= (mTag.gid() != ui.gidEdit->text().toLatin1());

    if (!mChanged && mChangedRIDs.isEmpty() && mRemovedRIDs.isEmpty()) {
        QDialog::accept();
        return;
    }

    mTag.setType(ui.typeEdit->text().toLatin1());
    mTag.setGid(ui.gidEdit->text().toLatin1());

    Q_FOREACH (const QString &removedAttr, mRemovedAttrs) {
        mTag.removeAttribute(removedAttr.toLatin1());
    }
    for (int i = 0; i < mAttributesModel->rowCount(); ++i) {
        const QModelIndex typeIndex = mAttributesModel->index(i, 0);
        Q_ASSERT(typeIndex.isValid());
        if (!mChangedAttrs.contains(typeIndex.data().toString())) {
            continue;
        }
        const QModelIndex valueIndex = mAttributesModel->index(i, 1);
        Attribute *attr = AttributeFactory::createAttribute(mAttributesModel->data(typeIndex).toString().toLatin1());
        if (!attr) {
            continue;
        }
        attr->deserialize(mAttributesModel->data(valueIndex).toString().toLatin1());
        mTag.addAttribute(attr);
    }

    bool queryOK = true;
    if (mTag.isValid() && (!mRemovedRIDs.isEmpty() || !mChangedRIDs.isEmpty())) {
        DbAccess::database().transaction();
    }

    if (mTag.isValid() && !mRemovedRIDs.isEmpty()) {
        QSqlQuery query(DbAccess::database());
        QString queryStr = QLatin1String("DELETE FROM TagRemoteIdResourceRelationTable "
                                         "WHERE tagId = ? AND "
                                               "resourceId IN (SELECT id "
                                                              "FROM ResourceTable "
                                                              "WHERE ");
        QStringList conds;
        for (int i = 0; i < mRemovedRIDs.count(); ++i) {
            conds << QLatin1String("name = ?");
        }
        queryStr += conds.join(QLatin1String(" OR ")) + QLatin1String(")");
        query.prepare(queryStr);
        query.addBindValue(mTag.id());
        Q_FOREACH (const QString &removedRid, mRemovedRIDs) {
            query.addBindValue(removedRid);
        }
        if (!query.exec()) {
            qCritical() << query.executedQuery();
            qCritical() << query.lastError().text();
            queryOK = false;
        }
    }
    if (queryOK && mTag.isValid() && !mChangedRIDs.isEmpty()) {
        QMap<QString, qint64> resourceNameToIdMap;
        QVector<qint64> existingResourceRecords;
        {
            QSqlQuery query(DbAccess::database());
            QString queryStr = QLatin1String("SELECT id, name FROM ResourceTable WHERE ");
            QStringList conds;
            for (int i = 0; i < mChangedRIDs.count(); ++i) {
                conds << QLatin1String("name = ?");
            }
            queryStr += conds.join(QLatin1String(" OR "));
            query.prepare(queryStr);
            Q_FOREACH (const QString &res, mChangedRIDs) {
                query.addBindValue(res);
            }
            if (!query.exec()) {
                qCritical() << query.executedQuery();
                qCritical() << query.lastError().text();
                queryOK = false;
            }

            while (query.next()) {
                resourceNameToIdMap[query.value(1).toString()] = query.value(0).toLongLong();
            }
        }

        // This is a workaround for PSQL not supporting UPSERTs
        {
            QSqlQuery query(DbAccess::database());
            query.prepare(QLatin1String("SELECT resourceId FROM TagRemoteIdResourceRelationTable WHERE tagId = ?"));
            query.addBindValue(mTag.id());
            if (!query.exec()) {
                qCritical() << query.executedQuery();
                qCritical() << query.lastError().text();
                queryOK = false;
            }

            while (query.next()) {
                existingResourceRecords << query.value(0).toLongLong();
            }
        }

        for (int i = 0; i < mRemoteIdsModel->rowCount() && queryOK; ++i) {
            const QModelIndex resIndex = mRemoteIdsModel->index(i, 0);
            Q_ASSERT(resIndex.isValid());
            if (!mChangedRIDs.contains(resIndex.data().toString())) {
                continue;
            }
            const QModelIndex valueIndex = mRemoteIdsModel->index(i, 1);

            QSqlQuery query(DbAccess::database());
            const qlonglong resourceId = resourceNameToIdMap[resIndex.data().toString()];
            if (existingResourceRecords.contains(resourceId)) {
                query.prepare(QLatin1String("UPDATE TagRemoteIdResourceRelationTable SET remoteId = ? WHERE tagId = ? AND resourceId = ?"));
                query.addBindValue(valueIndex.data().toString());
                query.addBindValue(mTag.id());
                query.addBindValue(resourceId);
            } else {
                query.prepare(QLatin1String("INSERT INTO TagRemoteIdResourceRelationTable (tagId, resourceId, remoteId) VALUES (?, ?, ?)"));
                query.addBindValue(mTag.id());
                query.addBindValue(resourceId);
                query.addBindValue(valueIndex.data().toString());
            }
            if (!query.exec()) {
                qCritical() << query.executedQuery();
                qCritical() << query.lastError().text();
                queryOK = false;
                break;
            }
        }
    }

    if (mTag.isValid() && (!mRemovedRIDs.isEmpty() || !mChangedRIDs.isEmpty())) {
        if (queryOK) {
            DbAccess::database().commit();
        } else {
            DbAccess::database().rollback();
        }
    }

    QDialog::accept();
}
