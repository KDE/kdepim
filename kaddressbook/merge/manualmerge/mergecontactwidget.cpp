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

#include <KABC/Addressee>

#include <Akonadi/CollectionComboBox>
#include <QListWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>
#include <QLabel>
#include <kaddressbook/merge/widgets/mergecontactinfowidget.h>
#include <kaddressbook/merge/widgets/mergecontactloseinformationwarning.h>
#include <kaddressbook/merge/job/mergecontacts.h>
#include <kaddressbook/merge/job/mergecontactsjob.h>

namespace KABMergeContacts {
KADDRESSBOOK_EXPORT QAbstractItemModel *_k_mergeStubModel = 0;
}


using namespace KABMergeContacts;
MergeContactWidget::MergeContactWidget(QWidget *parent)
    : QWidget(parent),
      mConflictTypes(MergeContacts::None)
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
    mListWidget->setObjectName(QLatin1String("listcontact"));
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

    lab = new QLabel(i18n("Select addressbook where to store merged contact:"));
    hbox->addWidget(lab);

    mCollectionCombobox = new Akonadi::CollectionComboBox(_k_mergeStubModel);
    mCollectionCombobox->setAccessRightsFilter(Akonadi::Collection::CanCreateItem);
    mCollectionCombobox->setMinimumWidth(250);
    mCollectionCombobox->setMimeTypeFilter( QStringList() << KABC::Addressee::mimeType() );
    mCollectionCombobox->setObjectName(QLatin1String("akonadicombobox"));
    connect(mCollectionCombobox, SIGNAL(currentIndexChanged(int)), SLOT(slotUpdateMergeButton()));
    connect(mCollectionCombobox, SIGNAL(activated(int)), SLOT(slotUpdateMergeButton()));

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
    mSelectedItems = listSelectedContacts();
    const Akonadi::Collection col = mCollectionCombobox->currentCollection();

    KABMergeContacts::MergeContacts mergeContacts;
    mergeContacts.setItems(mSelectedItems);
    mConflictTypes = mergeContacts.needManualSelectInformations();
    if (mConflictTypes != MergeContacts::None) {
        mMergeContactWarning->animatedShow();
    } else {
        slotAutomaticMerging();
    }
}

void MergeContactWidget::slotAutomaticMerging()
{
    KABMergeContacts::MergeContactsJob *job = new KABMergeContacts::MergeContactsJob(this);
    job->setListItem(mSelectedItems);
    job->setDestination(mCollectionCombobox->currentCollection());
    connect(job, SIGNAL(finished(Akonadi::Item)), this, SLOT(slotMergeDone(Akonadi::Item)));
    job->start();
}

void MergeContactWidget::slotCustomizeMergingContacts()
{
    Q_EMIT customizeMergeContact(mSelectedItems, mConflictTypes, mCollectionCombobox->currentCollection());
}

void MergeContactWidget::slotMergeDone(const Akonadi::Item &item)
{
    Q_EMIT contactMerged(item);
}
