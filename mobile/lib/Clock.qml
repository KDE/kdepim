/*
    Copyright (C) 2010 Anselmo Lacerda Silveira de Melo <anselmolsm@gmail.com>
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
import ClockHelper 4.5

Image {
    id: clock

    property alias minutes: clockHelper.minutes
    property alias hours: clockHelper.hours

    source: "images/clock-background.png"
    smooth: true

    MouseArea {
        id: clockMArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: clockHelper.unselectAll()
        onPositionChanged: clockHelper.setXY(mouseX, mouseY)
    }

    Image {
        id: minutesHand
        source: "images/clock-minutes.png"
        smooth: true

        x: clock.width/2 - minutesHand.width/2
        y: clock.height/2 - minutesHand.height*0.95
        height: clock.height/(372/148)
        width: clock.width/(370/12)

        transform: Rotation {
            origin.x: minutesHand.width/2
            origin.y: minutesHand.height*0.95
            angle: clockHelper.minutesAngle
//            Behavior on angle { PropertyAnimation {} }
        }

        MouseArea {
            anchors.fill: parent
            onPressed: clockHelper.selectMinute()
        }
    }

    Image {
        id: hoursHand
        source: "images/clock-hours.png"
        smooth: true

        x: clock.width/2 - hoursHand.width/2
        y: clock.height/2 - hoursHand.height*0.95
        height: clock.height/(372/113)
        width: clock.width/(370/12)

        transform: Rotation {
            origin.x: hoursHand.width/2
            origin.y: hoursHand.height*0.95
            angle: clockHelper.hoursAngle
//            Behavior on angle { PropertyAnimation {} }
        }

        MouseArea {
            anchors.fill: parent
            onPressed: clockHelper.selectHour()
        }
    }

    Image {
        id: center
        source: "images/clock-center.png"
        smooth: true

        anchors.centerIn: clock
        height: clock.height/(372/38)
        width: clock.width/(370/38)
    }

    ClockHelper {
        id: clockHelper

        originX: clockMArea.width / 2
        originY: clockMArea.height / 2
     }

    function clearSelection() {
        clockHelper.unselectAll()
    }
}
