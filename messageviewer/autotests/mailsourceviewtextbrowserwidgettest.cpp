/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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
#include "../widgets/mailsourceviewtextbrowserwidget.h"
#include "../findbar/findbarsourceview.h"
#include "pimcommon/texttospeech/texttospeechwidget.h"
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

    MessageViewer::MailSourceViewTextBrowser *textbrowser = widget.findChild<MessageViewer::MailSourceViewTextBrowser *>(QLatin1String("textbrowser"));
    QVERIFY(textbrowser);
    QVERIFY(!textbrowser->isHidden());
    MessageViewer::FindBarSourceView *findbar = widget.findChild<MessageViewer::FindBarSourceView *>(QLatin1String("findbar"));
    QVERIFY(findbar);
    QVERIFY(findbar->isHidden());

    PimCommon::TextToSpeechWidget *texttospeechwidget = widget.findChild<PimCommon::TextToSpeechWidget *>(QLatin1String("texttospeech"));
    QVERIFY(texttospeechwidget);
    QVERIFY(texttospeechwidget->isHidden());

}

QTEST_MAIN(MailSourceViewTextBrowserWidgetTest)
