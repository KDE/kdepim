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

#include "eventdatetimewidgettest.h"
#include "widgets/eventdatetimewidget.h"
#include <KDateComboBox>
#include <KTimeComboBox>
#include <qtest_kde.h>
#include <widgets/eventdatetimewidget.h>

EventDateTimeWidgetTest::EventDateTimeWidgetTest(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<KDateTime>();
}

EventDateTimeWidgetTest::~EventDateTimeWidgetTest()
{

}

void EventDateTimeWidgetTest::shouldHaveDefaultValue()
{
    MessageViewer::EventDateTimeWidget edit;
    KDateComboBox *datecombobox = qFindChild<KDateComboBox *>(&edit, QLatin1String("eventdatecombobox"));
    QVERIFY(datecombobox);
    KTimeComboBox *timecombobox = qFindChild<KTimeComboBox *>(&edit, QLatin1String("eventtimecombobox"));
    QVERIFY(timecombobox);
}

void EventDateTimeWidgetTest::shouldSetDateTime()
{
    MessageViewer::EventDateTimeWidget edit;
    const KDateTime currentDateTime = KDateTime::currentDateTime(KDateTime::LocalZone);
    edit.setDateTime(currentDateTime);
    QCOMPARE(edit.dateTime().time().minute(), currentDateTime.time().minute());
    QCOMPARE(edit.dateTime().time().hour(), currentDateTime.time().hour());
}

void EventDateTimeWidgetTest::shouldEmitSignalWhenDateTimeChanged()
{
    MessageViewer::EventDateTimeWidget edit;
    KDateTime currentDateTime = KDateTime::currentDateTime(KDateTime::LocalZone);
    edit.setDateTime(currentDateTime);

    QSignalSpy spy(&edit, SIGNAL(dateTimeChanged(KDateTime)));
    currentDateTime.setDate(currentDateTime.date().addDays(1));
    edit.setDateTime(currentDateTime);

    QCOMPARE(spy.count(), 1);
}

void EventDateTimeWidgetTest::shouldEmitSignalWhenJustTimeChanged()
{
    MessageViewer::EventDateTimeWidget edit;
    KDateTime currentDateTime = KDateTime::currentDateTime(KDateTime::LocalZone);
    edit.setDateTime(currentDateTime);

    QSignalSpy spy(&edit, SIGNAL(dateTimeChanged(KDateTime)));
    QTime time = currentDateTime.time().addSecs(3600);
    edit.setTime(time);

    QCOMPARE(spy.count(), 1);
}

void EventDateTimeWidgetTest::shouldEmitSignalWhenJustDateChanged()
{
    MessageViewer::EventDateTimeWidget edit;
    KDateTime currentDateTime = KDateTime::currentDateTime(KDateTime::LocalZone);
    edit.setDateTime(currentDateTime);

    QSignalSpy spy(&edit, SIGNAL(dateTimeChanged(KDateTime)));
    QDate date = currentDateTime.date().addDays(1);
    edit.setDate(date);

    QCOMPARE(spy.count(), 1);
}

void EventDateTimeWidgetTest::shouldNotEmitSignalWhenDateTimeWasNotChanged()
{
    MessageViewer::EventDateTimeWidget edit;
    KDateTime currentDateTime = KDateTime::currentDateTime(KDateTime::LocalZone);
    edit.setDateTime(currentDateTime);

    QSignalSpy spy(&edit, SIGNAL(dateTimeChanged(KDateTime)));
    currentDateTime.setDate(currentDateTime.date().addDays(1));
    edit.setDateTime(currentDateTime);

    QCOMPARE(spy.count(), 1);
    edit.setDateTime(currentDateTime);

    //FIX ME
    //QCOMPARE(spy.count(), 2);
}

QTEST_KDEMAIN(EventDateTimeWidgetTest, GUI)
