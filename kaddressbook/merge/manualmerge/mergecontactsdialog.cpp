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

#include <AkonadiCore/Item>

#include <KConfigGroup>
#include <KSharedConfig>
#include <KLocalizedString>
#include <KMessageBox>

#include <QLabel>
#include <QSplitter>
#include <QPointer>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QStackedWidget>

using namespace KABMergeContacts;
MergeContactsDialog::MergeContactsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Select Contacts to merge"));
    mButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    connect(mButtonBox, &QDialogButtonBox::accepted, this, &MergeContactsDialog::accept);
    connect(mButtonBox, &QDialogButtonBox::rejected, this, &MergeContactsDialog::reject);
    readConfig();

    mStackedWidget = new QStackedWidget(this);
    mStackedWidget->setObjectName(QStringLiteral("stackedwidget"));
    mainLayout->addWidget(mStackedWidget);

    mNoEnoughContactSelected = new KABMergeContacts::MergeContactErrorLabel(KABMergeContacts::MergeContactErrorLabel::NotEnoughContactsSelected);
    mNoEnoughContactSelected->setObjectName(QStringLiteral("notenoughcontactselected"));
    mStackedWidget->addWidget(mNoEnoughContactSelected);

    mNoContactSelected = new KABMergeContacts::MergeContactErrorLabel(MergeContactErrorLabel::NoContactSelected, this);
    mNoContactSelected->setObjectName(QStringLiteral("nocontactselected"));
    mStackedWidget->addWidget(mNoContactSelected);

    mManualMergeResultWidget = new KABMergeContacts::MergeContactWidget(this);
    mManualMergeResultWidget->setObjectName(QStringLiteral("manualmergeresultwidget"));
    mStackedWidget->addWidget(mManualMergeResultWidget);
    connect(mManualMergeResultWidget, &MergeContactWidget::customizeMergeContact, this, &MergeContactsDialog::slotCustomizeMergeContact);
    connect(mManualMergeResultWidget, &MergeContactWidget::contactMerged, this, &MergeContactsDialog::slotContactMerged);

    mSelectInformation = new KABMergeContacts::MergeContactSelectInformationScrollArea(this);
    mSelectInformation->setObjectName(QStringLiteral("selectioninformation"));
    mStackedWidget->addWidget(mSelectInformation);

    mMergeContactInfo = new KABMergeContacts::MergeContactInfoWidget;
    mMergeContactInfo->setObjectName(QStringLiteral("mergecontactinfowidget"));
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
    } else if (list.count() < 2) {
        mStackedWidget->setCurrentWidget(mNoEnoughContactSelected);
    } else {
        mManualMergeResultWidget->setContacts(list);
        mStackedWidget->setCurrentWidget(mManualMergeResultWidget);
    }
    mButtonBox->button(QDialogButtonBox::Close)->setEnabled(true);
}

void MergeContactsDialog::slotCustomizeMergeContact(const Akonadi::Item::List &lst, MergeContacts::ConflictInformations conflictType, const Akonadi::Collection &col)
{
    mSelectInformation->setContacts(conflictType, lst, col);
    mStackedWidget->setCurrentWidget(mSelectInformation);
}

void MergeContactsDialog::readConfig()
{
    KConfigGroup grp(KSharedConfig::openConfig(), "MergeContactsDialog");
    const QSize size = grp.readEntry("Size", QSize(300, 200));
    if (size.isValid()) {
        resize(size);
    }
}

void MergeContactsDialog::writeConfig()
{
    KConfigGroup grp(KSharedConfig::openConfig(), "MergeContactsDialog");
    grp.writeEntry("Size", size());
    grp.sync();
}

void MergeContactsDialog::slotContactMerged(const Akonadi::Item &item)
{
    mMergeContactInfo->setContact(item);
    mStackedWidget->setCurrentWidget(mMergeContactInfo);
}
