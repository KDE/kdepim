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

import QtQuick 1.1

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

    signal daySelected();

    CalendarHelper {
        id: calendarHelper
        onDayChanged: calendar.daySelected()
    }

    Row {
        id: title
        spacing: 8
        anchors.left: parent.left
        anchors.leftMargin: spacer.width + 5

        Text {
            text: KDE.i18n(calendarHelper.monthName)
            color: "#004bb8"
            font.pixelSize: 24
        }

        Text {
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

        property int dayBoxSize: 54
        property int headlineHeight: 15
        property int weekNumberWidth: 20

        // headline
        Item {
            id: spacer
            width: calendarGrid.weekNumberWidth
            height: calendarGrid.headlineHeight
        }

        Repeater {
            model: ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"]
            Text {
                width: calendarGrid.dayBoxSize
                height: calendarGrid.headlineHeight
                text: KDE.i18n(modelData)
                color: "#5c5c5c"
                font.pixelSize: 16
                style: Text.Sunken
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignBottom
            }
        }

        // first row
        Text {
            id: week1
            width: calendarGrid.weekNumberWidth
            height: calendarGrid.dayBoxSize
            color: "#828282"
            text: calendarHelper.weekForPosition(1)
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignTop
        }

        Repeater {
            model: 7
            CalendarDay {
                dayPos: index + 1
                currentDay: calendarHelper.day
                text: calendarHelper.dayForPosition(dayPos);
            }
        }


        // second row
        Text {
            id: week2
            width: calendarGrid.weekNumberWidth
            height: calendarGrid.dayBoxSize
            color: "#828282"
            text: calendarHelper.weekForPosition(2)
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignTop
        }

        Repeater {
            model: 7
            CalendarDay {
                dayPos: index + 8
                currentDay: calendarHelper.day
                text: calendarHelper.dayForPosition(dayPos);
            }
        }


        // third row
        Text {
            id: week3
            width: calendarGrid.weekNumberWidth
            height: calendarGrid.dayBoxSize
            color: "#828282"
            text: calendarHelper.weekForPosition(3)
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignTop
        }

        Repeater {
            model: 7
            CalendarDay {
                dayPos: index + 15
                currentDay: calendarHelper.day
                text: calendarHelper.dayForPosition(dayPos);
            }
        }


        // forth row
        Text {
            id: week4
            width: calendarGrid.weekNumberWidth
            height: calendarGrid.dayBoxSize
            color: "#828282"
            text: calendarHelper.weekForPosition(4)
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignTop
        }

        Repeater {
            model: 7
            CalendarDay {
                dayPos: index + 22
                currentDay: calendarHelper.day
                text: calendarHelper.dayForPosition(dayPos);
            }
        }


        // fifth row
        Text {
            id: week5
            width: calendarGrid.weekNumberWidth
            height: calendarGrid.dayBoxSize
            color: "#828282"
            text: calendarHelper.weekForPosition(5)
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignTop
        }

        Repeater {
            model: 7
            CalendarDay {
                dayPos: index + 29
                currentDay: calendarHelper.day
                text: calendarHelper.dayForPosition(dayPos);
            }
        }


        // sixth row
        Text {
            id: week6
            width: calendarGrid.weekNumberWidth
            height: calendarGrid.dayBoxSize
            color: "#828282"
            text: calendarHelper.weekForPosition(6)
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignTop
        }

        Repeater {
            model: 7
            CalendarDay {
                dayPos: index + 36
                currentDay: calendarHelper.day
                text: calendarHelper.dayForPosition(dayPos);
            }
        }
    }

    MouseArea {
        anchors.fill: calendarGrid
        anchors.topMargin: calendarGrid.headlineHeight
        anchors.leftMargin: calendarGrid.weekNumberWidth

        property int oldX : 0
        property int oldY : 0

        onPressed: {
          oldX = mouseX;
          oldY = mouseY;
        }

        onReleased: {
          var xDiff = oldX - mouseX;
          var yDiff = oldY - mouseY;

          if ( Math.abs( xDiff ) > width * 0.3 || Math.abs( yDiff ) > height * 0.3 ) {
            // distance is width enough to be a swipe action

            if ( Math.abs( xDiff ) > Math.abs( yDiff ) ) {
              if ( oldX > mouseX )
                calendarHelper.previousMonth();
              else
                calendarHelper.nextMonth();
            } else {
              if ( oldY > mouseY )
                calendarHelper.previousYear();
              else
                calendarHelper.nextYear();
            }
          } else {
            selectDay(mouse);
          }
        }

        //onPositionChanged: selectDay(mouse)

        function selectDay(mouse) {
            if ((mouse.x < 0) || (mouse.x >= width) || (mouse.y < 0) || (mouse.y >= height))
                return;

            var boxSize = calendarGrid.dayBoxSize + calendarGrid.spacing;
            var row = Math.floor(mouse.y / boxSize);
            var column = Math.floor(mouse.x / boxSize);
            var dayForPosition = calendarHelper.dayForPosition(row * 7 + column + 1);

            if (dayForPosition != "")
                calendarHelper.day = dayForPosition;
        }
    }

    Component.onCompleted: {
        calendarHelper.registerItems(calendarGrid);
    }
}

