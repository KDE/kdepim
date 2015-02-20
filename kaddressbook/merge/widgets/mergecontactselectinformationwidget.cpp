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
    if (conflictTypes & MergeContacts::Anniversary) {
        addInformationWidget(MergeContacts::Anniversary);
    }
    if (conflictTypes & MergeContacts::Name) {
        addInformationWidget(MergeContacts::Name);
    }
    if (conflictTypes & MergeContacts::NickName) {
        addInformationWidget(MergeContacts::NickName);
    }
    if (conflictTypes & MergeContacts::Blog) {
        addInformationWidget(MergeContacts::Blog);
    }
    if (conflictTypes & MergeContacts::HomePage) {
        addInformationWidget(MergeContacts::HomePage);
    }
    if (conflictTypes & MergeContacts::Organization) {
        addInformationWidget(MergeContacts::Organization);
    }
    if (conflictTypes & MergeContacts::Profession) {
        addInformationWidget(MergeContacts::Profession);
    }
    if (conflictTypes & MergeContacts::Title) {
        addInformationWidget(MergeContacts::Title);
    }
    if (conflictTypes & MergeContacts::Departement) {
        addInformationWidget(MergeContacts::Departement);
    }
    if (conflictTypes & MergeContacts::Office) {
        addInformationWidget(MergeContacts::Office);
    }
    if (conflictTypes & MergeContacts::ManagerName) {
        addInformationWidget(MergeContacts::ManagerName);
    }
    if (conflictTypes & MergeContacts::Assistant) {
        addInformationWidget(MergeContacts::Assistant);
    }
    if (conflictTypes & MergeContacts::FreeBusy) {
        addInformationWidget(MergeContacts::FreeBusy);
    }
    if (conflictTypes & MergeContacts::FamilyName) {
        addInformationWidget(MergeContacts::FamilyName);
    }
    if (conflictTypes & MergeContacts::PartnerName) {
        addInformationWidget(MergeContacts::PartnerName);
    }
    if (conflictTypes & MergeContacts::Key) {
        addInformationWidget(MergeContacts::Key);
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
        const int selectedContactIndex = listWidget->selectedContact();
        const MergeContacts::ConflictInformation conflictType = listWidget->conflictType();
        if (selectedContactIndex != -1) {
            switch(conflictType) {
            case MergeContacts::None:
                break;
            case MergeContacts::Birthday:
                addr.setBirthday(mAddressList.at(selectedContactIndex).birthday());
                break;
            case MergeContacts::Geo:
                addr.setGeo(mAddressList.at(selectedContactIndex).geo());
                break;
            case MergeContacts::Photo:
                addr.setPhoto(mAddressList.at(selectedContactIndex).photo());
                break;
            case MergeContacts::Logo:
                addr.setLogo(mAddressList.at(selectedContactIndex).logo());
                break;
            case MergeContacts::Anniversary:
                addr.setBirthday(mAddressList.at(selectedContactIndex).birthday());
                break;
            case MergeContacts::Name:
                addr.setName(mAddressList.at(selectedContactIndex).name());
                break;
            case MergeContacts::NickName:
                addr.setNickName(mAddressList.at(selectedContactIndex).nickName());
                break;
            case MergeContacts::Blog: {
                const QString newBlog = mAddressList.at(selectedContactIndex).custom(QLatin1String( "KADDRESSBOOK" ), QLatin1String( "BlogFeed" ));
                addr.insertCustom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "BlogFeed" ), newBlog );
                break;
            }
            case MergeContacts::HomePage:
                addr.setUrl(mAddressList.at(selectedContactIndex).url());
                break;
            case MergeContacts::Organization:
                addr.setOrganization(mAddressList.at(selectedContactIndex).organization());
                break;
            case MergeContacts::Profession: {
                const QString newValue = mAddressList.at(selectedContactIndex).custom(QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-Profession" ));
                addr.insertCustom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-Profession" ), newValue );
                break;
            }
            case MergeContacts::Title:
                addr.setTitle(mAddressList.at(selectedContactIndex).title());
                break;
            case MergeContacts::Departement:
                addr.setDepartment(mAddressList.at(selectedContactIndex).department());
                break;
            case MergeContacts::Office:{
                const QString newValue = mAddressList.at(selectedContactIndex).custom(QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-Office" ));
                addr.insertCustom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-Office" ), newValue );
                break;
            }
            case MergeContacts::ManagerName: {
                const QString newValue = mAddressList.at(selectedContactIndex).custom(QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-ManagersName" ));
                addr.insertCustom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-ManagersName" ), newValue );
                break;
            }
            case MergeContacts::Assistant: {
                const QString newValue = mAddressList.at(selectedContactIndex).custom(QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-AssistantsName" ));
                addr.insertCustom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-AssistantsName" ), newValue );
                break;
            }
            case MergeContacts::FreeBusy:
                //TODO
                break;
            case MergeContacts::FamilyName:
                addr.setFamilyName(mAddressList.at(selectedContactIndex).familyName());
                break;
            case MergeContacts::PartnerName: {
                const QString newValue = mAddressList.at(selectedContactIndex).custom(QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-SpousesName" ));
                addr.insertCustom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-SpousesName" ), newValue );
                break;
            }
            case MergeContacts::Key:
                addr.setKeys(mAddressList.at(selectedContactIndex).keys());
                break;
            }
        }
    }
}
