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
    id: selector
    width: 130
    height: 64

    property alias model: list.model
    property alias currentIndex: list.currentIndex

    Image {
        id: inputLeft
        source: "images/scrollinput-left" + (selector.focus ? "-active" : "") + ".png"

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.leftMargin: 5
    }

    BorderImage {
        id: inputCenter
        source: "images/scrollinput-center" + (selector.focus ? "-active" : "") + ".png"

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: inputLeft.right
        anchors.right: inputRight.left
    }

    Image {
        id: inputRight
        source: "images/scrollinput-right" + (selector.focus ? "-active" : "") + ".png"

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.rightMargin: 5
    }

    Component {
        id: delegate
        Item {
            id: wrapper
            width: 75
            height: 45
            anchors.verticalCenterOffset: 5
            anchors.verticalCenter: parent.verticalCenter
            Text {
                text: index + 1
                anchors.fill: parent
                color: "#004bb8"
                font.bold: true
                font.pixelSize: 32
                horizontalAlignment: Text.AlignHCenter
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    selector.focus = true;
                }
            }
        }
    }

    Component {
        id: highlight
        Rectangle {
            color: "red"
            opacity: 0
        }
    }

    ListView {
        id: list
        clip: true
        model: 2010
        spacing: 30
        delegate: delegate
        highlight: highlight
        highlightRangeMode: ListView.StrictlyEnforceRange
        highlightFollowsCurrentItem: true
        preferredHighlightBegin: 30
        preferredHighlightEnd: 100

        anchors.fill: parent
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter

        orientation: Qt.Horizontal
        snapMode: ListView.SnapToItem
    }
}