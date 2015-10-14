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

#include "richtextcomposercontrolertest.h"
#include "../richtextcomposercontroler.h"
#include "../richtextcomposer.h"
#include <KActionCollection>
#include <qtest.h>

RichTextComposerControlerTest::RichTextComposerControlerTest(QObject *parent)
    : QObject(parent)
{

}

RichTextComposerControlerTest::~RichTextComposerControlerTest()
{

}

void RichTextComposerControlerTest::shouldAlignLeft()
{
    MessageComposer::RichTextComposer composer;
    KActionCollection *actionCollection = new KActionCollection(&composer);
    composer.createActions(actionCollection);
    MessageComposer::RichTextComposerControler controler(&composer);
    composer.show();
    QTest::qWaitForWindowShown(&composer);
    controler.alignLeft();
    QVERIFY(controler.richTextComposer()->hasFocus());
    QCOMPARE(controler.richTextComposer()->alignment(), Qt::AlignLeft);
    QVERIFY(controler.richTextComposer()->acceptRichText());
}

void RichTextComposerControlerTest::shouldAlignRight()
{
    MessageComposer::RichTextComposer composer;
    KActionCollection *actionCollection = new KActionCollection(&composer);
    composer.createActions(actionCollection);
    MessageComposer::RichTextComposerControler controler(&composer);
    composer.show();
    QTest::qWaitForWindowShown(&composer);
    controler.alignRight();
    QVERIFY(controler.richTextComposer()->hasFocus());
    QCOMPARE(controler.richTextComposer()->alignment(), Qt::AlignRight);
    QVERIFY(controler.richTextComposer()->acceptRichText());
}

void RichTextComposerControlerTest::shouldAlignJustify()
{
    MessageComposer::RichTextComposer composer;
    KActionCollection *actionCollection = new KActionCollection(&composer);
    composer.createActions(actionCollection);
    MessageComposer::RichTextComposerControler controler(&composer);
    composer.show();
    QTest::qWaitForWindowShown(&composer);
    controler.alignJustify();
    QVERIFY(controler.richTextComposer()->hasFocus());
    QCOMPARE(controler.richTextComposer()->alignment(), Qt::AlignJustify);
    QVERIFY(controler.richTextComposer()->acceptRichText());
}

void RichTextComposerControlerTest::shouldAlignCenter()
{
    MessageComposer::RichTextComposer composer;
    KActionCollection *actionCollection = new KActionCollection(&composer);
    composer.createActions(actionCollection);
    MessageComposer::RichTextComposerControler controler(&composer);
    composer.show();
    QTest::qWaitForWindowShown(&composer);
    controler.alignCenter();
    QVERIFY(controler.richTextComposer()->hasFocus());
    QCOMPARE(controler.richTextComposer()->alignment(), Qt::AlignHCenter);
    QVERIFY(controler.richTextComposer()->acceptRichText());
}

void RichTextComposerControlerTest::shouldHaveDefaultValue()
{
    MessageComposer::RichTextComposer composer;
    MessageComposer::RichTextComposerControler controler(&composer);
    QVERIFY(!controler.painterActive());
    QVERIFY(!controler.richTextComposer()->acceptRichText());
}

void RichTextComposerControlerTest::shouldAddQuote()
{
#if 0
    MessageComposer::RichTextComposer composer;
    KActionCollection *actionCollection = new KActionCollection(&composer);
    composer.createActions(actionCollection);
    MessageComposer::RichTextComposerControler controler(&composer);

    controler.alignLeft();
#endif
}

void RichTextComposerControlerTest::shouldBoldText()
{
    MessageComposer::RichTextComposer composer;
    KActionCollection *actionCollection = new KActionCollection(&composer);
    composer.createActions(actionCollection);
    MessageComposer::RichTextComposerControler controler(&composer);
    composer.show();
    QTest::qWaitForWindowShown(&composer);
    controler.setTextBold(true);
    QVERIFY(controler.richTextComposer()->hasFocus());
    QVERIFY(controler.richTextComposer()->acceptRichText());
    //TODO text format.
}

