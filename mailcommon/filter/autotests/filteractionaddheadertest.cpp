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
    PimCommon::MinimumComboBox *comboBox = widget->findChild<PimCommon::MinimumComboBox *>(QLatin1String("combo"));
    QVERIFY(comboBox);
    QVERIFY(comboBox->isEditable());
    QLabel *label = widget->findChild<QLabel *>(QLatin1String("label_value"));
    QVERIFY(label);
    KLineEdit *lineEdit = widget->findChild<KLineEdit *>(QLatin1String("ledit"));
    QVERIFY(lineEdit);
    QVERIFY(lineEdit->text().isEmpty());
}

void FilterActionAddHeaderTest::shouldAddValue_data()
{
    QTest::addColumn<QString>("argsinput");
    QTest::addColumn<QString>("resultheader");
    QTest::addColumn<QString>("resultvalue");
    QTest::newRow("empty") <<  QString() << QString() << QString();
    QString val = QLatin1String("bla") + QLatin1Char('\t') + QLatin1String("blo");
    QTest::newRow("real value") <<  val << QString(QLatin1String("bla")) << QString(QLatin1String("blo"));
}

void FilterActionAddHeaderTest::shouldClearWidget()
{
    MailCommon::FilterActionAddHeader filter;
    QWidget* widget = filter.createParamWidget(0);
    PimCommon::MinimumComboBox *comboBox = widget->findChild<PimCommon::MinimumComboBox *>(QLatin1String("combo"));
    KLineEdit *lineEdit = widget->findChild<KLineEdit *>(QLatin1String("ledit"));
    comboBox->lineEdit()->setText(QLatin1String("test"));
    lineEdit->setText(QLatin1String("blo"));
    filter.clearParamWidget(widget);
    QVERIFY(comboBox->lineEdit()->text().isEmpty());
    QVERIFY(lineEdit->text().isEmpty());
}

void FilterActionAddHeaderTest::shouldReturnSieveCode()
{
    MailCommon::FilterActionAddHeader filter;
    QCOMPARE(filter.sieveRequires().join(QLatin1String(",")), QLatin1String("editheader"));
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
    PimCommon::MinimumComboBox *comboBox = widget->findChild<PimCommon::MinimumComboBox *>(QLatin1String("combo"));
    KLineEdit *lineEdit = widget->findChild<KLineEdit *>(QLatin1String("ledit"));
    QCOMPARE(comboBox->lineEdit()->text(), resultheader);
    QCOMPARE(lineEdit->text(), resultvalue);
}

QTEST_MAIN(FilterActionAddHeaderTest)
