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


#include "richtexteditortest.h"
#include "pimcommon/texteditor/richtexteditor/richtexteditor.h"
#include <qtest.h>

RichTextEditorTest::RichTextEditorTest(QObject *parent)
    : QObject(parent)
{
}


RichTextEditorTest::~RichTextEditorTest()
{

}

void RichTextEditorTest::shouldHaveDefaultValue()
{
    PimCommon::RichTextEditor editor;
    QCOMPARE(editor.spellCheckingSupport(), true);
    QCOMPARE(editor.textToSpeechSupport(), true);
    QCOMPARE(editor.searchSupport(), true);
}

void RichTextEditorTest::shouldChangeSpellCheckValue()
{
    PimCommon::RichTextEditor editor;
    editor.setSpellCheckingSupport(false);
    QCOMPARE(editor.spellCheckingSupport(), false);

    editor.setTextToSpeechSupport(false);
    QCOMPARE(editor.textToSpeechSupport(), false);

    editor.setSearchSupport(false);
    QCOMPARE(editor.searchSupport(), false);

    editor.setSpellCheckingSupport(true);
    QCOMPARE(editor.spellCheckingSupport(), true);

    editor.setTextToSpeechSupport(true);
    QCOMPARE(editor.textToSpeechSupport(), true);

    editor.setSearchSupport(true);
    QCOMPARE(editor.searchSupport(), true);
}

QTEST_MAIN(RichTextEditorTest)
