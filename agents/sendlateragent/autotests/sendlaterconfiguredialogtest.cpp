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


#include "sendlaterconfiguredialogtest.h"
#include "../sendlaterconfiguredialog.h"
#include "../sendlaterconfigurewidget.h"

#include <QTreeWidget>

#include <qtest.h>


SendLaterConfigureDialogTest::SendLaterConfigureDialogTest(QObject *parent)
    : QObject(parent)
{

}

SendLaterConfigureDialogTest::~SendLaterConfigureDialogTest()
{

}

void SendLaterConfigureDialogTest::shouldHaveDefaultValue()
{
    SendLaterConfigureDialog dlg;
    SendLaterWidget *infowidget = qFindChild<SendLaterWidget *>(&dlg, QLatin1String("sendlaterwidget"));
    QVERIFY(infowidget);

    QTreeWidget *treeWidget = qFindChild<QTreeWidget *>(infowidget, QLatin1String("treewidget"));
    QVERIFY(treeWidget);

    QCOMPARE(treeWidget->topLevelItemCount(), 0);

}

QTEST_MAIN(SendLaterConfigureDialogTest)
