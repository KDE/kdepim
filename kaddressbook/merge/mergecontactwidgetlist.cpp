/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#include "mergecontactwidgetlist.h"

using namespace KABMergeContacts;
MergeContactWidgetList::MergeContactWidgetList(QWidget *parent)
    : QListWidget(parent)
{
}

MergeContactWidgetList::~MergeContactWidgetList()
{

}

QString MergeContactWidgetList::itemName(const KABC::Addressee &address) const
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

void MergeContactWidgetList::fillListContact(const Akonadi::Item::List &items)
{
    Q_FOREACH(const Akonadi::Item &item, items) {
        if (item.hasPayload<KABC::Addressee>()) {
            MergeContactWidgetListItem *widgetItem = new MergeContactWidgetListItem(item, this);
            KABC::Addressee address = item.payload<KABC::Addressee>();

            widgetItem->setText(itemName(address));
        }
    }
}

Akonadi::Item::List MergeContactWidgetList::listSelectedContacts() const
{
    Akonadi::Item::List lstItems;
    for (int i = 0; i < count(); ++i) {
        QListWidgetItem *itemWidget = item(i);
        if (itemWidget->checkState() == Qt::Checked) {
            lstItems.append((static_cast<MergeContactWidgetListItem*>(itemWidget))->item());
        }
    }
    return lstItems;
}

Akonadi::Item MergeContactWidgetList::currentAkonadiItem() const
{
    QListWidgetItem *curr = currentItem();
    if (curr) {
        return (static_cast<MergeContactWidgetListItem*>(curr))->item();
    }
    return Akonadi::Item();
}



MergeContactWidgetListItem::MergeContactWidgetListItem(const Akonadi::Item &item, QListWidget *parent)
    : QListWidgetItem(parent),
      mItem(item)
{
    setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable );
    setCheckState( Qt::Unchecked );
}

Akonadi::Item MergeContactWidgetListItem::item() const
{
    return mItem;
}

