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

#include "mergecontactwidget.h"
#include "mergecontactwidgetlist.h"

#include <KLocalizedString>

#include <KABC/Addressee>

#include <AkonadiWidgets/CollectionComboBox>
#include <QListWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>
#include <QLabel>

namespace KABMergeContacts {
KADDRESSBOOK_EXPORT QAbstractItemModel *_k_mergeStubModel = 0;
}


using namespace KABMergeContacts;
MergeContactWidget::MergeContactWidget(const Akonadi::Item::List &items, QWidget *parent)
    : QWidget(parent),
      mItems(items)
{
    QVBoxLayout *lay = new QVBoxLayout;

    QLabel *lab = new QLabel(i18n("Select contacts that you want really to merge:"));
    lay->addWidget(lab);
    mListWidget = new MergeContactWidgetList;
    mListWidget->setObjectName(QLatin1String("listcontact"));
    mListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    lay->addWidget(mListWidget);
    connect(mListWidget, SIGNAL(itemSelectionChanged()), SLOT(slotUpdateMergeButton()));
    connect(mListWidget, SIGNAL(itemChanged(QListWidgetItem*)), SLOT(slotUpdateMergeButton()));

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addStretch();

    lab = new QLabel(i18n("Select addressbook where to store merged contact:"));
    hbox->addWidget(lab);

    mCollectionCombobox = new Akonadi::CollectionComboBox(_k_mergeStubModel);
    mCollectionCombobox->setAccessRightsFilter(Akonadi::Collection::CanCreateItem);
    mCollectionCombobox->setMinimumWidth(250);
    mCollectionCombobox->setMimeTypeFilter( QStringList() << KABC::Addressee::mimeType() );
    mCollectionCombobox->setObjectName(QLatin1String("akonadicombobox"));
    hbox->addWidget(mCollectionCombobox);

    lay->addLayout(hbox);

    hbox = new QHBoxLayout;
    hbox->addStretch();
    mMergeButton = new QPushButton(i18n("merge"));
    mMergeButton->setObjectName(QLatin1String("mergebutton"));
    hbox->addWidget(mMergeButton);
    mMergeButton->setEnabled(false);
    lay->addLayout(hbox);

    connect(mMergeButton, SIGNAL(clicked()), this, SLOT(slotMergeContacts()));

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
    mMergeButton->setEnabled(listCheckedItems.count()>=2);
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
