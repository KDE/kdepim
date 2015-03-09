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

#include "sievetextedittest.h"
#include "../sievetextedit.h"
#include <qtest_kde.h>

SieveTextEditTest::SieveTextEditTest(QObject *parent)
    : QObject(parent)
{

}

SieveTextEditTest::~SieveTextEditTest()
{

}

void SieveTextEditTest::shouldHaveDefaultValue()
{
    KSieveUi::SieveTextEdit edit;
    QVERIFY(edit.toPlainText().isEmpty());
}

void SieveTextEditTest::comment_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("result");
    QTest::addColumn<int>("startcursorposition");
    QTest::addColumn<int>("endcursorposition");
    QTest::newRow("onelinewithoutselection") <<  QString(QLatin1String("foo")) << QString(QLatin1String("#foo")) << -1 << -1;
    //Comment first line
    QTest::newRow("multilinewithoutselection") <<  QString(QLatin1String("foo\nbla")) << QString(QLatin1String("#foo\nbla")) << -1 << -1;
    QTest::newRow("onelinewithselection") <<  QString(QLatin1String("foo")) << QString(QLatin1String("#foo")) << 0 << 2;
    QTest::newRow("multilinewithselection") <<  QString(QLatin1String("foo\nbla")) << QString(QLatin1String("#foo\n#bla")) << 0 << 5;
}

void SieveTextEditTest::comment()
{
    QFETCH( QString, input );
    QFETCH( QString, result );
    QFETCH( int, startcursorposition);
    QFETCH( int, endcursorposition);
    KSieveUi::SieveTextEdit edit;
    edit.setPlainText(input);
    if (startcursorposition != -1 && endcursorposition != -1) {
        QTextCursor cursor = edit.textCursor();
        cursor.setPosition(startcursorposition);
        cursor.movePosition(QTextCursor::NextCharacter,QTextCursor::KeepAnchor, endcursorposition - startcursorposition );
        edit.setTextCursor(cursor);
    }
    edit.comment();
    QCOMPARE(edit.toPlainText(), result);
}


void SieveTextEditTest::uncomment_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("result");
    QTest::addColumn<int>("startcursorposition");
    QTest::addColumn<int>("endcursorposition");
    QTest::newRow("commentedline") <<  QString(QLatin1String("#foo")) << QString(QLatin1String("foo")) << -1 << -1;
    QTest::newRow("nocommentedline") <<  QString(QLatin1String("foo")) << QString(QLatin1String("foo")) << -1 << -1;
    QTest::newRow("onelinewithselection") <<  QString(QLatin1String("#foo")) << QString(QLatin1String("foo")) << 0 << 2;
}

void SieveTextEditTest::uncomment()
{
    QFETCH( QString, input );
    QFETCH( QString, result );
    QFETCH( int, startcursorposition);
    QFETCH( int, endcursorposition);
    KSieveUi::SieveTextEdit edit;
    edit.setPlainText(input);
    if (startcursorposition != -1 && endcursorposition != -1) {
        QTextCursor cursor = edit.textCursor();
        cursor.setPosition(endcursorposition);
        cursor.movePosition(QTextCursor::PreviousCharacter,QTextCursor::KeepAnchor, endcursorposition - startcursorposition );
        edit.setTextCursor(cursor);
    }

    edit.uncomment();
    QCOMPARE(edit.toPlainText(), result);
}

QTEST_KDEMAIN(SieveTextEditTest, GUI)
