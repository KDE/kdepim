/*
    Copyright (C) 2010 Artur Duque de Souza <asouza@kde.org>

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

import Qt 4.7

import org.kde 4.5
import CalendarHelper 4.5

Item {
    id: calendar
    width: 460
    height: 360

    property alias day: calendarHelper.day
    property alias month: calendarHelper.month
    property alias year: calendarHelper.year
    property alias daysInMonth: calendarHelper.daysInMonth

    CalendarHelper {
        id: calendarHelper
    }

    Row {
        id: title
        spacing: 8
        anchors.left: parent.left
        anchors.leftMargin: spacer.width + 5

        Text {
            id: month
            text: KDE.i18n(calendarHelper.monthName)
            color: "#004bb8"
            font.pixelSize: 24
        }

        Text {
            id: year
            text: calendarHelper.year
            color: "#004bb8"
            font.pixelSize: 24
        }
    }

    Grid {
        id: calendarGrid
        spacing: 3
        columns: 8
        rows: 7
        anchors.top: title.bottom
        anchors.topMargin: 10

        // headline
        Item {
            id: spacer
            width: 20
            height: 15
        }
        Text {
            id: sundayLabel
            width: 54
            height: 15
            text: KDE.i18n("Sun")
            color: "#5c5c5c"
            font.pixelSize: 16
            style: Text.Sunken
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignBottom
        }
        Text {
            id: mondayLabel
            width: 54
            height: 15
            text: KDE.i18n("Mon")
            color: "#5c5c5c"
            font.pixelSize: 16
            style: Text.Sunken
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignBottom
        }
        Text {
            id: tuesdayLabel
            width: 54
            height: 15
            text: KDE.i18n("Tue")
            color: "#5c5c5c"
            font.pixelSize: 16
            style: Text.Sunken
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignBottom
        }
        Text {
            id: wednesdayLabel
            width: 54
            height: 15
            text: KDE.i18n("Wed")
            color: "#5c5c5c"
            font.pixelSize: 16
            style: Text.Sunken
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignBottom
        }
        Text {
            id: thursdayLabel
            width: 54
            height: 15
            text: KDE.i18n("Thu")
            color: "#5c5c5c"
            font.pixelSize: 16
            style: Text.Sunken
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignBottom
        }
        Text {
            id: fridayLabel
            width: 54
            height: 15
            text: KDE.i18n("Fri")
            color: "#5c5c5c"
            font.pixelSize: 16
            style: Text.Sunken
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignBottom
        }
        Text {
            id: saturdayLabel
            width: 54
            height: 15
            text: KDE.i18n("Sat")
            color: "#5c5c5c"
            font.pixelSize: 16
            style: Text.Sunken
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignBottom
        }

        // first row
        Text {
            id: week1
            property int weekPos: 1
            width: 20
            height: 54
            color: "#828282"
            text: calendarHelper.weekForPosition(1)
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignTop
        }
        CalendarDay {
            id: day1
            dayPos: 1
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(1);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day2
            dayPos: 2
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(2);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day3
            dayPos: 3
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(3);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day4
            dayPos: 4
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(4);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day5
            dayPos: 5
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(5);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day6
            dayPos: 6
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(6);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day7
            dayPos: 7
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(7);
            onDaySelected: {
                calendarHelper.day = day
            }
        }

        // second row
        Text {
            id: week2
            property int weekPos: 2
            width: 20
            height: 54
            color: "#828282"
            text: calendarHelper.weekForPosition(2)
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignTop
        }
        CalendarDay {
            id: day8
            dayPos: 8
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(8);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day9
            dayPos: 9
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(9);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day10
            dayPos: 10
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(10);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day11
            dayPos: 11
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(11);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day12
            dayPos: 12
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(12);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day13
            dayPos: 13
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(13);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day14
            dayPos: 14
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(14);
            onDaySelected: {
                calendarHelper.day = day
            }
        }

        // third row
        Text {
            id: week3
            property int weekPos: 3
            width: 20
            height: 54
            color: "#828282"
            text: calendarHelper.weekForPosition(3)
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignTop
        }
        CalendarDay {
            id: day15
            dayPos: 15
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(15);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day16
            dayPos: 16
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(16);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day17
            dayPos: 17
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(17);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day18
            dayPos: 18
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(18);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day19
            dayPos: 19
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(19);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day20
            dayPos: 20
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(20);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day21
            dayPos: 21
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(21);
            onDaySelected: {
                calendarHelper.day = day
            }
        }

        // forth row
        Text {
            id: week4
            property int weekPos: 4
            width: 20
            height: 54
            color: "#828282"
            text: calendarHelper.weekForPosition(4)
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignTop
        }
        CalendarDay {
            id: day22
            dayPos: 22
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(22);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day23
            dayPos: 23
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(23);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day24
            dayPos: 24
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(24);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day25
            dayPos: 25
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(25);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day26
            dayPos: 26
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(26);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day27
            dayPos: 27
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(27);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day28
            dayPos: 28
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(28);
            onDaySelected: {
                calendarHelper.day = day
            }
        }

        // fifth row
        Text {
            id: week5
            property int weekPos: 5
            width: 20
            height: 54
            color: "#828282"
            text: calendarHelper.weekForPosition(5)
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignTop
        }
        CalendarDay {
            id: day29
            dayPos: 29
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(29);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day30
            dayPos: 30
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(30);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day31
            dayPos: 31
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(31);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day32
            dayPos: 32
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(32);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day33
            dayPos: 33
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(33);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day34
            dayPos: 34
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(34);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day35
            dayPos: 35
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(35);
            onDaySelected: {
                calendarHelper.day = day
            }
        }

        // sixth row
        Text {
            id: week6
            property int weekPos: 6
            width: 20
            height: 54
            color: "#828282"
            text: calendarHelper.weekForPosition(6)
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignTop
        }
        CalendarDay {
            id: day36
            dayPos: 36
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(36);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day37
            dayPos: 37
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(37);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day38
            dayPos: 38
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(38);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day39
            dayPos: 39
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(39);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day40
            dayPos: 40
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(40);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
        CalendarDay {
            id: day41
            dayPos: 41
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(41);
            onDaySelected: {
                calendarHelper.day = day
            }
        }

        CalendarDay {
            id: day42
            dayPos: 42
            currentDay: calendarHelper.day
            text: calendarHelper.dayForPosition(42);
            onDaySelected: {
                calendarHelper.day = day
            }
        }
    }

    Component.onCompleted: {
        calendarHelper.registerItems(calendarGrid);
    }
}

