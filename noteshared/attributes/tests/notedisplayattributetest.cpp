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

#include "notedisplayattributetest.h"
#include <qtest_kde.h>
#include "attributes/notedisplayattribute.h"

NoteDisplayAttributeTest::NoteDisplayAttributeTest(QObject *parent)
    : QObject(parent)
{

}

NoteDisplayAttributeTest::~NoteDisplayAttributeTest()
{

}

void NoteDisplayAttributeTest::shouldHaveDefaultValue()
{
#if 0
    mFont(KGlobalSettings::generalFont()),
    mTitleFont(KGlobalSettings::windowTitleFont()),
    mBackgroundColor(Qt::yellow),
    mForegroundgroundColor(Qt::black),
    mSize(300,300),
    mPosition(QPoint( -10000, -10000 )),
    mTabSize(4),
    mDesktop(-10),
    mRememberDesktop(true),
#endif


    NoteShared::NoteDisplayAttribute attribute;
    QVERIFY(attribute.autoIndent());
    QVERIFY(!attribute.keepBelow());
    QVERIFY(!attribute.keepAbove());
    QVERIFY(!attribute.showInTaskbar());
    QVERIFY(!attribute.isHidden());
#if 0
    QColor backgroundColor() const;
    QColor foregroundColor() const;
    QSize size() const;
    bool rememberDesktop() const;
    int tabSize() const;
    QFont font() const;
    QFont titleFont() const;
    int desktop() const;
    QPoint position() const;
    bool showInTaskbar() const;
#endif
}

QTEST_KDEMAIN(NoteDisplayAttributeTest, NoGUI)
