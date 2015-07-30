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

#include "richtextcomposertest.h"
#include "../richtextcomposer.h"
#include <qtest.h>
#include <qsignalspy.h>
#include <KActionCollection>
Q_DECLARE_METATYPE(MessageComposer::RichTextComposer::Mode)
RichTextComposerTest::RichTextComposerTest(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<MessageComposer::RichTextComposer::Mode>();
}

RichTextComposerTest::~RichTextComposerTest()
{

}

void RichTextComposerTest::shouldHaveDefaultValue()
{
    MessageComposer::RichTextComposer composer;
    QVERIFY(!composer.autocorrection());
    QCOMPARE(composer.linePosition(), 0);
    QCOMPARE(composer.columnNumber(), 0);
    QCOMPARE(composer.textMode(), MessageComposer::RichTextComposer::Plain);
    QVERIFY(!composer.acceptRichText());
    QVERIFY(!composer.quotePrefixName().isEmpty());
}

void RichTextComposerTest::shouldChangeMode()
{
    MessageComposer::RichTextComposer composer;
    KActionCollection *actionCollection = new KActionCollection(&composer);
    composer.createActions(actionCollection);
    QSignalSpy spy(&composer, SIGNAL(textModeChanged(MessageComposer::RichTextComposer::Mode)));
    composer.activateRichText();
    QCOMPARE(composer.textMode(), MessageComposer::RichTextComposer::Rich);
    QVERIFY(composer.acceptRichText());
    QCOMPARE(spy.count(), 1);
}

QTEST_MAIN(RichTextComposerTest)
