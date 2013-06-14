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

Image {
    id: calendarDay
    property alias text: myText.text
    property int currentDay: 0
    property int dayPos: 0

    width: 54
    height: 54

    source: "images/normaldate.png"

    Image {
        anchors.fill: parent
        source: {
            if (myText.text == "") {
                return "images/inactivedate.png";
            } else if (myText.text == currentDay) {
                return "images/activedate.png";
            } else {
                return "";
            }
        }
    }

    Text {
        id: myText
        anchors.centerIn: parent
        color: "#5ba0d4"
        font.bold: true
        font.pixelSize: 26
        style: Text.Sunken
    }
}
