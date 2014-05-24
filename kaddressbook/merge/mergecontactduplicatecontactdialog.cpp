/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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


#include "mergecontactduplicatecontactdialog.h"

#include "mergecontactshowresulttabwidget.h"

#include "searchpotentialduplicatecontactjob.h"

#include <KLocalizedString>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KGlobal>

#include <QStackedWidget>
#include <QLabel>

using namespace KABMergeContacts;

MergeContactDuplicateContactDialog::MergeContactDuplicateContactDialog(const Akonadi::Item::List &list, QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Select Contacts to merge" ) );
    setButtons( Close );
    mStackedWidget = new QStackedWidget(this);
    mStackedWidget->setObjectName(QLatin1String("stackedwidget"));

    mMergeContact = new MergeContactShowResultTabWidget;
    mMergeContact->setObjectName(QLatin1String("mergecontact"));
    mStackedWidget->addWidget(mMergeContact);

    mNoContactSelected = new QLabel(i18n("No contacts selected."));
    mNoContactSelected->setObjectName(QLatin1String("nocontactselected"));
    mStackedWidget->addWidget(mNoContactSelected);

    mNoDuplicateContactFound = new QLabel(i18n("No contact duplicated found."));
    mNoDuplicateContactFound->setObjectName(QLatin1String("noduplicatecontactfound"));
    mStackedWidget->addWidget(mNoDuplicateContactFound);


    mNoEnoughContactSelected = new QLabel(i18n("You must select at least two elements."));
    mNoEnoughContactSelected->setObjectName(QLatin1String("noenoughcontactselected"));
    mStackedWidget->addWidget(mNoEnoughContactSelected);


    setMainWidget(mStackedWidget);
    readConfig();
    searchPotentialDuplicateContacts(list);
}

MergeContactDuplicateContactDialog::~MergeContactDuplicateContactDialog()
{

}

void MergeContactDuplicateContactDialog::searchPotentialDuplicateContacts(const Akonadi::Item::List &list)
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

void MergeContactDuplicateContactDialog::readConfig()
{
    KConfigGroup grp( KGlobal::config(), "MergeContactDuplicateContactDialog" );
    const QSize size = grp.readEntry( "Size", QSize(300, 200) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void MergeContactDuplicateContactDialog::writeConfig()
{
    KConfigGroup grp( KGlobal::config(), "MergeContactDuplicateContactDialog");
    grp.writeEntry( "Size", size() );
    grp.sync();
}

void MergeContactDuplicateContactDialog::slotDuplicateFound(const QList<Akonadi::Item::List> &duplicate)
{
    if (duplicate.isEmpty()) {
        mStackedWidget->setCurrentWidget(mNoDuplicateContactFound);
    } else {
        //TODO
    }
}
