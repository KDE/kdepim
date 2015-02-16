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
#include <QSplitter>

#include <KContacts/Addressee>

#include <AkonadiWidgets/CollectionComboBox>
#include <QListWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <kaddressbook/merge/widgets/mergecontactinfowidget.h>
#include <kaddressbook/merge/widgets/mergecontactloseinformationwarning.h>
#include <kaddressbook/merge/job/mergecontacts.h>

namespace KABMergeContacts
{
KADDRESSBOOK_EXPORT QAbstractItemModel *_k_mergeStubModel = Q_NULLPTR;
}

using namespace KABMergeContacts;
MergeContactWidget::MergeContactWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *lay = new QVBoxLayout;

    QSplitter *splitter = new QSplitter;
    splitter->setObjectName(QLatin1String("splitter"));
    splitter->setChildrenCollapsible(false);
    lay->addWidget(splitter);


    QWidget *selectContactWidget = new QWidget;
    selectContactWidget->setObjectName(QLatin1String("selectcontactwidget"));
    QVBoxLayout *vbox = new QVBoxLayout;
    selectContactWidget->setLayout(vbox);
    QLabel *lab = new QLabel(i18n("Select contacts that you want really to merge:"));
    vbox->addWidget(lab);
    mListWidget = new MergeContactWidgetList;
    mListWidget->setObjectName(QStringLiteral("listcontact"));
    mListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    vbox->addWidget(mListWidget);
    connect(mListWidget, SIGNAL(itemSelectionChanged()), SLOT(slotUpdateMergeButton()));
    connect(mListWidget, SIGNAL(itemChanged(QListWidgetItem*)), SLOT(slotUpdateMergeButton()));
    splitter->addWidget(selectContactWidget);

    mMergeContactInfoWidget = new MergeContactInfoWidget;
    mMergeContactInfoWidget->setObjectName(QLatin1String("mergecontactinfowidget"));
    splitter->addWidget(mMergeContactInfoWidget);

    mMergeContactWarning = new MergeContactLoseInformationWarning;
    mMergeContactWarning->setObjectName(QLatin1String("mergecontactwarning"));
    connect(mMergeContactWarning, SIGNAL(continueMerging()), this, SLOT(slotAutomaticMerging()));
    connect(mMergeContactWarning, SIGNAL(customizeMergingContacts()), this, SLOT(slotCustomizeMergingContacts()));
    lay->addWidget(mMergeContactWarning);


    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addStretch();

    lab = new QLabel(i18n("Select the addressbook in which to store merged contacts:"));
    hbox->addWidget(lab);

    mCollectionCombobox = new Akonadi::CollectionComboBox(_k_mergeStubModel);
    mCollectionCombobox->setAccessRightsFilter(Akonadi::Collection::CanCreateItem);
    mCollectionCombobox->setMinimumWidth(250);
    mCollectionCombobox->setMimeTypeFilter(QStringList() << KContacts::Addressee::mimeType());
    mCollectionCombobox->setObjectName(QStringLiteral("akonadicombobox"));
    connect(mCollectionCombobox, static_cast<void (Akonadi::CollectionComboBox::*)(int)>(&Akonadi::CollectionComboBox::currentIndexChanged), this, &MergeContactWidget::slotUpdateMergeButton);
    connect(mCollectionCombobox, static_cast<void (Akonadi::CollectionComboBox::*)(int)>(&Akonadi::CollectionComboBox::activated), this, &MergeContactWidget::slotUpdateMergeButton);

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
}

MergeContactWidget::~MergeContactWidget()
{

}

void MergeContactWidget::clear()
{
    mListWidget->clear();
}

void MergeContactWidget::setContacts(const Akonadi::Item::List &items)
{
    mItems = items;
    fillListContact();
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
    mMergeContactInfoWidget->setContact(item);
    mMergeButton->setEnabled((listCheckedItems.count()>=2) && mCollectionCombobox->currentCollection().isValid());
}

void MergeContactWidget::slotMergeContacts()
{
    const Akonadi::Item::List lstItems = listSelectedContacts();
    const Akonadi::Collection col = mCollectionCombobox->currentCollection();

    KABMergeContacts::MergeContacts mergeContacts;
    mergeContacts.setItems(lstItems);
    const MergeContacts::ConflictInformations conflicts = mergeContacts.needManualSelectInformations();
    if (conflicts != MergeContacts::None) {
        mMergeContactWarning->animatedShow();
    } else {
        //Merge
    }

    /*
    MergeConflictResult result;
    result.list = lst;
    result.conflictInformation = conflicts;
    mResultConflictList.append(result);

    mMergeContact->setEnabled(false);
    if (!conflictFound) {
        //Detect if conflict.
        mergeContact();
    } else {
        mMergeContactWarning->animatedShow();
    }
}
    */
#if 0


    mMergeButton->setEnabled(false);
    mCollectionCombobox->setEnabled(false);

    const Akonadi::Item::List lstItems = listSelectedContacts();
    const Akonadi::Collection col = mCollectionCombobox->currentCollection();
    if (col.isValid()) {
        if (!lstItems.isEmpty()) {
            Q_EMIT mergeContact(lstItems, col);
        }
    }
#endif
}

void MergeContactWidget::slotAutomaticMerging()
{
    //TODO
}

void MergeContactWidget::slotCustomizeMergingContacts()
{
    //TODO
}
