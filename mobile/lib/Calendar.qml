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

Item {
    id: calendar
    width: 460
    height: 360

    Row {
        id: title
        spacing: 8
        anchors.left: parent.left
        anchors.leftMargin: spacer.width + 5

        Text {
            id: month
            text: calendarHelper.monthName
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

    Row {
        spacing: 6
        anchors.top: title.bottom
        anchors.topMargin: 10

        Column {
            id: weeks
            spacing: 6

            Item {
                id: spacer
                width: 20
                height: 20
            }

            Text {
                id: week1
                width: 20
                height: 54
                text: calendarHelper.weekForPosition(1)
                color: "#828282"
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignTop
            }

            Text {
                id: week2
                width: 20
                height: 54
                text: calendarHelper.weekForPosition(2)
                color: "#828282"
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignTop
            }

            Text {
                id: week3
                width: 20
                height: 54
                text: calendarHelper.weekForPosition(3)
                color: "#828282"
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignTop
            }

            Text {
                id: week4
                width: 20
                height: 54
                text: calendarHelper.weekForPosition(4)
                color: "#828282"
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignTop
            }

            Text {
                id: week5
                width: 20
                height: 54
                text: calendarHelper.weekForPosition(5)
                color: "#828282"
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignTop
            }
        }

        Column {
            id: sundays
            spacing: 6

            Text {
                id: sundayLabel
                width: 54
                height: 20
                text: "Sun"
                color: "#5c5c5c"
                style: Text.Sunken
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignBottom
            }

            CalendarDay {
                id: day1
                text: calendarHelper.dayForPosition(1);
            }

            CalendarDay {
                id: day8
                text: calendarHelper.dayForPosition(8);
            }

            CalendarDay {
                id: day15
                text: calendarHelper.dayForPosition(15);
            }

            CalendarDay {
                id: day22
                text: calendarHelper.dayForPosition(22);
            }

            CalendarDay {
                id: day29
                text: calendarHelper.dayForPosition(29);
            }

        }

        Column {
            id: mondays
            spacing: 6

            Text {
                id: mondayLabel
                width: 54
                height: 20
                text: "Mon"
                color: "#5c5c5c"
                style: Text.Sunken
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignBottom
            }

            CalendarDay {
                id: day2
                text: calendarHelper.dayForPosition(2);
            }

            CalendarDay {
                id: day9
                text: calendarHelper.dayForPosition(9);
            }

            CalendarDay {
                id: day16
                text: calendarHelper.dayForPosition(16);
            }

            CalendarDay {
                id: day23
                text: calendarHelper.dayForPosition(23);
            }

            CalendarDay {
                id: day30
                text: calendarHelper.dayForPosition(30);
            }
        }

        Column {
            id: tuesdays
            spacing: 6

            Text {
                id: tuesdayLabel
                width: 54
                height: 20
                text: "Tue"
                color: "#5c5c5c"
                style: Text.Sunken
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignBottom
            }

            CalendarDay {
                id: day3
                text: calendarHelper.dayForPosition(3);
            }

            CalendarDay {
                id: day10
                text: calendarHelper.dayForPosition(10);
            }

            CalendarDay {
                id: day17
                text: calendarHelper.dayForPosition(17);
            }

            CalendarDay {
                id: day24
                text: calendarHelper.dayForPosition(24);
            }

            CalendarDay {
                id: day31
                text: calendarHelper.dayForPosition(31);
            }
        }

        Column {
            id: wednesdays
            spacing: 6

            Text {
                id: wednesdayLabel
                width: 54
                height: 20
                text: "Wed"
                color: "#5c5c5c"
                style: Text.Sunken
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignBottom
            }

            CalendarDay {
                id: day4
                text: calendarHelper.dayForPosition(4);
            }

            CalendarDay {
                id: day11
                text: calendarHelper.dayForPosition(11);
            }

            CalendarDay {
                id: day18
                text: calendarHelper.dayForPosition(18);
            }

            CalendarDay {
                id: day25
                text: calendarHelper.dayForPosition(25);
            }

            CalendarDay {
                id: day32
                text: calendarHelper.dayForPosition(32);
            }
        }

        Column {
            id: thursdays
            spacing: 6

            Text {
                id: thursdayLabel
                width: 54
                height: 20
                text: "Thu"
                color: "#5c5c5c"
                style: Text.Sunken
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignBottom
            }

            CalendarDay {
                id: day5
                text: calendarHelper.dayForPosition(5);
            }

            CalendarDay {
                id: day12
                text: calendarHelper.dayForPosition(12);
            }

            CalendarDay {
                id: day19
                text: calendarHelper.dayForPosition(19);
            }

            CalendarDay {
                id: day26
                text: calendarHelper.dayForPosition(26);
            }

            CalendarDay {
                id: day33
                text: calendarHelper.dayForPosition(33);
            }
        }

        Column {
            id: fridays
            spacing: 6

            Text {
                id: fridayLabel
                width: 54
                height: 20
                text: "Fri"
                color: "#5c5c5c"
                style: Text.Sunken
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignBottom
            }

            CalendarDay {
                id: day6
                text: calendarHelper.dayForPosition(6);
            }

            CalendarDay {
                id: day13
                text: calendarHelper.dayForPosition(13);
            }

            CalendarDay {
                id: day20
                text: calendarHelper.dayForPosition(20);
            }

            CalendarDay {
                id: day27
                text: calendarHelper.dayForPosition(27);
            }

            CalendarDay {
                id: day34
                text: calendarHelper.dayForPosition(34);
            }
        }

        Column {
            id: saturdays
            spacing: 6

            Text {
                id: saturdayLabel
                width: 54
                height: 20
                text: "Sat"
                color: "#5c5c5c"
                style: Text.Sunken
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignBottom
            }

            CalendarDay {
                id: day7
                text: calendarHelper.dayForPosition(7);
            }

            CalendarDay {
                id: day14
                text: calendarHelper.dayForPosition(14);
            }

            CalendarDay {
                id: day21
                text: calendarHelper.dayForPosition(21);
            }

            CalendarDay {
                id: day28
                text: calendarHelper.dayForPosition(28);
            }

            CalendarDay {
                id: day35
                text: calendarHelper.dayForPosition(35);
            }
        }
    }

}