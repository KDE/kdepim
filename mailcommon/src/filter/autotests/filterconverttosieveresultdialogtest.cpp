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

#include "filterconverttosieveresultdialogtest.h"
#include "../filterconverter/filterconverttosieveresultdialog.h"
#include <QDialogButtonBox>
#include <qtest.h>
#include <QPushButton>
#include <kpimtextedit/plaintexteditorwidget.h>
FilterConvertToSieveResultDialogTest::FilterConvertToSieveResultDialogTest(QObject *parent)
    : QObject(parent)
{

}

FilterConvertToSieveResultDialogTest::~FilterConvertToSieveResultDialogTest()
{

}

void FilterConvertToSieveResultDialogTest::shouldHaveDefaultValue()
{
    MailCommon::FilterConvertToSieveResultDialog dlg;
    QDialogButtonBox *buttonBox = dlg.findChild<QDialogButtonBox *>(QStringLiteral("buttonbox"));
    QVERIFY(buttonBox);

    QPushButton *saveButton = dlg.findChild<QPushButton *>(QStringLiteral("savebutton"));
    QVERIFY(saveButton);

    KPIMTextEdit::PlainTextEditorWidget *editor = dlg.findChild<KPIMTextEdit::PlainTextEditorWidget *>(QStringLiteral("editor"));
    QVERIFY(editor);
    QVERIFY(editor->toPlainText().isEmpty());
}

void FilterConvertToSieveResultDialogTest::shouldAddCode()
{
    MailCommon::FilterConvertToSieveResultDialog dlg;

    KPIMTextEdit::PlainTextEditorWidget *editor = dlg.findChild<KPIMTextEdit::PlainTextEditorWidget *>(QStringLiteral("editor"));
    QVERIFY(editor->toPlainText().isEmpty());
    const QString code = QStringLiteral("foo");
    dlg.setCode(code);
    QCOMPARE(editor->toPlainText(), code);
}

QTEST_MAIN(FilterConvertToSieveResultDialogTest)
