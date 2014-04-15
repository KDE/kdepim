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

#include <KLocalizedString>

#include <QListWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>

//TODO add delegate to show address info.
using namespace KABMergeContacts;
MergeContactWidget::MergeContactWidget(const Akonadi::Item::List &items, QWidget *parent)
    : QWidget(parent),
      mItems(items)
{
    QVBoxLayout *lay = new QVBoxLayout;
    mListWidget = new QListWidget;
    mListWidget->setObjectName(QLatin1String("listcontact"));
    mListWidget->setSelectionMode(QAbstractItemView::MultiSelection);
    lay->addWidget(mListWidget);
    connect(mListWidget, SIGNAL(itemSelectionChanged()), SLOT(slotUpdateMergeButton()));

    QHBoxLayout *hbox = new QHBoxLayout;
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

void MergeContactWidget::fillListContact()
{
    Q_FOREACH(const Akonadi::Item &item, mItems) {
        MergeContactWidgetItem *widgetItem = new MergeContactWidgetItem(item, mListWidget);
        widgetItem->setText(QString::number(item.id()));
    }
}

void MergeContactWidget::slotUpdateMergeButton()
{
    mMergeButton->setEnabled(!mListWidget->selectedItems().isEmpty());
}

void MergeContactWidget::slotMergeContacts()
{
    Akonadi::Item::List lstItems;
    Q_FOREACH(QListWidgetItem *item, mListWidget->selectedItems()) {
        lstItems.append((static_cast<MergeContactWidgetItem*>(item))->item());
    }
    if (!lstItems.isEmpty()) {
        Q_EMIT mergeContact(lstItems);
    }
}


MergeContactWidgetItem::MergeContactWidgetItem(const Akonadi::Item &item, QListWidget *parent)
    : QListWidgetItem(parent),
      mItem(item)
{

}

Akonadi::Item MergeContactWidgetItem::item() const
{
    return mItem;
}
