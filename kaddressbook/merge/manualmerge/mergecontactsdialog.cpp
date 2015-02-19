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

#include "mergecontactsdialog.h"
#include "utils.h"
#include "merge/manualmerge/mergecontactwidget.h"
#include "merge/widgets/mergecontactinfowidget.h"
#include "merge/job/mergecontactsjob.h"
#include "merge/widgets/mergecontacterrorlabel.h"
#include "merge/widgets/mergecontactselectinformationscrollarea.h"
#include <kaddressbook/merge/widgets/mergecontactselectinformationscrollarea.h>

#include <Akonadi/Item>

#include <KConfigGroup>
#include <KSharedConfig>
#include <KLocalizedString>
#include <KMessageBox>

#include <QItemSelectionModel>
#include <QLabel>
#include <QSplitter>
#include <QHBoxLayout>
#include <QPointer>
#include <QStackedWidget>

using namespace KABMergeContacts;
MergeContactsDialog::MergeContactsDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Select Contacts to merge" ) );
    setButtons( Close );
    readConfig();

    mStackedWidget = new QStackedWidget(this);
    mStackedWidget->setObjectName(QLatin1String("stackedwidget"));
    setMainWidget(mStackedWidget);

    mNoEnoughContactSelected = new KABMergeContacts::MergeContactErrorLabel(KABMergeContacts::MergeContactErrorLabel::NotEnoughContactsSelected);
    mNoEnoughContactSelected->setObjectName(QLatin1String("notenoughcontactselected"));
    mStackedWidget->addWidget(mNoEnoughContactSelected);

    mNoContactSelected = new KABMergeContacts::MergeContactErrorLabel(MergeContactErrorLabel::NoContactSelected, this);
    mNoContactSelected->setObjectName(QLatin1String("nocontactselected"));
    mStackedWidget->addWidget(mNoContactSelected);

    mManualMergeResultWidget = new KABMergeContacts::MergeContactWidget(this);
    mManualMergeResultWidget->setObjectName(QLatin1String("manualmergeresultwidget"));
    mStackedWidget->addWidget(mManualMergeResultWidget);
    connect(mManualMergeResultWidget, SIGNAL(customizeMergeContact(Akonadi::Item::List,MergeContacts::ConflictInformations,Akonadi::Collection)),
            this, SLOT(slotCustomizeMergeContact(Akonadi::Item::List,MergeContacts::ConflictInformations,Akonadi::Collection)));
    connect(mManualMergeResultWidget, SIGNAL(contactMerged(Akonadi::Item)),
            this, SLOT(slotContactMerged(Akonadi::Item)));

    mSelectInformation = new KABMergeContacts::MergeContactSelectInformationScrollArea(this);
    mSelectInformation->setObjectName(QLatin1String("selectioninformation"));
    mStackedWidget->addWidget(mSelectInformation);


    mMergeContactInfo = new KABMergeContacts::MergeContactInfoWidget;
    mMergeContactInfo->setObjectName(QLatin1String("mergecontactinfowidget"));
    mStackedWidget->addWidget(mMergeContactInfo);


    mStackedWidget->setCurrentWidget(mNoContactSelected);
}

MergeContactsDialog::~MergeContactsDialog()
{
    writeConfig();
}

void MergeContactsDialog::setContacts(const Akonadi::Item::List &list)
{
    if (list.isEmpty()) {
        mStackedWidget->setCurrentWidget(mNoContactSelected);
    } else if (list.count() < 2){
        mStackedWidget->setCurrentWidget(mNoEnoughContactSelected);
    } else {
        mManualMergeResultWidget->setContacts(list);
        mStackedWidget->setCurrentWidget(mManualMergeResultWidget);
    }
}

void MergeContactsDialog::slotCustomizeMergeContact(const Akonadi::Item::List &lst, MergeContacts::ConflictInformations conflictType, const Akonadi::Collection &col)
{
    mSelectInformation->setContacts(conflictType, lst, col);
    mStackedWidget->setCurrentWidget(mSelectInformation);
}

void MergeContactsDialog::readConfig()
{
    KConfigGroup grp( KGlobal::config(), "MergeContactsDialog" );
    const QSize size = grp.readEntry( "Size", QSize(300, 200) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void MergeContactsDialog::writeConfig()
{
    KConfigGroup grp( KGlobal::config(), "MergeContactsDialog");
    grp.writeEntry( "Size", size() );
    grp.sync();
}


void MergeContactsDialog::slotContactMerged(const Akonadi::Item &item)
{
    mMergeContactInfo->setContact(item);
    mStackedWidget->setCurrentWidget(mMergeContactInfo);
}
