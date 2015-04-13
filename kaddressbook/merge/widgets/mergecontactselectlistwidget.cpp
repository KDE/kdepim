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
#include <KContacts/Addressee>
#include <KLocalizedString>
#include <KGlobal>
#include <KLocale>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>
using namespace KABMergeContacts;
using namespace KContacts;

MergeContactSelectListWidget::MergeContactSelectListWidget(QWidget *parent)
    : QWidget(parent),
      mConflictType(MergeContacts::None)
{
    QVBoxLayout *vbox = new QVBoxLayout;
    setLayout(vbox);
    mTitle = new QLabel;
    mTitle->setObjectName(QStringLiteral("title"));
    vbox->addWidget(mTitle);
    mSelectListWidget = new QListWidget;
    mSelectListWidget->setObjectName(QStringLiteral("listwidget"));
    vbox->addWidget(mSelectListWidget);
}

MergeContactSelectListWidget::~MergeContactSelectListWidget()
{

}

void MergeContactSelectListWidget::setContacts(MergeContacts::ConflictInformation conflictType, const KContacts::Addressee::List &lst)
{
    mConflictType = conflictType;
    if (lst.isEmpty() || (conflictType == MergeContacts::None)) {
        return;
    }
    updateTitle();
    fillList(lst);
}

void MergeContactSelectListWidget::updateTitle()
{
    QString title;
    switch (mConflictType) {
    case MergeContacts::None:
        qWarning() << " MergeContacts::None used in updateTitle. It's a bug";
        // it's not possible.
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
        title = i18nc("The wedding anniversary of a contact", "Anniversary");
        break;
    case MergeContacts::Name:
        title = Addressee::nameLabel();
        break;
    case MergeContacts::NickName:
        title = Addressee::nickNameLabel();
        break;
    case MergeContacts::Blog:
        title = i18n("Blog Feed");
        break;
    case MergeContacts::HomePage:
        title = QStringLiteral("HomePage");
        break;
    case MergeContacts::Organization:
        title = Addressee::organizationLabel();
        break;
    case MergeContacts::Profession:
        title = i18n("Profession");
        break;
    case MergeContacts::Title:
        title = Addressee::titleLabel();
        break;
    case MergeContacts::Departement:
        title = Addressee::departmentLabel();
        break;
    case MergeContacts::Office:
        title = i18n("Office");
        break;
    case MergeContacts::ManagerName:
        title = i18n("Manager");
        break;
    case MergeContacts::Assistant:
        title = i18n("Assistant");
        break;
    case MergeContacts::FreeBusy:
        title = QStringLiteral("FreeBusy");
        break;
    case MergeContacts::FamilyName:
        title = Addressee::familyNameLabel();
        break;
    case MergeContacts::PartnerName:
        title = i18n("Spouse");
        break;
    case MergeContacts::Keys:
        title = QStringLiteral("Keys");
        break;
    }

    mTitle->setText(title);
}

void MergeContactSelectListWidget::addItem(const QString &str, const QIcon &icon)
{
    if (str.isEmpty()) {
        QListWidgetItem *item = new QListWidgetItem(mSelectListWidget);
        item->setFlags(Qt::NoItemFlags);
        item->setText(i18n("(Undefined)"));
        mSelectListWidget->addItem(item);
    } else {
        if (!icon.isNull()) {
            QListWidgetItem *item = new QListWidgetItem(mSelectListWidget);
            item->setText(str);
            item->setIcon(icon);
            mSelectListWidget->addItem(item);
        } else {
            mSelectListWidget->addItem(str);
        }
    }
}

void MergeContactSelectListWidget::fillList(const KContacts::Addressee::List &lst)
{
    mSelectListWidget->clear();
    Q_FOREACH (const Addressee &addr, lst) {
        switch (mConflictType) {
        case MergeContacts::None:
            break;
        case MergeContacts::Birthday: {
            const QDate birdthDt = addr.birthday().date();
            QString birdth;
            if (birdthDt.isValid()) {
                birdth = KGlobal::locale()->formatDate(birdthDt);
            }
            addItem(birdth);
            break;
        }
        case MergeContacts::Geo: {
            const Geo geo = addr.geo();
            const QString str = QStringLiteral("%1-%2").arg(geo.latitude(), geo.longitude());
            addItem(str);
            break;
        }
        case MergeContacts::Photo:
            //TODO fix when it's an url
            addItem(QString(), QIcon(QPixmap::fromImage(addr.photo().data())));
            //FIXME add icon ?
            break;
        case MergeContacts::Logo:
            addItem(QString(), QIcon(QPixmap::fromImage(addr.logo().data())));
            //FIXME add icon ?
            break;
        case MergeContacts::Anniversary: {
            QString anniversary;
            const QDate anniversaryDt = QDate::fromString(addr.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Anniversary")), Qt::ISODate);
            if (anniversaryDt.isValid()) {
                anniversary = KGlobal::locale()->formatDate(anniversaryDt);
            }
            addItem(anniversary);
            break;
        }
        case MergeContacts::Name:
            addItem(addr.name());
            break;
        case MergeContacts::NickName:
            addItem(addr.nickName());
            break;
        case MergeContacts::Blog: {
            const QString newBlog = addr.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("BlogFeed"));
            addItem(newBlog);
            break;
        }
        case MergeContacts::HomePage:
            addItem(addr.url().toString());
            break;
        case MergeContacts::Organization:
            addItem(addr.organization());
            break;
        case MergeContacts::Profession: {
            const QString newBlog = addr.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Profession"));
            addItem(newBlog);
            break;
        }
        case MergeContacts::Title:
            addItem(addr.title());
            break;
        case MergeContacts::Departement:
            addItem(addr.department());
            break;
        case MergeContacts::Office: {
            const QString newBlog = addr.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Office"));
            addItem(newBlog);
            break;
        }
        case MergeContacts::ManagerName: {
            const QString newBlog = addr.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-ManagersName"));
            addItem(newBlog);
            break;
        }
        case MergeContacts::Assistant: {
            const QString newBlog = addr.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-AssistantsName"));
            addItem(newBlog);
            break;
        }
        case MergeContacts::FreeBusy:
            //FIXME
            break;
        case MergeContacts::FamilyName:
            addItem(addr.familyName());
            break;
        case MergeContacts::PartnerName: {
            const QString newBlog = addr.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-SpousesName"));
            addItem(newBlog);
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

bool MergeContactSelectListWidget::verifySelectedInfo() const
{
    return (selectedContact() != -1);
}
