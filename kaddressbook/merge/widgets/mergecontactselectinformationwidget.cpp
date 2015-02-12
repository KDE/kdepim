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

#include "mergecontactselectinformationwidget.h"
#include "merge/widgets/mergecontactselectlistwidget.h"
#include <KLocalizedString>
#include <QVBoxLayout>
#include <QLabel>
#include <QTreeWidget>

using namespace KABMergeContacts;
MergeContactSelectInformationWidget::MergeContactSelectInformationWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
}

MergeContactSelectInformationWidget::~MergeContactSelectInformationWidget()
{

}

void MergeContactSelectInformationWidget::setContacts(MergeContacts::ConflictInformations conflictTypes, const Akonadi::Item::List &listItem)
{
    Q_FOREACH(const Akonadi::Item &item, listItem) {
        if (item.hasPayload<KABC::Addressee>()) {
            const KABC::Addressee address = item.payload<KABC::Addressee>();
            mAddressList.append(address);
        }
    }
    if (conflictTypes & MergeContacts::Birthday) {
        addInformationWidget(MergeContacts::Birthday);
    }
    if (conflictTypes & MergeContacts::Geo) {
        addInformationWidget(MergeContacts::Geo);
    }
    if (conflictTypes & MergeContacts::Photo) {
        addInformationWidget(MergeContacts::Photo);
    }
    if (conflictTypes & MergeContacts::Logo) {
        addInformationWidget(MergeContacts::Logo);
    }
}

void MergeContactSelectInformationWidget::addInformationWidget(MergeContacts::ConflictInformation conflictType)
{
    MergeContactSelectListWidget *widget = new MergeContactSelectListWidget;
    widget->setContacts(conflictType, mAddressList);
    layout()->addWidget(widget);
    mListMergeSelectInformation.append(widget);
}

void MergeContactSelectInformationWidget::createContact(KABC::Addressee &addr)
{
    Q_FOREACH(MergeContactSelectListWidget *listWidget, mListMergeSelectInformation) {
        const int selectedContact = listWidget->selectedContact();
        const MergeContacts::ConflictInformation conflictType = listWidget->conflictType();
        if (selectedContact != -1) {
            //TODO
        }
    }
}
