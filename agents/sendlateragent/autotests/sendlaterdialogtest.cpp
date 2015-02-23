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
#include <QPushButton>
#include <qtest.h>
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
    KTimeComboBox *timeCombo = dlg.findChild<KTimeComboBox *>(QLatin1String("time_sendlater"));
    QVERIFY(timeCombo);
    KDateComboBox *dateCombo = dlg.findChild<KDateComboBox *>(QLatin1String("date_sendlater"));
    QVERIFY(dateCombo);
    QVERIFY(dateCombo->date().isValid());
    QPushButton *okButton = dlg.findChild<QPushButton *>(QLatin1String("okbutton"));
    QVERIFY(okButton);
    QVERIFY(okButton->isEnabled());
    dateCombo->lineEdit()->clear();
    QVERIFY(!dateCombo->date().isValid());
    QVERIFY(!okButton->isEnabled());
}

QTEST_MAIN(SendLaterDialogTest)
