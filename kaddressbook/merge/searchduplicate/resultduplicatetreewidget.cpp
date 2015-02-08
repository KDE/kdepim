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
    //kf5 add i18n
    setHeaderLabel(QLatin1String("Contacts"));
    connect(this, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(slotItemActivated(QTreeWidgetItem*,int)));
}

ResultDuplicateTreeWidget::~ResultDuplicateTreeWidget()
{

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
        ResultDuplicateTreeWidgetItem *topLevelItem = new ResultDuplicateTreeWidgetItem(this, false);
        topLevelItem->setText(0, i18n("Duplicate contact %1", i));
        topLevelItem->setText(0, QString::fromLatin1("Duplicate contact %1").arg(i));
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

ResultDuplicateTreeWidgetItem::ResultDuplicateTreeWidgetItem(QTreeWidget *parent, bool hasCheckableItem)
    : QTreeWidgetItem(parent)
{
    setFlags(hasCheckableItem ? Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled : Qt::ItemIsEnabled);
    if (hasCheckableItem) {
        setCheckState(0, Qt::Unchecked);
    }
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
