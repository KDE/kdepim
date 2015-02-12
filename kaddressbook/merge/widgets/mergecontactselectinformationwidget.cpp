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
    if (conflictTypes & MergeContacts::Birthday) {
        addInformationWidget(MergeContacts::Birthday, listItem);
    }
    if (conflictTypes & MergeContacts::Geo) {
        addInformationWidget(MergeContacts::Geo, listItem);
    }
    if (conflictTypes & MergeContacts::Photo) {
        addInformationWidget(MergeContacts::Photo, listItem);
    }
    if (conflictTypes & MergeContacts::Logo) {
        addInformationWidget(MergeContacts::Logo, listItem);
    }
}

void MergeContactSelectInformationWidget::addInformationWidget(MergeContacts::ConflictInformation conflictType, const Akonadi::Item::List &listItem)
{
    MergeContactSelectListWidget *widget = new MergeContactSelectListWidget;
    widget->setContacts(conflictType, listItem);
    layout()->addWidget(widget);
    mListMergeSelectInformation.append(widget);
}

KContacts::Addressee MergeContactSelectInformationWidget::createContact()
{
    //TODO
    return KContacts::Addressee();
}
