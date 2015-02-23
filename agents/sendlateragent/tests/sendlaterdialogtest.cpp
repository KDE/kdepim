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


#include "sendlaterdialogtest.h"
#include "../sendlaterdialog.h"
#include <KTimeComboBox>
#include <KDateComboBox>
#include <QLineEdit>
#include <qtest_kde.h>
SendLaterDialogTest::SendLaterDialogTest(QObject *parent)
    : QObject(parent)
{

}

SendLaterDialogTest::~SendLaterDialogTest()
{

}

void SendLaterDialogTest::shouldHaveDefaultValue()
{
    SendLater::SendLaterDialog dlg(0);
    KTimeComboBox *timeCombo = qFindChild<KTimeComboBox *>(&dlg, QLatin1String("time_sendlater"));
    QVERIFY(timeCombo);
    KDateComboBox *dateCombo = qFindChild<KDateComboBox *>(&dlg, QLatin1String("date_sendlater"));
    QVERIFY(dateCombo);
    QVERIFY(dateCombo->date().isValid());
    QVERIFY(dlg.isButtonEnabled(KDialog::Ok));
    dateCombo->lineEdit()->clear();
    QVERIFY(!dateCombo->date().isValid());
    QVERIFY(!dlg.isButtonEnabled(KDialog::Ok));
}

QTEST_KDEMAIN(SendLaterDialogTest, GUI)
