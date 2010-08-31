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
    id: calendarday
    property alias text: myText.text
    property alias active: active.visible
    property alias valid: inactive.visible

    width: 54
    height: 54
    Image {
        id: normal
        source: "images/normaldate.png"
        anchors.fill: parent
    }
    Image {
        id: inactive
        source: "images/inactivedate.png"
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
}
