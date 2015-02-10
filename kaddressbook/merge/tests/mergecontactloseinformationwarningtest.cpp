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
#include <qtest_kde.h>

MergeContactLoseInformationWarningTest::MergeContactLoseInformationWarningTest(QObject *parent)
    : QObject(parent)
{

}

void MergeContactLoseInformationWarningTest::shouldHaveDefaultValue()
{
    KABMergeContacts::MergeContactLoseInformationWarning w;
    QVERIFY(!w.isCloseButtonVisible());
    QVERIFY(!w.isVisible());
    QAction *customize = qFindChild<QAction *>(&w, QLatin1String("customize"));
    QVERIFY(customize);
    QAction *automatic = qFindChild<QAction *>(&w, QLatin1String("automatic"));
    QVERIFY(automatic);
}

QTEST_KDEMAIN(MergeContactLoseInformationWarningTest, GUI)
