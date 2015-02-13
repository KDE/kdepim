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

#include "searchduplicateresultwidget.h"
#include "merge/widgets/mergecontactloseinformationwarning.h"
#include "merge/job/mergecontactsjob.h"
#include "resultduplicatetreewidget.h"
#include <KLocalizedString>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QSplitter>
#include <QLabel>
#include <QPushButton>
#include <kaddressbookgrantlee/widget/grantleecontactviewer.h>
#include <AkonadiWidgets/CollectionComboBox>

namespace KABMergeContacts
{
KADDRESSBOOK_EXPORT QAbstractItemModel *_k_searchDuplicateResultStubModel = Q_NULLPTR;
}

using namespace KABMergeContacts;
SearchDuplicateResultWidget::SearchDuplicateResultWidget(QWidget *parent)
    : QWidget(parent),
      mIndexListContact(0)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QSplitter *splitter = new QSplitter;
    splitter->setObjectName(QLatin1String("splitter"));
    splitter->setChildrenCollapsible(false);
    mainLayout->addWidget(splitter);
    mResult = new ResultDuplicateTreeWidget;
    mResult->setObjectName(QLatin1String("result_treewidget"));
    mContactViewer = new KAddressBookGrantlee::GrantleeContactViewer;
    mContactViewer->setObjectName(QLatin1String("contact_viewer"));
    splitter->addWidget(mResult);
    splitter->addWidget(mContactViewer);
    connect(mResult, &ResultDuplicateTreeWidget::showContactPreview, mContactViewer, &KAddressBookGrantlee::GrantleeContactViewer::setContact);

    mMergeContactWarning = new MergeContactLoseInformationWarning;
    mMergeContactWarning->setObjectName(QLatin1String("mergecontactwarning"));
    connect(mMergeContactWarning, &MergeContactLoseInformationWarning::continueMerging, this, &SearchDuplicateResultWidget::slotAutomaticMerging);
    connect(mMergeContactWarning, &MergeContactLoseInformationWarning::customizeMergingContacts, this, &SearchDuplicateResultWidget::slotCustomizeMergingContacts);
    mainLayout->addWidget(mMergeContactWarning);

    QHBoxLayout *mergeLayout = new QHBoxLayout;
    mainLayout->addLayout(mergeLayout);
    mergeLayout->addStretch();

    QLabel *lab = new QLabel(i18n("Select AddressBook:"));
    lab->setObjectName(QLatin1String("select_addressbook_label"));
    mergeLayout->addWidget(lab);

    mCollectionCombobox = new Akonadi::CollectionComboBox(_k_searchDuplicateResultStubModel);
    mCollectionCombobox->setAccessRightsFilter(Akonadi::Collection::CanCreateItem);
    mCollectionCombobox->setMinimumWidth(250);
    mCollectionCombobox->setMimeTypeFilter(QStringList() << KContacts::Addressee::mimeType());
    mCollectionCombobox->setObjectName(QLatin1String("akonadicombobox"));
    connect(mCollectionCombobox, static_cast<void (Akonadi::CollectionComboBox::*)(int)>(&Akonadi::CollectionComboBox::currentIndexChanged), this, &SearchDuplicateResultWidget::slotUpdateMergeButton);
    connect(mCollectionCombobox, static_cast<void (Akonadi::CollectionComboBox::*)(int)>(&Akonadi::CollectionComboBox::activated), this, &SearchDuplicateResultWidget::slotUpdateMergeButton);
    mergeLayout->addWidget(mCollectionCombobox);

    mMergeContact = new QPushButton(i18n("Merge"));
    mMergeContact->setObjectName(QLatin1String("merge_contact_button"));
    connect(mMergeContact, &QPushButton::clicked, this, &SearchDuplicateResultWidget::slotMergeContact);
    mergeLayout->addWidget(mMergeContact);
    mMergeContact->setEnabled(false);
    //TODO make mMergeContact enable when selected item and collection valid
}

SearchDuplicateResultWidget::~SearchDuplicateResultWidget()
{

}

void SearchDuplicateResultWidget::setContacts(const QList<Akonadi::Item::List> &lstItem)
{
    mResult->setContacts(lstItem);
}

void SearchDuplicateResultWidget::slotMergeContact()
{
    mIndexListContact = 0;
    mListContactToMerge = mResult->selectedContactsToMerge();
    if (!mListContactToMerge.isEmpty()) {
        mMergeContact->setEnabled(false);
        //Detect if conflict.
        mergeContact();
    }
}

void SearchDuplicateResultWidget::mergeContact()
{
    //TODO add progress indicator.
    if (mIndexListContact < mListContactToMerge.count()) {
        KABMergeContacts::MergeContactsJob *job = new KABMergeContacts::MergeContactsJob(this);
        job->setListItem(mListContactToMerge.at(mIndexListContact));
        job->setDestination(mCollectionCombobox->currentCollection());
        connect(job, &KABMergeContacts::MergeContactsJob::finished, this, &SearchDuplicateResultWidget::slotMergeDone);
        job->start();
    } else {
        Q_EMIT mergeDone();
    }
}

void SearchDuplicateResultWidget::slotMergeDone(const Akonadi::Item &item)
{
    ++mIndexListContact;
    Q_EMIT contactMerged(item);
    mergeContact();
}

void SearchDuplicateResultWidget::slotUpdateMergeButton()
{
    mMergeContact->setEnabled(mCollectionCombobox->currentCollection().isValid());
}

void SearchDuplicateResultWidget::slotAutomaticMerging()
{
    mergeContact();
}

void SearchDuplicateResultWidget::slotCustomizeMergingContacts()
{
    //TODO
}
