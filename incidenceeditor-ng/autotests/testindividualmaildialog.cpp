/*
Copyright (c) 2014 Sandro Knauß <knauss@kolabsys.com>

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Library General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to the
Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.
 */

#include "individualmaildialog.h"

#include <QObject>
#include <qtest.h>

using namespace IncidenceEditorNG;

class TestIndividualMailDialog : public QObject
{
    Q_OBJECT
private slots:
    void testDialog()
    {
        KCalCore::Attendee::List attendees;
        KGuiItem buttonYes = KGuiItem(QLatin1String("Send Email"));
        KGuiItem buttonNo = KGuiItem(QLatin1String("Do not send"));

        KCalCore::Attendee::Ptr attendee1(new KCalCore::Attendee(QLatin1String("test1"), QLatin1String("test1@example.com")));
        KCalCore::Attendee::Ptr attendee2(new KCalCore::Attendee(QLatin1String("test2"), QLatin1String("test2@example.com")));
        KCalCore::Attendee::Ptr attendee3(new KCalCore::Attendee(QLatin1String("test3"), QLatin1String("test3@example.com")));

        attendees << attendee1 << attendee2 << attendee3;

        IndividualMailDialog dialog(QLatin1String("title"), attendees, buttonYes, buttonNo, 0);

        QCOMPARE(dialog.editAttendees().count(), 0);
        QCOMPARE(dialog.updateAttendees().count(), 3);

        // Just make sure, that the QCombobox is sorted like we think
        QComboBox *first = dialog.mAttendeeDecision[attendees[0]];
        QCOMPARE((IndividualMailDialog::Decisions)first->itemData(0, Qt::UserRole).toInt(), IndividualMailDialog::Update);
        QCOMPARE((IndividualMailDialog::Decisions)first->itemData(1, Qt::UserRole).toInt(), IndividualMailDialog::NoUpdate);
        QCOMPARE((IndividualMailDialog::Decisions)first->itemData(2, Qt::UserRole).toInt(), IndividualMailDialog::Edit);

        // No update for first attendee, other default
        first->setCurrentIndex(1);
        QCOMPARE(dialog.editAttendees().count(), 0);
        QCOMPARE(dialog.updateAttendees().count(), 2);
        QVERIFY(dialog.updateAttendees().contains(attendee2));
        QVERIFY(dialog.updateAttendees().contains(attendee3));

        // edit for frist attende, other default
        first->setCurrentIndex(2);
        QCOMPARE(dialog.editAttendees().count(), 1);
        QCOMPARE(dialog.updateAttendees().count(), 2);
        QCOMPARE(dialog.editAttendees()[0], attendee1);
    }
};

QTEST_MAIN(TestIndividualMailDialog)

#include "testindividualmaildialog.moc"
