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

#include "mailsourceviewtextbrowserwidgettest.h"
#include "../src/widgets/mailsourceviewtextbrowserwidget.h"
#include "../src/findbar/findbarsourceview.h"
#include "kpimtextedit/texttospeechwidget.h"
#include <qtest.h>

MailSourceViewTextBrowserWidgetTest::MailSourceViewTextBrowserWidgetTest(QObject *parent)
    : QObject(parent)
{

}

MailSourceViewTextBrowserWidgetTest::~MailSourceViewTextBrowserWidgetTest()
{

}

void MailSourceViewTextBrowserWidgetTest::shouldHaveDefaultValue()
{
    MessageViewer::MailSourceViewTextBrowserWidget widget;

    MessageViewer::MailSourceViewTextBrowser *textbrowser = widget.findChild<MessageViewer::MailSourceViewTextBrowser *>(QStringLiteral("textbrowser"));
    QVERIFY(textbrowser);
    QVERIFY(!textbrowser->isHidden());
    MessageViewer::FindBarSourceView *findbar = widget.findChild<MessageViewer::FindBarSourceView *>(QStringLiteral("findbar"));
    QVERIFY(findbar);
    QVERIFY(findbar->isHidden());

    KPIMTextEdit::TextToSpeechWidget *texttospeechwidget = widget.findChild<KPIMTextEdit::TextToSpeechWidget *>(QStringLiteral("texttospeech"));
    QVERIFY(texttospeechwidget);
    QVERIFY(texttospeechwidget->isHidden());

}

QTEST_MAIN(MailSourceViewTextBrowserWidgetTest)
