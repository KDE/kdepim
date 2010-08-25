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
    id: fadeselector
    width: 130
    height: 64

    property alias model: list.model
    property alias currentIndex: list.currentIndex


    Image {
        id: inputLeft
        source: "images/scrollinput-left" + (fadeselector.focus ? "-active" : "") + ".png"

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.leftMargin: 5
    }

    BorderImage {
        id: inputCenter
        source: "images/scrollinput-center" + (fadeselector.focus ? "-active" : "") + ".png"

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: inputLeft.right
        anchors.right: inputRight.left
    }

    Image {
        id: inputRight
        source: "images/scrollinput-right" + (fadeselector.focus ? "-active" : "") + ".png"

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.rightMargin: 5
    }

    function relativeopacity(x1, x2, width) {
        var opacity = (x1 + x2) / width;
        if (opacity > 1)
            return 2.4 - opacity;
        return opacity + 0.4
    }

    Component {
        id: fadeDelegate
        Item {
            id: fadewrapper
            width: 50
            height: 45
            anchors.verticalCenterOffset: 5
            anchors.verticalCenter: parent.verticalCenter
            opacity: relativeopacity(parent.x, x, width);
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
                    fadeselector.focus = true;
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
        model: 30
        delegate: fadeDelegate
        highlight: highlight
        highlightRangeMode: ListView.StrictlyEnforceRange
        highlightFollowsCurrentItem: true
        preferredHighlightBegin: 40
        preferredHighlightEnd: 90

        anchors.fill: parent
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter

        orientation: Qt.Horizontal
        snapMode: ListView.SnapToItem
    }

}