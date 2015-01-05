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
#include "../viewer/converthtmltoplaintext.h"
#include <qtest_kde.h>

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
    convert.setHtmlString(QLatin1String("foo bla"));
    QVERIFY(!convert.generatePlainText().isEmpty());
}

QTEST_KDEMAIN(ConvertHtmlToPlainTextTest, NoGUI)
