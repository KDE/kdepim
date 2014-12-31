/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include "mergecontactwidget.h"
#include "mergecontactwidgetlist.h"

#include <KLocalizedString>

#include <KContacts/Addressee>

#include <AkonadiWidgets/CollectionComboBox>
#include <QListWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

namespace KABMergeContacts
{
KADDRESSBOOK_EXPORT QAbstractItemModel *_k_mergeStubModel = 0;
}

using namespace KABMergeContacts;
MergeContactWidget::MergeContactWidget(const Akonadi::Item::List &items, QWidget *parent)
    : QWidget(parent),
      mItems(items)
{
    QVBoxLayout *lay = new QVBoxLayout;

    QLabel *lab = new QLabel(i18n("Select contacts that you really want to merge:"));
    lay->addWidget(lab);
    mListWidget = new MergeContactWidgetList;
    mListWidget->setObjectName(QStringLiteral("listcontact"));
    mListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    lay->addWidget(mListWidget);
    connect(mListWidget, &MergeContactWidgetList::itemSelectionChanged, this, &MergeContactWidget::slotUpdateMergeButton);
    connect(mListWidget, &MergeContactWidgetList::itemChanged, this, &MergeContactWidget::slotUpdateMergeButton);

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addStretch();

    lab = new QLabel(i18n("Select the addressbook in which to store merged contacts:"));
    hbox->addWidget(lab);

    mCollectionCombobox = new Akonadi::CollectionComboBox(_k_mergeStubModel);
    mCollectionCombobox->setAccessRightsFilter(Akonadi::Collection::CanCreateItem);
    mCollectionCombobox->setMinimumWidth(250);
    mCollectionCombobox->setMimeTypeFilter(QStringList() << KContacts::Addressee::mimeType());
    mCollectionCombobox->setObjectName(QStringLiteral("akonadicombobox"));
    hbox->addWidget(mCollectionCombobox);

    lay->addLayout(hbox);

    hbox = new QHBoxLayout;
    hbox->addStretch();
    mMergeButton = new QPushButton(i18n("merge"));
    mMergeButton->setObjectName(QStringLiteral("mergebutton"));
    hbox->addWidget(mMergeButton);
    mMergeButton->setEnabled(false);
    lay->addLayout(hbox);

    connect(mMergeButton, &QPushButton::clicked, this, &MergeContactWidget::slotMergeContacts);

    setLayout(lay);
    fillListContact();
}

MergeContactWidget::~MergeContactWidget()
{

}

void MergeContactWidget::clear()
{
    mListWidget->clear();
}

void MergeContactWidget::fillListContact()
{
    mListWidget->fillListContact(mItems);
}

Akonadi::Item::List MergeContactWidget::listSelectedContacts() const
{
    return mListWidget->listSelectedContacts();
}

Akonadi::Item MergeContactWidget::currentItem() const
{
    return mListWidget->currentAkonadiItem();
}

void MergeContactWidget::slotUpdateMergeButton()
{
    const Akonadi::Item::List listCheckedItems = listSelectedContacts();
    Akonadi::Item item = currentItem();
    Q_EMIT contactSelected(item);
    mMergeButton->setEnabled(listCheckedItems.count() >= 2);
}

void MergeContactWidget::slotMergeContacts()
{
    const Akonadi::Item::List lstItems = listSelectedContacts();
    const Akonadi::Collection col = mCollectionCombobox->currentCollection();
    if (col.isValid()) {
        if (!lstItems.isEmpty()) {
            Q_EMIT mergeContact(lstItems, col);
        }
    }
}
