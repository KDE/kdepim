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


#include "searchandmergecontactduplicatecontactdialog.h"

#include "mergecontactselectinformationtabwidget.h"
#include "merge/searchduplicate/searchduplicateresultwidget.h"
#include "merge/widgets/mergecontactshowresulttabwidget.h"
#include "merge/job/searchpotentialduplicatecontactjob.h"

#include <KLocalizedString>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KGlobal>

#include <QStackedWidget>
#include <QLabel>

using namespace KABMergeContacts;

SearchAndMergeContactDuplicateContactDialog::SearchAndMergeContactDuplicateContactDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Select Contacts to merge" ) );
    setButtons( Close );
    mStackedWidget = new QStackedWidget(this);
    mStackedWidget->setObjectName(QLatin1String("stackedwidget"));

    mSearchResult = new SearchDuplicateResultWidget;
    mSearchResult->setObjectName(QLatin1String("mergecontact"));
    mStackedWidget->addWidget(mSearchResult);
    connect(mSearchResult, SIGNAL(contactMerged(Akonadi::Item)), this, SLOT(slotContactMerged(Akonadi::Item)));
    connect(mSearchResult, SIGNAL(mergeDone()), this, SLOT(slotMergeDone()));
    connect(mSearchResult, SIGNAL(customizeMergeContact(QList<KABMergeContacts::MergeConflictResult>)), this, SLOT(slotCustomizeMergeContacts(QList<KABMergeContacts::MergeConflictResult>)));

    mNoContactSelected = new QLabel(i18n("No contacts selected."));
    mNoContactSelected->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    mNoContactSelected->setObjectName(QLatin1String("nocontactselected"));
    QFont font;
    font.setBold(true);
    mNoContactSelected->setFont(font);
    mStackedWidget->addWidget(mNoContactSelected);

    mNoDuplicateContactFound = new QLabel(i18n("No contact duplicated found."));
    mNoDuplicateContactFound->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    mNoDuplicateContactFound->setObjectName(QLatin1String("noduplicatecontactfound"));
    mNoDuplicateContactFound->setFont(font);
    mStackedWidget->addWidget(mNoDuplicateContactFound);

    mMergeContactResult = new MergeContactShowResultTabWidget(this);
    mMergeContactResult->setObjectName(QLatin1String("mergecontactresult"));
    mStackedWidget->addWidget(mMergeContactResult);

    mNoEnoughContactSelected = new QLabel(i18n("You must select at least two elements."));
    mNoEnoughContactSelected->setObjectName(QLatin1String("noenoughcontactselected"));
    mNoEnoughContactSelected->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    mNoEnoughContactSelected->setFont(font);
    mStackedWidget->addWidget(mNoEnoughContactSelected);
    mStackedWidget->setCurrentWidget(mNoContactSelected);

    mSelectInformation = new KABMergeContacts::MergeContactSelectInformationTabWidget(this);
    mSelectInformation->setObjectName(QLatin1String("selectioninformation"));
    mStackedWidget->addWidget(mSelectInformation);

    setMainWidget(mStackedWidget);
    readConfig();
}

SearchAndMergeContactDuplicateContactDialog::~SearchAndMergeContactDuplicateContactDialog()
{
    writeConfig();
}

void SearchAndMergeContactDuplicateContactDialog::searchPotentialDuplicateContacts(const Akonadi::Item::List &list)
{
    if (list.isEmpty()) {
        mStackedWidget->setCurrentWidget(mNoContactSelected);
    } else if (list.count() < 2){
        mStackedWidget->setCurrentWidget(mNoEnoughContactSelected);
    } else {
        SearchPotentialDuplicateContactJob *job = new SearchPotentialDuplicateContactJob(list, this);
        connect(job,SIGNAL(finished(QList<Akonadi::Item::List>)), this, SLOT(slotDuplicateFound(QList<Akonadi::Item::List>)));
        job->start();
    }
}

void SearchAndMergeContactDuplicateContactDialog::readConfig()
{
    KConfigGroup grp( KGlobal::config(), "MergeContactDuplicateContactDialog" );
    const QSize size = grp.readEntry( "Size", QSize(300, 200) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void SearchAndMergeContactDuplicateContactDialog::writeConfig()
{
    KConfigGroup grp( KGlobal::config(), "MergeContactDuplicateContactDialog");
    grp.writeEntry( "Size", size() );
    grp.sync();
}

void SearchAndMergeContactDuplicateContactDialog::slotDuplicateFound(const QList<Akonadi::Item::List> &duplicate)
{
    if (duplicate.isEmpty()) {
        mStackedWidget->setCurrentWidget(mNoDuplicateContactFound);
    } else {
        mStackedWidget->setCurrentWidget(mSearchResult);
        mSearchResult->setContacts(duplicate);
    }
}

void SearchAndMergeContactDuplicateContactDialog::slotContactMerged(const Akonadi::Item &item)
{
    mMergeContactResult->addMergedContact(item, true);
}

void SearchAndMergeContactDuplicateContactDialog::slotMergeDone()
{
    mStackedWidget->setCurrentWidget(mMergeContactResult);
}

void SearchAndMergeContactDuplicateContactDialog::slotCustomizeMergeContacts(const QList<MergeConflictResult> &lst)
{
    //TODO add infos
    mSelectInformation->setNeedSelectInformationWidgets(lst);
    mStackedWidget->setCurrentWidget(mSelectInformation);
}
