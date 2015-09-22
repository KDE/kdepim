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

#include "webshortcutmenumanagertest.h"
#include "../webshortcutmenumanager.h"
#include <qtest.h>

WebShortcutMenuManagerTest::WebShortcutMenuManagerTest(QObject *parent)
    : QObject(parent)
{

}

WebShortcutMenuManagerTest::~WebShortcutMenuManagerTest()
{

}

void WebShortcutMenuManagerTest::shouldHaveDefaultValue()
{
    PimCommon::WebShortcutMenuManager shortcutManager;
    QVERIFY(shortcutManager.selectedText().isEmpty());
}

void WebShortcutMenuManagerTest::shouldAssignSelectedText()
{
    PimCommon::WebShortcutMenuManager shortcutManager;
    const QString selectText = QStringLiteral("foo");
    shortcutManager.setSelectedText(selectText);
    QCOMPARE(shortcutManager.selectedText(), selectText);
}

QTEST_MAIN(WebShortcutMenuManagerTest)