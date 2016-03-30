/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#include "quicksearchlinetest.h"
#include <qtest.h>
#include "core/widgets/quicksearchline.h"
#include <qtestkeyboard.h>
#include <qtestmouse.h>
#include <KLineEdit>
#include <QToolButton>
#include <QPushButton>
#include <KComboBox>
#include <QSignalSpy>

using namespace MessageList::Core;
QuickSearchLineTest::QuickSearchLineTest()
{
}

void QuickSearchLineTest::shouldHaveDefaultValueOnCreation()
{
    QuickSearchLine searchLine;
    QVERIFY(searchLine.searchEdit()->text().isEmpty());
    QCOMPARE(searchLine.containsOutboundMessages(), false);
}

void QuickSearchLineTest::shouldEmitTextChanged()
{
    QuickSearchLine searchLine;
    searchLine.show();
    QTest::qWaitForWindowExposed(&searchLine);
    QSignalSpy spy(&searchLine, SIGNAL(searchEditTextEdited(QString)));
    searchLine.searchEdit()->setText(QStringLiteral("F"));
    QCOMPARE(spy.count(), 0);

    searchLine.searchEdit()->clear();
    searchLine.searchEdit()->setText(QStringLiteral("FO"));
    QCOMPARE(spy.count(), 0);

    searchLine.searchEdit()->clear();
    searchLine.searchEdit()->setText(QStringLiteral("FOO"));
    QCOMPARE(spy.count(), 1);

    searchLine.searchEdit()->clear();
    searchLine.searchEdit()->setText(QStringLiteral("FOOO"));
    QCOMPARE(spy.count(), 2);

    searchLine.searchEdit()->clear();
    searchLine.searchEdit()->setText(QStringLiteral("FOOOO"));
    QCOMPARE(spy.count(), 3);
}

void QuickSearchLineTest::shouldShowExtraOptionWidget()
{
    QuickSearchLine searchLine;
    searchLine.show();
    QTest::keyClick(searchLine.searchEdit(), 'F');
    QTest::qWaitForWindowExposed(&searchLine);

    searchLine.searchEdit()->clear();
    searchLine.searchEdit()->setText(QStringLiteral("FOOOO"));
    QTest::qWaitForWindowExposed(&searchLine);

}

void QuickSearchLineTest::shouldHideExtraOptionWidgetWhenClearLineEdit()
{
    QuickSearchLine searchLine;
    searchLine.show();
    QTest::keyClicks(searchLine.searchEdit(), QStringLiteral("FOOFOO"));
    QTest::qWaitForWindowExposed(&searchLine);

    searchLine.searchEdit()->clear();
}

void QuickSearchLineTest::shouldHideExtraOptionWidgetWhenResetFilter()
{
    QuickSearchLine searchLine;
    searchLine.show();
    QTest::keyClicks(searchLine.searchEdit(), QStringLiteral("FOOFOO"));
    QTest::qWaitForWindowExposed(&searchLine);

    searchLine.resetFilter();
}

void QuickSearchLineTest::shouldEmitSearchOptionChangedWhenUseTabPress()
{
    QuickSearchLine searchLine;
    searchLine.show();
    QTest::qWaitForWindowExposed(&searchLine);
}

void QuickSearchLineTest::shouldResetAllWhenResetFilter()
{
    QuickSearchLine searchLine;
    searchLine.show();
    searchLine.resetFilter();
    QCOMPARE(searchLine.status().count(), 0);
    QCOMPARE(searchLine.tagFilterComboBox()->currentIndex(), -1);
}

void QuickSearchLineTest::shouldShowTagComboBox()
{
    QuickSearchLine searchLine;
    searchLine.show();
    QTest::qWaitForWindowExposed(&searchLine);
    QCOMPARE(searchLine.tagFilterComboBox()->isVisible(), false);
    searchLine.tagFilterComboBox()->addItems(QStringList() << QStringLiteral("1") << QStringLiteral("2"));
    searchLine.updateComboboxVisibility();
    QCOMPARE(searchLine.tagFilterComboBox()->isVisible(), true);
}

void QuickSearchLineTest::shouldResetComboboxWhenResetFilter()
{
    QuickSearchLine searchLine;
    searchLine.show();
    QTest::qWaitForWindowExposed(&searchLine);
    QCOMPARE(searchLine.tagFilterComboBox()->isVisible(), false);
    searchLine.tagFilterComboBox()->addItems(QStringList() << QStringLiteral("1") << QStringLiteral("2"));
    searchLine.updateComboboxVisibility();
    QCOMPARE(searchLine.tagFilterComboBox()->isVisible(), true);
    searchLine.tagFilterComboBox()->setCurrentIndex(1);
    QCOMPARE(searchLine.tagFilterComboBox()->currentIndex(), 1);
    searchLine.resetFilter();
    QCOMPARE(searchLine.tagFilterComboBox()->currentIndex(), 0);
}

