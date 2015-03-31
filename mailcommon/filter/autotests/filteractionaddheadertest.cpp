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
#include <KLineEdit>
#include <QLabel>
#include <qtest.h>
#include <widgets/minimumcombobox.h>

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
    QTest::newRow("real value") <<  val << QString(QStringLiteral("bla")) << QString(QStringLiteral("blo"));
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
    filter.argsFromString(QStringLiteral("foo\tbla"));
    QVERIFY(!filter.isEmpty());
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

QTEST_MAIN(FilterActionAddHeaderTest)
