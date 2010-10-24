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
    id: calendarDay
    property alias text: myText.text
    property int currentDay: 0
    property int dayPos: 0
    signal daySelected(string day)

    width: 54
    height: 54
    Image {
        id: inactive
        source: "images/inactivedate.png"
        visible: false
        anchors.fill: parent
    }
    Image {
        id: normal
        source: "images/normaldate.png"
        visible: true
        anchors.fill: parent
    }
    Image {
        id: active
        visible: false
        source: "images/activedate.png"
        anchors.fill: parent
    }
    Text {
        id: myText
        color: "#5ba0d4"
        font.bold: true
        font.pixelSize: 26
        style: Text.Sunken
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
    }
    MouseArea {
        id: dayArea
        anchors.fill: parent
        onClicked: {
            if (myText.text != "")
                calendarDay.daySelected(myText.text)
        }
    }

    states: [
        State {
            name: "active"
            when: (currentDay == myText.text)
            PropertyChanges {
                target: active
                visible: true
            }
            PropertyChanges {
                target: inactive
                visible: false
            }
        },
        State {
            name: "inactive"
            when: (myText.text == "")
            PropertyChanges {
                target: active
                visible: false
            }
            PropertyChanges {
                target: inactive
                visible: true
            }
        }
    ]
}
