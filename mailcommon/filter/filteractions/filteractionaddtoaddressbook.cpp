/*
 * Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "filteractionaddtoaddressbook.h"

#include "pimcommon/widgets/minimumcombobox.h"

#include "libkdepim/job/addcontactjob.h"
#include "libkdepim/widgets/tagwidgets.h"

#include <CollectionComboBox>
#include <KContacts/Addressee>
#include <KLineEdit>
#include <KLocalizedString>
#include <KEmailAddress>

#include <QGridLayout>
#include <QLabel>

using namespace MailCommon;

FilterAction *FilterActionAddToAddressBook::newAction()
{
    return new FilterActionAddToAddressBook;
}

FilterActionAddToAddressBook::FilterActionAddToAddressBook(QObject *parent)
    : FilterActionWithStringList(QStringLiteral("add to address book"), i18n("Add to Address Book"), parent),
      mFromStr(i18nc("Email sender", "From")),
      mToStr(i18nc("Email recipient", "To")),
      mCCStr(i18n("CC")),
      mBCCStr(i18n("BCC")),
      mHeaderType(UnknownHeader),
      mCollectionId(-1),
      mCategory(i18n("KMail Filter"))
{
}

bool FilterActionAddToAddressBook::isEmpty() const
{
    return (mCollectionId == -1) || (mHeaderType == UnknownHeader);
}

FilterAction::ReturnCode FilterActionAddToAddressBook::process(ItemContext &context , bool) const
{
    if (isEmpty()) {
        return ErrorButGoOn;
    }

    const KMime::Message::Ptr msg = context.item().payload<KMime::Message::Ptr>();

    QString headerLine;
    switch (mHeaderType) {
    case FromHeader: headerLine = msg->from()->asUnicodeString(); break;
    case ToHeader: headerLine = msg->to()->asUnicodeString(); break;
    case CcHeader: headerLine = msg->cc()->asUnicodeString(); break;
    case BccHeader: headerLine = msg->bcc()->asUnicodeString(); break;
    case UnknownHeader: break;
    }
    if (headerLine.isEmpty()) {
        return ErrorButGoOn;
    }

    const QStringList emails = KEmailAddress::splitAddressList(headerLine);

    foreach (const QString &singleEmail, emails) {
        QString name, email;
        KContacts::Addressee::parseEmailAddress(singleEmail, name, email);

        KContacts::Addressee contact;
        contact.setNameFromString(name);
        contact.insertEmail(email, true);
        if (!mCategory.isEmpty()) {
            contact.setCategories(mCategory.split(QStringLiteral(";")));
        }

        KPIM::AddContactJob *job = new KPIM::AddContactJob(contact, Akonadi::Collection(mCollectionId));
        job->showMessageBox(false);
        job->start();
    }

    return GoOn;
}

SearchRule::RequiredPart FilterActionAddToAddressBook::requiredPart() const
{
    return SearchRule::Envelope;
}

QWidget *FilterActionAddToAddressBook::createParamWidget(QWidget *parent) const
{
    QWidget *widget = new QWidget(parent);
    QGridLayout *layout = new QGridLayout(widget);

    PimCommon::MinimumComboBox *headerCombo = new PimCommon::MinimumComboBox(widget);
    headerCombo->setObjectName(QStringLiteral("HeaderComboBox"));
    layout->addWidget(headerCombo, 0, 0, 2, 1, Qt::AlignVCenter);

    QLabel *label = new QLabel(i18n("with category"), widget);
    label->setObjectName(QStringLiteral("label_with_category"));
    layout->addWidget(label, 0, 1);

    KPIM::TagWidget *categoryEdit = new KPIM::TagWidget(widget);
    categoryEdit->setObjectName(QStringLiteral("CategoryEdit"));
    layout->addWidget(categoryEdit, 0, 2);

    label = new QLabel(i18n("in address book"), widget);
    label->setObjectName(QStringLiteral("label_in_addressbook"));
    layout->addWidget(label, 1, 1);

    Akonadi::CollectionComboBox *collectionComboBox = new Akonadi::CollectionComboBox(widget);
    collectionComboBox->setMimeTypeFilter(QStringList() << KContacts::Addressee::mimeType());
    collectionComboBox->setAccessRightsFilter(Akonadi::Collection::CanCreateItem);

    collectionComboBox->setObjectName(QStringLiteral("AddressBookComboBox"));
    collectionComboBox->setToolTip(i18n("<p>This defines the preferred address book.<br />"
                                        "If it is not accessible, the filter will fallback to the default address book.</p>"));
    layout->addWidget(collectionComboBox, 1, 2);

    connect(headerCombo, static_cast<void (PimCommon::MinimumComboBox::*)(int)>(&PimCommon::MinimumComboBox::currentIndexChanged), this, &FilterActionAddToAddressBook::filterActionModified);
    connect(collectionComboBox, static_cast<void (Akonadi::CollectionComboBox::*)(int)>(&Akonadi::CollectionComboBox::activated), this, &FilterActionAddToAddressBook::filterActionModified);
    connect(categoryEdit, SIGNAL(selectionChanged(QStringList)),
            this, SIGNAL(filterActionModified()));

    setParamWidgetValue(widget);

    return widget;
}

void FilterActionAddToAddressBook::setParamWidgetValue(QWidget *paramWidget) const
{
    PimCommon::MinimumComboBox *headerCombo = paramWidget->findChild<PimCommon::MinimumComboBox *>(QStringLiteral("HeaderComboBox"));
    Q_ASSERT(headerCombo);
    headerCombo->clear();
    headerCombo->addItem(mFromStr, FromHeader);
    headerCombo->addItem(mToStr, ToHeader);
    headerCombo->addItem(mCCStr, CcHeader);
    headerCombo->addItem(mBCCStr, BccHeader);

    headerCombo->setCurrentIndex(headerCombo->findData(mHeaderType));

    KPIM::TagWidget *categoryEdit = paramWidget->findChild<KPIM::TagWidget *>(QStringLiteral("CategoryEdit"));
    Q_ASSERT(categoryEdit);
    categoryEdit->setSelection(mCategory.split(QStringLiteral(";")));

    Akonadi::CollectionComboBox *collectionComboBox = paramWidget->findChild<Akonadi::CollectionComboBox *>(QStringLiteral("AddressBookComboBox"));
    Q_ASSERT(collectionComboBox);
    collectionComboBox->setDefaultCollection(Akonadi::Collection(mCollectionId));
    collectionComboBox->setProperty("collectionId", mCollectionId);
}

void FilterActionAddToAddressBook::applyParamWidgetValue(QWidget *paramWidget)
{
    const PimCommon::MinimumComboBox *headerCombo = paramWidget->findChild<PimCommon::MinimumComboBox *>(QStringLiteral("HeaderComboBox"));
    Q_ASSERT(headerCombo);
    mHeaderType = static_cast<HeaderType>(headerCombo->itemData(headerCombo->currentIndex()).toInt());

    const KPIM::TagWidget *categoryEdit = paramWidget->findChild<KPIM::TagWidget *>(QStringLiteral("CategoryEdit"));
    Q_ASSERT(categoryEdit);
    mCategory = categoryEdit->selection().join(QStringLiteral(";"));

    const Akonadi::CollectionComboBox *collectionComboBox = paramWidget->findChild<Akonadi::CollectionComboBox *>(QStringLiteral("AddressBookComboBox"));
    Q_ASSERT(collectionComboBox);
    const Akonadi::Collection collection = collectionComboBox->currentCollection();

    // it might be that the model of collectionComboBox has not finished loading yet, so
    // we use the previously 'stored' value from the 'collectionId' property
    if (collection.isValid()) {
        mCollectionId = collection.id();
        connect(collectionComboBox, static_cast<void (Akonadi::CollectionComboBox::*)(int)>(&Akonadi::CollectionComboBox::currentIndexChanged), this, &FilterActionAddToAddressBook::filterActionModified);
    } else {
        const QVariant value = collectionComboBox->property("collectionId");
        if (value.isValid()) {
            mCollectionId = value.toLongLong();
        }
    }
}

void FilterActionAddToAddressBook::clearParamWidget(QWidget *paramWidget) const
{
    PimCommon::MinimumComboBox *headerCombo = paramWidget->findChild<PimCommon::MinimumComboBox *>(QStringLiteral("HeaderComboBox"));
    Q_ASSERT(headerCombo);
    headerCombo->setCurrentIndex(0);

    KPIM::TagWidget *categoryEdit = paramWidget->findChild<KPIM::TagWidget *>(QStringLiteral("CategoryEdit"));
    Q_ASSERT(categoryEdit);
    categoryEdit->setSelection(mCategory.split(QStringLiteral(";")));
}

QString FilterActionAddToAddressBook::argsAsString() const
{
    QString result;

    switch (mHeaderType) {
    case FromHeader: result = QStringLiteral("From"); break;
    case ToHeader: result = QStringLiteral("To"); break;
    case CcHeader: result = QStringLiteral("CC"); break;
    case BccHeader: result = QStringLiteral("BCC"); break;
    case UnknownHeader: break;
    }

    result += QLatin1Char('\t');
    result += QString::number(mCollectionId);
    result += QLatin1Char('\t');
    result += mCategory;

    return result;
}

void FilterActionAddToAddressBook::argsFromString(const QString &argsStr)
{
    const QStringList parts = argsStr.split(QLatin1Char('\t'), QString::KeepEmptyParts);
    const QString firstElement = parts[ 0 ];
    if (firstElement == QStringLiteral("From")) {
        mHeaderType = FromHeader;
    } else if (firstElement == QStringLiteral("To")) {
        mHeaderType = ToHeader;
    } else if (firstElement == QStringLiteral("CC")) {
        mHeaderType = CcHeader;
    } else if (firstElement == QStringLiteral("BCC")) {
        mHeaderType = BccHeader;
    } else {
        mHeaderType = UnknownHeader;
    }
    if (parts.count() >= 2) {
        mCollectionId = parts[ 1 ].toLongLong();
    }

    if (parts.count() < 3) {
        mCategory.clear();
    } else {
        mCategory = parts[ 2 ];
    }
}

QString FilterActionAddToAddressBook::informationAboutNotValidAction() const
{
    //KF5 add i18n
    QString result;
    if (mHeaderType == UnknownHeader) {
        result = QLatin1String("Header type selected is unknown.");
    }
    if (mCollectionId == -1) {
        if (!result.isEmpty()) {
            result += QLatin1Char('\n');
        }
        result += QLatin1String("No addressbook selected.");
    }
    return result;
}
