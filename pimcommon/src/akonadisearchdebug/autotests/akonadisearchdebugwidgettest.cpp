/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#include "akonadisearchdebugwidgettest.h"
#include "texteditor/plaintexteditor/plaintexteditorwidget.h"
#include "../akonadisearchdebugwidget.h"
#include <qtest.h>
#include "../akonadisearchdebugsearchpathcombobox.h"
#include <KLineEdit>
#include <QPushButton>

AkonadiSearchDebugWidgetTest::AkonadiSearchDebugWidgetTest(QObject *parent)
    : QObject(parent)
{

}

AkonadiSearchDebugWidgetTest::~AkonadiSearchDebugWidgetTest()
{

}

void AkonadiSearchDebugWidgetTest::shouldHaveDefaultValue()
{
    PimCommon::AkonadiSearchDebugWidget widget;
    QPushButton *button = widget.findChild<QPushButton *>(QStringLiteral("searchbutton"));
    QVERIFY(button);
    QVERIFY(!button->isEnabled());
    KLineEdit *lineEdit = widget.findChild<KLineEdit *>(QStringLiteral("lineedit"));
    QVERIFY(lineEdit);
    QVERIFY(lineEdit->text().isEmpty());
    QVERIFY(lineEdit->trapReturnKey());
    QVERIFY(lineEdit->isClearButtonShown());
    PimCommon::PlainTextEditorWidget *editorWidget = widget.findChild<PimCommon::PlainTextEditorWidget *>(QStringLiteral("plaintexteditor"));
    QVERIFY(editorWidget->isReadOnly());
    QVERIFY(editorWidget);
    QVERIFY(editorWidget->toPlainText().isEmpty());
    PimCommon::AkonadiSearchDebugSearchPathComboBox *searchCombo = widget.findChild<PimCommon::AkonadiSearchDebugSearchPathComboBox *>(QStringLiteral("searchpathcombo"));
    QVERIFY(searchCombo);
}

void AkonadiSearchDebugWidgetTest::shouldFillLineEditWhenWeWantToSearchItem()
{
    PimCommon::AkonadiSearchDebugWidget widget;
    KLineEdit *lineEdit = widget.findChild<KLineEdit *>(QStringLiteral("lineedit"));
    const int value = 42;
    const QString akonadiItem = QString::number(value);
    widget.setAkonadiId(value);
    QCOMPARE(lineEdit->text(), akonadiItem);
}

void AkonadiSearchDebugWidgetTest::shouldEnabledPushButtonWhenLineEditIsNotEmpty()
{
    PimCommon::AkonadiSearchDebugWidget widget;
    const int value = 42;
    widget.setAkonadiId(value);
    QPushButton *button = widget.findChild<QPushButton *>(QStringLiteral("searchbutton"));
    QVERIFY(button->isEnabled());

    KLineEdit *lineEdit = widget.findChild<KLineEdit *>(QStringLiteral("lineedit"));
    lineEdit->setText(QStringLiteral(""));
    QVERIFY(!button->isEnabled());

    //trimmed string
    lineEdit->setText(QStringLiteral(" "));
    QVERIFY(!button->isEnabled());

}

void AkonadiSearchDebugWidgetTest::shouldChangeSearchType()
{
    PimCommon::AkonadiSearchDebugWidget widget;
    PimCommon::AkonadiSearchDebugSearchPathComboBox::SearchType type = PimCommon::AkonadiSearchDebugSearchPathComboBox::Emails;
    widget.setSearchType(type);
    PimCommon::AkonadiSearchDebugSearchPathComboBox *searchCombo = widget.findChild<PimCommon::AkonadiSearchDebugSearchPathComboBox *>(QStringLiteral("searchpathcombo"));
    const QString path = searchCombo->pathFromEnum(type);
    QCOMPARE(searchCombo->searchPath(), path);

}

QTEST_MAIN(AkonadiSearchDebugWidgetTest)
