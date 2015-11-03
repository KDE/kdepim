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

#include "filteractionaddheadertest.h"
#include "../filteractions/filteractionaddheader.h"
#include "filter/itemcontext.h"

#include <KLineEdit>
#include <QLabel>
#include <qtest.h>
#include <pimcommon/minimumcombobox.h>

FilterActionAddHeaderTest::FilterActionAddHeaderTest(QObject *parent)
    : QObject(parent)
{

}

FilterActionAddHeaderTest::~FilterActionAddHeaderTest()
{

}

void FilterActionAddHeaderTest::shouldCreateWidget()
{
    MailCommon::FilterActionAddHeader filter;
    QWidget *widget = filter.createParamWidget(0);
    QVERIFY(widget);
    PimCommon::MinimumComboBox *comboBox = widget->findChild<PimCommon::MinimumComboBox *>(QStringLiteral("combo"));
    QVERIFY(comboBox);
    QVERIFY(comboBox->isEditable());
    QLabel *label = widget->findChild<QLabel *>(QStringLiteral("label_value"));
    QVERIFY(label);
    KLineEdit *lineEdit = widget->findChild<KLineEdit *>(QStringLiteral("ledit"));
    QVERIFY(lineEdit);
    QVERIFY(lineEdit->text().isEmpty());
}

void FilterActionAddHeaderTest::shouldAddValue_data()
{
    QTest::addColumn<QString>("argsinput");
    QTest::addColumn<QString>("resultheader");
    QTest::addColumn<QString>("resultvalue");
    QTest::newRow("empty") <<  QString() << QString() << QString();
    QString val = QStringLiteral("bla") + QLatin1Char('\t') + QStringLiteral("blo");
    QTest::newRow("real value") <<  val << QStringLiteral("bla") << QStringLiteral("blo");
}

void FilterActionAddHeaderTest::shouldClearWidget()
{
    MailCommon::FilterActionAddHeader filter;
    QWidget *widget = filter.createParamWidget(0);
    PimCommon::MinimumComboBox *comboBox = widget->findChild<PimCommon::MinimumComboBox *>(QStringLiteral("combo"));
    KLineEdit *lineEdit = widget->findChild<KLineEdit *>(QStringLiteral("ledit"));
    comboBox->lineEdit()->setText(QStringLiteral("test"));
    lineEdit->setText(QStringLiteral("blo"));
    filter.clearParamWidget(widget);
    QVERIFY(comboBox->lineEdit()->text().isEmpty());
    QVERIFY(lineEdit->text().isEmpty());
}

void FilterActionAddHeaderTest::shouldReturnSieveCode()
{
    MailCommon::FilterActionAddHeader filter;
    QCOMPARE(filter.sieveRequires().join(QStringLiteral(",")), QStringLiteral("editheader"));
}

void FilterActionAddHeaderTest::shouldBeEmpty()
{
    MailCommon::FilterActionAddHeader filter;
    QVERIFY(filter.isEmpty());

    filter.argsFromString(QString());
    QVERIFY(filter.isEmpty());

    filter.argsFromString(QStringLiteral("foo\tbla"));
    QVERIFY(!filter.isEmpty());
}

void FilterActionAddHeaderTest::shouldNotExecuteActionWhenParameterIsEmpty()
{
    MailCommon::FilterActionAddHeader filter(this);
    KMime::Message::Ptr msgPtr = KMime::Message::Ptr(new KMime::Message());
    Akonadi::Item item;
    item.setPayload<KMime::Message::Ptr>(msgPtr);
    MailCommon::ItemContext context(item, true);

    filter.argsFromString("");
    QCOMPARE(filter.process(context, false), MailCommon::FilterAction::ErrorButGoOn);
    QCOMPARE(context.needsPayloadStore(), false);
    QCOMPARE(context.deleteItem(), false);
    QCOMPARE(context.needsFlagStore(), false);
}

void FilterActionAddHeaderTest::shouldNotExecuteActionWhenValueIsEmpty()
{
    MailCommon::FilterActionAddHeader filter(this);
    KMime::Message::Ptr msgPtr = KMime::Message::Ptr(new KMime::Message());
    Akonadi::Item item;
    item.setPayload<KMime::Message::Ptr>(msgPtr);
    MailCommon::ItemContext context(item, true);

    filter.argsFromString("foo");
    QCOMPARE(filter.process(context, false), MailCommon::FilterAction::ErrorButGoOn);
    QCOMPARE(context.needsPayloadStore(), false);
    QCOMPARE(context.deleteItem(), false);
    QCOMPARE(context.needsFlagStore(), false);
}

