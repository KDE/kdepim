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
    QVERIFY(!searchLine.lockSearch()->isChecked());
    QWidget *widget = searchLine.findChild<QWidget *>(QStringLiteral("extraoptions"));
    QVERIFY(widget);
    QVERIFY(widget->isHidden());
    QPushButton *moreButton = searchLine.findChild<QPushButton *>(QStringLiteral("moreoptions"));
    QVERIFY(moreButton);
    QCOMPARE(moreButton->icon().name(), QStringLiteral("arrow-down-double"));
    QWidget *quickSearchFilterWidget = searchLine.findChild<QWidget *>(QStringLiteral("quicksearchfilterwidget"));
    QVERIFY(quickSearchFilterWidget);
    QVERIFY(quickSearchFilterWidget->isHidden());
    QCOMPARE(searchLine.containsOutboundMessages(), false);
    QPushButton *fullMessageButton = searchLine.findChild<QPushButton *>(QStringLiteral("full_message"));
    QVERIFY(!fullMessageButton->isVisible());
    QCOMPARE(fullMessageButton->isChecked(), true);

    QPushButton *bodyButton = searchLine.findChild<QPushButton *>(QStringLiteral("body"));
    QVERIFY(!bodyButton->isVisible());
    QCOMPARE(bodyButton->isChecked(), false);

    QPushButton *subjectButton = searchLine.findChild<QPushButton *>(QStringLiteral("subject"));
    QVERIFY(!subjectButton->isVisible());
    QCOMPARE(subjectButton->isChecked(), false);

    QPushButton *fromOrToButton = searchLine.findChild<QPushButton *>(QStringLiteral("fromorto"));
    QVERIFY(!fromOrToButton->isVisible());
    QCOMPARE(fromOrToButton->isChecked(), false);

    QPushButton *bccButton = searchLine.findChild<QPushButton *>(QStringLiteral("bcc"));
    QVERIFY(!bccButton->isVisible());
    QCOMPARE(bccButton->isChecked(), false);
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
    QWidget *widget = searchLine.findChild<QWidget *>(QStringLiteral("extraoptions"));
    QVERIFY(!widget->isVisible());

    searchLine.searchEdit()->clear();
    searchLine.searchEdit()->setText(QStringLiteral("FOOOO"));
    QTest::qWaitForWindowExposed(&searchLine);
    QVERIFY(widget->isVisible());

}

void QuickSearchLineTest::shouldHideExtraOptionWidgetWhenClearLineEdit()
{
    QuickSearchLine searchLine;
    searchLine.show();
    QTest::keyClicks(searchLine.searchEdit(), QStringLiteral("FOOFOO"));
    QTest::qWaitForWindowExposed(&searchLine);
    QWidget *widget = searchLine.findChild<QWidget *>(QStringLiteral("extraoptions"));

    searchLine.searchEdit()->clear();
    QVERIFY(!widget->isVisible());
}

void QuickSearchLineTest::shouldHideExtraOptionWidgetWhenResetFilter()
{
    QuickSearchLine searchLine;
    searchLine.show();
    QTest::keyClicks(searchLine.searchEdit(), QStringLiteral("FOOFOO"));
    QTest::qWaitForWindowExposed(&searchLine);
    QWidget *widget = searchLine.findChild<QWidget *>(QStringLiteral("extraoptions"));

    searchLine.resetFilter();
    QVERIFY(!widget->isVisible());
}

void QuickSearchLineTest::shouldEmitSearchOptionChanged()
{
    QuickSearchLine searchLine;
    searchLine.show();
    QSignalSpy spy(&searchLine, SIGNAL(searchOptionChanged()));
    QPushButton *button = searchLine.findChild<QPushButton *>(QStringLiteral("subject"));
    QTest::mouseClick(button, Qt::LeftButton);
    QCOMPARE(spy.count(), 1);
}

