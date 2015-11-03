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

#include "filteractionrewriteheadertest.h"
#include <qtest.h>
#include "../filteractions/filteractionrewriteheader.h"
#include <KLineEdit>
#include <QLabel>
#include <QWidget>
#include <pimcommon/minimumcombobox.h>
FilterActionRewriteHeaderTest::FilterActionRewriteHeaderTest(QObject *parent)
    : QObject(parent)
{

}

FilterActionRewriteHeaderTest::~FilterActionRewriteHeaderTest()
{

}

void FilterActionRewriteHeaderTest::shouldHaveDefaultValue()
{
    MailCommon::FilterActionRewriteHeader filter;
    QWidget *w = filter.createParamWidget(0);
    PimCommon::MinimumComboBox *combo = w->findChild<PimCommon::MinimumComboBox *>(QStringLiteral("combo"));
    QVERIFY(combo);
    QVERIFY(combo->count() > 0);

    QLabel *label = w->findChild<QLabel *>(QStringLiteral("label_replace"));
    QVERIFY(label);

    KLineEdit *regExpLineEdit = w->findChild<KLineEdit *>(QStringLiteral("search"));
    QVERIFY(regExpLineEdit);
    QVERIFY(regExpLineEdit->text().isEmpty());

    label = w->findChild<QLabel *>(QStringLiteral("label_with"));
    QVERIFY(label);

    KLineEdit *lineEdit = w->findChild<KLineEdit *>(QStringLiteral("replace"));
    QVERIFY(lineEdit);
    QVERIFY(lineEdit->text().isEmpty());
}

void FilterActionRewriteHeaderTest::shouldBeEmpty()
{
    MailCommon::FilterActionRewriteHeader filter;
    QVERIFY(filter.isEmpty());
    filter.argsFromString(QStringLiteral("foo\tbla"));
    QVERIFY(filter.isEmpty());
    filter.argsFromString(QStringLiteral("foo\tbla\tkde"));
    QVERIFY(!filter.isEmpty());
}

void FilterActionRewriteHeaderTest::shouldNotExecuteActionWhenParameterIsEmpty()
{
    MailCommon::FilterActionRewriteHeader filter(this);
    KMime::Message::Ptr msgPtr = KMime::Message::Ptr(new KMime::Message());
    Akonadi::Item item;
    item.setPayload<KMime::Message::Ptr>(msgPtr);
    MailCommon::ItemContext context(item, true);

    filter.argsFromString("");
    QCOMPARE(filter.process(context, false), MailCommon::FilterAction::ErrorButGoOn);
    QCOMPARE(context.needsPayloadStore(), false);
}

void FilterActionRewriteHeaderTest::shouldNotExecuteActionWhenValueIsEmpty()
{
    MailCommon::FilterActionRewriteHeader filter(this);
    KMime::Message::Ptr msgPtr = KMime::Message::Ptr(new KMime::Message());
    Akonadi::Item item;
    item.setPayload<KMime::Message::Ptr>(msgPtr);
    MailCommon::ItemContext context(item, true);

    filter.argsFromString("foo");
    QCOMPARE(filter.process(context, false), MailCommon::FilterAction::ErrorButGoOn);
    QCOMPARE(context.needsPayloadStore(), false);

    filter.argsFromString("foo\tbla");
    QCOMPARE(filter.process(context, false), MailCommon::FilterAction::ErrorButGoOn);
    QCOMPARE(context.needsPayloadStore(), false);
}

void FilterActionRewriteHeaderTest::shouldRewriteHeader()
{
    const QByteArray data = "From: foo@kde.org\n"
                            "To: foo@kde.org\n"
                            "Subject: test\n"
                            "testheader: foo\n"
                            "Date: Wed, 01 Apr 2015 09:33:01 +0200\n"
                            "MIME-Version: 1.0\n"
                            "\n"
                            "test";
    const QByteArray output = "From: foo@kde.org\n"
                              "To: foo@kde.org\n"
                              "Subject: test\n"
                              "Date: Wed, 01 Apr 2015 09:33:01 +0200\n"
                              "MIME-Version: 1.0\n"
                              "testheader: bla\n"
                              "\n"
                              "test";

    MailCommon::FilterActionRewriteHeader filter(this);
    KMime::Message::Ptr msgPtr = KMime::Message::Ptr(new KMime::Message());
    msgPtr->setContent(data);
    msgPtr->parse();
    Akonadi::Item item;
    item.setPayload<KMime::Message::Ptr>(msgPtr);
    MailCommon::ItemContext context(item, true);

    filter.argsFromString("testheader\tfoo\tbla");
    QCOMPARE(filter.process(context, false), MailCommon::FilterAction::GoOn);
    QCOMPARE(context.needsPayloadStore(), true);
    QCOMPARE(msgPtr->encodedContent(), output);
}

void FilterActionRewriteHeaderTest::shouldNotRewriteHeaderWhenHeaderNotFound()
{
    const QByteArray data = "From: foo@kde.org\n"
                            "To: foo@kde.org\n"
                            "Subject: test\n"
                            "Date: Wed, 01 Apr 2015 09:33:01 +0200\n"
                            "MIME-Version: 1.0\n"
                            "\n"
                            "test";

    MailCommon::FilterActionRewriteHeader filter(this);
    KMime::Message::Ptr msgPtr = KMime::Message::Ptr(new KMime::Message());
    msgPtr->setContent(data);
    msgPtr->parse();
    Akonadi::Item item;
    item.setPayload<KMime::Message::Ptr>(msgPtr);
    MailCommon::ItemContext context(item, true);

    filter.argsFromString("testheader\tfoo\tbla");
    QCOMPARE(filter.process(context, false), MailCommon::FilterAction::GoOn);
    QCOMPARE(context.needsPayloadStore(), false);
    QCOMPARE(msgPtr->encodedContent(), data);
}

void FilterActionRewriteHeaderTest::shouldNotRewriteHeaderWhenRegexpNotFound()
{
    const QByteArray data = "From: foo@kde.org\n"
                            "To: foo@kde.org\n"
                            "Subject: test\n"
                            "Date: Wed, 01 Apr 2015 09:33:01 +0200\n"
                            "MIME-Version: 1.0\n"
                            "testheader: bla\n"
                            "\n"
                            "test";

    MailCommon::FilterActionRewriteHeader filter(this);
    KMime::Message::Ptr msgPtr = KMime::Message::Ptr(new KMime::Message());
    msgPtr->setContent(data);
    msgPtr->parse();
    Akonadi::Item item;
    item.setPayload<KMime::Message::Ptr>(msgPtr);
    MailCommon::ItemContext context(item, true);

    filter.argsFromString("testheader\tfoo\tbla");
    QCOMPARE(filter.process(context, false), MailCommon::FilterAction::GoOn);
    QCOMPARE(context.needsPayloadStore(), false);
    QCOMPARE(msgPtr->encodedContent(), data);
}

void FilterActionRewriteHeaderTest::shouldHaveRequiredPart()
{
    MailCommon::FilterActionRewriteHeader filter;
    QCOMPARE(filter.requiredPart(), MailCommon::SearchRule::CompleteMessage);
}

QTEST_MAIN(FilterActionRewriteHeaderTest)
