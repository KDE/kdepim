/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "converthtmltoplaintexttest.h"
#include "../src/viewer/converthtmltoplaintext.h"
#include <qtest.h>

ConvertHtmlToPlainTextTest::ConvertHtmlToPlainTextTest(QObject *parent)
    : QObject(parent)
{

}

ConvertHtmlToPlainTextTest::~ConvertHtmlToPlainTextTest()
{

}

void ConvertHtmlToPlainTextTest::shouldHaveDefaultValue()
{
    MessageViewer::ConvertHtmlToPlainText convert;
    QVERIFY(convert.htmlString().isEmpty());
}

void ConvertHtmlToPlainTextTest::shouldReturnEmptyStringIfInputTextIsEmpty()
{
    MessageViewer::ConvertHtmlToPlainText convert;
    convert.setHtmlString(QString());
    QVERIFY(convert.generatePlainText().isEmpty());
}

void ConvertHtmlToPlainTextTest::shouldReturnNotEmptyStringIfInputTextIsNotEmpty()
{
    MessageViewer::ConvertHtmlToPlainText convert;
    const QString str = QStringLiteral("foo bla");
    convert.setHtmlString(str);
    const QString result = convert.generatePlainText();
    QVERIFY(!result.isEmpty());
    QCOMPARE(result, QString(str + QLatin1String("\n")));
}

void ConvertHtmlToPlainTextTest::shouldConvertToPlainText_data()
{
    QTest::addColumn<QString>("inputText");
    QTest::addColumn<QString>("convertedText");
    QTest::newRow("plainText") << "foo" << "foo\n";
    QTest::newRow("htmlText") << "<html><body>Hi! This is a KDE test</body></html>" << "Hi! This is a KDE test\n";
    QTest::newRow("htmlTextWithBold") << "<html><body><b>Hi!</b> This is a KDE test</body></html>" << "*Hi!* This is a KDE test\n";
    QTest::newRow("htmlTextWithH1") << "<html><body><h1>Hi!</h1> This is a KDE test</body></html>" << "*Hi!*\nThis is a KDE test\n";
    QTest::newRow("htmlTextWithUnderLine") << "<html><body><u>Hi!</u> This is a KDE test</body></html>" << "_Hi!_ This is a KDE test\n";
}

void ConvertHtmlToPlainTextTest::shouldConvertToPlainText()
{
    QFETCH(QString, inputText);
    QFETCH(QString, convertedText);

    MessageViewer::ConvertHtmlToPlainText convert;
    convert.setHtmlString(inputText);
    const QString result = convert.generatePlainText();
    QVERIFY(!result.isEmpty());
    QCOMPARE(result, convertedText);
}

QTEST_MAIN(ConvertHtmlToPlainTextTest)
