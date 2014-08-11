/*
    Copyright 2011  José Millán Soto <fid@gpul.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "accessiblemailwebviewtest.h"

#define QT_GUI_LIB

#include "messageviewer/viewer/mailwebview.h"

#include <QTest>
#include <QAccessible>
#include <QAccessibleTextInterface>


void AccessibleMailWebViewTest::testWebViewText()
{
  MessageViewer::MailWebView *widget = new MessageViewer::MailWebView();
  QAccessibleInterface *interface = QAccessible::queryAccessibleInterface(widget);
  widget->setHtml(QLatin1String("<html><body><h1>Hi!</h1>This is a KDE test</body></html>"), QUrl());

  QVERIFY( interface );
  QVERIFY( interface->textInterface() );
  QCOMPARE( interface->textInterface()->characterCount(), 23 );
  QCOMPARE( interface->textInterface()->text(15, 18), QLatin1String("KDE") );
}

QTEST_MAIN( AccessibleMailWebViewTest)