void QuickSearchLineTest::shouldNotEmitTextChangedWhenTextTrimmedIsEmpty()
{
    QuickSearchLine searchLine;
    QSignalSpy spy(&searchLine, SIGNAL(searchEditTextEdited(QString)));
    searchLine.searchEdit()->setText(QStringLiteral("      "));
    QCOMPARE(spy.count(), 0);

    searchLine.searchEdit()->setText(QStringLiteral(" FOO"));
    QCOMPARE(spy.count(), 1);
}

void QuickSearchLineTest::shouldShowExtraOptionWidgetWhenTextTrimmedIsNotEmpty()
{
    QuickSearchLine searchLine;
    searchLine.show();
    searchLine.searchEdit()->setText(QStringLiteral(" "));
    QTest::qWaitForWindowExposed(&searchLine);
    searchLine.searchEdit()->clear();
    searchLine.searchEdit()->setText(QStringLiteral(" "));

    searchLine.searchEdit()->clear();
    searchLine.searchEdit()->setText(QStringLiteral(""));

    searchLine.searchEdit()->clear();
    searchLine.searchEdit()->setText(QStringLiteral("FOO0 "));

    searchLine.searchEdit()->clear();
    searchLine.searchEdit()->setText(QStringLiteral("FOOO "));

}

void QuickSearchLineTest::shouldShowMoreOptionWhenClickOnMoreButton()
{
    QuickSearchLine searchLine;
    searchLine.show();
    QTest::qWaitForWindowExposed(&searchLine);
}

void QuickSearchLineTest::shouldChangeFromButtonLabelWhenChangeOutboundMessagesValue()
{
    QuickSearchLine searchLine;
    searchLine.setContainsOutboundMessages(true);
    searchLine.setContainsOutboundMessages(false);
}

void QuickSearchLineTest::shouldSearchToOrFrom()
{
    QuickSearchLine searchLine;
    searchLine.setContainsOutboundMessages(true);
    searchLine.setContainsOutboundMessages(false);
}

void QuickSearchLineTest::shouldHideShowWidgetWhenWeChangeVisibility()
{
    QuickSearchLine searchLine;
    searchLine.show();

    searchLine.changeQuicksearchVisibility(false);
    QCOMPARE(searchLine.searchEdit()->isVisible(), false);
    QCOMPARE(searchLine.tagFilterComboBox()->isVisible(), false);

    searchLine.changeQuicksearchVisibility(true);
    QCOMPARE(searchLine.searchEdit()->isVisible(), true);
    QCOMPARE(searchLine.tagFilterComboBox()->isVisible(), false);

    //Fill Combobox
    searchLine.tagFilterComboBox()->addItems(QStringList() << QStringLiteral("1") << QStringLiteral("2"));
    searchLine.changeQuicksearchVisibility(false);
    QCOMPARE(searchLine.tagFilterComboBox()->isVisible(), false);

    searchLine.changeQuicksearchVisibility(true);
    QCOMPARE(searchLine.tagFilterComboBox()->isVisible(), true);
}

void QuickSearchLineTest::shouldNotShowComboboxWhenWeAddNewItemWhenWeHiddedQuickSearchBarWidget()
{
    QuickSearchLine searchLine;
    searchLine.show();
    searchLine.tagFilterComboBox()->addItems(QStringList() << QStringLiteral("1") << QStringLiteral("2"));
    searchLine.updateComboboxVisibility();
    QCOMPARE(searchLine.tagFilterComboBox()->isVisible(), true);

    searchLine.changeQuicksearchVisibility(false);
    searchLine.tagFilterComboBox()->addItems(QStringList() << QStringLiteral("1") << QStringLiteral("2"));
    searchLine.updateComboboxVisibility();
    QCOMPARE(searchLine.tagFilterComboBox()->isVisible(), false);

}

void QuickSearchLineTest::shouldRestoreDefaultSearchOptionWhenTextIsEmpied()
{
    QuickSearchLine searchLine;
    searchLine.show();

    searchLine.resetFilter();
}

void QuickSearchLineTest::shouldHideExtraOptionWidgetWhenResetFilterWhenSetEmptyText()
{
    QuickSearchLine searchLine;
    searchLine.show();

    searchLine.searchEdit()->setText(QStringLiteral("FOOFOO"));
    QTest::qWaitForWindowExposed(&searchLine);

    searchLine.searchEdit()->clear();
}

QTEST_MAIN(QuickSearchLineTest)
