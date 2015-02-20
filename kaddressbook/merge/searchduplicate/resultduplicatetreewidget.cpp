/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "resultduplicatetreewidget.h"
#include <KLocalizedString>

#include <KContacts/Addressee>

using namespace KABMergeContacts;

ResultDuplicateTreeWidget::ResultDuplicateTreeWidget(QWidget *parent)
    : QTreeWidget(parent)
{
    setHeaderLabel(i18n("Contacts"));
    connect(this, &QTreeWidget::itemActivated, this, &ResultDuplicateTreeWidget::slotItemActivated);
    connect(this, SIGNAL(itemChanged(QTreeWidgetItem*,int)), SLOT(slotItemChanged(QTreeWidgetItem*,int)));
}

ResultDuplicateTreeWidget::~ResultDuplicateTreeWidget()
{

}

void ResultDuplicateTreeWidget::slotItemChanged(QTreeWidgetItem *item, int column)
{
    if (column != 0) {
        return;
    }
    //Parent
    if (item->childCount() != 0) {
        changeState(item, item->checkState(0) == Qt::Checked);
    } else { //child
        QTreeWidgetItem *parent = item->parent();
        if (parent) {
            blockSignals(true);
            Qt::CheckState state = Qt::PartiallyChecked;
            for (int i = 0; i < parent->childCount(); ++i) {
                if (i == 0) {
                    state = parent->child(i)->checkState(0);
                } else {
                    if (state != parent->child(i)->checkState(0)) {
                        state = Qt::PartiallyChecked;
                        break;
                    }
                }
            }
            parent->setCheckState(0, state);
            blockSignals(false);
        }
    }
}

void ResultDuplicateTreeWidget::changeState(QTreeWidgetItem *item, bool b)
{
    blockSignals(true);
    item->setCheckState(0, b ? Qt::Checked : Qt::Unchecked);
    for (int i = 0; i < item->childCount(); ++i) {
        item->child(i)->setCheckState(0, b ? Qt::Checked : Qt::Unchecked);
    }
    blockSignals(false);
}

void ResultDuplicateTreeWidget::slotItemActivated(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    ResultDuplicateTreeWidgetItem *resultItem = dynamic_cast<ResultDuplicateTreeWidgetItem *>(item);
    if (resultItem) {
        Q_EMIT showContactPreview(resultItem->item());
    }
}

void ResultDuplicateTreeWidget::setContacts(const QList<Akonadi::Item::List> &lstItem)
{
    clear();
    int i = 1;
    Q_FOREACH (const Akonadi::Item::List &lst, lstItem) {
        ResultDuplicateTreeWidgetItem *topLevelItem = new ResultDuplicateTreeWidgetItem(this);
        topLevelItem->setText(0, i18n("Duplicate contact %1", i));
        Q_FOREACH (const Akonadi::Item &item, lst) {
            ResultDuplicateTreeWidgetItem *childItem = new ResultDuplicateTreeWidgetItem;
            topLevelItem->addChild(childItem);
            childItem->setItem(item);
        }
        ++i;
    }
    expandAll();
}

QList<Akonadi::Item::List> ResultDuplicateTreeWidget::selectedContactsToMerge() const
{
    QList<Akonadi::Item::List> listItems;
    for (int i = 0; i < topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = topLevelItem(i);
        const int childCount = item->childCount();
        if (childCount > 0) {
            Akonadi::Item::List items;
            for (int child = 0; child < childCount; ++child) {
                ResultDuplicateTreeWidgetItem *childItem = static_cast<ResultDuplicateTreeWidgetItem *>(item->child(child));
                if (childItem->checkState(0) == Qt::Checked) {
                    items << childItem->item();
                }
            }
            if (items.count() > 1) {
                listItems << items;
            }
        }
    }
    return listItems;
}

ResultDuplicateTreeWidgetItem::ResultDuplicateTreeWidgetItem(QTreeWidget *parent)
    : QTreeWidgetItem(parent)
{
    setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    setCheckState(0, Qt::Unchecked);
}

ResultDuplicateTreeWidgetItem::~ResultDuplicateTreeWidgetItem()
{

}

Akonadi::Item ResultDuplicateTreeWidgetItem::item() const
{
    return mItem;
}

void ResultDuplicateTreeWidgetItem::setItem(const Akonadi::Item &item)
{
    mItem = item;
    setDisplayName();
}

QString ResultDuplicateTreeWidgetItem::contactName(const KContacts::Addressee &address)
{
    const QString realName = address.realName();
    if (!realName.isEmpty()) {
        return realName;
    }
    const QString name = address.name();
    if (!name.isEmpty()) {
        return name;
    }
    return address.fullEmail();
}

void ResultDuplicateTreeWidgetItem::setDisplayName()
{
    if (mItem.isValid()) {
        const KContacts::Addressee address = mItem.payload<KContacts::Addressee>();
        setText(0, contactName(address));
    }
}