void FilterActionAddHeaderTest::shouldAddNewHeaderWhenNotExistingHeader()
{
    const QByteArray data = "From: foo@kde.org\n"
                            "To: foo@kde.org\n"
                            "Subject: test\n"
                            "Date: Wed, 01 Apr 2015 09:33:01 +0200\n"
                            "MIME-Version: 1.0\n"
                            "\n"
                            "test";
    const QByteArray output = "From: foo@kde.org\n"
                              "To: foo@kde.org\n"
                              "Subject: test\n"
                              "Date: Wed, 01 Apr 2015 09:33:01 +0200\n"
                              "MIME-Version: 1.0\n"
                              "testheader: foo\n"
                              "\n"
                              "test";

    MailCommon::FilterActionAddHeader filter(this);
    KMime::Message::Ptr msgPtr = KMime::Message::Ptr(new KMime::Message());
    msgPtr->setContent(data);
    msgPtr->parse();
    Akonadi::Item item;
    item.setPayload<KMime::Message::Ptr>(msgPtr);
    MailCommon::ItemContext context(item, true);

    filter.argsFromString("testheader\tfoo");
    QCOMPARE(filter.process(context, false), MailCommon::FilterAction::GoOn);
    QCOMPARE(context.needsPayloadStore(), true);
    QCOMPARE(msgPtr->encodedContent(), output);
    QCOMPARE(context.deleteItem(), false);
    QCOMPARE(context.needsFlagStore(), false);
}

void FilterActionAddHeaderTest::shouldReplaceHeaderWhenExistingHeader()
{
    const QByteArray data = "From: foo@kde.org\n"
                            "To: foo@kde.org\n"
                            "Subject: test\n"
                            "Date: Wed, 01 Apr 2015 09:33:01 +0200\n"
                            "MIME-Version: 1.0\n"
                            "testheader: bla\n"
                            "\n"
                            "test";
    const QByteArray output = "From: foo@kde.org\n"
                              "To: foo@kde.org\n"
                              "Subject: test\n"
                              "Date: Wed, 01 Apr 2015 09:33:01 +0200\n"
                              "MIME-Version: 1.0\n"
                              "testheader: foo\n"
                              "\n"
                              "test";

    MailCommon::FilterActionAddHeader filter(this);
    KMime::Message::Ptr msgPtr = KMime::Message::Ptr(new KMime::Message());
    msgPtr->setContent(data);
    msgPtr->parse();
    Akonadi::Item item;
    item.setPayload<KMime::Message::Ptr>(msgPtr);
    MailCommon::ItemContext context(item, true);

    filter.argsFromString("testheader\tfoo");
    QCOMPARE(filter.process(context, false), MailCommon::FilterAction::GoOn);
    QCOMPARE(context.needsPayloadStore(), true);
    QCOMPARE(msgPtr->encodedContent(), output);
    QCOMPARE(context.deleteItem(), false);
    QCOMPARE(context.needsFlagStore(), false);
}

void FilterActionAddHeaderTest::shouldAddValue()
{
    QFETCH(QString, argsinput);
    QFETCH(QString, resultheader);
    QFETCH(QString, resultvalue);

    MailCommon::FilterActionAddHeader filter;
    QWidget *widget = filter.createParamWidget(0);
    filter.argsFromString(argsinput);
    filter.setParamWidgetValue(widget);
    PimCommon::MinimumComboBox *comboBox = widget->findChild<PimCommon::MinimumComboBox *>(QStringLiteral("combo"));
    KLineEdit *lineEdit = widget->findChild<KLineEdit *>(QStringLiteral("ledit"));
    QCOMPARE(comboBox->lineEdit()->text(), resultheader);
    QCOMPARE(lineEdit->text(), resultvalue);
}

void FilterActionAddHeaderTest::shouldRequiresSieve()
{
    MailCommon::FilterActionAddHeader filter;
    QCOMPARE(filter.sieveRequires(), QStringList() << QStringLiteral("editheader"));
}

QTEST_MAIN(FilterActionAddHeaderTest)
