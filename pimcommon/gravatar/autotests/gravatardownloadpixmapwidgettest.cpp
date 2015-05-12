/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  based on code from Sune Vuorela <sune@vuorela.dk> (Rawatar source code)

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


#include "gravatardownloadpixmapwidgettest.h"
#include "../widgets/gravatardownloadpixmapwidget.h"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <qtest.h>

GravatarDownloadPixmapWidgetTest::GravatarDownloadPixmapWidgetTest(QObject *parent)
    : QObject(parent)
{

}

GravatarDownloadPixmapWidgetTest::~GravatarDownloadPixmapWidgetTest()
{

}

void GravatarDownloadPixmapWidgetTest::shouldHaveDefaultValue()
{
    PimCommon::GravatarDownloadPixmapWidget w;
    QLabel *lab = qFindChild<QLabel *>(&w, QStringLiteral("labemail"));
    QVERIFY(lab);

    QLineEdit *lineEdit = qFindChild<QLineEdit *>(&w, QStringLiteral("email"));
    QVERIFY(lineEdit);

    QPushButton *getPixmapButton = qFindChild<QPushButton *>(&w, QStringLiteral("searchbutton"));
    QVERIFY(getPixmapButton);
    QVERIFY(!getPixmapButton->isEnabled());


    QLabel *resultLabel = qFindChild<QLabel *>(&w, QStringLiteral("resultlabel"));
    QVERIFY(resultLabel);
}

void GravatarDownloadPixmapWidgetTest::shouldChangeButtonEnableState()
{
    PimCommon::GravatarDownloadPixmapWidget w;
    QLineEdit *lineEdit = qFindChild<QLineEdit *>(&w, QStringLiteral("email"));

    QPushButton *getPixmapButton = qFindChild<QPushButton *>(&w, QStringLiteral("searchbutton"));
    QVERIFY(!getPixmapButton->isEnabled());

    lineEdit->setText(QStringLiteral("foo"));
    QVERIFY(getPixmapButton->isEnabled());

    lineEdit->setText(QStringLiteral("   "));
    QVERIFY(!getPixmapButton->isEnabled());
}

QTEST_MAIN(GravatarDownloadPixmapWidgetTest)
