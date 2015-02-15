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
#include "merge/job/mergecontacts.h"
#include "resultduplicatetreewidget.h"
#include <KLocalizedString>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QSplitter>
#include <QLabel>
#include <KPushButton>
#include <kaddressbookgrantlee/widget/grantleecontactviewer.h>
#include <akonadi/collectioncombobox.h>


namespace KABMergeContacts {
KADDRESSBOOK_EXPORT QAbstractItemModel *_k_searchDuplicateResultStubModel = 0;
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
    connect(mResult, SIGNAL(showContactPreview(Akonadi::Item)), mContactViewer, SLOT(setContact(Akonadi::Item)));

    mMergeContactWarning = new MergeContactLoseInformationWarning;
    mMergeContactWarning->setObjectName(QLatin1String("mergecontactwarning"));
    connect(mMergeContactWarning, SIGNAL(continueMerging()), this, SLOT(slotAutomaticMerging()));
    connect(mMergeContactWarning, SIGNAL(customizeMergingContacts()), this, SLOT(slotCustomizeMergingContacts()));
    mainLayout->addWidget(mMergeContactWarning);

    QHBoxLayout *mergeLayout = new QHBoxLayout;
    mainLayout->addLayout(mergeLayout);
    mergeLayout->addStretch();
    //KF5 add i18n
    QLabel *lab = new QLabel(QLatin1String("Select AddressBook:"));
    lab->setObjectName(QLatin1String("select_addressbook_label"));
    mergeLayout->addWidget(lab);

    mCollectionCombobox = new Akonadi::CollectionComboBox(_k_searchDuplicateResultStubModel);
    mCollectionCombobox->setAccessRightsFilter(Akonadi::Collection::CanCreateItem);
    mCollectionCombobox->setMinimumWidth(250);
    mCollectionCombobox->setMimeTypeFilter( QStringList() << KABC::Addressee::mimeType() );
    mCollectionCombobox->setObjectName(QLatin1String("akonadicombobox"));
    connect(mCollectionCombobox, SIGNAL(currentIndexChanged(int)), SLOT(slotUpdateMergeButton()));
    connect(mCollectionCombobox, SIGNAL(activated(int)), SLOT(slotUpdateMergeButton()));
    mergeLayout->addWidget(mCollectionCombobox);

    //KF5 add i18n
    mMergeContact = new KPushButton(QLatin1String("Merge"));
    mMergeContact->setObjectName(QLatin1String("merge_contact_button"));
    connect(mMergeContact, SIGNAL(clicked()), this, SLOT(slotMergeContact()));
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
        KABMergeContacts::MergeContacts mergeContacts;
        bool conflictFound = false;
        mResultConflictList.clear();
        Q_FOREACH(const Akonadi::Item::List & lst, mListContactToMerge) {
            mergeContacts.setItems(lst);
            const MergeContacts::ConflictInformations conflicts = mergeContacts.needManualSelectInformations();
            if (conflicts != MergeContacts::None) {
                conflictFound = true;
            }
            MergeConflictResult result;
            result.list = lst;
            result.conflictInformation = conflicts;
            mResultConflictList.append(result);
        }

        mMergeContact->setEnabled(false);
        if (!conflictFound) {
            //Detect if conflict.
            mergeContact();
        } else {
            mMergeContactWarning->animatedShow();
        }
    }
}

void SearchDuplicateResultWidget::mergeContact()
{
    //TODO add progress indicator.
    if (mIndexListContact < mListContactToMerge.count()) {
        KABMergeContacts::MergeContactsJob *job = new KABMergeContacts::MergeContactsJob(this);
        job->setListItem(mListContactToMerge.at(mIndexListContact));
        job->setDestination(mCollectionCombobox->currentCollection());
        connect(job, SIGNAL(finished(Akonadi::Item)), this, SLOT(slotMergeDone(Akonadi::Item)));
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
    //TODO send info too
    Q_EMIT customizeMergeContact(mResultConflictList, mCollectionCombobox->currentCollection());
    //TODO
}
