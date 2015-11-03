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

#include "filteractionaddtoaddressbooktest.h"
#include "../filteractions/filteractionaddtoaddressbook.h"
#include <qtest.h>
#include <QWidget>
#include <QLabel>
#include <klineedit.h>
#include <pimcommon/minimumcombobox.h>
#include <libkdepim/tagwidgets.h>
#include <AkonadiWidgets/CollectionComboBox>

FilterActionAddToAddressBookTest::FilterActionAddToAddressBookTest(QObject *parent)
    : QObject(parent)
{

}

FilterActionAddToAddressBookTest::~FilterActionAddToAddressBookTest()
{

}

void FilterActionAddToAddressBookTest::shouldHaveDefaultValue()
{
    MailCommon::FilterActionAddToAddressBook filter;
    QWidget *w = filter.createParamWidget(0);

    PimCommon::MinimumComboBox *headerCombo = w->findChild<PimCommon::MinimumComboBox *>(QStringLiteral("HeaderComboBox"));
    QVERIFY(headerCombo);

    QLabel *label = w->findChild<QLabel *>(QStringLiteral("label_with_category"));
    QVERIFY(label);

    KPIM::TagWidget *categoryEdit = w->findChild<KPIM::TagWidget *>(QStringLiteral("CategoryEdit"));
    QVERIFY(categoryEdit);

    label = w->findChild<QLabel *>(QStringLiteral("label_in_addressbook"));
    QVERIFY(label);

    Akonadi::CollectionComboBox *collectionComboBox = w->findChild<Akonadi::CollectionComboBox *>(QStringLiteral("AddressBookComboBox"));
    QVERIFY(collectionComboBox);
}

void FilterActionAddToAddressBookTest::shouldReportErrorWhenArgumentIsEmpty()
{
    MailCommon::FilterActionAddToAddressBook filter;
    KMime::Message::Ptr msgPtr = KMime::Message::Ptr(new KMime::Message());
    Akonadi::Item item;
    item.setPayload<KMime::Message::Ptr>(msgPtr);
    MailCommon::ItemContext context(item, false);

    filter.argsFromString("");
    QVERIFY(filter.isEmpty());
    QCOMPARE(filter.process(context, false), MailCommon::FilterAction::ErrorButGoOn);
    QCOMPARE(context.needsPayloadStore(), false);
    QCOMPARE(context.needsFlagStore(), false);
    QCOMPARE(context.needsFullPayload(), false);
}

void FilterActionAddToAddressBookTest::shouldReportErrorWhenCollectionIsInvalid()
{
    MailCommon::FilterActionAddToAddressBook filter;
    KMime::Message::Ptr msgPtr = KMime::Message::Ptr(new KMime::Message());
    Akonadi::Item item;
    item.setPayload<KMime::Message::Ptr>(msgPtr);
    MailCommon::ItemContext context(item, false);

    filter.argsFromString("foo\t-1\tddd");
    QVERIFY(filter.isEmpty());
    QCOMPARE(filter.process(context, false), MailCommon::FilterAction::ErrorButGoOn);
    QCOMPARE(context.needsPayloadStore(), false);
    QCOMPARE(context.needsFlagStore(), false);
    QCOMPARE(context.needsFullPayload(), false);
}

void FilterActionAddToAddressBookTest::shouldRequiresPart()
{
    MailCommon::FilterActionAddToAddressBook filter;
    QCOMPARE(filter.requiredPart(), MailCommon::SearchRule::Envelope);
}

QTEST_MAIN(FilterActionAddToAddressBookTest)