void QuickSearchLineTest::shouldEmitSearchOptionChangedWhenUseTabPress()
{
    QuickSearchLine searchLine;
    searchLine.show();
    QTest::qWaitForWindowExposed(&searchLine);
    QPushButton *button = searchLine.findChild<QPushButton *>(QStringLiteral("full_message"));
    QTest::mouseClick(button, Qt::LeftButton);
    QTest::keyClick(button, Qt::Key_Right);
    QSignalSpy spy(&searchLine, SIGNAL(searchOptionChanged()));
    button = searchLine.findChild<QPushButton *>(QStringLiteral("body"));
    QTest::mouseClick(button, Qt::LeftButton);
    QCOMPARE(spy.count(), 1);
}

void QuickSearchLineTest::shouldResetAllWhenResetFilter()
{
    QuickSearchLine searchLine;
    searchLine.show();
    searchLine.resetFilter();
    QCOMPARE(searchLine.status().count(), 0);
    QCOMPARE(searchLine.lockSearch()->isChecked(), false);
    QCOMPARE(searchLine.tagFilterComboBox()->currentIndex(), -1);
    QuickSearchLine::SearchOptions options;
    options = QuickSearchLine::SearchEveryWhere;
    QCOMPARE(searchLine.searchOptions(), options);
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
    QWidget *widget = searchLine.findChild<QWidget *>(QStringLiteral("extraoptions"));
    QVERIFY(!widget->isVisible());
    searchLine.searchEdit()->clear();
    searchLine.searchEdit()->setText(QStringLiteral(" "));
    QVERIFY(!widget->isVisible());

    searchLine.searchEdit()->clear();
    searchLine.searchEdit()->setText(QStringLiteral(""));
    QVERIFY(!widget->isVisible());

    searchLine.searchEdit()->clear();
    searchLine.searchEdit()->setText(QStringLiteral("FOO0 "));
    QVERIFY(widget->isVisible());

    searchLine.searchEdit()->clear();
    searchLine.searchEdit()->setText(QStringLiteral("FOOO "));
    QVERIFY(widget->isVisible());

}

void QuickSearchLineTest::shouldShowMoreOptionWhenClickOnMoreButton()
{
    QuickSearchLine searchLine;
    searchLine.show();
    QTest::qWaitForWindowExposed(&searchLine);
    QPushButton *moreButton = searchLine.findChild<QPushButton *>(QStringLiteral("moreoptions"));
    QTest::mouseClick(moreButton, Qt::LeftButton);
    QWidget *quickSearchFilterWidget = searchLine.findChild<QWidget *>(QStringLiteral("quicksearchfilterwidget"));
    QVERIFY(quickSearchFilterWidget->isVisible());
    QCOMPARE(moreButton->icon().name(), QStringLiteral("arrow-up-double"));

    QTest::mouseClick(moreButton, Qt::LeftButton);
    QVERIFY(!quickSearchFilterWidget->isVisible());
    QCOMPARE(moreButton->icon().name(), QStringLiteral("arrow-down-double"));
}

void QuickSearchLineTest::shouldChangeFromButtonLabelWhenChangeOutboundMessagesValue()
{
    QuickSearchLine searchLine;
    QPushButton *button = searchLine.findChild<QPushButton *>(QStringLiteral("fromorto"));
    const QString buttonName = button->text();
    searchLine.setContainsOutboundMessages(true);
    QVERIFY(button->text() != buttonName);
    searchLine.setContainsOutboundMessages(false);
    QCOMPARE(button->text(), buttonName);
}

void QuickSearchLineTest::shouldSearchToOrFrom()
{
    QuickSearchLine searchLine;
    QPushButton *button = searchLine.findChild<QPushButton *>(QStringLiteral("fromorto"));
    QTest::mouseClick(button, Qt::LeftButton);
    searchLine.setContainsOutboundMessages(true);
    QuickSearchLine::SearchOptions options;
    options = QuickSearchLine::SearchAgainstTo;
    QCOMPARE(searchLine.searchOptions(), options);

    searchLine.setContainsOutboundMessages(false);
    options = QuickSearchLine::SearchAgainstFrom;
    QCOMPARE(searchLine.searchOptions(), options);
}

