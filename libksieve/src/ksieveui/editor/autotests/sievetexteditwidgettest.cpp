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

#include "sievetexteditwidgettest.h"
#include "../sievetexteditwidget.h"
#include <QTest>
#include <editor/sievetextedit.h>
#include <kpimtextedit/plaintexteditfindbar.h>
#include <kpimtextedit/slidecontainer.h>

SieveTextEditWidgetTest::SieveTextEditWidgetTest(QObject *parent)
    : QObject(parent)
{

}

SieveTextEditWidgetTest::~SieveTextEditWidgetTest()
{

}

void SieveTextEditWidgetTest::shouldHaveDefaultValue()
{
    KSieveUi::SieveTextEditWidget w;
    KSieveUi::SieveTextEdit *textedit = w.findChild<KSieveUi::SieveTextEdit *>(QStringLiteral("textedit"));
    QVERIFY(textedit);
    QVERIFY(!textedit->isReadOnly());

    KPIMTextEdit::SlideContainer *slider = w.findChild<KPIMTextEdit::SlideContainer *>(QStringLiteral("slidercontainer"));
    QVERIFY(slider);
    QWidget *contentWidget = slider->content();
    QVERIFY(contentWidget);
    QVERIFY(dynamic_cast<KPIMTextEdit::PlainTextEditFindBar *>(contentWidget));

    QVERIFY(w.textEdit());
}

QTEST_MAIN(SieveTextEditWidgetTest)
