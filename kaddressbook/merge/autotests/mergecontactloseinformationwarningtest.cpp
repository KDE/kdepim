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

#include "mergecontactloseinformationwarningtest.h"
#include "../widgets/mergecontactloseinformationwarning.h"
#include <QAction>
#include <qtest.h>
#include <QSignalSpy>
#include <qtestmouse.h>

MergeContactLoseInformationWarningTest::MergeContactLoseInformationWarningTest(QObject *parent)
    : QObject(parent)
{

}

void MergeContactLoseInformationWarningTest::shouldHaveDefaultValue()
{
    KABMergeContacts::MergeContactLoseInformationWarning w;
    QVERIFY(!w.isCloseButtonVisible());
    QVERIFY(!w.isVisible());
    QAction *customize = w.findChild<QAction *>(QLatin1String("customize"));
    QVERIFY(customize);
    QAction *automatic = w.findChild<QAction *>(QLatin1String("automatic"));
    QVERIFY(automatic);
}

void MergeContactLoseInformationWarningTest::shouldEmitSignals()
{
    KABMergeContacts::MergeContactLoseInformationWarning w;
    w.show();
    QTest::qWaitForWindowShown(&w);
    QVERIFY(w.isVisible());
    QAction *customize = w.findChild<QAction *>(QLatin1String("customize"));
    QAction *automatic = w.findChild<QAction *>(QLatin1String("automatic"));
    QSignalSpy spy1(&w, SIGNAL(continueMerging()));
    QSignalSpy spy2(&w, SIGNAL(customizeMergingContacts()));
    customize->trigger();
    QCOMPARE(spy1.count(), 0);
    QCOMPARE(spy2.count(), 1);
    automatic->trigger();
    QCOMPARE(spy2.count(), 1);
    QCOMPARE(spy1.count(), 1);
}

QTEST_MAIN(MergeContactLoseInformationWarningTest)
