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

#include "editorutiltest.h"
#include "../editorutil.h"
#include <QTextDocument>
#include <qtest.h>

EditorUtilTest::EditorUtilTest(QObject *parent)
    : QObject(parent)
{

}

EditorUtilTest::~EditorUtilTest()
{

}

void EditorUtilTest::testUpperCase_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("result");
    QTest::addColumn<int>("startcursorposition");
    QTest::addColumn<int>("endcursorposition");
    QTest::newRow("onelinewithoutselection") <<  QStringLiteral("foo") << QStringLiteral("foo") << -1 << -1;
    QTest::newRow("onelinewithselection") <<  QStringLiteral("foo") << QStringLiteral("FOO") << 0 << 3;
    QTest::newRow("onelinewithselectionuppercase") <<  QStringLiteral("FOO") << QStringLiteral("FOO") << 0 << 3;
}

void EditorUtilTest::testUpperCase()
{
    QFETCH(QString, input);
    QFETCH(QString, result);
    QFETCH(int, startcursorposition);
    QFETCH(int, endcursorposition);
    QTextDocument *document = new QTextDocument(this);
    document->setPlainText(input);
    QTextCursor textCursor(document);
    if (startcursorposition != -1 && endcursorposition != -1) {
        textCursor.setPosition(startcursorposition);
        textCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, endcursorposition - startcursorposition);
    }
    PimCommon::EditorUtil editorUtil;
    editorUtil.upperCase(textCursor);
    QCOMPARE(textCursor.document()->toPlainText(), result);
    delete document;
}

void EditorUtilTest::testLowerCase_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("result");
    QTest::addColumn<int>("startcursorposition");
    QTest::addColumn<int>("endcursorposition");
    QTest::newRow("onelinewithoutselection") <<  QStringLiteral("foo") << QStringLiteral("foo") << -1 << -1;
    QTest::newRow("onelinewithselection") <<  QStringLiteral("foo") << QStringLiteral("foo") << 0 << 3;
    QTest::newRow("onelinewithselectionuppercase") <<  QStringLiteral("FOO") << QStringLiteral("foo") << 0 << 3;
}

void EditorUtilTest::testLowerCase()
{
    QFETCH(QString, input);
    QFETCH(QString, result);
    QFETCH(int, startcursorposition);
    QFETCH(int, endcursorposition);
    QTextDocument *document = new QTextDocument(this);
    document->setPlainText(input);
    QTextCursor textCursor(document);
    if (startcursorposition != -1 && endcursorposition != -1) {
        textCursor.setPosition(startcursorposition);
        textCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, endcursorposition - startcursorposition);
    }
    PimCommon::EditorUtil editorUtil;
    editorUtil.lowerCase(textCursor);
    QCOMPARE(textCursor.document()->toPlainText(), result);
    delete document;
}

void EditorUtilTest::testSentenceCase_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("result");
    QTest::addColumn<int>("startcursorposition");
    QTest::addColumn<int>("endcursorposition");
    QTest::newRow("onelinewithoutselection") <<  QStringLiteral("foo") << QStringLiteral("foo") << -1 << -1;
    QTest::newRow("onelinewithselection") <<  QStringLiteral("foo") << QStringLiteral("Foo") << 0 << 3;
    QTest::newRow("bigline") <<  QStringLiteral("foo bla foo.\nddd") << QStringLiteral("Foo bla foo.\nDdd") << 0 << 16;
}

void EditorUtilTest::testSentenceCase()
{
    QFETCH(QString, input);
    QFETCH(QString, result);
    QFETCH(int, startcursorposition);
    QFETCH(int, endcursorposition);
    QTextDocument *document = new QTextDocument(this);
    document->setPlainText(input);
    QTextCursor textCursor(document);
    if (startcursorposition != -1 && endcursorposition != -1) {
        textCursor.setPosition(startcursorposition);
        textCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, endcursorposition - startcursorposition);
    }
    PimCommon::EditorUtil editorUtil;
    editorUtil.sentenceCase(textCursor);
    QCOMPARE(textCursor.document()->toPlainText(), result);
    delete document;
}

void EditorUtilTest::testReverseCase_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("result");
    QTest::addColumn<int>("startcursorposition");
    QTest::addColumn<int>("endcursorposition");
    QTest::newRow("onelinewithoutselection") <<  QStringLiteral("foo") << QStringLiteral("foo") << -1 << -1;
    QTest::newRow("onelinewithselection") <<  QStringLiteral("foo") << QStringLiteral("FOO")<< 0 << 3;
    QTest::newRow("reverseCase") <<  QStringLiteral("fOo bla\tfOO") << QStringLiteral("FoO BLA\tFoo") << 0 << 12;
    QTest::newRow("notallineselected") <<  QStringLiteral("fOo bla\tfOO") << QStringLiteral("FoO bla\tfOO") << 0 << 3;
}

void EditorUtilTest::testReverseCase()
{
    QFETCH(QString, input);
    QFETCH(QString, result);
    QFETCH(int, startcursorposition);
    QFETCH(int, endcursorposition);

    QTextDocument *document = new QTextDocument(this);
    document->setPlainText(input);
    QTextCursor textCursor(document);
    if (startcursorposition != -1 && endcursorposition != -1) {
        textCursor.setPosition(startcursorposition);
        textCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, endcursorposition - startcursorposition);
    }
    PimCommon::EditorUtil editorUtil;
    editorUtil.reverseCase(textCursor);
    QCOMPARE(textCursor.document()->toPlainText(), result);
    delete document;
}

QTEST_MAIN(EditorUtilTest)
