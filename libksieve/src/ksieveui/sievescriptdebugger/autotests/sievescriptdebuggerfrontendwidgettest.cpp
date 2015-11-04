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

#include "sievescriptdebuggerfrontendwidgettest.h"
#include "../sievescriptdebuggerfrontendwidget.h"
#include <KLineEdit>
#include <KPIMTextEdit/kpimtextedit/texttospeechwidget.h>
#include <KUrlRequester>
#include <QLabel>
#include <QSplitter>
#include <QTest>
#include <sievescriptdebugger/sievescriptdebuggerwarning.h>

SieveScriptDebuggerFrontEndWidgetTest::SieveScriptDebuggerFrontEndWidgetTest(QObject *parent)
    : QObject(parent)
{

}

SieveScriptDebuggerFrontEndWidgetTest::~SieveScriptDebuggerFrontEndWidgetTest()
{

}

void SieveScriptDebuggerFrontEndWidgetTest::shouldHaveDefaultValue()
{
    KSieveUi::SieveScriptDebuggerFrontEndWidget w;
    QVERIFY(w.script().isEmpty());

    QSplitter *splitter = w.findChild<QSplitter *>(QStringLiteral("splitter"));
    QVERIFY(splitter);
    QCOMPARE(splitter->count(), 2);

    KSieveUi::SieveScriptDebuggerWarning *warning = w.findChild<KSieveUi::SieveScriptDebuggerWarning *>(QStringLiteral("sievescriptdebuggerwarning"));
    QVERIFY(warning);

    QLabel *emailLab = w.findChild<QLabel *>(QStringLiteral("emaillab"));
    QVERIFY(emailLab);

    KUrlRequester *emailPath = w.findChild<KUrlRequester *>(QStringLiteral("emailpath"));
    QVERIFY(emailPath);

    QPushButton *debugScriptButton = w.findChild<QPushButton *>(QStringLiteral("debugbutton"));
    QVERIFY(debugScriptButton);
    QVERIFY(!debugScriptButton->isEnabled());

    QLabel *extensionLab = w.findChild<QLabel *>(QStringLiteral("extensionlab"));
    QVERIFY(extensionLab);

    KLineEdit *extension = w.findChild<KLineEdit *>(QStringLiteral("extension"));
    QVERIFY(extension);
    QVERIFY(extension->text().isEmpty());
    QVERIFY(extension->isClearButtonEnabled());
    QVERIFY(extension->trapReturnKey());

    KPIMTextEdit::TextToSpeechWidget *textToSpeechWidget = w.findChild<KPIMTextEdit::TextToSpeechWidget *>(QStringLiteral("texttospeechwidget"));
    QVERIFY(textToSpeechWidget);
}

void SieveScriptDebuggerFrontEndWidgetTest::shouldChangeButtonEnabledState()
{
    KSieveUi::SieveScriptDebuggerFrontEndWidget w;
    QVERIFY(w.script().isEmpty());

    KUrlRequester *emailPath = w.findChild<KUrlRequester *>(QStringLiteral("emailpath"));
    emailPath->setUrl(QUrl::fromLocalFile(QStringLiteral("/")));
    QPushButton *debugScriptButton = w.findChild<QPushButton *>(QStringLiteral("debugbutton"));

    QVERIFY(debugScriptButton->isEnabled());

    emailPath->setUrl(QUrl::fromLocalFile(QStringLiteral("    ")));
    QVERIFY(!debugScriptButton->isEnabled());
}

void SieveScriptDebuggerFrontEndWidgetTest::shouldAddScript()
{
    KSieveUi::SieveScriptDebuggerFrontEndWidget w;
    const QString script = QStringLiteral("foo foo \n bla bla");
    w.setScript(script);
    QCOMPARE(w.script(), script);
}

void SieveScriptDebuggerFrontEndWidgetTest::shouldNotAccept()
{
    KSieveUi::SieveScriptDebuggerFrontEndWidget w;
    w.show();
    w.setScript(QStringLiteral("debug_log"));
    KSieveUi::SieveScriptDebuggerWarning *warning = w.findChild<KSieveUi::SieveScriptDebuggerWarning *>(QStringLiteral("sievescriptdebuggerwarning"));

    QVERIFY(!w.canAccept());
    QVERIFY(warning->isVisible());
    w.setScript(QStringLiteral("foo foo \n bla bla"));
    QVERIFY(w.canAccept());
    QVERIFY(!warning->isVisible());

    w.setScript(QStringLiteral("require \"vnd.dovecot.debug\""));
    QVERIFY(!w.canAccept());
    QVERIFY(warning->isVisible());
}

QTEST_MAIN(SieveScriptDebuggerFrontEndWidgetTest)
