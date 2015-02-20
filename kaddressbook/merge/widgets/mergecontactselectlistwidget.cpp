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

#include "mergecontactselectlistwidget.h"
#include <KABC/Addressee>
#include <KLocalizedString>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>
using namespace KABMergeContacts;
using namespace KABC;

MergeContactSelectListWidget::MergeContactSelectListWidget(QWidget *parent)
    : QWidget(parent),
      mConflictType(MergeContacts::None)
{
    QVBoxLayout *vbox = new QVBoxLayout;
    setLayout(vbox);
    mTitle = new QLabel;
    mTitle->setObjectName(QLatin1String("title"));
    vbox->addWidget(mTitle);
    mSelectListWidget = new QListWidget;
    mSelectListWidget->setObjectName(QLatin1String("listwidget"));
    vbox->addWidget(mSelectListWidget);
}

MergeContactSelectListWidget::~MergeContactSelectListWidget()
{

}

void MergeContactSelectListWidget::setContacts(MergeContacts::ConflictInformation conflictType, const KABC::Addressee::List &lst)
{
    mConflictType = conflictType;
    if (lst.isEmpty() || ( conflictType == MergeContacts::None) ) {
        return;
    }
    updateTitle();
    fillList(lst);
}

void MergeContactSelectListWidget::updateTitle()
{
    QString title;
    //TODO
    switch(mConflictType) {
    case MergeContacts::None:
        break;
    case MergeContacts::Birthday:
        title = Addressee::birthdayLabel();
        break;
    case MergeContacts::Geo:
        title = Addressee::geoLabel();
        break;
    case MergeContacts::Photo:
        title = Addressee::photoLabel();
        break;
    case MergeContacts::Logo:
        title = Addressee::logoLabel();
        break;
    case MergeContacts::Anniversary:
        title = i18nc( "The wedding anniversary of a contact", "Anniversary" );
        break;
    case MergeContacts::Name:
        title = Addressee::nameLabel();
        break;
    case MergeContacts::NickName:
        title = Addressee::nickNameLabel();
        break;
    case MergeContacts::Blog:
        title = i18n( "Blog Feed" );
        break;
    case MergeContacts::HomePage:
        title = QLatin1String("HomePage");
        break;
    case MergeContacts::Organization:
        title = Addressee::organizationLabel();
        break;
    case MergeContacts::Profession:
        title = i18n( "Profession" );
        break;
    case MergeContacts::Title:
        title = Addressee::titleLabel();
        break;
    case MergeContacts::Departement:
        title = Addressee::departmentLabel();
        break;
    case MergeContacts::Office:
        title = i18n( "Office" );
        break;
    case MergeContacts::ManagerName:
        title = i18n( "Manager" );
        break;
    case MergeContacts::Assistant:
        title = i18n( "Assistant" );
        break;
    case MergeContacts::FreeBusy:
        title = QLatin1String("FreeBusy");
        break;
    case MergeContacts::FamilyName:
        title = Addressee::familyNameLabel();
        break;
    case MergeContacts::PartnerName:
        title = i18n( "Spouse" );
        break;
    case MergeContacts::Keys:
        title = QLatin1String("Keys");
        break;
    }

    mTitle->setText(title);
}

void MergeContactSelectListWidget::fillList(const KABC::Addressee::List &lst)
{
    qDebug()<<"mConflictType"<<mConflictType;
    Q_FOREACH(const KABC::Addressee &addr, lst ) {
        switch(mConflictType) {
        case MergeContacts::None:
            break;
        case MergeContacts::Birthday:
            //FIXME
            break;
        case MergeContacts::Geo: {
            mSelectListWidget->addItem(addr.geo().toString());
            break;
        }
        case MergeContacts::Photo:
            //FIXME
            break;
        case MergeContacts::Logo:
            //FIXME
            break;
        case MergeContacts::Anniversary: {
            const QString newBlog = addr.custom(QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-Anniversary" ));
            mSelectListWidget->addItem(newBlog);
            break;
        }
        case MergeContacts::Name:
            mSelectListWidget->addItem(addr.name());
            break;
        case MergeContacts::NickName:
            mSelectListWidget->addItem(addr.nickName());
            break;
        case MergeContacts::Blog: {
            const QString newBlog = addr.custom(QLatin1String( "KADDRESSBOOK" ), QLatin1String( "BlogFeed" ));
            mSelectListWidget->addItem(newBlog);
            break;
        }
        case MergeContacts::HomePage:
            mSelectListWidget->addItem(addr.url().prettyUrl());
            break;
        case MergeContacts::Organization:
            mSelectListWidget->addItem(addr.organization());
            break;
        case MergeContacts::Profession: {
            const QString newBlog = addr.custom(QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-Profession" ));
            mSelectListWidget->addItem(newBlog);
            break;
        }
        case MergeContacts::Title:
            mSelectListWidget->addItem(addr.title());
            break;
        case MergeContacts::Departement:
            mSelectListWidget->addItem(addr.department());
            break;
        case MergeContacts::Office: {
            const QString newBlog = addr.custom(QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-Office" ));
            mSelectListWidget->addItem(newBlog);
            break;
        }
        case MergeContacts::ManagerName: {
            const QString newBlog = addr.custom(QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-ManagersName" ));
            mSelectListWidget->addItem(newBlog);
            break;
        }
        case MergeContacts::Assistant: {
            const QString newBlog = addr.custom(QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-AssistantsName" ));
            mSelectListWidget->addItem(newBlog);
            break;
        }
        case MergeContacts::FreeBusy:
            //FIXME
            break;
        case MergeContacts::FamilyName:
            mSelectListWidget->addItem(addr.familyName());
            break;
        case MergeContacts::PartnerName: {
            const QString newBlog = addr.custom(QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-SpousesName" ));
            mSelectListWidget->addItem(newBlog);
            break;
        }
        case MergeContacts::Keys:
            //TODO
            break;
        }
    }
}

int MergeContactSelectListWidget::selectedContact() const
{
    return mSelectListWidget->currentRow();
}

MergeContacts::ConflictInformation MergeContactSelectListWidget::conflictType() const
{
    return mConflictType;
}