void QuickSearchLineTest::shouldHideShowWidgetWhenWeChangeVisibility()
{
    QuickSearchLine searchLine;
    searchLine.show();

    QWidget *widget = searchLine.findChild<QWidget *>(QStringLiteral("extraoptions"));

    QPushButton *moreButton = searchLine.findChild<QPushButton *>(QStringLiteral("moreoptions"));
    QWidget *quickSearchFilterWidget = searchLine.findChild<QWidget *>(QStringLiteral("quicksearchfilterwidget"));
    searchLine.changeQuicksearchVisibility(false);
    QCOMPARE(quickSearchFilterWidget->isVisible(), false);
    QCOMPARE(moreButton->isVisible(), false);
    QCOMPARE(widget->isVisible(), false);
    QCOMPARE(searchLine.searchEdit()->isVisible(), false);
    QCOMPARE(searchLine.tagFilterComboBox()->isVisible(), false);

    searchLine.changeQuicksearchVisibility(true);
    QCOMPARE(quickSearchFilterWidget->isVisible(), false);
    QCOMPARE(moreButton->isVisible(), true);
    QCOMPARE(widget->isVisible(), false);
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

    QPushButton *moreButton = searchLine.findChild<QPushButton *>(QStringLiteral("moreoptions"));
    QCOMPARE(moreButton->isVisible(), true);
    QTest::mouseClick(moreButton, Qt::LeftButton);

    QPushButton *fullMessageButton = searchLine.findChild<QPushButton *>(QStringLiteral("full_message"));
    QVERIFY(!fullMessageButton->isVisible());
    QCOMPARE(fullMessageButton->isChecked(), true);

    QPushButton *bodyButton = searchLine.findChild<QPushButton *>(QStringLiteral("body"));
    QVERIFY(!bodyButton->isVisible());
    QCOMPARE(bodyButton->isChecked(), false);

    QPushButton *subjectButton = searchLine.findChild<QPushButton *>(QStringLiteral("subject"));
    QVERIFY(!subjectButton->isVisible());
    QCOMPARE(subjectButton->isChecked(), false);

    QPushButton *fromOrToButton = searchLine.findChild<QPushButton *>(QStringLiteral("fromorto"));
    QVERIFY(!fromOrToButton->isVisible());
    QCOMPARE(fromOrToButton->isChecked(), false);

    QPushButton *bccButton = searchLine.findChild<QPushButton *>(QStringLiteral("bcc"));
    QVERIFY(!bccButton->isVisible());
    QCOMPARE(bccButton->isChecked(), false);

    QTest::mouseClick(bccButton, Qt::LeftButton);
    QCOMPARE(fullMessageButton->isChecked(), false);
    QCOMPARE(bodyButton->isChecked(), false);
    QCOMPARE(subjectButton->isChecked(), false);
    QCOMPARE(fromOrToButton->isChecked(), false);
    QCOMPARE(bccButton->isChecked(), true);

    searchLine.resetFilter();
    QCOMPARE(fullMessageButton->isChecked(), true);
    QCOMPARE(bodyButton->isChecked(), false);
    QCOMPARE(subjectButton->isChecked(), false);
    QCOMPARE(fromOrToButton->isChecked(), false);
    QCOMPARE(bccButton->isChecked(), false);

}

void QuickSearchLineTest::shouldHideExtraOptionWidgetWhenResetFilterWhenSetEmptyText()
{
    QuickSearchLine searchLine;
    searchLine.show();

    searchLine.searchEdit()->setText(QStringLiteral("FOOFOO"));
    QTest::qWaitForWindowExposed(&searchLine);
    QWidget *widget = searchLine.findChild<QWidget *>(QStringLiteral("extraoptions"));

    QVERIFY(widget->isVisible());

    searchLine.searchEdit()->clear();
    QVERIFY(!widget->isVisible());

}

QTEST_MAIN(QuickSearchLineTest)
