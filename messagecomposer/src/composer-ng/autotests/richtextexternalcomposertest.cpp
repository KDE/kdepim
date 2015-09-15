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

#include "richtextexternalcomposertest.h"
#include "../richtextexternalcomposer.h"
#include "../richtextcomposer.h"
#include <qtest.h>

RichTextExternalComposerTest::RichTextExternalComposerTest(QObject *parent)
    : QObject(parent)
{

}

RichTextExternalComposerTest::~RichTextExternalComposerTest()
{

}

void RichTextExternalComposerTest::shouldHaveDefaultValue()
{
    MessageComposer::RichTextComposer composer;
    MessageComposer::RichTextExternalComposer richTextExternal(&composer);
    QVERIFY(!richTextExternal.useExternalEditor());
    QVERIFY(richTextExternal.externalEditorPath().isEmpty());
    QVERIFY(!richTextExternal.isInProgress());
}

void RichTextExternalComposerTest::shouldChangeUseExternalComposer()
{
    MessageComposer::RichTextComposer composer;
    MessageComposer::RichTextExternalComposer richTextExternal(&composer);
    richTextExternal.setUseExternalEditor(true);
    QVERIFY(richTextExternal.useExternalEditor());
    richTextExternal.setUseExternalEditor(false);
    QVERIFY(!richTextExternal.useExternalEditor());
}

QTEST_MAIN(RichTextExternalComposerTest)