void RichTextComposerControlerTest::shouldItalicText()
{
    MessageComposer::RichTextComposer composer;
    KActionCollection *actionCollection = new KActionCollection(&composer);
    composer.createActions(actionCollection);
    MessageComposer::RichTextComposerControler controler(&composer);
    composer.show();
    QTest::qWaitForWindowShown(&composer);
    controler.setTextItalic(true);
    QVERIFY(controler.richTextComposer()->hasFocus());
    QVERIFY(controler.richTextComposer()->acceptRichText());
    //TODO text format.
}

void RichTextComposerControlerTest::shouldTextUnderline()
{
    MessageComposer::RichTextComposer composer;
    KActionCollection *actionCollection = new KActionCollection(&composer);
    composer.createActions(actionCollection);
    MessageComposer::RichTextComposerControler controler(&composer);
    composer.show();
    QTest::qWaitForWindowShown(&composer);
    controler.setTextUnderline(true);
    QVERIFY(controler.richTextComposer()->hasFocus());
    QVERIFY(controler.richTextComposer()->acceptRichText());
    //TODO text format.

}

void RichTextComposerControlerTest::shouldTextStrikeOut()
{
    MessageComposer::RichTextComposer composer;
    KActionCollection *actionCollection = new KActionCollection(&composer);
    composer.createActions(actionCollection);
    MessageComposer::RichTextComposerControler controler(&composer);
    composer.show();
    QTest::qWaitForWindowShown(&composer);
    controler.setTextStrikeOut(true);
    QVERIFY(controler.richTextComposer()->hasFocus());
    QVERIFY(controler.richTextComposer()->acceptRichText());
    //TODO text format.

}

void RichTextComposerControlerTest::shouldFontFamily()
{
    MessageComposer::RichTextComposer composer;
    KActionCollection *actionCollection = new KActionCollection(&composer);
    composer.createActions(actionCollection);
    MessageComposer::RichTextComposerControler controler(&composer);
    composer.show();
    QTest::qWaitForWindowShown(&composer);
    //TODO
}

void RichTextComposerControlerTest::shouldFontSize()
{
    MessageComposer::RichTextComposer composer;
    KActionCollection *actionCollection = new KActionCollection(&composer);
    composer.createActions(actionCollection);
    MessageComposer::RichTextComposerControler controler(&composer);
    composer.show();
    QTest::qWaitForWindowShown(&composer);
    //TODO

}

void RichTextComposerControlerTest::shouldFont()
{
    MessageComposer::RichTextComposer composer;
    KActionCollection *actionCollection = new KActionCollection(&composer);
    composer.createActions(actionCollection);
    MessageComposer::RichTextComposerControler controler(&composer);
    composer.show();
    QTest::qWaitForWindowShown(&composer);
    //TODO

}

void RichTextComposerControlerTest::shouldTextSuperScript()
{
    MessageComposer::RichTextComposer composer;
    KActionCollection *actionCollection = new KActionCollection(&composer);
    composer.createActions(actionCollection);
    MessageComposer::RichTextComposerControler controler(&composer);
    composer.show();
    QTest::qWaitForWindowShown(&composer);
    controler.setTextSuperScript(true);
    QVERIFY(controler.richTextComposer()->hasFocus());
    QVERIFY(controler.richTextComposer()->acceptRichText());
    //TODO

}

void RichTextComposerControlerTest::shouldTextSubScript()
{
    MessageComposer::RichTextComposer composer;
    KActionCollection *actionCollection = new KActionCollection(&composer);
    composer.createActions(actionCollection);
    MessageComposer::RichTextComposerControler controler(&composer);
    composer.show();
    QTest::qWaitForWindowShown(&composer);
    controler.setTextSubScript(true);
    QVERIFY(controler.richTextComposer()->hasFocus());
    QVERIFY(controler.richTextComposer()->acceptRichText());

    //TODO

}

QTEST_MAIN(RichTextComposerControlerTest)
