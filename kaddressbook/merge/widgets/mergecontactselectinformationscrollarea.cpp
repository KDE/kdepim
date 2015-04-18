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

#include "mergecontactselectinformationscrollarea.h"
#include "mergecontactselectinformationwidget.h"
#include "merge/job/mergecontactsjob.h"
#include "merge/widgets/mergecontactinfowidget.h"
#include "kaddressbook_debug.h"
#include <KLocalizedString>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QStackedWidget>
#include <QPushButton>
#include <KMessageBox>
using namespace KABMergeContacts;

MergeContactSelectInformationScrollArea::MergeContactSelectInformationScrollArea(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *vbox = new QVBoxLayout;
    setLayout(vbox);
    mStackWidget = new QStackedWidget;
    mStackWidget->setObjectName(QStringLiteral("stackwidget"));
    vbox->addWidget(mStackWidget);

    QWidget *selectMergeWidget = new QWidget;
    selectMergeWidget->setObjectName(QStringLiteral("selectwidget"));
    QVBoxLayout *layout = new QVBoxLayout;
    selectMergeWidget->setLayout(layout);
    QScrollArea *area = new QScrollArea;
    area->setWidgetResizable(true);
    area->setObjectName(QStringLiteral("scrollarea"));
    layout->addWidget(area);
    mSelectInformationWidget = new MergeContactSelectInformationWidget;
    mSelectInformationWidget->setObjectName(QStringLiteral("selectinformationwidget"));
    area->setWidget(mSelectInformationWidget);

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addStretch();
    QPushButton *mergeButton = new QPushButton(i18n("Merge"));
    mergeButton->setObjectName(QStringLiteral("merge"));
    hbox->addWidget(mergeButton);
    layout->addLayout(hbox);
    connect(mergeButton, &QPushButton::clicked, this, &MergeContactSelectInformationScrollArea::slotMergeContacts);

    mStackWidget->addWidget(selectMergeWidget);

    mMergedContactWidget = new MergeContactInfoWidget;
    mMergedContactWidget->setObjectName(QStringLiteral("mergedcontactwidget"));
    mStackWidget->addWidget(mMergedContactWidget);
    mStackWidget->setCurrentWidget(selectMergeWidget);
}

MergeContactSelectInformationScrollArea::~MergeContactSelectInformationScrollArea()
{

}

void MergeContactSelectInformationScrollArea::setContacts(MergeContacts::ConflictInformations conflictTypes, const Akonadi::Item::List &listItem, const Akonadi::Collection &col)
{
    mCollection = col;
    mListItem = listItem;
    if (conflictTypes == MergeContacts::None)  {
        MergeContacts contact(mListItem);
        KContacts::Addressee addr = contact.mergedContact();
        mergeContact(addr);
    } else {
        mSelectInformationWidget->setContacts(conflictTypes, listItem);
    }
}

void MergeContactSelectInformationScrollArea::slotMergeContacts()
{
    if (!mCollection.isValid()) {
        qCDebug(KADDRESSBOOK_LOG) << " Invalid colletion";
        return;
    }
    if (mListItem.isEmpty()) {
        qCDebug(KADDRESSBOOK_LOG) << " item list is empty";
        return;
    }
    const bool result = mSelectInformationWidget->verifySelectedInfo();
    if (!result) {
        if (KMessageBox::No == KMessageBox::warningYesNo(this, i18n("Some information was not selected. You can lose this information. Do you want to continue merging?"),
                i18n("Missing Selected Information"))) {
            return;
        }
    }
    MergeContacts contact(mListItem);
    KContacts::Addressee addr = contact.mergedContact(true);
    mSelectInformationWidget->createContact(addr);
    mergeContact(addr);
}

void MergeContactSelectInformationScrollArea::mergeContact(const KContacts::Addressee &addr)
{
    if (!addr.isEmpty()) {
        KABMergeContacts::MergeContactsJob *job = new KABMergeContacts::MergeContactsJob(this);
        job->setNewContact(addr);
        job->setDestination(mCollection);
        job->setListItem(mListItem);
        connect(job, &MergeContactsJob::finished, this, &MergeContactSelectInformationScrollArea::slotMergeDone);
        job->start();
    } else {
        qCDebug(KADDRESSBOOK_LOG) << " Address is empty";
    }
}

void MergeContactSelectInformationScrollArea::slotMergeDone(const Akonadi::Item &item)
{
    mMergedContactWidget->setContact(item);
    mStackWidget->setCurrentWidget(mMergedContactWidget);
}
