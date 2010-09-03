/*
    Copyright (C) 2010 Anselmo Lacerda Silveira de Melo <anselmolsm@gmail.com>

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
import ClockHelper 4.5
import org.kde.pim.mobileui 4.5 as KPIM

Rectangle {
    id: timeSelector
    height: 400
    width: 600
    color: "red"
    MouseArea {
        anchors.fill:parent
        onClicked: {
            seconds.state = "unselected"
            minutes.state = "unselected"
            hours.state = "unselected"
        }
    }

    KPIM.Clock {
        id: clock
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom

            topMargin: 10
            bottomMargin: 10
            leftMargin: 20
        }

        seconds: seconds.currentIndex*6
        minutes: minutes.currentIndex*6
        hours: hours.currentIndex*30
    }

    KPIM.VerticalFadeSelector {
        id: seconds
        height: 100
        width: 150
        model: 61

        anchors {
            right: parent.right
            rightMargin: 40
        }

        onStateChanged: {
            if (state == "selected") {
                minutes.state = "unselected";
                hours.state = "unselected";
            }
        }
    }
    Text {
        id: secondsLabel
        text: "seconds"
        font.pointSize: 15

        anchors {
            top: seconds.bottom
            right: parent.right
            topMargin: -15
            rightMargin: 40
        }
    }

    KPIM.VerticalFadeSelector {
        id: minutes
        height: 100
        width: 150
        model: 61
        currentIndex: clockHelper.minutes

        anchors {
            top: secondsLabel.bottom
            right: parent.right
            topMargin: -5
            rightMargin: 40
        }

        onStateChanged: {
            if (state == "selected") {
                seconds.state = "unselected";
                hours.state = "unselected";
            }
        }
    }
    Text {
        id: minutesLabel
        text: "minutes"
        font.pointSize: 15

        anchors {
            top: minutes.bottom
            right: parent.right
            topMargin: -15
            rightMargin: 40
        }
    }

    KPIM.VerticalFadeSelector {
        id: hours
        height: 100
        width: 150
        model: 13
        currentIndex: 0

        anchors {
            top: minutesLabel.bottom
            right: parent.right
            topMargin: -5
            rightMargin: 40
        }
        onStateChanged: {
            if (state == "selected") {
                minutes.state = "unselected";
                seconds.state = "unselected";
            }
        }
    }

    Text {
        text: "hours"
        font.pointSize: 15

        anchors {
            top: hours.bottom
            right: parent.right
            topMargin: -15
            rightMargin: 40
        }
    }
}
